#ifndef THREADS_THREAD_H
#define THREADS_THREAD_H

#include <debug.h>
#include <list.h>
#include <stdint.h>

#include"threads/fixed_point.h"

/* States in a thread's life cycle. */
enum thread_status
  {
    THREAD_RUNNING,     /* Running thread. */
    THREAD_READY,       /* Not running but ready to run. */
    THREAD_BLOCKED,     /* Waiting for an event to trigger. */
    THREAD_DYING        /* About to be destroyed. */
  };

/* Thread identifier type.
   You can redefine this to whatever type you like. */
typedef int tid_t;
#define TID_ERROR ((tid_t) -1)          /* Error value for tid_t. */

/* Thread priorities. *///线程优先级。
#define PRI_MIN 0                       /* Lowest priority. *///最低优先级。
#define PRI_DEFAULT 31                  /* Default priority. *///默认优先级。
#define PRI_MAX 63                      /* Highest priority. *///最高优先级。

/* A kernel thread or user process.

   Each thread structure is stored in its own 4 kB page.  The
   thread structure itself sits at the very bottom of the page
   (at offset 0).  The rest of the page is reserved for the
   thread's kernel stack, which grows downward from the top of
   the page (at offset 4 kB).  Here's an illustration:

        4 kB +---------------------------------+
             |          kernel stack           |
             |                |                |
             |                |                |
             |                V                |
             |         grows downward          |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             +---------------------------------+
             |              magic              |
             |                :                |
             |                :                |
             |               name              |
             |              status             |
        0 kB +---------------------------------+

   The upshot of this is twofold:

      1. First, `struct thread' must not be allowed to grow too
         big.  If it does, then there will not be enough room for
         the kernel stack.  Our base `struct thread' is only a
         few bytes in size.  It probably should stay well under 1
         kB.

      2. Second, kernel stacks must not be allowed to grow too
         large.  If a stack overflows, it will corrupt the thread
         state.  Thus, kernel functions should not allocate large
         structures or arrays as non-static local variables.  Use
         dynamic allocation with malloc() or palloc_get_page()
         instead.

   The first symptom of either of these problems will probably be
   an assertion failure in thread_current(), which checks that
   the `magic' member of the running thread's `struct thread' is
   set to THREAD_MAGIC.  Stack overflow will normally change this
   value, triggering the assertion. */
/* The `elem' member has a dual purpose.  It can be an element in the run queue (thread.c), or it can be an element in a semaphore wait list (synch.c).  It can be used these two ways only because they are mutually exclusive: only a thread in the ready state is on the run queue, whereas only a thread in the blocked state is on a semaphore wait list. */
//“elem”成员有双重目的。它可以是运行队列（thread.c）中的元素，也可以是信号量等待列表（synch.c）中的元素。这两种方法之所以可以使用，是因为它们是互斥的：只有处于就绪状态的线程在运行队列中，而只有处于阻塞状态的线程位于信号量等待列表中。
struct thread
{
	/* Owned by thread.c. *///归thread.c所有。
	tid_t tid;                          /* Thread identifier. *///线程标识符。 
	enum thread_status status;          /* Thread state. *///线程状态。
	char name[16];                      /* Name (for debugging purposes). *///名称（用于调试）。
	uint8_t *stack;                     /* Saved stack pointer. *///已保存堆栈指针。
	int priority;                       /* Priority. *///优先。
	struct list_elem allelem;           /* List element for all threads list. *///所有线程的列表元素列表。
	/* Shared between thread.c and synch.c. *///在thread.c和synch.c之间共享。
	struct list_elem elem;              /* List element. *///列表元素。
#ifdef USERPROG
	/* Owned by userprog/process.c. *///由userprog/process.c所有。
	uint32_t *pagedir;                  /* Page directory. *///页面目录。
#endif
    /* Owned by thread.c. *///归thread.c所有。
	unsigned magic;                     /* Detects stack overflow. *///检测堆栈溢出。
	
	int64_t ticks_blocked;//记录线程被阻塞的时间。
	
	int base_priority;//基本优先级。
	struct list locks;//锁定线程所在的锁。
	struct lock *lock_waiting;//线程正在等待的锁。
	
	int nice;//亲切值。
	fixed_t recent_cpu;//最近的CPU。
};

/* If false (default), use round-robin scheduler.
   If true, use multi-level feedback queue scheduler.
   Controlled by kernel command-line option "-o mlfqs". */
extern bool thread_mlfqs;

void thread_init (void);
void thread_start (void);

void thread_tick (void);
void thread_print_stats (void);

typedef void thread_func (void *aux);
tid_t thread_create (const char *name, int priority, thread_func *, void *);

void thread_block (void);
void thread_unblock (struct thread *);

struct thread *thread_current (void);
tid_t thread_tid (void);
const char *thread_name (void);

void thread_exit (void) NO_RETURN;
void thread_yield (void);

/* Performs some operation on thread t, given auxiliary data AUX. */
typedef void thread_action_func (struct thread *t, void *aux);
void thread_foreach (thread_action_func *, void *);

int thread_get_priority (void);
void thread_set_priority (int);

int thread_get_nice (void);
void thread_set_nice (int);
int thread_get_recent_cpu (void);
int thread_get_load_avg (void);

void blocked_thread_check(struct thread *t,void *aux UNUSED);//检查堵塞的线程 

bool thread_cmp_priority(const struct list_elem *a,const struct list_elem *b,void *aux);//优先级比较功能。

void thread_hold_the_lock(struct lock *lock);//让线程锁上

void thread_donate_priority(struct thread *t);//将当前优先级捐赠给线程t。

void thread_remove_lock(struct lock *lock);//拆下锁。

void thread_update_priority(struct thread *t);//更新优先级。

void thread_mlfqs_increase_recent_cpu_by_one(void);//最近的cpu增加1。

void thread_mlfqs_update_load_avg_and_recent_cpu(void);//每秒刷新所有线程的平均负载和最近的cpu。

void thread_mlfqs_update_priority(struct thread *t);//更新优先级。

#endif /* threads/thread.h */
