#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>

#include "trace_slab_alloc.skel.h"

static volatile int exiting = 0;

void sig_handler(int sig)
{
	exiting = 1;
}

int main(void)
{
	struct trace_slab_alloc_bpf *skel;
	const char *desc[] = {
		"___slab_alloc called",
		"get_partial(THIS_NODE)",
		"get_partial(other)",
		"new_slab(THIS_NODE)",
		"new_slab(other)"
	};
	__u64 values[5] = {0};
	__u32 key;
	int err;

	signal(SIGINT, sig_handler);

	// 1. Open and load skeleton
	skel = trace_slab_alloc_bpf__open_and_load();
	if (!skel) {
		fprintf(stderr, "Failed to open and load skeleton\n");
		return 1;
	}

	// 2. Attach all programs
	err = trace_slab_alloc_bpf__attach(skel);
	if (err) {
		fprintf(stderr, "Failed to attach BPF programs\n");
		goto cleanup;
	}

	printf("Tracing... Press Ctrl-C to stop.\n");
	while (!exiting) {
		sleep(1);
	}

	printf("\nResults:\n");
	for (key = 0; key < 5; key++) {
		err = bpf_map_lookup_elem(bpf_map__fd(skel->maps.counters), &key, &values[key]);
		if (err == 0)
			printf("%-25s: %llu\n", desc[key], values[key]);
		else
			printf("%-25s: (error reading)\n", desc[key]);
	}

cleanup:
	trace_slab_alloc_bpf__destroy(skel);
	return err < 0 ? -err : 0;
}
