#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/wait.h>
#include <linux/kfifo.h>

#define DEMO_NAME "my_demo_dev"
#define MAX_DEMO_DEVICES 8
#define MAX_BUFFER_SIZE 64

struct demo_device {
    char name[MAX_BUFFER_SIZE];
    struct device *dev;
    wait_queue_head_t readQueue;
    wait_queue_head_t writeQueue;
    struct kfifo fifo;
};

struct demo_private_data {
    struct demo_device *device;
    char name[MAX_BUFFER_SIZE];
};

static dev_t g_dev;
static struct cdev *g_cdev;
static struct demo_device *g_demoDevs[MAX_DEMO_DEVICES];

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
    return 0;
}

static ssize_t demo_write(struct file *file, const char __user *buf, size_t count, loff_t *pos)
{
    return 0;
}

static const struct file_operations g_fileOps = {
    .owner = THIS_MODULE,
    .open = demo_open,
    .release = demo_release,
    .read = demo_read,
    .write = demo_write
};

static int __init demo_init(void)
{
    int ret;
    int i;
    struct demo_device *device;

    ret = alloc_chrdev_region(&g_dev, 0, MAX_DEMO_DEVICES, DEMO_NAME);
    if (ret < 0) {
        printk("[demo] alloc_chrdev_region() failed\n");
        return ret;
    }
    printk("[demo] alloc_chrdev_region() succeeded\n");

    g_cdev = cdev_alloc();
    if (!g_cdev) {
        printk("[demo] cdev_alloc() failed\n");
        ret = -ENOMEM;
        goto UNGISTER_CHRDEV;
    }
    printk("[demo] cdev_alloc() succeeded\n");

    cdev_init(g_cdev, &g_fileOps);

    ret = cdev_add(g_cdev, g_dev, MAX_DEMO_DEVICES);
    if (ret < 0) {
        printk("[demo] cdev_add() failed\n");
        goto CDEV_FAIL;
    }
    printk("[demo] cdev_add() succeeded\n");

    for (i = 0; i < MAX_DEMO_DEVICES; i++) {
        device = kmalloc(sizeof(struct demo_device), GFP_KERNEL);
        if (!device) {
            ret = -ENOMEM;
            goto FREE_DEVICE;
        }
        sprintf(device->name, "%s%d", DEMO_NAME, i);
        g_demoDevs[i] = device;
    }


    return 0;

FREE_DEVICE:
    for (i = 0; i < MAX_DEMO_DEVICES, i++) {
        if (g_demoDevs[i]) {
            kfree(g_demoDevs[i]);
        }
    }

CDEV_FAIL:
    cdev_del(g_cdev);
UNGISTER_CHRDEV:
    unregister_chrdev_region(g_dev, MAX_DEMO_DEVICES);
    return ret;
}

static void __exit demo_exit(void)
{

}

module_init(demo_init);
module_exit(demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Chris ZT");
MODULE_DESCRIPTION("demo kernel module");
MODULE_ALIAS("demo");
