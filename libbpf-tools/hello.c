#include <stdio.h>
#include <stdlib.h>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include <errno.h>
#include <unistd.h>

static int libbpf_print_fn(enum libbpf_print_level level, const char *format, va_list args)
{
	return vfprintf(stderr, format, args);
}

int main()
{
	struct bpf_object *obj;
	int err;

	libbpf_set_print(libbpf_print_fn);

	obj = bpf_object__open_file("hello.bpf.o", NULL);
	if (!obj) {
		fprintf(stderr, "Failed to open BPF object\n");
		return 1;
	}

	err = bpf_object__load(obj);
	if (err) {
		fprintf(stderr, "Failed to load BPF object: %s\n", strerror(-err));
		bpf_object__close(obj);
		return 1;
	}

	struct bpf_program *prog = bpf_object__find_program_by_name(obj, "my_prog");
	if (!prog) {
		fprintf(stderr, "Failed to find BPF program\n");
		bpf_object__close(obj);
		return 1;
	}

	struct bpf_link *link = bpf_program__attach(prog);
	if (!link) {
		fprintf(stderr, "Failed to attach kprobe: %s\n", strerror(errno));
		bpf_object__close(obj);
		return 1;
	}

	printf("eBPF program loaded and attached to kprobe/exec\n");
	printf("Check /sys/kernel/debug/tracing/trace_pipe for output\n");

	while (1)
		sleep(1);

	bpf_link__destroy(link);
	bpf_object__close(obj);

	return 0;
}
