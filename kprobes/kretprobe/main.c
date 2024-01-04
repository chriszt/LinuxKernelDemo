#include <linux/init.h>
#include <linux/module.h>
#include <linux/kprobes.h>

static char g_func_name[NAME_MAX] = "kernel_clone";
module_param_string(func, g_func_name, NAME_MAX, S_IRUGO);
MODULE_PARM_DESC(func, "Function to kretprobe; this module will report the function's execution time");

struct my_data {
    ktime_t entry_stamp;
};

static int demo_entry_handler(struct kretprobe_instance *inst, struct pt_regs *regs)
{
    struct my_data *data;
    if (!current->mm) {
        return 1;
    }
    data = (struct my_data *)inst->data;
    data->entry_stamp = ktime_get();
    return 0;
}
NOKPROBE_SYMBOL(demo_entry_handler);

static int demo_ret_handler(struct kretprobe_instance *inst, struct pt_regs *regs)
{
    unsigned long retVal = regs_return_value(regs);
    struct my_data *data = (struct my_data *)inst->data;
    s64 delta;
    ktime_t now;

    now = ktime_get();
    delta = ktime_to_ns(ktime_sub(now, data->entry_stamp));
    pr_info("%s returned %lu and took %lld ns to execute\n", g_func_name, retVal, delta);
    return 0;
}
NOKPROBE_SYMBOL(demo_ret_handler);

static struct kretprobe g_kretprobe = {
    .kp.symbol_name = g_func_name,
    .entry_handler = demo_entry_handler,
    .handler = demo_ret_handler,
    .data_size = sizeof(struct my_data),
    .maxactive = 20
};

static int __init demo_init(void)
{
    int ret = register_kretprobe(&g_kretprobe);
    if (ret < 0) {
        pr_err("register_kretprobe failed, ret=%d\n", ret);
        return ret;
    }
    pr_info("planted return probe at %s: %p\n", g_kretprobe.kp.symbol_name, g_kretprobe.kp.addr);
    return 0;
}

static void __exit demo_exit(void)
{
    unregister_kretprobe(&g_kretprobe);
    pr_info("kretprobe at %p unregistered\n", g_kretprobe.kp.addr);
    pr_info("missed probing %d instances of %s\n", g_kretprobe.nmissed, g_kretprobe.kp.symbol_name);
}

module_init(demo_init);
module_exit(demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Chris ZT");
MODULE_DESCRIPTION("demo kernel module");
MODULE_ALIAS("demo");
