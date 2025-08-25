/* SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause) */
#ifndef __TRACE_SLAB_ALLOC_h
#define __TRACE_SLAB_ALLOC_h

// hist time range: 0-180s，180 buckets, 1 sec per bucket

#define BUCKET_SIZE_NS 1000000000ULL  // 1 s = 1,000,000,000 ns

#define TASK_COMM_LEN	16
#define MAX_SLOTS	180

struct hist {
	__u32 slots[MAX_SLOTS];
	__u32 event_type;
};

#endif // __TRACE_SLAB_ALLOC_h
