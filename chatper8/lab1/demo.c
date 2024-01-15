#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/sched.h>

#define DEMO_DEVICE_NAME "demo_dev"

////////////////////////////////////////////////////////////

#define PRT(a, b) pr_info("%-15s=%10d %10ld %8ld\n", \
    (a), (b), (PAGE_SIZE * (b)) / 1024, (PAGE_SIZE * (b)) / 1024 / 1024)

static void PhyPageTest(void)
{
    unsigned long i, pfn, valid = 0;
    unsigned long numPhysPages;
    int free = 0, locked = 0, reserved = 0, swapCache = 0, referenced = 0, active = 0,
        slab = 0, private = 0, uptodate = 0, dirty = 0, writeBack = 0, mappedToDisk = 0;
    struct page *pg;

    numPhysPages = get_num_physpages();
    for (i = 0; i < numPhysPages; i++) {
        pfn = i + 0;  // ARCH_PFN_OFFSET not exist in x86 arch
        if (pfn_valid(pfn) == 0) {
            continue;
        }
        valid++;
        pg = pfn_to_page(pfn);
        if (pg == NULL) {
            continue;
        }
        // printk("_count=%d, _mapcount=%d\n", page_count(pg), page_mapcount(pg));
        if (page_count(pg) == 0) {
            free++;
            continue;
        }
        if (PageLocked(pg)) {
            locked++;
        }
        if (PageReserved(pg)) {
            reserved++;
        }
        if (PageSwapCache(pg)) {
            swapCache++;
        }
        if (PageReferenced(pg)) {
            referenced++;
        }
        if (PageActive(pg)) {
            active++;
        }
        if (PageSlab(pg)) {
            slab++;
        }
        if (PagePrivate(pg)) {
            private++;
        }
        if (PageUptodate(pg)) {
            uptodate++;
        }
        if (PageDirty(pg)) {
            dirty++;
        }
        if (PageWriteback(pg)) {
            writeBack++;
        }
        if (PageMappedToDisk(pg)) {
            mappedToDisk++;
        }
    }
    pr_info("examining %ld pages (get_num_physpages) = %ld MB",
        numPhysPages, numPhysPages * PAGE_SIZE / 1024 / 1024);
    pr_info("pages with valid PFN's=%ld, = %ld MB",
        valid, valid * PAGE_SIZE / 1024 / 1024);
    PRT("free", free);
    PRT("locked", locked);
    PRT("reserved", reserved);
    PRT("swapCache", swapCache);
    PRT("referenced", referenced);
    PRT("active", active);
    PRT("slab", slab);
    PRT("private", private);
    PRT("uptodate", uptodate);
    PRT("dirty", dirty);
    PRT("writeBack", writeBack);
    PRT("mappedToDisk", mappedToDisk);
}

////////////////////////////////////////////////////////////

static void AllocMemoryTest(void)
{
    unsigned long order, size;
    char *buf;
    const unsigned long MB = 1024 * 1024;
    const unsigned long mem = 64;

    
    // try __get_free_pages()
    for (order = 0, size = PAGE_SIZE; order < MAX_ORDER; order++, size *= 2) {
        pr_info("order=%2lu, pages=%5lu, size=%8lu\n", order, size / PAGE_SIZE, size);
        buf = (char *)__get_free_pages(GFP_ATOMIC, order);
        if (!buf) {
            pr_err("... __get_free_pages failed\n");
            break;
        }
        pr_info("... __get_free_pages OK, buf=0x%p\n", buf);
        free_pages((unsigned long)buf, order);
    }

    // try kmalloc
    for (order = 0, size = PAGE_SIZE; order < MAX_ORDER; order++, size *= 2) {
        pr_info("order=%2lu, pages=%5lu, size=%8lu\n", order, size / PAGE_SIZE, size);
        buf = kmalloc(size, GFP_ATOMIC);
        if (!buf) {
            pr_err("... kmalloc failed\n");
            break;
        }
        pr_info("... kmalloc OK, buf=0x%p\n", buf);
        kfree(buf);
    }

    // try vmalloc
    for (size = 4 * MB; size <= mem * MB; size += 4 * MB) {
        pr_info("pages=%6lu, size=%8lu\n", size / PAGE_SIZE, size / MB);
        buf = vmalloc(size);
        if (!buf) {
            pr_err("... vmalloc failed\n");
            break;
        }
        pr_info("... vmalloc OK, buf=0x%p\n", buf);
        vfree(buf);
    }
}

