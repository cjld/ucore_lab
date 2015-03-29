Lab1 report

## 练习一

> 理解通过make生成执行文件的过程。（要求在报告中写出对下述问题的回答）

> 在此练习中，大家需要通过静态分析代码来了解：

> 1. 操作系统镜像文件ucore.img是如何一步一步生成的？(需要比较详细地解释Makefile中每一条相关命令和命令参数的含义，以及说明命令导致的结果)

我们可以再makefile里找到这么一段代码
```
bin/ucore.img
| $(UCOREIMG): $(kernel) $(bootblock)
|	$(V)dd if=/dev/zero of=$@ count=10000
|	$(V)dd if=$(bootblock) of=$@ conv=notrunc
|	$(V)dd if=$(kernel) of=$@ seek=1 conv=notrunc
```
这里我们可以看到dd指令, 在这个指令中, 首先对kernel写入了大量
的0, 然后本别写入了bootblock和kernel

然后就是 bootblock, 他是通过编译器生成的, 主要文件是 boot 文件
夹下的文件

然后就是kernal, kernel的对应源码在 `kern` 文件夹下

通过阅读 makefile, 让我们对lab的整个框架有了比较基础的了解

> 2. 一个被系统认为是符合规范的硬盘主引导扇区的特征是什么？

`sign.c` 文件里包含了判断是否是符合规范的硬盘主引导扇区的代码

首先是大小为512, 最后两位是 `0x55`和`0xAA`, 对于前面的数据
要求大小不超过510

## 练习2

> 使用qemu执行并调试lab1中的软件。（要求在报告中简要写出练习过程）

> 为了熟悉使用qemu和gdb进行的调试工作，我们进行如下的小练习：

我是用的工作环境是zylin+eclipse, 首先我们添加启动的command 如下:
```
target remote:1234
file /home/moocos/moocos/ucore_lab/labcodes_answer/lab1_result/obj/bootblock.o
break bootmain
```
然后是配置外部运行的gdb, 这里我们直接使用`make gdb` 指令


> 1. 从CPU加电后执行的第一条指令开始，单步跟踪BIOS的执行。

使用gdb调试, 成功输出了以下信息:
```
source .gdbinit
.gdbinit: No such file or directory.
target remote:1234
Remote debugging using :1234
0x0000fff0 in ?? ()
file /home/moocos/moocos/ucore_lab/labcodes_answer/lab1_result/obj/bootblock.o
Reading symbols from /home/moocos/moocos/ucore_lab/labcodes_answer/lab1_result/obj/bootblock.o...done.
```
> 1. 在初始化位置0x7c00设置实地址断点,测试断点正常。

gdb输出了如下信息, 代表我们断点设置正常
```
break bootmain
Breakpoint 1 at 0x7d00: file boot/bootmain.c, line 88.

c
Continuing.

Breakpoint 1, bootmain () at boot/bootmain.c:88
88	    readseg((uintptr_t)ELFHDR, SECTSIZE * 8, 0);
```
我们的编辑器也跳到了对应行的代码

> 1. 从0x7c00开始跟踪代码运行,将单步跟踪反汇编得到的代码与bootasm.S和 bootblock.asm进行比较。

使用gdb 指令 `disassemble 0x7c00, 0x7d00` 获得gdb的汇编信息如下:

```
Dump of assembler code from 0x7c00 to 0x7d00:
   0x00007c00 <start+0>:	cli
   0x00007c01 <start+1>:	cld
   0x00007c02 <start+2>:	xor    %eax,%eax
   0x00007c04 <start+4>:	mov    %eax,%ds
   0x00007c06 <start+6>:	mov    %eax,%es
   0x00007c08 <start+8>:	mov    %eax,%ss
   0x00007c0a <seta20.1+0>:	in     $0x64,%al
   ......
```

发现和 `bootasm.S` 一致

> 1. 自己找一个bootloader或内核中的代码位置，设置断点并进行测试。

通过设置断点在106行, 然后continue, 得到了一下结果
```
b 106
Breakpoint 2 at 0x7d55: file boot/bootmain.c, line 106.
c
c
Continuing.

Breakpoint 2, bootmain () at boot/bootmain.c:106
106	    ((void (*)(void))(ELFHDR->e_entry & 0xFFFFFF))();
```
这也就是我们bootload进入kernel的地方

##练习3

> 分析bootloader进入保护模式的过程

> * 为何开启A20，以及如何开启A20

开启之前首先等待8042键盘控制器不忙, 然后通过
`outb` 指令打开A20

