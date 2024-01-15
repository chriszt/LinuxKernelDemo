#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>

static DEFINE_SPINLOCK(g_lck);
static struct page *g_page;

static int __init demo_init(void)
{
    spin_lock(&g_lck);

    g_page = alloc_pages(GFP_KERNEL, 5);
    if (!g_page) {
        spin_unlock(&g_lck);
        pr_err("alloc page failed\n");
        return -ENOMEM;
    }
    msleep(10000);

    spin_unlock(&g_lck);

    return 0;
}

static void __exit demo_exit(void)
{
    __free_pages(g_page, 5);
}

module_init(demo_init);
module_exit(demo_exit);

MODULE_LICENSE("GPL");

////////////////////////////////////////////////////////////
