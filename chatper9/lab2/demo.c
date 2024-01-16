#include <linux/init.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#define MAX_THREAD_NUM 3
static struct task_struct *g_threads[MAX_THREAD_NUM];
static struct mutex g_mtx;
static int g_counter;

static void DoTask(void)
{
    if (g_counter > 10) {
        return;
    }
    pr_info("pid: %d, g_counter=%d\n", current->pid, ++g_counter);
}

static int ThreadProc(void *param)
{
    while (!kthread_should_stop()) {
        mutex_lock(&g_mtx);
        DoTask();
        mutex_unlock(&g_mtx);
        msleep(1000);
    }
    return 0;
}

static int __init demo_init(void)
{
    int i;
    mutex_init(&g_mtx);
    for (i = 0; i < MAX_THREAD_NUM; i++) {
        g_threads[i] = kthread_run(ThreadProc, NULL, "thread%d", i);
        if (IS_ERR(g_threads[i])) {
            pr_err("launch threads failed\n");
            return -1;
        }
    }
    pr_info("launch threads succeeded\n");
    return 0;
}

static void __exit demo_exit(void)
{
    int i;
    for (i = 0; i < MAX_THREAD_NUM; i++) {
        if (g_threads[i]) {
            kthread_stop(g_threads[i]);
        }
    }
    pr_info("stop threads succeeded\n");
}

module_init(demo_init);
module_exit(demo_exit);

MODULE_LICENSE("GPL");

////////////////////////////////////////////////////////////
