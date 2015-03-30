# lab2实验报告
### 2012011373 梁盾


##练习0
> 填写已有实验

通过meld merge工具实现

## 练习1

> 实现 first-fit 连续物理内存分配算法

首先可以发现有一个函数
```
static void
default_check(void)
```

这个函数负责检查我们程序的正确性,
而我们主要需要更改的函数就是

* `default_init_memmap`
* `default_alloc_pages`
* `default_free_pages`

为此我们首先需要了解 struct list, 他的主要函数有:

* list_init
* list_add(list_add_after)
* list_add_before
* list_del
* list_next
* list_prev

我们一个函数一个函数的解读

###default_init

这个函数用于初始化, 可以喊道他讲 free_list设置为空, nr_free 设置为 0, 所以我们要在后面
添加 free pages

### default_init_memmap

这个函数用于初始化, 在这里对于页管理的约定如下:

* `p->flags` 包含两个flags:
  * `PG_property` 1 表示这个page是free的, 同时是一段连续内存的头
  * `PG_reserved` 是否被kernel使用, 1表示已经被使用, 不能继续用
* `p->ref` 表示被谁使用, free的时候应当是0
* `p->property` 表示连续free内存的个数

观察对应代码

```
static void
default_init_memmap(struct Page *base, size_t n) {
    assert(n > 0);
    struct Page *p = base;
    for (; p != base + n; p ++) {
        assert(PageReserved(p));
        p->flags = p->property = 0;
        set_page_ref(p, 0);
    }
    base->property = n;
    SetPageProperty(base);
    nr_free += n;
    list_add(&free_list, &(base->page_link));
}
```
可以发现这段代码是符合要求的

### default_alloc_pages

查看代码
```
struct Page *page = NULL;
list_entry_t *le = &free_list;
while ((le = list_next(le)) != &free_list) {
    struct Page *p = le2page(le, page_link);
    if (p->property >= n) {
        page = p;
        break;
    }
}
```

可以发现这一段代码是查找第一个可用的page, 并没有
什么问题

我们看接下来的代码

```
list_del(&(page->page_link));
if (page->property > n) {
    struct Page *p = page + n;
    p->property = page->property - n;
    list_add(&free_list, &(p->page_link));
}
nr_free -= n;
ClearPageProperty(page);
```

这里就有问题了, 因为我们寻找方法是 *First-Fit Mem Alloc*
紧接在我们之后的page我们不能直接插入free_list,
应该插入当前page之后, 另一个问题就是
需要设置property, 所以代码应当改成这样:
```
if (page->property > n) {
    struct Page *p = page + n;
    p->property = page->property - n;
    SetPageProperty(p);
    list_add(&free_list, &(p->page_link));
}
list_del(&(page->page_link));
nr_free -= n;
ClearPageProperty(page);
```

### default_free_pages

观察此函数, 发现和`default_alloc_pages` 翻了同样的毛病,
就是插入到了表头, 而不是该插得位置, 这里最后的
插入代码我们修改如下:

```
list_entry_t *lef = &free_list;
while (le != &free_list) {
    p = le2page(le, page_link);
    le = list_next(le);
    if (p < base) lef = &(p->page_link);
    if (base + base->property == p) {
        base->property += p->property;
        ClearPageProperty(p);
        list_del(&(p->page_link));
    }
    else if (p + p->property == base) {
        p->property += base->property;
        ClearPageProperty(base);
        base = p;
        lef = p->page_link->prev;
        list_del(&(p->page_link));
    }
}
nr_free += n;
list_add(&lef, &(base->page_link));
```
我们添加了一个 lef 变量来帮助我们寻找
正确的插入位置.

这样, 我们就正确的通过了测试.

### 改进复杂度

我们可以吧alloc, free的复杂度都优化到
`O(logN)`, N 是当前不连续的页的个数,
使用红黑树可以帮助我们快速找到符合条件的块
(最优或最差匹配以块的大小作为键值), 然后再
多维护一个指针用来保存相邻快.

## 练习2

> 实现寻找虚拟地址对应的页表项

按照注释中的代码, 我们可以很轻松的完成这个练习

* 找到虚拟地址la对应的pde表的index；
* 判断该entry是否是符合要求的；
* 如果不是，则判断时候需要分配一个二级页表；
* 如果需要，这分配一个新的二级页表，并设置好相关参数并将内容清空；
* 最后返回pte表的一个条目。

代码如下:
```
pde_t *pdep = pgdir + PDX(la);   					// (1) find page directory entry
  if ((*pdep & PTE_P) == 0) {              			// (2) check if entry is not present
    if (create == 0) return NULL;                  	// (3) check if creating is needed, then alloc page for page table
    struct Page *page = alloc_page();               // CAUTION: this page is used for page table, not for common data page
    if (page == NULL) return NULL;
    set_page_ref(page, 1);							// (4) set page reference
    uintptr_t pa = page2pa(page); 					// (5) get linear address of page
    memset(KADDR(pa), 0, PGSIZE);                  	// (6) clear page content using memset
    *pdep = pa | PTE_P | PTE_W | PTE_U;        		// (7) set page directory entry's permission
  }
  return (pte_t*)KADDR(PDE_ADDR(*pdep)) + PTX(la);   // (8) return page table entry
```

### 练习3

这个练习是要修改pmm.c中的page_remove_pte函数，其功能是释放一个包含某虚地址的物理内存页。 首先需要让对应此物理内存页的管理数据结构Page做相关的清除处理，使得此物理内存页成为空闲；另外还需把表示虚地址与物理地址对应关系的二级页表项清除。 在注释中，很清晰的描述了该函数的工作流程：

* 判断该pte是否是符合要求的；
* 找到该pte对应的page；
* page->ref减1,如果减到0,这释放该page；
* flush tlb。


代码如下:
```
if ((*ptep & PTE_P) == 1) {                      	//(1) check if this page table entry is present
    struct Page *page = pte2page(*ptep); 			//(2) find corresponding page to pte
    page_ref_dec(page);                          	//(3) decrease page reference
    if (page->ref == 0) free_page(page);     		//(4) and free this page when page reference reachs 0
    *ptep = 0;                          			//(5) clear second page table entry
    tlb_invalidate(pgdir, la);               		//(6) flush tlb
}
```
