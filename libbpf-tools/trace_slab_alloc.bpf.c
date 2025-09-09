/* SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause) */
// Copyright (c) 2025 Ze Huang
// Inspired by cpudist
#include <vmlinux.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>
#include "trace_slab_alloc.h"

#define ___GFP_KSWAPD_RECLAIM	(1 << ___GFP_KSWAPD_RECLAIM_BIT)
#define __GFP_NOWARN (1 << ___GFP_NOWARN_BIT)
#define GFP_NOWAIT	(___GFP_KSWAPD_RECLAIM | __GFP_NOWARN)
#define __GFP_THISNODE (1 << ___GFP_THISNODE_BIT)

struct {
	__uint(type, BPF_MAP_TYPE_ARRAY);
	__type(key, u32);
	__type(value, u64);
	__uint(max_entries, 1);
} start_time_map SEC(".maps");

static struct hist initial_hist;

struct {
	__uint(type, BPF_MAP_TYPE_HASH);
	__uint(max_entries, 5);
	__type(key, u32);
	__type(value, struct hist);
} hists SEC(".maps");

enum {
	SLAB_ALLOC = 0,
	SLAB_ALLOC_SLOW,
	GET_PARTIAL_THIS_NODE,
	GET_PARTIAL_OTHER,
	NEW_SLAB_THIS_NODE,
	NEW_SLAB_OTHER,
};

// all kinds of fentry update here
static __always_inline void update_hist(u32 event_enum, u64 ts)
{
	u64 delta, *start_time, slot;
	struct hist *histp;
	u32 zero = 0;

	start_time = bpf_map_lookup_elem(&start_time_map, &zero);
	if (!start_time || ts < *start_time)
		return;

	histp = bpf_map_lookup_elem(&hists, &event_enum);
	if (!histp) {
		bpf_map_update_elem(&hists, &event_enum, &initial_hist, 0);
		histp = bpf_map_lookup_elem(&hists, &event_enum);
		if (!histp)
			return;
		histp->event_type = event_enum;
	}

	delta = ts - *start_time;
	slot = delta / BUCKET_SIZE_NS;
	if (slot >= MAX_SLOTS)
		slot = MAX_SLOTS - 1;
	__sync_fetch_and_add(&histp->slots[slot], 1);
}

SEC("fentry/__slab_alloc_node")
int BPF_PROG(trace_slab_alloc)
{
	u64 ts = bpf_ktime_get_ns();

	update_hist(SLAB_ALLOC, ts);

	return 0;
}

SEC("fentry/___slab_alloc")
int BPF_PROG(trace_slab_alloc_slow)
{
	u64 ts = bpf_ktime_get_ns();

	update_hist(SLAB_ALLOC_SLOW, ts);

	return 0;
}

SEC("fentry/get_partial")
int BPF_PROG(get_partial_probe, struct kmem_cache *s, int node, struct partial_context *pc)
{
	gfp_t flags = BPF_CORE_READ(pc, flags);
	u64 ts = bpf_ktime_get_ns();
	u32 event_type;

	event_type = (flags == (GFP_NOWAIT | __GFP_THISNODE)) ? GET_PARTIAL_THIS_NODE : GET_PARTIAL_OTHER;

	update_hist(event_type, ts);

	return 0;
}

SEC("fentry/new_slab")
int BPF_PROG(trace_new_slab, struct kmem_cache *s, gfp_t flags, int node)
{
	u64 ts = bpf_ktime_get_ns();
	u32 event_type;

	event_type = (flags == (GFP_NOWAIT | __GFP_THISNODE)) ? NEW_SLAB_THIS_NODE : NEW_SLAB_OTHER;

	update_hist(event_type, ts);

	return 0;
}

char LICENSE[] SEC("license") = "GPL";
