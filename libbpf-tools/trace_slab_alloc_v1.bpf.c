#include <vmlinux.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>

#define ___GFP_KSWAPD_RECLAIM	(1 << ___GFP_KSWAPD_RECLAIM_BIT)
#define __GFP_NOWARN (1 << ___GFP_NOWARN_BIT)
#define GFP_NOWAIT	(___GFP_KSWAPD_RECLAIM | __GFP_NOWARN)
#define __GFP_THISNODE (1 << ___GFP_THISNODE_BIT)

struct {
	__uint(type, BPF_MAP_TYPE_ARRAY);
	__uint(max_entries, 5);
	__type(key, u32);
	__type(value, u64);
} counters SEC(".maps");

enum {
	SLAB_ALLOC = 0,
	GET_PARTIAL_THIS_NODE,
	GET_PARTIAL_OTHER,
	NEW_SLAB_THIS_NODE,
	NEW_SLAB_OTHER
};

SEC("kprobe/___slab_alloc")
int BPF_KPROBE(trace_slab_alloc)
{
	u64 *val = bpf_map_lookup_elem(&counters, &key);
	u32 key = SLAB_ALLOC;

	if (val)
		__sync_fetch_and_add(val, 1);

	return 0;
}

SEC("kprobe/get_partial")
int BPF_KPROBE(get_partial_probe, struct kmem_cache *s, int node, struct partial_context *pc)
{
	gfp_t flags;
	u64 *val;
	u32 key;

	flags = BPF_CORE_READ(pc, flags);
	key = (flags == (GFP_NOWAIT | __GFP_THISNODE)) ? GET_PARTIAL_THIS_NODE : GET_PARTIAL_OTHER;
	val = bpf_map_lookup_elem(&counters, &key);

	if (val)
		__sync_fetch_and_add(val, 1);

	return 0;
}

SEC("kprobe/new_slab")
int BPF_KPROBE(trace_new_slab, struct kmem_cache *s, gfp_t flags, int node)
{
	u64 *val;
	u32 key;

	key = (flags == (GFP_NOWAIT | __GFP_THISNODE)) ? NEW_SLAB_THIS_NODE : NEW_SLAB_OTHER;
	val = bpf_map_lookup_elem(&counters, &key);

	if (val)
		__sync_fetch_and_add(val, 1);

	return 0;
}

char LICENSE[] SEC("license") = "GPL";
