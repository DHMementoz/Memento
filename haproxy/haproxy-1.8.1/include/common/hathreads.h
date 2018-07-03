/*
 * include/common/hathreads.h
 * definitions, macros and inline functions about threads.
 *
 * Copyright (C) 2017 Christopher Fauet - cfaulet@haproxy.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation, version 2.1
 * exclusively.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _COMMON_HATHREADS_H
#define _COMMON_HATHREADS_H

#include <common/config.h>

#define MAX_THREADS_MASK ((unsigned long)-1)
extern THREAD_LOCAL unsigned int tid;     /* The thread id */
extern THREAD_LOCAL unsigned long tid_bit; /* The bit corresponding to the thread id */

#ifndef USE_THREAD

#define __decl_hathreads(decl)

#define HA_ATOMIC_CAS(val, old, new) ({((*val) == (*old)) ? (*(val) = (new) , 1) : (*(old) = *(val), 0);})
#define HA_ATOMIC_ADD(val, i)        ({*(val) += (i);})
#define HA_ATOMIC_SUB(val, i)        ({*(val) -= (i);})
#define HA_ATOMIC_AND(val, flags)    ({*(val) &= (flags);})
#define HA_ATOMIC_OR(val, flags)     ({*(val) |= (flags);})
#define HA_ATOMIC_XCHG(val, new)					\
	({								\
		typeof(*(val)) __old = *(val);				\
		*(val) = new;						\
		__old;							\
	})
#define HA_ATOMIC_STORE(val, new)    ({*(val) = new;})
#define HA_ATOMIC_UPDATE_MAX(val, new)					\
	({								\
		typeof(*(val)) __new = (new);				\
									\
		if (*(val) < __new)					\
			*(val) = __new;					\
		*(val);							\
	})

#define HA_ATOMIC_UPDATE_MIN(val, new)					\
	({								\
		typeof(*(val)) __new = (new);				\
									\
		if (*(val) > __new)					\
			*(val) = __new;					\
		*(val);							\
	})

#define HA_BARRIER() do { } while (0)

#define THREAD_SYNC_INIT(m)  do { /* do nothing */ } while(0)
#define THREAD_SYNC_ENABLE() do { /* do nothing */ } while(0)
#define THREAD_WANT_SYNC()   do { /* do nothing */ } while(0)
#define THREAD_ENTER_SYNC()  do { /* do nothing */ } while(0)
#define THREAD_EXIT_SYNC()   do { /* do nothing */ } while(0)
#define THREAD_NO_SYNC()     ({ 0; })
#define THREAD_NEED_SYNC()   ({ 1; })

#define HA_SPIN_INIT(l)         do { /* do nothing */ } while(0)
#define HA_SPIN_DESTROY(l)      do { /* do nothing */ } while(0)
#define HA_SPIN_LOCK(lbl, l)    do { /* do nothing */ } while(0)
#define HA_SPIN_TRYLOCK(lbl, l) ({ 0; })
#define HA_SPIN_UNLOCK(lbl, l)  do { /* do nothing */ } while(0)

#define HA_RWLOCK_INIT(l)          do { /* do nothing */ } while(0)
#define HA_RWLOCK_DESTROY(l)       do { /* do nothing */ } while(0)
#define HA_RWLOCK_WRLOCK(lbl, l)   do { /* do nothing */ } while(0)
#define HA_RWLOCK_TRYWRLOCK(lbl, l)   ({ 0; })
#define HA_RWLOCK_WRUNLOCK(lbl, l) do { /* do nothing */ } while(0)
#define HA_RWLOCK_RDLOCK(lbl, l)   do { /* do nothing */ } while(0)
#define HA_RWLOCK_TRYRDLOCK(lbl, l)   ({ 0; })
#define HA_RWLOCK_RDUNLOCK(lbl, l) do { /* do nothing */ } while(0)

#else /* USE_THREAD */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <import/plock.h>

#define __decl_hathreads(decl) decl

