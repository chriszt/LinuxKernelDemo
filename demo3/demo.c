#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>

#define DEMO_DEV_NAME "demo_dev"
static struct device *g_dev;

static int demo_open(struct inode *inode, struct file *file)
{
    int major = MAJOR(inode->i_rdev);
    int minor = MINOR(inode->i_rdev);
    printk("[demo] %s: major=%d, minor=%d\n", __func__, major, minor);
    return 0;
}

static int demo_release(struct inode *inode, struct file *file)
{
    printk("[demo] %s\n", __func__);
    return 0;
}

static ssize_t demo_read(struct file *file, char __user *buf, size_t bufLen, loff_t *ppos)
{
    printk("[demo] %s\n", __func__);
    return 0;
}

static ssize_t demo_write(struct file *file, const char __user *buf, size_t bufLen, loff_t *fpos)
{
    printk("[demo] %s\n", __func__);
    return 0;
}

static const struct file_operations g_fops = {
    .owner = THIS_MODULE,
    .open = demo_open,
    .release = demo_release,
    .read = demo_read,
    .write = demo_write
};

static struct miscdevice g_miscDev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEMO_DEV_NAME,
    .fops = &g_fops
};

static int __init demo_init(void)
{
    int ret;
    ret = misc_register(&g_miscDev);
    if (ret) {
        printk("[demo] failed register misc device: ret=%d\n", ret);
        return ret;
    }
    g_dev = g_miscDev.this_device;
    printk("[demo] succeeded register char device: %s\n", DEMO_DEV_NAME);
    return 0;
}

static void __exit demo_exit(void)
{
    misc_deregister(&g_miscDev);
    printk("[demo] removed char device\n");
}

module_init(demo_init);
module_exit(demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Chris ZT");
MODULE_DESCRIPTION("demo kernel module");
MODULE_ALIAS("demo");
