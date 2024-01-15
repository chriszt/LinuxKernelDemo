#include <linux/init.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/freezer.h>
#include <linux/delay.h>

static DEFINE_SPINLOCK(g_lck);
static struct task_struct *g_thread;
static struct page *g_page;

static int DoProc(void)
{
    int order = 5;

    spin_lock(&g_lck);
    g_page = alloc_pages(GFP_KERNEL, order);
    if (!g_page) {
        pr_err("allocate pages failed\n");
        return -ENOMEM;
    }
    spin_lock(&g_lck);
    msleep(10);
    __free_pages(g_page, order);
    spin_unlock(&g_lck);
    spin_unlock(&g_lck);

    return 0;
}

static int ThreadProc(void *param)
{
    set_freezable();
    set_user_nice(current, 0);

    while (!kthread_should_stop()) {
        msleep(10);
        DoProc();
    }
    return 0;
}

static int __init demo_init(void)
{
    g_thread = kthread_run(ThreadProc, NULL, "LockThread");
    if (IS_ERR(g_thread)) {
        pr_err("kthread_run failed\n");
        return PTR_ERR(g_thread);
    }
    pr_info("kthread_run succeeded\n");
    return 0;
}

static void __exit demo_exit(void)
{
    if (g_thread) {
        kthread_stop(g_thread);
        pr_info("kthread_stop succeeded\n");
    }
}

module_init(demo_init);
module_exit(demo_exit);

MODULE_LICENSE("GPL");

////////////////////////////////////////////////////////////
