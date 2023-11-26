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

static int __init demo_init(void)
{
    dprintk("[demo] init\n");
    dprintk("[demo] module param=%d\n", mytest);
    return 0;
}

static void __exit demo_exit(void)
{
    printk("[demo] goog bye!\n");
}

module_init(demo_init);
module_exit(demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Chris ZT");
MODULE_DESCRIPTION("demo kernel module");
MODULE_ALIAS("demo");
