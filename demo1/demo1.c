#include <linux/init.h>
#include <linux/module.h>

static int debug = 1;
module_param(debug, int, 0644);
MODULE_PARM_DESC(debug, "Enable Debuging Infomation");

#define dprintk(args ...) \
    if (debug) { \
        printk(KERN_DEBUG args); \
    }

static int mytest = 100;
module_param(mytest, int, 0644);
MODULE_PARM_DESC(mytest, "Test for Module Parameter");

static int __init demo1_init(void)
{
    dprintk("[demo1] init\n");
    dprintk("[demo1] module param=%d\n", mytest);
    return 0;
}

static void __exit demo1_exit(void)
{
    printk("[demo1] goog bye!\n");
}

module_init(demo1_init);
module_exit(demo1_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Chris ZT");
MODULE_DESCRIPTION("demo1 kernel module");
MODULE_ALIAS("demo1");
