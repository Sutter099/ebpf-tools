/* SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause) */
#ifndef __TRACE_SLAB_ALLOC_h
#define __TRACE_SLAB_ALLOC_h

// hist time range: 0-32s，320 buckets, 100 mili-sec per bucket
#define BUCKET_SIZE_100MS 100000000ULL  // 100 ms = 100,000,000 ns
#define MAX_SLOTS	320

struct hist {
	__u32 slots[MAX_SLOTS];
	__u32 event_type;
};

#endif // __TRACE_SLAB_ALLOC_h