////////////////////////////////////////////////////////////

static void SlabTest(void)
{
    int size = 20;
    struct kmem_cache *myCache;
    char *buf;

    if (size > KMALLOC_MAX_SIZE) {  // 4MB
        pr_info("size=%d is too large, you can NOT have more than %lu\n",
            size, KMALLOC_MAX_SIZE);
        return;
    }
    
    myCache = kmem_cache_create("MyCache", size, 0, SLAB_HWCACHE_ALIGN, NULL);
    if (!myCache) {
        pr_err("kmem_cache_create failed\n");
        return; // return -ENOMEM;
    }
    pr_info("kmem_cache_create succeeded\n");

    buf = kmem_cache_alloc(myCache, GFP_ATOMIC);
    if (!buf) {
        pr_err("kmem_cache_alloc failed\n");
        kmem_cache_destroy(myCache);
        return;
    }
    pr_err("kmem_cache_alloc succeeded\n");

    if (buf) {
        kmem_cache_free(myCache, buf);
        pr_info("kmem_cache_free succeeded\n");
    }

    if (myCache) {
        kmem_cache_destroy(myCache);
        pr_info("kmem_cache_destroy succeeded\n");
    }
}

////////////////////////////////////////////////////////////

static void PrintIt(struct task_struct *tsk)
{
    struct mm_struct *mm;
    struct vm_area_struct *vma;
    int j = 0;
    unsigned long start, end, length;
    mm = tsk->mm;
    pr_info("mm_struct addr=0x%p\n", mm);
    vma = mm->mmap;
    down_read(&mm->mmap_lock);
    pr_info("vmas:    vma    start    end    length\n");
    while (vma) {
        j++;
        start = vma->vm_start;
        end = vma->vm_end;
        length = end - start;
        pr_info("%6d: %16p %12lx %12lx    %8ld\n", j, vma, start, end, length);
        vma = vma->vm_next;
    }
    up_read(&mm->mmap_lock);
}

static void VmaTest(void)
{
    struct task_struct *tsk;
    int pid = 1;
    if (pid == 0) {
        tsk = current;
        pid = current->pid;
        pr_info("using current process\n");
    } else {
        tsk = pid_task(find_vpid(pid), PIDTYPE_PID);
    }
    if (!tsk) {
        pr_err("can NOT find pid=%d\n", pid);
        return;
    }
    pr_info("examing vma's for pid=%d, command=%s\n", pid, tsk->comm);
    PrintIt(tsk);
}

////////////////////////////////////////////////////////////

static int demo_open(struct inode *i, struct file *f)
{
    // PhyPageTest();
    // AllocMemoryTest();
    // SlabTest();
    VmaTest();
    return 0;
}

static int demo_release(struct inode *i, struct file *f)
{
    return 0;
}

struct file_operations g_fops = {
    .owner = THIS_MODULE,
    .open = demo_open,
    .release = demo_release
};

struct miscdevice g_miscDev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEMO_DEVICE_NAME,
    .fops = &g_fops
};

static int __init demo_init(void)
{
    int ret = misc_register(&g_miscDev);
    if (ret) {
        printk("[demo] misc_register() failed(%d)\n", ret);
        return ret;
    }
    printk("[demo] misc_register() succeeded");
    return 0;
}

static void __exit demo_exit(void)
{
    misc_deregister(&g_miscDev);
    printk("[demo] misc_deregister() succeeded");
}

module_init(demo_init);
module_exit(demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Chris ZT");
MODULE_DESCRIPTION("demo kernel module");
MODULE_ALIAS("demo");

////////////////////////////////////////////////////////////
