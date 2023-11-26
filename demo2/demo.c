#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>

#define DEMO_NAME "my_demo_dev"
static dev_t g_dev;
static struct cdev *g_cdev;
static signed g_count = 1;

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

static int __init demo_init(void)
{
    int ret;
    ret = alloc_chrdev_region(&g_dev, 0, g_count, DEMO_NAME);
    if (ret) {
        printk("[demo] failed to allocate char device region, ret=%d\n", ret);
        return ret;
    }
    g_cdev = cdev_alloc();
    if (!g_cdev) {
        printk("[demo] cdev_alloc failed\n");
        goto UNREGISTER_CHRDEV;
    }
    cdev_init(g_cdev, &g_fops);
    ret = cdev_add(g_cdev, g_dev, g_count);
    if (ret) {
        printk("[demo] cdev_add failed\n");
        goto CDEV_FAILED;
    }
    printk("[demo] succeeded register char device: %s\n", DEMO_NAME);
    printk("[demo] Major number=%d, Minor number=%d\n", MAJOR(g_dev), MINOR(g_dev));
    return 0;

CDEV_FAILED:
    cdev_del(g_cdev);
UNREGISTER_CHRDEV:
    unregister_chrdev_region(g_dev, g_count);
    return ret;    
}

static void __exit demo_exit(void)
{
    if (g_cdev) {
        cdev_del(g_cdev);
    }
    unregister_chrdev_region(g_dev, g_count);
    printk("[demo] demo exited\n");
}

module_init(demo_init);
module_exit(demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Chris ZT");
MODULE_DESCRIPTION("demo kernel module");
MODULE_ALIAS("demo");
