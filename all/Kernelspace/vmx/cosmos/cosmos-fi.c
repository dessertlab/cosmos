#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/atomic.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/semaphore.h>
#include <linux/ktime.h>

#include "cosmos_lib.h"

#define DEVICE_NAME "cosmos_fi"
#define MAJOR_NUM 101

#define SIGNAL_SEM_DUMP 20
#define SET_DUMP_VMCS 22
#define CONTROL_EXEC 27
#define INJECT_OP 28

enum {
	CDEV_NOT_USED = 0,
	CDEV_EXCLUSIVE_OPEN = 1,
};

struct semaphore SEM_DUMP_WAIT_fi; 
uint8_t enable_dump_vmcs12_fi = 0; 
uint8_t control_execution = 0; 
vmexit* vmexit_to_inject; 

EXPORT_SYMBOL(SEM_DUMP_WAIT_fi);
EXPORT_SYMBOL(enable_dump_vmcs12_fi);
EXPORT_SYMBOL(control_execution);
EXPORT_SYMBOL(vmexit_to_inject);

static int open(struct inode *inode, struct file *file)
{
    sema_init(&SEM_DUMP_WAIT_fi, 0); 
    enable_dump_vmcs12_fi = 0; 

    printk(KERN_INFO "[COSMOS-Fi] Opened.\n");

    return 0;
}

static int release(struct inode *inode, struct file *file)
{
    enable_dump_vmcs12_fi = 0; 
    printk(KERN_INFO "[COSMOS-Fi] Released.\n");

    return 0;
}

static long int ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{

    unsigned long bytes_copied = 0; 

    switch (cmd) {

    case SET_DUMP_VMCS: 

            if (arg < 0)
                return -EINVAL; 

            enable_dump_vmcs12_fi = arg; 

            break; 

    case CONTROL_EXEC: 

        if (arg < 0)
            return -EINVAL; 

        control_execution = arg; 
        printk("[COSMOS-Fi] Control_execution set to %lu.\n", arg);

        break;

    case INJECT_OP: 

        vmexit_to_inject = kvmalloc(sizeof(vmexit), GFP_KERNEL);
        bytes_copied = copy_from_user(vmexit_to_inject, (const void __user *)arg, sizeof(vmexit));
        if (bytes_copied != 0) {
            printk("[COSMOS-Fi] Failing the copy_from_user of vmexit_to_inject, bytes: %zu.\n", bytes_copied);
            return -EFAULT;
        }

        up(&SEM_DUMP_WAIT_fi);

        break; 

    case SIGNAL_SEM_DUMP: 

            up(&SEM_DUMP_WAIT_fi);

            break; 

    default:

        printk("Invalid operation: %u.\n", cmd);
        
        break; 
    }

    return 0;
}

static struct file_operations cosmos_fi_fops = {
    .owner = THIS_MODULE,
    .open = open,
    .release = release,
    .unlocked_ioctl = ioctl,
};

static int __init cosmos_fi_init(void)
{
    // Register the character device
    if (register_chrdev(MAJOR_NUM, DEVICE_NAME, &cosmos_fi_fops) < 0) {
        return -EINVAL;
    }

    printk(KERN_INFO "COSMOS-Fi module initialized\n");
    return 0;
}

static void __exit cosmos_fi_exit(void)
{
    // unregister the character device
    unregister_chrdev(MAJOR_NUM, "COSMOS-Fi");
    printk(KERN_INFO "COSMOS-Fi module exited\n");
}

module_init(cosmos_fi_init);
module_exit(cosmos_fi_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Giuseppe De Rosa");
MODULE_DESCRIPTION("COSMOS-Fi for KVM");
