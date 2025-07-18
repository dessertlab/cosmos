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

#include "cosmos_lib.h"

#define DEVICE_NAME "cosmos_utilities"
#define MAJOR_NUM 103

/* Not used features*/
#define TEST_GUEST_ERROR 18
#define GET_GUEST_ERROR 19
/* Only used feature */
#define DUMP_VMCS12 20

enum GuestError {
    Success,
    GuestInvalidState, // nested_vmx_check_guest_state
    GuestInvalidControls, // nested_vmx_check_controls
    GuestInvalidAddressSpaceSize, // nested_vmx_check_address_space_size
    GuestInvalidHostState, // nested_vmx_check_host_state
    GuestInvalidExecutionControls // nested_check_vm_execution_controls
};
static enum GuestError guestError;
extern int nested_vmx_check_guestError(vmexit* exit);
extern int nested_vmx_dump_vmcs12(dump_vmcs12_t* d_vmcs12);

static int open(struct inode *inode, struct file *file)
{
    guestError = 0;

    printk(KERN_INFO "[COSMOS UT] Opened\n");

    return 0;
}

static int release(struct inode *inode, struct file *file)
{
    guestError = 0;

    printk(KERN_INFO "[COSMOS UT] Released\n");

    return 0;
}

static long int ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{

    ssize_t ret;
    unsigned long bytes_copied = 0; 

    switch (cmd) {

    case TEST_GUEST_ERROR:

        vmexit* test_exit = kmalloc(sizeof(vmexit), GFP_KERNEL); 

        ret = copy_from_user(test_exit, (const void __user *)arg, sizeof(vmexit));
        if (ret != 0) {
            printk("[COSMOS UT] Failing the copy_from_user of fuzzing test_exit.\n");
            return -EFAULT;
        }

        guestError = nested_vmx_check_guestError(test_exit);

        kfree(test_exit);

        break; 

    case GET_GUEST_ERROR: 

        bytes_copied = copy_to_user((void __user *)arg, &guestError, sizeof(enum GuestError));
        if (bytes_copied != 0) {
            printk(KERN_INFO "[COSMOS UT] error copying guestError %d. Bytes copied: %lu\n", guestError, bytes_copied);
            return -EFAULT;  // Error copying data to user space
        }

        break; 

    case DUMP_VMCS12:

        dump_vmcs12_t* d_vmcs12 = kvmalloc(sizeof(dump_vmcs12_t), GFP_KERNEL);

        int res = nested_vmx_dump_vmcs12(d_vmcs12);
        if (res) {
            bytes_copied = copy_to_user((void __user *)arg, d_vmcs12, sizeof(dump_vmcs12_t));
            kvfree(d_vmcs12);
            if (bytes_copied != 0) {
                printk(KERN_INFO "[COSMOS UT] error copying dump_vmcs12. Bytes copied: %lu\n", bytes_copied);     
                return -EFAULT;  // Error copying data to user space
            }
            return 1; 
        }
        else {
            kvfree(d_vmcs12);
            return 0; 
        }
    
        break; 

    default:

        printk("[COSMOS UT] Invalid operation.\n");
        
        break; 
    }

    return 0;
}

static struct file_operations buffer_fops = {
    .owner = THIS_MODULE,
    .open = open,
    .release = release,
    .unlocked_ioctl = ioctl,  // Support for IOCTL commands
};

static int __init buffer_init(void)
{
    // Register the character device
    if (register_chrdev(MAJOR_NUM, DEVICE_NAME, &buffer_fops) < 0) {
        return -EINVAL;
    }

    printk(KERN_INFO "COSMOS UT module initialized\n");
    return 0;
}

static void __exit buffer_exit(void)
{
    // unregister the character device
    unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
    printk(KERN_INFO "COSMOS UT module exited\n");
}

module_init(buffer_init);
module_exit(buffer_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Giuseppe De Rosa");
MODULE_DESCRIPTION("COSMOS UT for KVM");
