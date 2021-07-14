# OS-Homework1
操作系统实验1

一如往常。这一届使用的是Stanford-CS140的实验1。

代码仅供参考。

对于C文件构成的系统，.h文件可以看作一个类/对象，或者多个类/对象，其中定义该类/对象的属性（结构体元素）和方法（一堆函数头）。在.c文件中，为方法（.h中的函数头）进行实现。

仅实验1，需要额外补充一个fixed-point.h头文件，位于threads目录下，被thread.c、thread.h，以及devices目录下的timer.c调用。它的全体内容可以是：

    #ifndef __THREAD_FIXED_POINT_H
    #define __THREAD_FIXED_POINT_H

    //定点值的基本定义。
    typedef int fixed_t;
    //16 LSB用于小数部分。
    #define FP_SHIFT_AMOUNT 16
    //将值转换为定点值。
    #define FP_CONST(A) ((fixed_t)(A<<FP_SHIFT_AMOUNT))
    //两个定点值相加。
    #define FP_ADD(A,B) (A+B)
    //定点值a加上整型值B。
    #define FP_ADD_MIX(A,B) (A+(B<<FP_SHIFT_AMOUNT))
    //两个定点值相减。
    #define FP_SUB(A,B) (A-B)
    //定点值a减去整型值B。 
    #define FP_SUB_MIX(A,B) (A-(B<<FP_SHIFT_AMOUNT))
    //定点值a乘以整型值B。
    #define FP_MULT_MIX(A,B) (A*B)
    //定点值a除以整型值B。
    #define FP_DIV_MIX(A,B) (A/B)
    //两个定点值相乘。
    #define FP_MULT(A,B) ((fixed_t)(((int64_t)A)*B>>FP_SHIFT_AMOUNT))
    //两个定点值相除。
    #define FP_DIV(A,B) ((fixed_t)((((int64_t)A)<<FP_SHIFT_AMOUNT)/B))
    //获取定点值的整数部分。
    #define FP_INT_PART(A) (A>>FP_SHIFT_AMOUNT)
    //获取定点值的舍入整数。
    #define FP_ROUND(A) (A>=0?((A+(1<<(FP_SHIFT_AMOUNT-1)))>>FP_SHIFT_AMOUNT):((A-(1<<(FP_SHIFT_AMOUNT-1)))>>FP_SHIFT_AMOUNT))
    
    #endif //threads/fixed_point.h

由于没有提供，事实上也可以自己写这个文件。本次实验应该提供一个这样的文件。

实验1与后面的实验不兼容，不能同时更改。后面的实验需要从全新的pint操作系统上进行。

实验1需要修改threads目录下的thread.c、synch.c及其对应的头文件，并改动devices目录下的timer.c。

在.h文件中的增加方法略。在.h文件中需要增加的属性是：

在synch.h中：

锁结构体增加：

    struct list_elem elem;//优先捐赠的列表元素。
    int max_priority;//获取锁的线程之间的最大优先级。

在thread.h中：

    int64_t ticks_blocked;//记录线程被阻塞的时间。
    int base_priority;//基本优先级。
    struct list locks;//锁定线程所在的锁。
    struct lock *lock_waiting;//线程正在等待的锁。
    int nice;//亲切值。
    fixed_t recent_cpu;//最近的CPU。

需要在.c文件中实现上面这些属性对应的功能。

在timer.c中需要重构timer_sleep和timer_interrupt两个函数的功能。原本的timer_sleep函数为：

    void timer_sleep(int64_t ticks) 
    {
    	int64_t start = timer_ticks();
    	ASSERT(intr_get_level() == INTR_ON);
    	while(timer_elapsed(start) < ticks) 
    		thread_yield();
    }

原本是借助thread_yield()和thread_tick()实现功能，但是这里的while循环和thread_yield()效率太低。所以这里开一个中断：

    //在ticks时间内，如果线程处于running状态，就不断把它扔到就绪队列，不让它执行
    void timer_sleep(int64_t ticks)
    {
    	if(ticks<=0)
    	{
    		return;
    	}
    	ASSERT(intr_get_level()==INTR_ON);//断言必须可以被中断，不然会一直死循环下去
    	enum intr_level old_level=intr_disable();//原子操作
    	struct thread *current_thread=thread_current();
    	current_thread->ticks_blocked=ticks;//这个ticks是函数的形参，不是全局变量
    	thread_block();
    	intr_set_level(old_level);//原子操作
    }

原本的timer_interrupt函数为：

    static void timer_interrupt(struct intr_frame *args UNUSED)
    {
    	ticks++;
    	thread_tick();
    }

在最后一个任务要重构此函数。

    static void timer_interrupt(struct intr_frame *args UNUSED)
    {
    	ticks++;
    	thread_foreach(blocked_thread_check,NULL);//加入线程sleep时间的检测
    	thread_tick();
    	if(thread_mlfqs)
    	{
    		thread_mlfqs_increase_recent_cpu_by_one();//thread.h新函数。最近的cpu增加1。
    		if(ticks%TIMER_FREQ==0)
    		{
    			thread_mlfqs_update_load_avg_and_recent_cpu();//thread.h新函数。每秒刷新所有线程的平均负载和最近的cpu。
    		}
    		else if(ticks%4==0)
    		{
    			thread_mlfqs_update_priority(thread_current());//thread.h新函数。更新优先级。
    		}
    	}
    }

这三个新函数就是在thread.c里需要实现的方法。

    void thread_mlfqs_increase_recent_cpu_by_one(void)//最近的cpu增加1。
    {
    	ASSERT(thread_mlfqs);
    	ASSERT(intr_context());
    	struct thread *current_thread=thread_current();
    	if(current_thread==idle_thread)
    	{
    		return;
    	}
    	current_thread->recent_cpu=FP_ADD_MIX(current_thread->recent_cpu,1);
    }
    
    void thread_mlfqs_update_load_avg_and_recent_cpu(void)//每秒刷新所有线程的平均负载和最近的cpu。
    {
    	ASSERT(thread_mlfqs);
    	ASSERT(intr_context());
    	size_t ready_threads=list_size(&ready_list);
    	if(thread_current()!=idle_thread)
    	{
    		ready_threads++;
    	}
    	load_avg=FP_ADD(FP_DIV_MIX(FP_MULT_MIX(load_avg,59),60),FP_DIV_MIX(FP_CONST(ready_threads),60));
    	struct thread *t;
    	struct list_elem *e=list_begin(&all_list);
    	for(;e!=list_end(&all_list);e=list_next(e))
    	{
    		t=list_entry(e,struct thread,allelem);
    		if (t!=idle_thread)
    		{
    			t->recent_cpu=FP_ADD_MIX(FP_MULT(FP_DIV(FP_MULT_MIX(load_avg,2),FP_ADD_MIX(FP_MULT_MIX(load_avg,2),1)),t->recent_cpu),t->nice);
    			thread_mlfqs_update_priority(t);
    		}
    	}
    }
    
    void thread_mlfqs_update_priority(struct thread *t)//更新优先级。
    {
    	if(t==idle_thread)
    	{
    		return;
    	}
    	ASSERT(thread_mlfqs);
    	ASSERT(t!=idle_thread);
    	t->priority=FP_INT_PART(FP_SUB_MIX(FP_SUB(FP_CONST(PRI_MAX),FP_DIV_MIX(t->recent_cpu,4)),2*t->nice));
    	t->priority=t->priority<PRI_MIN?PRI_MIN:t->priority;
    	t->priority=t->priority>PRI_MAX?PRI_MAX:t->priority;
    }


