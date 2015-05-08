# lab3实验报告
### 2012011373 梁盾


##练习0
> 填写已有实验

通过meld merge工具实现.

merge之后成功通过测试, 并且输出了大量page fault, 也就是我们在lab3需要处理的事情
```
page fault at 0x00000100: K/W [no page found].
page fault at 0x00000100: K/W [no page found].
page fault at 0x00000100: K/W [no page found].
page fault at 0x00000100: K/W [no page found].
page fault at 0x00000100: K/W [no page found].
page fault at 0x00000100: K/W [no page found].
page fault at 0x00000100: K/W [no page found].
page fault at 0x00000100: K/W [no page found].
page fault at 0x00000100: K/W [no page found].
page fault at 0x00000100: K/W [no page found].
```

## 练习1

> 完成do_pgfault（mm/vmm.c）函数，给未被映射的地址映射上物理页。
设置访问权限 的时候需要参考页面所在 VMA 的权限，
同时需要注意映射物理页时需要操作内存控制 结构所指定的页表，
而不是内核的页表。注意：在LAB2 EXERCISE 1处填写代码。执行

我添加的代码如下:
```
ptep = get_pte(mm->pgdir, addr, 1);
if (ptep == NULL) goto failed;
if (*ptep == 0) {
    //(2) if the phy addr isn't exist, then alloc a page & map the phy addr with logical addr
    struct Page *p = pgdir_alloc_page(mm->pgdir, addr, perm);
    if (p == NULL) goto failed;
    cprintf("map %08x -> %08x\n", addr, page2pa(p));
}
```

这里使用了函数 pgdir_alloc_page, 并且输出了map的情况, 这里我们打印我们的输出结果如下:

```
page fault at 0x00000100: K/W [no page found].
map 00000000 -> 00305000
check_pgfault() succeeded!
```

这里我为了测试连续多次页错误会出现什么情况, 我修改了`check_pgfault`的代码如下:

```
int i, sum = 0, stride = 200;
for (i = 0; i < 100; i ++) {
    *(char *)(addr + i*stride) = i;
    sum += i;
}
for (i = 0; i < 100; i ++) {
    sum -= *(char *)(addr + i*stride);
}
```

运行结果如下:

```
page fault at 0x00000100: K/W [no page found].
map 00000000 -> 00305000
page fault at 0x000010a0: K/W [no page found].
map 00001000 -> 00306000
page fault at 0x00002040: K/W [no page found].
map 00002000 -> 00307000
page fault at 0x000030a8: K/W [no page found].
map 00003000 -> 00308000
page fault at 0x00004048: K/W [no page found].
map 00004000 -> 00309000
```

可以发现, 200*100/4096 < 5, 所以一共发生了5次pgfault.

### 问题回答

*   请描述页目录项（Pag Director Entry）和页表（Page Table Entry）中组成部分对ucore实现页替换算法的潜在用处。

    答: 页目录项和页表项中的一些信息都可以帮助我们实现替换算法, 比如是否写了, 是否被访问

*   如果ucore的缺页服务例程在执行过程中访问内存，出现了页访问异常，请问硬件要做哪些事情？

    答: 和其他异常一样, 需要保存当前访问进程的现场，然后调用缺页中断处理服务，将控制权交给操作系统，根据中断号查找IDT，采取相应的策略处理，最后返回缺页服务例程处理缺页异常。

## 练习2

补充完成基于FIFO的页面替换算法

完成vmm.c中的do_pgfault函数，并且在实现FIFO算法的swap_fifo.c中完成map_swappable和swap_out_vistim函数。通过对swap的测试。

这里, 我先研究 `check_swap` 函数, 可以发现, 这个函数为我们构造了个可用页只有四页的环境,
然后调用`sm->check_swap();`, 然后在`_fifo_check_swap`, 一次对不同的页写, 然后通过
检查page fault的次数来确保fifo的正确性.

可以发现程序在第一次出现pagefault的时候在alloc新的page时又出现了一个pagefault.

如果要在ucore上实现"extended clock页替换算法"请给你的设计方案，现有的swap_manager框架是否足以支持在ucore中实现此算法？

答: 由于页表项已经提供了记录页的访问和修改信息的位，基本能够支持实现"extended clock页替换算法"的要求。只需要修改swapout，将替换方式改为循环遍历链表，沿途按照11到10、10到00、01到00的方式修改修改位和访问位，并将遇到的第一个00的页替换掉即可。需要被换出的页的特征是修改位和访问位均为0，在ucore中可借助页表项的PTE_A和PTE_D位来判断，找到第一个00的页时进行换入和换出操作

## 其他

与标准答案的区别:

加了大量的cprintf函数以理解整个运行过程.

修改了check函数做更全面的测试
