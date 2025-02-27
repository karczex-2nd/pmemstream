// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2022, Intel Corporation */

/* Internal Header */

#ifndef LIBPMEMSTREAM_MPMC_QUEUE_H
#define LIBPMEMSTREAM_MPMC_QUEUE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MPMC_QUEUE_OFFSET_MAX UINT64_MAX

/*
 * mpmc_queue is a lock-free, bounded, multi-producer, multi-consumer queue.
 *
 * Producers call 'mpmc_queue_acquire' to get an offset which specifies some range [offset, offset + size)
 * such that no other producer gets the same or overlapping range. After producer is ready to
 * make its range available to consume (e.g. wants to publish its data) it calls 'mpmc_queue_produce'.
 *
 * Consumers can call 'mpmc_queue_consume' to get 'size' and 'ready_offset'. [ready_offset, ready_offset + size)
 * is the biggest, ready to be consumed range. If size returned by the function is 0, it means there is
 * nothing to consume. However, 'ready_offset' is still set in that case.
 */

/* XXX: add support for dynamic producer registration? */
struct mpmc_queue *mpmc_queue_new(size_t num_producers, size_t size);
void mpmc_queue_destroy(struct mpmc_queue *queue);

/* Returns offset reserved for the producer. No other producer will get the same offset.
 * No consumer can go beyond the offset until mpmc_queue_produce is called.
 *
 * Returns MPMC_QUEUE_OFFSET_MAX on error. This call can fail if 'size' + returned offset would be
 * bigger than size of the queue.
 * */
uint64_t mpmc_queue_acquire(struct mpmc_queue *queue, uint64_t producer_id, size_t size);
/* Marks offset acquired via call to mpmc_queue_acquire as ready to be consumed. */
void mpmc_queue_produce(struct mpmc_queue *queue, uint64_t producer_id);

/* Returns number of bytes consumed, 'ready_offset' is start offset of consumed data. */
uint64_t mpmc_queue_consume(struct mpmc_queue *queue, uint64_t max_producer_id, size_t *ready_offset);

/* Returns offset up to which data is already consumed. */
uint64_t mpmc_queue_get_consumed_offset(struct mpmc_queue *queue);

/* Set producer and consumer offset to 'offset' */
void mpmc_queue_reset(struct mpmc_queue *queue, uint64_t offset);

/* Creates a copy of the queue - used for TESTING. */
struct mpmc_queue *mpmc_queue_copy(struct mpmc_queue *queue);

#ifdef __cplusplus
} /* end extern "C" */
#endif
#endif /* LIBPMEMSTREAM_MPMC_QUEUE_H */
