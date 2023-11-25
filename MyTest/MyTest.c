#include <linux/init.h>
#include <linux/module.h>

void Func1(void)
{
    printk("this is Func1()\n");
}

static int __init my_test_init(void)
{
    int i;
    Func1();
    printk("my first kernel module init\n");
    for (i = 0; i < 5; i++) {
        printk("init: i=%d\n", i);
    }
    printk("my first kernel module init finished\n");
    return 0;
}

void Func2(void)
{
    printk("this is Func2()\n");
}

static void __exit my_test_exit(void)
{
    int i;
    Func2();
    printk("good bye1\n");
    for (i = 0; i < 5; i++) {
        printk("exit: i=%d\n", i);
    }
    printk("good bye2\n");
}

module_init(my_test_init);
module_exit(my_test_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Chris ZT");
MODULE_DESCRIPTION("my test kernel module");
MODULE_ALIAS("mytest");
