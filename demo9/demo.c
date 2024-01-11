#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/mm.h>

#define DEMO_DEVICE_NAME "demo_dev"

#define PRT(a, b) pr_info("%-15s=%10d %10ld %8ld\n", \
    (a), (b), (PAGE_SIZE * (b)) / 1024, (PAGE_SIZE * (b)) / 1024 / 1024)

static void ShowPhyMemory(void)
{
    unsigned long i, pfn, valid = 0;
    unsigned long numPhysPages;
    int free = 0, locked = 0, reserved = 0, swapCache = 0, referenced = 0, active = 0,
        slab = 0, private = 0, uptodate = 0, dirty = 0, writeBack = 0, mappedToDisk = 0;
    struct page *pg;
    // int ret;

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

static int demo_open(struct inode *i, struct file *f)
{
    ShowPhyMemory();
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
