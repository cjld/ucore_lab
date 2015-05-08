### 练习一
---

1、请理解并分析sched_calss中各个函数指针的用法，并接合Round Robin 调度算法描ucore的调度执行过程

sched_calss中包含以下函数指针：

void (*init)(struct run_queue *rq)：初始化调度队列

void (*enqueue)(struct run_queue *rq, struct proc_struct *proc)：把进程放到队列中

void (*dequeue)(struct run_queue *rq, struct proc_struct *proc)：把进程从队列中取出

struct proc_struct *(*pick_next)(struct run_queue *rq)：从队列中选择下一个运行的进程

void (*proc_tick)(struct run_queue *rq, struct proc_struct *proc)：处理时钟中断

调度时首先进行初始化，将进程数置0，如果进程是RUNNABLE的，将其加入runlist中，设置进程的rq为当前运行队列，并更新进程数和时间片。然后挑选一个合适的进程运行，将其从队列中pop出去，并更新进程数。具体到Round Robin算法，维护一个进程队列，每次挑选队首进程运行，当进程时间片用完时，调用schedule将其放到队尾。

2、请在实验报告中简要说明如何设计实现”多级反馈队列调度算法“，给出概要设计，鼓励给出详细设计

维护N个运行队列，分别对应n个优先级，并进行初始化。当进程进入待调度的队列等待时，首先挑选优先级最高的队列。选取的进执行过一个时间片后，如果还没有执行完毕，则将其加入到下一优先级队列的尾部。即enqueue时记录优先级n，dequeue时将其加入优先级为(n+1)的队列中，选取时按优先级顺序这样在每个队列中顺序选取。

---
### 练习二
---

1、实现 Stride Scheduling 调度算法

按照注释中的提示，首先需要修改proc.c中的初始化内容，增加对rq、priority、stride等变量的初始化；然后还需要修改trap中时钟中断的相关代码，之前的写法tick每到100会清零，而priority的线程只有tick到1000时才会退出，会导致死循环。此外，还需要设置BIG_STRIDE的值，为了支持尽可能多的优先级选取了0x7FFFFFFF。

然后需要初始化，我直接用list_init(&(rq->run_list))进行初始化。入队函数采用斜堆结构，用skew_heap_insert实现，并更新时间片、进程数、rq等信息。出队函数同样使用斜堆，通过skew_heap_remove实现。选取下个进程函数当队列非空时，由于采用了斜堆，直接返回rq->lab6_run_pool对应的第一个进程即可。时钟中断处理函数只需将time_slice减1，减至0时将need_resched置1即可。

在评测过程中发现，由于时间片长度太短切换太快，forktree和priority的评测结果不稳定，有时会出错。

---
### Others
---

1、与标准答案的区别

在proc.c中初始化priority时，答案中初始化为0，而我初始化为1；相应地在pick_next中我也没有像答案中一样判断priority是否为0，而是直接除以priority。其余部分由于均为按注释提示实现，与答案中的斜堆部分大同小异。

2、涉及的重要知识点

进程调度、进程队列维护、调度算法等。

3、未在实验中体现的知识点

除Round Robin外的其它调度算法。



















