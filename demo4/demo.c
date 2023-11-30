#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>

#define DEMO_NAME "my_demo_dev"
#define MAX_DEVICE_BUFFER_SIZE 64
static struct device *g_dev;
static char g_devBuf[MAX_DEVICE_BUFFER_SIZE];

static int demo_open(struct inode *inode, struct file *file)
{
    int major = MAJOR(inode->i_rdev);
    int minor = MINOR(inode->i_rdev);
    printk("[demo] %s, major=%d, minor=%d\n", __func__, major, minor);
    return 0;
}

static int demo_release(struct inode *inode, struct file *file)
{
    printk("[demo] %s\n", __func__);
    return 0;
}

static ssize_t demo_read(struct file *file, char __user *buf, size_t count, loff_t *pos)
{
    int actualReaded;
    int ret;
    int maxFree = MAX_DEVICE_BUFFER_SIZE - *pos;
    int needRead = min(maxFree, (int)count);
    if (needRead == 0) {
        dev_warn(g_dev, "[demo] no space for read");
    }
    ret = copy_to_user(buf, g_devBuf + *pos, needRead);
    if (ret == needRead) {
        return -EFAULT;
    }
    actualReaded = needRead - ret;
    *pos += actualReaded;
    printk("[demo] %s, actualReaded=%d, pos=%lld\n", __func__, actualReaded, *pos);
    return actualReaded;
}

static ssize_t demo_write(struct file *file, const char __user *buf, size_t count, loff_t *pos)
{
    int actualWrited;
    int ret;
    int maxFree = MAX_DEVICE_BUFFER_SIZE - *pos;
    int needWrite = min(maxFree, (int)count);
    if (needWrite == 0) {
        dev_warn(g_dev, "[demo] no space for read");
    }
    ret = copy_from_user(g_devBuf + *pos, buf, needWrite);
    if (ret == needWrite) {
        return -EFAULT;
    }
    actualWrited = needWrite - ret;
    *pos += actualWrited;
    printk("[demo] %s, actualWrited=%d, pos=%lld\n", __func__, actualWrited, *pos);
    return actualWrited;
}

static const struct file_operations g_fileOps = {
    .owner = THIS_MODULE,
    .open = demo_open,
    .release = demo_release,
    .read = demo_read,
    .write = demo_write
};

static struct miscdevice g_miscDev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEMO_NAME,
    .fops = &g_fileOps
};

static int __init demo_init(void)
{
    int ret = misc_register(&g_miscDev);
    if (ret < 0) {
        printk("[demo] register misc device failed(%d)\n", ret);
        return ret;
    }
    g_dev = g_miscDev.this_device;
    printk("[demo] register misc device succeeded: %s\n", DEMO_NAME);
    return 0;
}

static void __exit demo_exit(void)
{
    misc_deregister(&g_miscDev);
    printk("[demo] deregister misc device succeeded\n");
}

module_init(demo_init);
module_exit(demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Chris ZT");
MODULE_DESCRIPTION("demo kernel module");
MODULE_ALIAS("demo");