/* TODO: thread: For now, we rely on GCC builtins but it could be a good idea to
 * have a header file regrouping all functions dealing with threads. */
#define HA_ATOMIC_CAS(val, old, new) __atomic_compare_exchange_n(val, old, new, 0, 0, 0)
#define HA_ATOMIC_ADD(val, i)        __atomic_add_fetch(val, i, 0)
#define HA_ATOMIC_SUB(val, i)        __atomic_sub_fetch(val, i, 0)
#define HA_ATOMIC_AND(val, flags)    __atomic_and_fetch(val, flags, 0)
#define HA_ATOMIC_OR(val, flags)     __atomic_or_fetch(val,  flags, 0)
#define HA_ATOMIC_XCHG(val, new)     __atomic_exchange_n(val, new, 0)
#define HA_ATOMIC_STORE(val, new)    __atomic_store_n(val, new, 0)
#define HA_ATOMIC_UPDATE_MAX(val, new)					\
	({								\
		typeof(*(val)) __old = *(val);				\
		typeof(*(val)) __new = (new);				\
									\
		while (__old < __new && !HA_ATOMIC_CAS(val, &__old, __new)); \
		(*val);							\
	})
#define HA_ATOMIC_UPDATE_MIN(val, new)					\
	({								\
		typeof((*val)) __old = *(val);				\
		typeof((*val)) __new = (new);				\
									\
		while (__old > __new && !HA_ATOMIC_CAS(val, &__old, __new)); \
		(*val);							\
	})

#define HA_BARRIER() pl_barrier()

#define THREAD_SYNC_INIT(m)   thread_sync_init(m)
#define THREAD_SYNC_ENABLE()  thread_sync_enable()
#define THREAD_WANT_SYNC()    thread_want_sync()
#define THREAD_ENTER_SYNC()   thread_enter_sync()
#define THREAD_EXIT_SYNC()    thread_exit_sync()
#define THREAD_NO_SYNC()      thread_no_sync()
#define THREAD_NEED_SYNC()    thread_need_sync()

int  thread_sync_init(unsigned long mask);
void thread_sync_enable(void);
void thread_want_sync(void);
void thread_enter_sync(void);
void thread_exit_sync(void);
int  thread_no_sync(void);
int  thread_need_sync(void);

#if defined(DEBUG_THREAD) || defined(DEBUG_FULL)

enum lock_label {
	THREAD_SYNC_LOCK = 0,
	FDTAB_LOCK,
	FDCACHE_LOCK,
	FD_LOCK,
	POLL_LOCK,
	TASK_RQ_LOCK,
	TASK_WQ_LOCK,
	POOL_LOCK,
	LISTENER_LOCK,
	LISTENER_QUEUE_LOCK,
	PROXY_LOCK,
	SERVER_LOCK,
	UPDATED_SERVERS_LOCK,
	LBPRM_LOCK,
	SIGNALS_LOCK,
	STK_TABLE_LOCK,
	STK_SESS_LOCK,
	APPLETS_LOCK,
	PEER_LOCK,
	BUF_WQ_LOCK,
	STRMS_LOCK,
	SSL_LOCK,
	SSL_GEN_CERTS_LOCK,
	PATREF_LOCK,
	PATEXP_LOCK,
	PATLRU_LOCK,
	VARS_LOCK,
	COMP_POOL_LOCK,
	LUA_LOCK,
	NOTIF_LOCK,
	SPOE_APPLET_LOCK,
	DNS_LOCK,
	PID_LIST_LOCK,
	EMAIL_ALERTS_LOCK,
	PIPES_LOCK,
	LOCK_LABELS
};
struct lock_stat {
	uint64_t nsec_wait_for_write;
	uint64_t nsec_wait_for_read;
	uint64_t num_write_locked;
	uint64_t num_write_unlocked;
	uint64_t num_read_locked;
	uint64_t num_read_unlocked;
};

extern struct lock_stat lock_stats[LOCK_LABELS];

#define __HA_SPINLOCK_T      unsigned long