> * 如何初始化GDT表

一个简单的GDT表和其描述符已经静态储存在引导区中，载入即可

	    lgdt gdtdesc

> * 如何使能和进入保护模式

1. 设置cr0寄存器PE位置
2. 设置段寄存器
3. 建立堆栈

## 练习4

> 分析bootloader加载ELF格式的OS的过程

> 通过阅读bootmain.c，了解bootloader如何加载ELF文件。通过分析源代码和通过qemu来运行并调试bootloader&OS，

> bootloader如何读取硬盘扇区的？

在这段代码中, 包含了读取到正确位置的代码
```
for (; ph < eph; ph ++) {
    readseg(ph->p_va & 0xFFFFFF, ph->p_memsz, ph->p_offset);
}
```


> bootloader是如何加载ELF格式的OS？

通过这条代码启动kernel
```
((void (*)(void))(ELFHDR->e_entry & 0xFFFFFF))();
```

# 练习5

> 实现函数调用堆栈跟踪函数

这里我的代码同时输出了当前栈帧的大小
```
uint32_t ebp = read_ebp(), eip = read_eip();

int i, j;
for (i = 0; ebp != 0 && i < STACKFRAME_DEPTH; i ++) {
    cprintf("ebp:0x%08x eip:0x%08x frame_size: 0x%08x args:", ebp, eip, ((uint32_t *)ebp)[0] - ebp);
    uint32_t *args = (uint32_t *)ebp + 2;
    for (j = 0; j < 4; j ++) {
        cprintf("0x%08x ", args[j]);
    }
    cprintf("\n");
    print_debuginfo(eip - 1);
    eip = ((uint32_t *)ebp)[1];
    ebp = ((uint32_t *)ebp)[0];
}
```

输出结果如下

```
ebp:0x00007b28 eip:0x00100998 frame_size: 0x00000010 args:0x00010094 0x00010094 0x00007b58 0x00100079
    kern/debug/kdebug.c:305: print_stackframe+21
ebp:0x00007b38 eip:0x00100c93 frame_size: 0x00000020 args:0x00000000 0x00000000 0x00000000 0x00007ba8
    kern/debug/kmonitor.c:125: mon_backtrace+10
ebp:0x00007b58 eip:0x00100079 frame_size: 0x00000020 args:0x00000000 0x00007b80 0xffff0000 0x00007b84
    kern/init/init.c:50: grade_backtrace2+33
ebp:0x00007b78 eip:0x001000a2 frame_size: 0x00000020 args:0x00000000 0xffff0000 0x00007ba4 0x00000029
    kern/init/init.c:55: grade_backtrace1+38
ebp:0x00007b98 eip:0x001000c0 frame_size: 0x00000020 args:0x00000000 0x00100000 0xffff0000 0x0000001d
    kern/init/init.c:60: grade_backtrace0+23
ebp:0x00007bb8 eip:0x001000e5 frame_size: 0x00000030 args:0x0010357c 0x00103560 0x0000136a 0x00000000
    kern/init/init.c:65: grade_backtrace+34
ebp:0x00007be8 eip:0x00100055 frame_size: 0x00000010 args:0x00000000 0x00000000 0x00000000 0x00007c4f
    kern/init/init.c:28: kern_init+84
ebp:0x00007bf8 eip:0x00007d64 frame_size: 0xffff8408 args:0xc031fcfa 0xc08ed88e 0x64e4d08e 0xfa7502a8
    <unknow>: -- 0x00007d63 --
```

练习6：完善中断初始化和处理

首先是初始化中断向量表, 使用setgate设置, 代码如下:
注意到这里面的__vectors是在另一个汇编程序中实现的.

```
extern uintptr_t __vectors[];
int i;
for (i = 0; i < sizeof(idt) / sizeof(struct gatedesc); i ++)
    SETGATE(idt[i], 0, GD_KTEXT, __vectors[i], DPL_KERNEL);

SETGATE(idt[T_SWITCH_TOK], 0, GD_KTEXT, __vectors[T_SWITCH_TOK], DPL_USER);

lidt(&idt_pd);
```

然后就是在trap_dispatch中处理我们的中断了.

添加的代码如下
```
ticks ++;
 if (ticks >= TICK_NUM) {
   ticks = 0;
   print_ticks();
 }
```

> 中断向量表一个项占多少字节？其哪几位代表中断处理码的入口？

中断向量表一个项共8个字节，其中入口包括16－31位的段选择子、48-63位的段内偏移高16位，
0－15位的段内偏移低16位。
