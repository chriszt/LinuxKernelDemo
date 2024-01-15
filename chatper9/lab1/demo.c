#include <linux/init.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>
// #include <linux/miscdevice.h>
// #include <linux/slab.h>
// #include <linux/vmalloc.h>
// #include <linux/mm.h>
// #include <linux/sched.h>

static DEFINE_SPINLOCK(g_lck);
static struct task_struct *g_lockThreads[2];
// static struct page *myPage;
static int g_count = 0;

static int ThreadProc(void *param)
{
    while (!kthread_should_stop()) {
        msleep(10);
        spin_lock(&g_lck);
        if (g_count < 10) {
            g_count++;
            pr_info("pid=%d g_count=%d\n", current->pid, g_count);
        }
        spin_unlock(&g_lck);
    }
    return 0;
}

static int __init demo_init(void)
{
    g_lockThreads[0] = kthread_run(ThreadProc, NULL, "LockThread");
    g_lockThreads[1] = kthread_run(ThreadProc, NULL, "LockThread");
    if (IS_ERR(g_lockThreads[0]) || IS_ERR(g_lockThreads[1])) {
        pr_err("kthread_run failed\n");
        // return PTR_ERR(g_lockThread);
        return -1;
    }
    pr_info("kthread_run succeeded\n");
    
    return 0;
}

static void __exit demo_exit(void)
{
    // if (g_lockThread) {
    //     kthread_stop(g_lockThread);
    //     pr_info("kthread_stop succeeded\n");
    // }
    kthread_stop(g_lockThreads[0]);
    kthread_stop(g_lockThreads[1]);
}

module_init(demo_init);
module_exit(demo_exit);

MODULE_LICENSE("GPL");

////////////////////////////////////////////////////////////
