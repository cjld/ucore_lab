# lab5实验报告
### 2012011373 梁盾

## 练习1: 加载应用程序并执行

1.  设置系统调用时候产生trap的特权级

    这里和lab1的代码类似, 只不过我们添加这么一行代码
    ```
    SETGATE(idt[T_SYSCALL], 1, GD_KTEXT, __vectors[T_SYSCALL], DPL_USER);
    ```
    代表了syscall不同的处理方式

2.  设置 `trapframe`

    主要代码在`load_icode`中, 其中, 设置代码如下
    ```
    tf->tf_cs = USER_CS;
    tf->tf_ds = tf->tf_es = tf->tf_ss = USER_DS;
    tf->tf_esp = USTACKTOP;
    tf->tf_eip = elf->e_entry;
    tf->tf_eflags = FL_IF;
    //#define FL_IF           0x00000200  // Interrupt Flag
    ```
    这里可以得知tf各个部分的功能是: tf的cs段为用户代码段，ds、es、ss段为用户数据段，
eip为程序开头，esp为用户栈顶，eflags允许中断。

## 练习二 父进程复制自己的内存空间给子进程

按照注释, 添加了如下代码:
```
void * src_kvaddr = page2kva(page);
void * dst_kvaddr = page2kva(npage);

memcpy(dst_kvaddr, src_kvaddr, PGSIZE);

ret = page_insert(to, npage, start, perm);
```
COW的实现思路大概如家, 可以让两个mm_struct共享同一份vma list,
并且设置为只读, 当出现page fault的时候再尝试copy_range

## 练习三 阅读分析源代码，理解进程执行 fork/exec/wait/exit 的实现，以及系统调用的实现

*   fork：创建一个新的进程，把父进程的当前状态复制之后，令新的进程状态为RUNNABLE。
*   exec：通过一个load_icode，把新的程序复制进来，再去执行。
*   wait: 通过schedule让自己变为SLEEPING的，直到再次schedule
达到唤醒条件
*   exit：首先回收大部分资源，并把自己变成ZONBIE状态，然后查看父进程，如果在等待状态，则唤醒父进程；
*   系统调用则是通过trap实现
