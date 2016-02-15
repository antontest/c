#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

static int init_hello_4(void)
{
    //printk(KERN_ALERT "Hello, world 4 ");
    printk("in module");  
    return 0;
}

static void cleanup_hello_4(void)
{
    //printk(KERN_ALERT "Goodbye, world 4 ");
    printk("out module");
}

module_init(init_hello_4);
module_exit(cleanup_hello_4);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("antonio");
MODULE_DESCRIPTION("Test only!");