#define __SPIN_INIT(l)         ({ (*l) = 0; })
#define __SPIN_DESTROY(l)      ({ (*l) = 0; })
#define __SPIN_LOCK(l)         pl_take_s(l)
#define __SPIN_TRYLOCK(l)      !pl_try_s(l)
#define __SPIN_UNLOCK(l)       pl_drop_s(l)

#define __HA_RWLOCK_T		unsigned long

#define __RWLOCK_INIT(l)          ({ (*l) = 0; })
#define __RWLOCK_DESTROY(l)       ({ (*l) = 0; })
#define __RWLOCK_WRLOCK(l)        pl_take_w(l)
#define __RWLOCK_TRYWRLOCK(l)     !pl_try_w(l)
#define __RWLOCK_WRUNLOCK(l)      pl_drop_w(l)
#define __RWLOCK_RDLOCK(l)        pl_take_r(l)
#define __RWLOCK_TRYRDLOCK(l)     !pl_try_r(l)
#define __RWLOCK_RDUNLOCK(l)      pl_drop_r(l)

#define HA_SPINLOCK_T       struct ha_spinlock

#define HA_SPIN_INIT(l)        __spin_init(l)
#define HA_SPIN_DESTROY(l)      __spin_destroy(l)

#define HA_SPIN_LOCK(lbl, l)    __spin_lock(lbl, l, __func__, __FILE__, __LINE__)
#define HA_SPIN_TRYLOCK(lbl, l) __spin_trylock(lbl, l, __func__, __FILE__, __LINE__)
#define HA_SPIN_UNLOCK(lbl, l)  __spin_unlock(lbl, l, __func__, __FILE__, __LINE__)

#define HA_RWLOCK_T         struct ha_rwlock

#define HA_RWLOCK_INIT(l)          __ha_rwlock_init((l))
#define HA_RWLOCK_DESTROY(l)       __ha_rwlock_destroy((l))
#define HA_RWLOCK_WRLOCK(lbl,l)    __ha_rwlock_wrlock(lbl, l, __func__, __FILE__, __LINE__)
#define HA_RWLOCK_TRYWRLOCK(lbl,l) __ha_rwlock_trywrlock(lbl, l, __func__, __FILE__, __LINE__)
#define HA_RWLOCK_WRUNLOCK(lbl,l)  __ha_rwlock_wrunlock(lbl, l, __func__, __FILE__, __LINE__)
#define HA_RWLOCK_RDLOCK(lbl,l)    __ha_rwlock_rdlock(lbl, l)
#define HA_RWLOCK_TRYRDLOCK(lbl,l) __ha_rwlock_tryrdlock(lbl, l)
#define HA_RWLOCK_RDUNLOCK(lbl,l)  __ha_rwlock_rdunlock(lbl, l)

struct ha_spinlock {
	__HA_SPINLOCK_T lock;
	struct {
		unsigned long owner; /* a bit is set to 1 << tid for the lock owner */
		unsigned long waiters; /* a bit is set to 1 << tid for waiting threads  */
		struct {
			const char *function;
			const char *file;
			int line;
		} last_location; /* location of the last owner */
	} info;
};

struct ha_rwlock {
	__HA_RWLOCK_T lock;
	struct {
		unsigned long cur_writer; /* a bit is set to 1 << tid for the lock owner */
		unsigned long wait_writers; /* a bit is set to 1 << tid for waiting writers */
		unsigned long cur_readers; /* a bit is set to 1 << tid for current readers */
		unsigned long wait_readers; /* a bit is set to 1 << tid for waiting waiters */
		struct {
			const char *function;
			const char *file;
			int line;
		} last_location; /* location of the last write owner */
	} info;
};

