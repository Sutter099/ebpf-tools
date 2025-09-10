SUBDIRS := zstd zlib libelf libbpf-tools ply

.PHONY: all clean distclean $(SUBDIRS)

all: libbpf-tools ply

CC ?= gcc
ARCH ?= x86_64

CURR_DIR := $(shell pwd)
DESTDIR ?= $(shell pwd)/_install

APPS_LINK_FLAG ?= -static
PLY_LDFLAGS ?= -all-static
ifeq ($(DYNAMIC_LINK),1)
APPS_LINK_FLAG=
PLY_LDFLAGS=
endif

ZSTD_PATH=$(CURR_DIR)/zstd
ZLIB_PATH=$(CURR_DIR)/zlib
LIBELF_PATH=$(CURR_DIR)/libelf

LIBZSTD=$(ZSTD_PATH)/lib/libzstd.a

$(LIBZSTD):
	$(MAKE) -C zstd

# ply prepare
ply/Makefile:
	cd ply && ./autogen.sh
	cd ply && ./configure --host=$(ARCH) CC=$(CC)

ply: ply/Makefile
	make -C ply LDFLAGS=$(PLY_LDFLAGS)

ply_clean:
	$(MAKE) -C ply clean
	$(MAKE) -C ply distclean

# zlib prepare
zlib/configure.log:
	cd zlib && ./configure CC=$(CC) CHOST=$(ARCH)

zlib: zlib/configure.log
	$(MAKE) -C zlib

zlib_clean:
	$(MAKE) -C zlib clean
	$(MAKE) -C zlib distclean

libelf: $(LIBZSTD) zlib
	$(MAKE) -C libelf LDFLAGS="-L$(ZSTD_PATH)/lib -L$(ZLIB_PATH)" CFLAGS="-I$(ZSTD_PATH)/lib -I$(ZLIB_PATH)"

libbpf-tools: $(LIBZSTD) zlib libelf
	$(MAKE) -C libbpf-tools \
		EXTRA_CFLAGS="-I$(ZSTD_PATH)/lib -I$(ZLIB_PATH) -I$(LIBELF_PATH)/include" \
		EXTRA_LDFLAGS="$(APPS_LINK_FLAG) -L$(ZSTD_PATH)/lib -L$(ZLIB_PATH) -L$(LIBELF_PATH)"

install:
	$(MAKE) -C libbpf-tools DESTDIR=$(DESTDIR) install
	$(MAKE) -C ply DESTDIR=$(DESTDIR) install

clean:
	$(MAKE) -C ply clean
	$(MAKE) -C zstd clean
	$(MAKE) -C zlib clean
	$(MAKE) -C libelf clean
	$(MAKE) -C libbpf-tools clean

distclean: ply_clean zlib_clean
