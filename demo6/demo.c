#include <linux/init.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/kfifo.h>
#include <linux/wait.h>

#define DEMO_NAME "my_demo_dev"
#define MAX_DEVICE_BUFFER_SIZE 64

DEFINE_KFIFO(g_kfifo, char, MAX_DEVICE_BUFFER_SIZE);

static wait_queue_head_t g_readQueue;
static wait_queue_head_t g_writeQueue;

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
    if (kfifo_is_empty(&g_kfifo)) {
        if (file->f_flags & O_NONBLOCK) {
            return -EAGAIN;
        }
        printk("[demo] %s: pid=%d going to sleep\n", __func__, current->pid);
        ret = wait_event_interruptible(g_readQueue, !kfifo_is_empty(&g_kfifo));
        if (ret) {
            return ret;
        }
    }
    ret = kfifo_to_user(&g_kfifo, buf, count, &actualReaded);
    if (ret) {
        return -EIO;
    }
    if (!kfifo_is_full(&g_kfifo)) {
        wake_up_interruptible(&g_writeQueue);
    }
    printk("[demo] %s, actualReaded=%d, pos=%lld\n", __func__, actualReaded, *pos);
    return actualReaded;
}

static ssize_t demo_write(struct file *file, const char __user *buf, size_t count, loff_t *pos)
{
    int actualWrited;
    int ret;
    if (kfifo_is_full(&g_kfifo)) {
        if (file->f_flags & O_NONBLOCK) {
            return -EAGAIN;
        }
        printk("[demo] %s: pid=%d going to sleep\n", __func__, current->pid);
        ret = wait_event_interruptible(g_writeQueue, !kfifo_is_full(&g_kfifo));
        if (ret) {
            return ret;
        }
    }
    ret = kfifo_from_user(&g_kfifo, buf, count, &actualWrited);
    if (ret) {
        return -EIO;
    }
    if (!kfifo_is_empty(&g_kfifo)) {
        wake_up_interruptible(&g_readQueue);
    }
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
    init_waitqueue_head(&g_readQueue);
    init_waitqueue_head(&g_writeQueue);
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
