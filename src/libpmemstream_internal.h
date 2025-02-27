// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021-2022, Intel Corporation */

/* Internal Header */

#ifndef LIBPMEMSTREAM_INTERNAL_H
#define LIBPMEMSTREAM_INTERNAL_H

#include <assert.h>

#include "iterator.h"
#include "libpmemstream.h"
#include "mpmc_queue.h"
#include "pmemstream_runtime.h"
#include "region.h"
#include "region_allocator/allocator_base.h"
#include "span.h"
#include "thread_id.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PMEMSTREAM_SIGNATURE ("PMEMSTREAM")
#define PMEMSTREAM_SIGNATURE_SIZE (64)

#define PMEMSTREAM_INVALID_TIMESTAMP 0ULL

/* XXX: lift this requirement */
#define PMEMSTREAM_MAX_CONCURRENCY 64ULL

struct pmemstream_header {
	char signature[PMEMSTREAM_SIGNATURE_SIZE];
	uint64_t stream_size;
	uint64_t block_size;

	/* XXX: investigate if it makes sense to store 'shadow' value in DRAM (for reads) */
	/* XXX: we can 'distribute' persisted_timestamp and store multiple variables (one per thread) to speed up writes
	 */
	/* All entries with timestamp strictly less than 'persisted_timestamp' can be treated as persisted. */
	uint64_t persisted_timestamp;

	struct allocator_header region_allocator_header;
};

struct pmemstream {
	/* Points to pmem-resided header. */
	struct pmemstream_header *header;

	/* Describes data location and memory operations. */
	struct pmemstream_runtime data;

	size_t stream_size;
	size_t usable_size;
	size_t block_size;

	struct region_runtimes_map *region_runtimes_map;
	struct mpmc_queue *timestamp_queue;
	struct thread_id *thread_id;
};

static inline int pmemstream_validate_stream_and_offset(struct pmemstream *stream, uint64_t offset)
{
	if (!stream) {
		return -1;
	}
	if (stream->header->stream_size <= offset) {
		return -1;
	}
	return 0;
}

/* Convert offset to pointer to span. offset must be 8-bytes aligned. */
static inline const struct span_base *span_offset_to_span_ptr(const struct pmemstream_runtime *data, uint64_t offset)
{
	assert(offset % sizeof(struct span_base) == 0);
	return (const struct span_base *)pmemstream_offset_to_ptr(data, offset);
}

/* XXX: make those functions public */
static inline uint64_t pmemstream_persisted_timestamp(struct pmemstream *stream)
{
	return __atomic_load_n(&stream->header->persisted_timestamp, __ATOMIC_ACQUIRE);
}

static inline uint64_t pmemstream_committed_timestamp(struct pmemstream *stream)
{
	return mpmc_queue_get_consumed_offset(stream->timestamp_queue) + 1;
}

#ifdef __cplusplus
} /* end extern "C" */
#endif
#endif /* LIBPMEMSTREAM_INTERNAL_H */