static inline void show_lock_stats()
{
	const char *labels[LOCK_LABELS] = {"THREAD_SYNC", "FDTAB", "FDCACHE", "FD", "POLL",
					   "TASK_RQ", "TASK_WQ", "POOL",
					   "LISTENER", "LISTENER_QUEUE", "PROXY", "SERVER",
					   "UPDATED_SERVERS", "LBPRM", "SIGNALS", "STK_TABLE", "STK_SESS",
					   "APPLETS", "PEER", "BUF_WQ", "STREAMS", "SSL", "SSL_GEN_CERTS",
					   "PATREF", "PATEXP", "PATLRU", "VARS", "COMP_POOL", "LUA",
					   "NOTIF", "SPOE_APPLET", "DNS", "PID_LIST", "EMAIL_ALERTS",
					   "PIPES" };
	int lbl;

	for (lbl = 0; lbl < LOCK_LABELS; lbl++) {
		fprintf(stderr,
			"Stats about Lock %s: \n"
			"\t # write lock  : %lu\n"
			"\t # write unlock: %lu (%ld)\n"
			"\t # wait time for write     : %.3f msec\n"
			"\t # wait time for write/lock: %.3f nsec\n"
			"\t # read lock   : %lu\n"
			"\t # read unlock : %lu (%ld)\n"
			"\t # wait time for read      : %.3f msec\n"
			"\t # wait time for read/lock : %.3f nsec\n",
			labels[lbl],
			lock_stats[lbl].num_write_locked,
			lock_stats[lbl].num_write_unlocked,
			lock_stats[lbl].num_write_unlocked - lock_stats[lbl].num_write_locked,
			(double)lock_stats[lbl].nsec_wait_for_write / 1000000.0,
			lock_stats[lbl].num_write_locked ? ((double)lock_stats[lbl].nsec_wait_for_write / (double)lock_stats[lbl].num_write_locked) : 0,
			lock_stats[lbl].num_read_locked,
			lock_stats[lbl].num_read_unlocked,
			lock_stats[lbl].num_read_unlocked - lock_stats[lbl].num_read_locked,
			(double)lock_stats[lbl].nsec_wait_for_read / 1000000.0,
			lock_stats[lbl].num_read_locked ? ((double)lock_stats[lbl].nsec_wait_for_read / (double)lock_stats[lbl].num_read_locked) : 0);
	}
}

/* Following functions are used to collect some stats about locks. We wrap
 * pthread functions to known how much time we wait in a lock. */

static uint64_t nsec_now(void) {
        struct timespec ts;

        clock_gettime(CLOCK_MONOTONIC, &ts);
        return ((uint64_t) ts.tv_sec * 1000000000ULL +
                (uint64_t) ts.tv_nsec);
}

static inline void __ha_rwlock_init(struct ha_rwlock *l)
{
	memset(l, 0, sizeof(struct ha_rwlock));
	__RWLOCK_INIT(&l->lock);
}

static inline void __ha_rwlock_destroy(struct ha_rwlock *l)
{
	__RWLOCK_DESTROY(&l->lock);
	memset(l, 0, sizeof(struct ha_rwlock));
}


static inline void __ha_rwlock_wrlock(enum lock_label lbl, struct ha_rwlock *l,
				      const char *func, const char *file, int line)
{
	uint64_t start_time;

	if (unlikely(l->info.cur_writer & tid_bit)) {
		/* the thread is already owning the lock for write */
		abort();
	}

	if (unlikely(l->info.cur_readers & tid_bit)) {
		/* the thread is already owning the lock for read */
		abort();
	}

	HA_ATOMIC_OR(&l->info.wait_writers, tid_bit);

	start_time = nsec_now();
	__RWLOCK_WRLOCK(&l->lock);
	HA_ATOMIC_ADD(&lock_stats[lbl].nsec_wait_for_write, (nsec_now() - start_time));

	HA_ATOMIC_ADD(&lock_stats[lbl].num_write_locked, 1);

	l->info.cur_writer             = tid_bit;
	l->info.last_location.function = func;
	l->info.last_location.file     = file;
	l->info.last_location.line     = line;

	HA_ATOMIC_AND(&l->info.wait_writers, ~tid_bit);
}

