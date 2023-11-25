#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>

#define DEMO2_NAME "demo2_dev"
static dev_t g_dev;
static struct cdev *g_cdev;
static signed g_count = 1;

static int demo2_open(struct inode *inode, struct file *file)
{
    int major = MAJOR(inode->i_rdev);
    int minor = MINOR(inode->i_rdev);
    printk("[demo2] %s: major=%d, minor=%d\n", __func__, major, minor);
    return 0;
}

static int demo2_release(struct inode *inode, struct file *file)
{
    printk("[demo2] %s\n", __func__);
    return 0;
}

static ssize_t demo2_read(struct file *file, char __user *buf, size_t bufLen, loff_t *ppos)
{
    printk("[demo2] %s\n", __func__);
    return 0;
}

static ssize_t demo2_write(struct file *file, const char __user *buf, size_t bufLen, loff_t *fpos)
{
    printk("[demo2] %s\n", __func__);
    return 0;
}

static const struct file_operations g_fops = {
    .owner = THIS_MODULE,
    .open = demo2_open,
    .release = demo2_release,
    .read = demo2_read,
    .write = demo2_write
};

static int __init demo2_init(void)
{
    int ret;
    ret = alloc_chrdev_region(&g_dev, 0, g_count, DEMO2_NAME);
    if (ret) {
        printk("[demo2] failed to allocate char device region, ret=%d\n", ret);
        return ret;
    }
    g_cdev = cdev_alloc();
    if (!g_cdev) {

    }
    return 0;
}

static void __exit demo2_exit(void)
{
    if (g_cdev) {
        cdev_del(g_cdev);
    }
    unregister_chrdev_region(g_dev, g_count);
    printk("[demo2] demo2 exited\n");
}

module_init(demo2_init);
module_exit(demo2_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Chris ZT");
MODULE_DESCRIPTION("demo2 kernel module");
MODULE_ALIAS("demo2");
