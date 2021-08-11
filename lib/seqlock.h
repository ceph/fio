#ifndef FIO_SEQLOCK_H
#define FIO_SEQLOCK_H

#ifdef __cplusplus
#include <atomic>
#else
#include "types.h"
#include "../arch/arch.h"
#endif

struct seqlock {
#ifdef __cplusplus
	std::atomic<unsigned int> sequence;
#else
	volatile unsigned int sequence;
#endif
};

static inline void seqlock_init(struct seqlock *s)
{
	s->sequence = 0;
}

static inline unsigned int read_seqlock_begin(struct seqlock *s)
{
	unsigned int seq;

	do {
#ifdef __cplusplus
		seq = s->sequence.load(std::memory_order_acquire);
#else
		seq = atomic_load_acquire(&s->sequence);
#endif
		if (!(seq & 1))
			break;
		nop;
	} while (1);

	return seq;
}

static inline bool read_seqlock_retry(struct seqlock *s, unsigned int seq)
{
#ifdef __cplusplus
	atomic_thread_fence(std::memory_order_acquire);
	return s->sequence.load(std::memory_order_relaxed) != seq;
#else
	read_barrier();
	return s->sequence != seq;
#endif
}

static inline void write_seqlock_begin(struct seqlock *s)
{
#ifdef __cplusplus
	s->sequence.fetch_add(1, std::memory_order_acquire);
#else
	s->sequence = atomic_load_acquire(&s->sequence) + 1;
#endif
}

static inline void write_seqlock_end(struct seqlock *s)
{
#ifdef __cplusplus
	s->sequence.fetch_add(1, std::memory_order_release);
#else
	atomic_store_release(&s->sequence, s->sequence + 1);
#endif
}

#endif
