#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>

#define DEMO_MISC_DEVICE_NAME "demo_dev"

static int demo_open(struct inode *i, struct file *f)
{
    ktime_t now;
    now = ktime_get();
    printk("demo_open(), %lu, %llu\n", sizeof(ktime_t), now);
    return 0;
}

static int demo_release(struct inode *i, struct file *f)
{
    printk("demo_release()\n");
    return 0;
}

struct file_operations g_fops = {
    .owner = THIS_MODULE,
    .open = demo_open,
    .release = demo_release
};

struct miscdevice g_miscDev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEMO_MISC_DEVICE_NAME,
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
