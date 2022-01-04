// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021, Intel Corporation */

/* Internal Header */

#ifndef LIBPMEMSTREAM_SPAN_H
#define LIBPMEMSTREAM_SPAN_H

#include "common/util.h"
#include "libpmemstream.h"

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Span is a contiguous sequence of bytes, which are located on peristent storage.
 *
 * Span is always 8-bytes aligned. Its first 8 bytes hold information about size and type of a span. Type can be one
 * of the following:
 * - region
 * - entry
 * - empty space
 *
 * Span can be created using span_create_* family of functions. Access to a span is performed through a helper
 * span_runtime structure which exposes span's type, size and other metadata depending on its type. span_runtime
 * can be obtained from span_get_runtime* family of functions.
 */

enum span_type { SPAN_EMPTY = 00ULL << 62, SPAN_REGION = 11ULL << 62, SPAN_ENTRY = 10ULL << 62 };

#define SPAN_TYPE_MASK (11ULL << 62)
#define SPAN_EXTRA_MASK (~SPAN_TYPE_MASK)

struct span_runtime {
	enum span_type type;
	size_t total_size;
	uint64_t data_offset;
	union {
		struct {
			uint64_t size;
		} empty;
		struct {
			uint64_t size;
		} region;
		struct {
			uint64_t size;
			uint64_t popcount;
		} entry;
	};
};

#define SPAN_EMPTY_METADATA_SIZE (MEMBER_SIZE(span_runtime, empty))
#define SPAN_REGION_METADATA_SIZE (MEMBER_SIZE(span_runtime, region))
#define SPAN_ENTRY_METADATA_SIZE (MEMBER_SIZE(span_runtime, entry))

typedef uint64_t span_bytes;

/* Convert offset to pointer to span. offset must be 8-bytes aligned. */
span_bytes *span_offset_to_span_ptr(struct pmemstream *stream, uint64_t offset);

/* Those functions create appropriate span at specified offset. offset must be 8-bytes aligned. */
void span_create_empty(struct pmemstream *stream, uint64_t offset, size_t data_size);
void span_create_entry(struct pmemstream *stream, uint64_t offset, const void *data, size_t data_size, size_t popcount,
		       int flags);
void span_create_region(struct pmemstream *stream, uint64_t offset, size_t size);

uint64_t span_get_size(span_bytes *span);
enum span_type span_get_type(span_bytes *span);

/* Obtain span_runtime structure describing span at 'offset'. offset must be 8-bytes aligned. */
struct span_runtime span_get_runtime(struct pmemstream *stream, uint64_t offset);

/* Works similar to the function above but span must be of certain type. */
struct span_runtime span_get_empty_runtime(struct pmemstream *stream, uint64_t offset);
struct span_runtime span_get_entry_runtime(struct pmemstream *stream, uint64_t offset);
struct span_runtime span_get_region_runtime(struct pmemstream *stream, uint64_t offset);

#ifdef __cplusplus
} /* end extern "C" */
#endif
#endif /* LIBPMEMSTREAM_SPAN_H */