static inline int __ha_rwlock_trywrlock(enum lock_label lbl, struct ha_rwlock *l,
				        const char *func, const char *file, int line)
{
	uint64_t start_time;
	int r;

	if (unlikely(l->info.cur_writer & tid_bit)) {
		/* the thread is already owning the lock for write */
		abort();
	}

	if (unlikely(l->info.cur_readers & tid_bit)) {
		/* the thread is already owning the lock for read */
		abort();
	}

	/* We set waiting writer because trywrlock could wait for readers to quit */
	HA_ATOMIC_OR(&l->info.wait_writers, tid_bit);

	start_time = nsec_now();
	r = __RWLOCK_TRYWRLOCK(&l->lock);
	HA_ATOMIC_ADD(&lock_stats[lbl].nsec_wait_for_write, (nsec_now() - start_time));
	if (unlikely(r)) {
		HA_ATOMIC_AND(&l->info.wait_writers, ~tid_bit);
		return r;
	}
	HA_ATOMIC_ADD(&lock_stats[lbl].num_write_locked, 1);

	l->info.cur_writer             = tid_bit;
	l->info.last_location.function = func;
	l->info.last_location.file     = file;
	l->info.last_location.line     = line;

	HA_ATOMIC_AND(&l->info.wait_writers, ~tid_bit);

	return 0;
}

static inline void __ha_rwlock_wrunlock(enum lock_label lbl,struct ha_rwlock *l,
				        const char *func, const char *file, int line)
{
	if (unlikely(!(l->info.cur_writer & tid_bit))) {
		/* the thread is not owning the lock for write */
		abort();
	}

	l->info.cur_writer             = 0;
	l->info.last_location.function = func;
	l->info.last_location.file     = file;
	l->info.last_location.line     = line;

	__RWLOCK_WRUNLOCK(&l->lock);

	HA_ATOMIC_ADD(&lock_stats[lbl].num_write_unlocked, 1);
}

static inline void __ha_rwlock_rdlock(enum lock_label lbl,struct ha_rwlock *l)
{
	uint64_t start_time;

	if (unlikely(l->info.cur_writer & tid_bit)) {
		/* the thread is already owning the lock for write */
		abort();
	}

	if (unlikely(l->info.cur_readers & tid_bit)) {
		/* the thread is already owning the lock for read */
		abort();
	}

	HA_ATOMIC_OR(&l->info.wait_readers, tid_bit);

	start_time = nsec_now();
	__RWLOCK_RDLOCK(&l->lock);
	HA_ATOMIC_ADD(&lock_stats[lbl].nsec_wait_for_read, (nsec_now() - start_time));
	HA_ATOMIC_ADD(&lock_stats[lbl].num_read_locked, 1);

	HA_ATOMIC_OR(&l->info.cur_readers, tid_bit);

	HA_ATOMIC_AND(&l->info.wait_readers, ~tid_bit);
}

static inline int __ha_rwlock_tryrdlock(enum lock_label lbl,struct ha_rwlock *l)
{
	int r;

	if (unlikely(l->info.cur_writer & tid_bit)) {
		/* the thread is already owning the lock for write */
		abort();
	}

	if (unlikely(l->info.cur_readers & tid_bit)) {
		/* the thread is already owning the lock for read */
		abort();
	}

	/* try read should never wait */
	r = __RWLOCK_TRYRDLOCK(&l->lock);
	if (unlikely(r))
		return r;
	HA_ATOMIC_ADD(&lock_stats[lbl].num_read_locked, 1);

	HA_ATOMIC_OR(&l->info.cur_readers, tid_bit);

	return 0;
}

static inline void __ha_rwlock_rdunlock(enum lock_label lbl,struct ha_rwlock *l)
{
	if (unlikely(!(l->info.cur_readers & tid_bit))) {
		/* the thread is not owning the lock for read */
		abort();
	}

	HA_ATOMIC_AND(&l->info.cur_readers, ~tid_bit);

	__RWLOCK_RDUNLOCK(&l->lock);

	HA_ATOMIC_ADD(&lock_stats[lbl].num_read_unlocked, 1);
}

