#include <linux/init.h>
#include <linux/module.h>
#include <linux/kprobes.h>

#define MAX_SYMBOL_LEN 64
// static char g_symbol[MAX_SYMBOL_LEN] = "_do_fork";
static char g_symbol[MAX_SYMBOL_LEN] = "kernel_clone";
module_param_string(g_symbol, g_symbol, sizeof(g_symbol), 0644);

int demo_pre_handler(struct kprobe *p, struct pt_regs *regs)
{
    pr_info("<%s> pre_handler: p->addr=0x%p, regs->ip=0x%lx, regs->flags=0x%lx\n",
        p->symbol_name, p->addr, regs->ip, regs->flags);
    return 0;
}

void demo_post_handler(struct kprobe *p, struct pt_regs *regs, unsigned long flags)
{
    pr_info("<%s> post_handler: p->addr=0x%p, regs->ip=0x%lx, regs->flags=0x%lx, flags=0x%lx\n",
        p->symbol_name, p->addr, regs->ip, regs->flags, flags);
}

static struct kprobe g_kp = {
    .symbol_name = g_symbol,
    .pre_handler = demo_pre_handler,
    .post_handler = demo_post_handler
};

static int __init demo_init(void)
{
    int ret = register_kprobe(&g_kp);
    if (ret < 0) {
        pr_err("register_kprobe() failed, ret=%d\n", ret);
        return ret;
    }
    pr_info("register_kprobe() succeeded, planted kprobe at 0x%p\n", g_kp.addr);
    return 0;
}

static void __exit demo_exit(void)
{
    unregister_kprobe(&g_kp);
    pr_info("unregister_kprobe() succeeded, planted kprobe at 0x%p\n", g_kp.addr);
}

module_init(demo_init);
module_exit(demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Chris ZT");
MODULE_DESCRIPTION("demo kernel module");
MODULE_ALIAS("demo");
