/* SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause) */
// Copyright (c) 2025 Ze Huang
// Inspired by cpudist
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>

#include "trace_helpers.h"
#include "trace_slab_alloc.h"
#include "trace_slab_alloc.skel.h"

static volatile int exiting = 0;
static volatile __u64 start_time = 0;

const char *desc[] = {
	"slab_alloc total",
	"slab_alloc slow",
	"get_partial(THIS_NODE)",
	"get_partial(other)",
	"new_slab(THIS_NODE)",
	"new_slab(other)",
};

void sig_handler(int sig)
{
	exiting = true;
}

static __u64 get_current_time_ns(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	return (__u64)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

static int print_linear_hists(int fd)
{
	__u32 lookup_key = -2, next_key;
	char *units = "secs";
	struct hist hist;
	int err;

	while (!bpf_map_get_next_key(fd, &lookup_key, &next_key)) {
		err = bpf_map_lookup_elem(fd, &next_key, &hist);
		if (err < 0) {
			fprintf(stderr, "failed to lookup hist: %d\n", err);
			return -1;
		}

		printf("%s\n", desc[hist.event_type]);
		print_linear_hist(hist.slots, MAX_SLOTS, 0, 1, units);
		lookup_key = next_key;
	}

	lookup_key = -2;
	while (!bpf_map_get_next_key(fd, &lookup_key, &next_key)) {
		err = bpf_map_delete_elem(fd, &next_key);
		if (err < 0) {
			fprintf(stderr, "failed to cleanup hist : %d\n", err);
			return -1;
		}
		lookup_key = next_key;
	}

	return 0;
}

int main(void)
{
	struct trace_slab_alloc_bpf *skel;
	int err, fd;

	skel = trace_slab_alloc_bpf__open();
	if (!skel) {
		fprintf(stderr, "Failed to open skeleton\n");
		return 1;
	}

	bpf_map__set_max_entries(skel->maps.hists, 6);

	err = trace_slab_alloc_bpf__load(skel);
	if (err) {
		fprintf(stderr, "failed to load BPF object: %d\n", err);
		goto cleanup;
	}

	// init start_time_map
	{
		__u32 zero = 0;
		__u64 start_time_ns = get_current_time_ns();
		int map_fd = bpf_map__fd(skel->maps.start_time_map);
		err = bpf_map_update_elem(map_fd, &zero, &start_time_ns, 0);
		if (err) {
			fprintf(stderr, "Failed to set start_time_map: %d\n", err);
			goto cleanup;
		}
	}

	err = trace_slab_alloc_bpf__attach(skel);
	if (err) {
		fprintf(stderr, "Failed to attach BPF programs\n");
		goto cleanup;
	}

	fd = bpf_map__fd(skel->maps.hists);

	signal(SIGINT, sig_handler);

	printf("Tracing... Press Ctrl-C to stop.\n");

	while (!exiting) {
		sleep(1);
	}

	err = print_linear_hists(fd);
	if (err) {
		fprintf(stderr, "Failed to print hists\n");
		goto cleanup;
	}

cleanup:
	trace_slab_alloc_bpf__destroy(skel);
	return err < 0 ? -err : 0;
}
