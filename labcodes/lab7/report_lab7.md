# lab5实验报告
### 2012011373 梁盾


## 练习1: 理解内核级信号量的实现和基于内核级信号量的哲学家就餐问题（不需要编码）

首先我们可以查看 `semaphore_t` 这个类, 主要函数是 up, down

进入这两个函数首先会关闭中断,
如果使用add, 则会查看 `wait_queue` 是否为空, 为空则value+1, 否则取出队首元素, 将其
唤醒.
如果使用down, 查看value是否大于0, 如果大于零的话value-1直接退出, 否则加入wait_queue
并且reschedule.

然后我们在查看使用信号量实现的哲学家就餐问题, 在check_sync.c文件中, 我们可以看到这么四个
函数,

*   phi_test_sema(i) 哲学家 i 检查左右是否进餐, 尝试进餐
*   phi_take_forks_sema(i) 哲学家 i 进餐, 阻塞到进餐为止
*   phi_take_forks_sema(i) 哲学家 i 放下叉子, 并且通知左右的哲学家.
*   philosopher_using_semaphore(i) 吃4次饭

## 练习2: 完成内核级条件变量和基于内核级条件变量的哲学家就餐问题（需要编码）

观察下面数据结构
```
typedef struct condvar{
    semaphore_t sem;        // the sem semaphore  is used to down the waiting proc, and the signaling proc should up the waiting proc
    int count;              // the number of waiters on condvar
    monitor_t * owner;      // the owner(monitor) of this condvar
} condvar_t;

typedef struct monitor{
    semaphore_t mutex;      // the mutex lock for going into the routines in monitor, should be initialized to 1
    semaphore_t next;       // the next semaphore is used to down the signaling proc itself, and the other OR wakeuped waiting proc should wake up the sleeped signaling proc.
    int next_count;         // the number of of sleeped signaling proc
    condvar_t *cv;          // the condvars in monitor
} monitor_t;
```
monitor.mutex 用来保证管线内只有一个线程在运行, 而 next 保存了其他在管线中并且等待的线程

根据 count, 我们可以知道当前是否有在wait的进程 , 根据 next_count, 我们可以知道是否有进程
在sleep

和答案不同的是 phi_take_forks_condvar 中的 while循环可以直接使用if, 因为 wait 被唤醒
必然已经吃到.
