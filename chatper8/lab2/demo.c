#include <linux/init.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>

static struct task_struct *g_tsks[NR_CPUS];

static void ShowRegs(void)
{

}

static void ShowPrio(void)
{
    struct task_struct *task = current;
    printk("%s pid:%d, nice:%d prio:%d static_prio:%d normal_prio:%d\n",
        task->comm, task->pid, PRIO_TO_NICE(task->static_prio), task->prio, task->static_prio, task->normal_prio);
}

static void PrintCpu(const char *msg)
{
    preempt_disable();
    pr_info("%s cpu=%d\n", msg, smp_processor_id());
    preempt_enable();
}

static int ThreadProc(void *param)
{
    pr_info("thread pid=%d, mm=0x%p\n", current->pid, current->mm);
    do {
        PrintCpu("SLEEP in Thread Proc");
        msleep_interruptible(2000);
        PrintCpu("msleep over in Thread Proc");
        ShowRegs();
        ShowPrio();
    } while (!kthread_should_stop());
    return 0;
}

static int __init demo_init(void)
{
    int i;
    PrintCpu("loading module");
    for_each_online_cpu(i) {
        g_tsks[i] = kthread_create(ThreadProc, NULL, "thread%d", i); 
        if (!g_tsks[i]) {
            pr_err("create g_tsks[%d] failed\n", i);
            return -1;
        }
        kthread_bind(g_tsks[i], i);  // bind thread to cpu[i]
        pr_info("about to wake up and run the thread for cpu=%d\n", i);
        wake_up_process(g_tsks[i]);  // new threads begin to run
        pr_info("starting thread for cpu=%d\n", i);
        PrintCpu("on");
    }
    return 0;
}

static void __exit demo_exit(void)
{
    int i;
    for_each_online_cpu(i) {
        pr_info("kill thread %d\n", i);
        kthread_stop(g_tsks[i]);
        PrintCpu("kill was done on");
    }
}

module_init(demo_init);
module_exit(demo_exit);
MODULE_LICENSE("GPL");
