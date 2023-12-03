#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/wait.h>
#include <linux/kfifo.h>
#include <linux/poll.h>

#define DEMO_NAME "my_demo_dev"
#define MAX_DEMO_DEVICES 8
#define MAX_BUFFER_SIZE 64
#define FIFO_SIZE 64

struct demo_device {
    char name[MAX_BUFFER_SIZE];
    struct device *dev;
    wait_queue_head_t readQueue;
    wait_queue_head_t writeQueue;
    struct kfifo fifo;
};

struct demo_private_data {
    struct demo_device *demoDev;
    char name[MAX_BUFFER_SIZE];
};

static dev_t g_dev;
static struct cdev *g_cdev;
static struct demo_device *g_demoDevs[MAX_DEMO_DEVICES];

static int demo_open(struct inode *inode, struct file *file)
{
    unsigned int minor = iminor(inode);
    struct demo_private_data *priData;
    struct demo_device *demoDev = g_demoDevs[minor];
    
    printk("[demo] %s, major=%d, minor=%d, demoDev=%s\n", __func__, MAJOR(inode->i_rdev), MINOR(inode->i_rdev), demoDev->name);

    priData = kmalloc(sizeof(struct demo_private_data), GFP_KERNEL);
    if (!priData) {
        return -ENOMEM;
    }

    sprintf(priData->name, "private_data_%d", minor);
    priData->demoDev = demoDev;

    file->private_data = priData;

    return 0;
}

static int demo_release(struct inode *inode, struct file *file)
{
    struct demo_private_data *priData = file->private_data;
    kfree(priData);
    printk("[demo] %s\n", __func__);
    return 0;
}

static ssize_t demo_read(struct file *file, char __user *buf, size_t count, loff_t *pos)
{
    struct demo_private_data *priData = file->private_data;
    struct demo_device *demoDev = priData->demoDev;
    int actualReaded;
    int ret;

    if (kfifo_is_empty(&demoDev->fifo)) {
        if (file->f_flags & O_NONBLOCK) {
            return -EAGAIN;
        }
        printk("[demo] %s:%s pid=%d, going to sleep, %s\n", __func__, demoDev->name, current->pid, priData->name);
        ret = wait_event_interruptible(demoDev->readQueue, !kfifo_is_empty(&demoDev->fifo));
        if (ret) {
            return ret;
        }
    }

    ret = kfifo_to_user(&demoDev->fifo, buf, count, &actualReaded);
    if (ret) {
        return -EIO;
    }

    if (!kfifo_is_full(&demoDev->fifo)) {
        wake_up_interruptible(&demoDev->writeQueue);
    }

    printk("[demo] %s:%s pid=%d, actualReaded=%d, pos=%lld\n", __func__, demoDev->name, current->pid, actualReaded, *pos);

    return actualReaded;
}

static ssize_t demo_write(struct file *file, const char __user *buf, size_t count, loff_t *pos)
{
    struct demo_private_data *priData = file->private_data;
    struct demo_device *demoDev = priData->demoDev;
    int actualWritten;
    int ret;

    if (kfifo_is_full(&demoDev->fifo)) {
        if (file->f_flags & O_NONBLOCK) {
            return -EAGAIN;
        }
        printk("[demo] %s:%s pid=%d, going to sleep, %s\n", __func__, demoDev->name, current->pid, priData->name);
        ret = wait_event_interruptible(demoDev->writeQueue, !kfifo_is_full(&demoDev->fifo));
        if (ret) {
            return ret;
        }
    }

    ret = kfifo_from_user(&demoDev->fifo, buf, count, &actualWritten);
    if (ret) {
        return -EIO;
    }

    if (!kfifo_is_empty(&demoDev->fifo)) {
        wake_up_interruptible(&demoDev->readQueue);
    }

    printk("[demo] %s:%s pid=%d, actualWritten=%d, pos=%lld\n", __func__, demoDev->name, current->pid, actualWritten, *pos);

    return actualWritten;
}

__poll_t demo_poll(struct file *file, struct poll_table_struct *wait)
{
    __poll_t mask = 0;
    struct demo_private_data *priData = file->private_data;
    struct demo_device *demoDev = priData->demoDev;

    poll_wait(file, &demoDev->readQueue, wait);
    poll_wait(file, &demoDev->writeQueue, wait);
    
    if (!kfifo_is_empty(&demoDev->fifo)) {
        mask |= POLLIN | POLLRDNORM;
    }
    if (!kfifo_is_full(&demoDev->fifo)) {
        mask |= POLLOUT | POLLWRNORM;
    }
    
    return mask;
}

static const struct file_operations g_fileOps = {
    .owner = THIS_MODULE,
    .open = demo_open,
    .release = demo_release,
    .read = demo_read,
    .write = demo_write,
    .poll = demo_poll
};

static int __init demo_init(void)
{
    int ret;
    int i;
    struct demo_device *demoDev;

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
        demoDev = kmalloc(sizeof(struct demo_device), GFP_KERNEL);
        if (!demoDev) {
            ret = -ENOMEM;
            goto FREE_DEVICE;
        }
        sprintf(demoDev->name, "%s%d", DEMO_NAME, i);
        init_waitqueue_head(&demoDev->readQueue);
        init_waitqueue_head(&demoDev->writeQueue);
        ret = kfifo_alloc(&demoDev->fifo, FIFO_SIZE, GFP_KERNEL);
        if (ret) {
            ret = -ENOMEM;
            goto FREE_KFIFO;
        }
        g_demoDevs[i] = demoDev;
        printk("[demo] device name=%s\n", demoDev->name);
    }

    printk("[demo] register char device succeeded: %s\n", DEMO_NAME);

    return 0;

FREE_KFIFO:
    for (i = 0; i < MAX_DEMO_DEVICES; i++) {
        if (&demoDev->fifo) {
            kfifo_free(&demoDev->fifo);
        }
    }

FREE_DEVICE:
    for (i = 0; i < MAX_DEMO_DEVICES; i++) {
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
    int i;
    for (i = 0; i < MAX_DEMO_DEVICES; i++) {
        if (g_demoDevs[i]) {
            if (&g_demoDevs[i]->fifo) {
                kfifo_free(&g_demoDevs[i]->fifo);
            }
            kfree(g_demoDevs[i]);
        }
    }
    cdev_del(g_cdev);
    unregister_chrdev_region(g_dev, MAX_DEMO_DEVICES);
    printk("[demo] unregister char device succeeded: %s\n", DEMO_NAME);
}

module_init(demo_init);
module_exit(demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Chris ZT");
MODULE_DESCRIPTION("demo kernel module");
MODULE_ALIAS("demo");
