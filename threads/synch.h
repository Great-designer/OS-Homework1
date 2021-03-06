#ifndef THREADS_SYNCH_H
#define THREADS_SYNCH_H

#include <list.h>
#include <stdbool.h>

/* A counting semaphore. */
struct semaphore//计数信号灯。
{
	unsigned value;             /* Current value. *///当前值。
	struct list waiters;        /* List of waiting threads. *///等待线程的列表。
};

void sema_init (struct semaphore *, unsigned value);
void sema_down (struct semaphore *);
bool sema_try_down (struct semaphore *);
void sema_up (struct semaphore *);
void sema_self_test (void);

/* Lock. */
struct lock//锁。
{
	struct thread *holder;      /* Thread holding lock (for debugging). *///线程保持锁（用于调试）。
	struct semaphore semaphore; /* Binary semaphore controlling access. *///控制访问的二进制信号量。
	
	struct list_elem elem;//优先捐赠的列表元素。
	int max_priority;//获取锁的线程之间的最大优先级。
};

void lock_init (struct lock *);
void lock_acquire (struct lock *);
bool lock_try_acquire (struct lock *);
void lock_release (struct lock *);
bool lock_held_by_current_thread (const struct lock *);

/* Condition variable. */
struct condition 
  {
    struct list waiters;        /* List of waiting threads. */
  };

void cond_init (struct condition *);
void cond_wait (struct condition *, struct lock *);
void cond_signal (struct condition *, struct lock *);
void cond_broadcast (struct condition *, struct lock *);

bool lock_cmp_priority(const struct list_elem *a,const struct list_elem *b,void *aux);//锁比较功能

bool cond_sema_cmp_priority(const struct list_elem *a,const struct list_elem *b,void *aux);//cond-sema比较函数

/* Optimization barrier.

   The compiler will not reorder operations across an
   optimization barrier.  See "Optimization Barriers" in the
   reference guide for more information.*/
#define barrier() asm volatile ("" : : : "memory")

#endif /* threads/synch.h */