static inline void __spin_init(struct ha_spinlock *l)
{
	memset(l, 0, sizeof(struct ha_spinlock));
	__SPIN_INIT(&l->lock);
}

static inline void __spin_destroy(struct ha_spinlock *l)
{
	__SPIN_DESTROY(&l->lock);
	memset(l, 0, sizeof(struct ha_spinlock));
}

static inline void __spin_lock(enum lock_label lbl, struct ha_spinlock *l,
			      const char *func, const char *file, int line)
{
	uint64_t start_time;

	if (unlikely(l->info.owner & tid_bit)) {
		/* the thread is already owning the lock */
		abort();
	}

	HA_ATOMIC_OR(&l->info.waiters, tid_bit);

	start_time = nsec_now();
	__SPIN_LOCK(&l->lock);
	HA_ATOMIC_ADD(&lock_stats[lbl].nsec_wait_for_write, (nsec_now() - start_time));

	HA_ATOMIC_ADD(&lock_stats[lbl].num_write_locked, 1);


	l->info.owner                  = tid_bit;
	l->info.last_location.function = func;
	l->info.last_location.file     = file;
	l->info.last_location.line     = line;

	HA_ATOMIC_AND(&l->info.waiters, ~tid_bit);
}

static inline int __spin_trylock(enum lock_label lbl, struct ha_spinlock *l,
				 const char *func, const char *file, int line)
{
	int r;

	if (unlikely(l->info.owner & tid_bit)) {
		/* the thread is already owning the lock */
		abort();
	}

	/* try read should never wait */
	r = __SPIN_TRYLOCK(&l->lock);
	if (unlikely(r))
		return r;
	HA_ATOMIC_ADD(&lock_stats[lbl].num_write_locked, 1);

	l->info.owner                  = tid_bit;
	l->info.last_location.function = func;
	l->info.last_location.file     = file;
	l->info.last_location.line     = line;

	return 0;
}

static inline void __spin_unlock(enum lock_label lbl, struct ha_spinlock *l,
				 const char *func, const char *file, int line)
{
	if (unlikely(!(l->info.owner & tid_bit))) {
		/* the thread is not owning the lock */
		abort();
	}

	l->info.owner                  = 0;
	l->info.last_location.function = func;
	l->info.last_location.file     = file;
	l->info.last_location.line     = line;

	__SPIN_UNLOCK(&l->lock);
	HA_ATOMIC_ADD(&lock_stats[lbl].num_write_unlocked, 1);
}

#else /* DEBUG_THREAD */

#define HA_SPINLOCK_T        unsigned long

#define HA_SPIN_INIT(l)         ({ (*l) = 0; })
#define HA_SPIN_DESTROY(l)      ({ (*l) = 0; })
#define HA_SPIN_LOCK(lbl, l)    pl_take_s(l)
#define HA_SPIN_TRYLOCK(lbl, l) !pl_try_s(l)
#define HA_SPIN_UNLOCK(lbl, l)  pl_drop_s(l)

#define HA_RWLOCK_T		unsigned long

#define HA_RWLOCK_INIT(l)          ({ (*l) = 0; })
#define HA_RWLOCK_DESTROY(l)       ({ (*l) = 0; })
#define HA_RWLOCK_WRLOCK(lbl,l)    pl_take_w(l)
#define HA_RWLOCK_TRYWRLOCK(lbl,l) !pl_try_w(l)
#define HA_RWLOCK_WRUNLOCK(lbl,l)  pl_drop_w(l)
#define HA_RWLOCK_RDLOCK(lbl,l)    pl_take_r(l)
#define HA_RWLOCK_TRYRDLOCK(lbl,l) !pl_try_r(l)
#define HA_RWLOCK_RDUNLOCK(lbl,l)  pl_drop_r(l)

#endif  /* DEBUG_THREAD */

#endif /* USE_THREAD */

#endif /* _COMMON_HATHREADS_H */
