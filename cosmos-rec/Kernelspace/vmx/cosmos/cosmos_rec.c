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

#define DEVICE_NAME "cosmos-rec"
#define MAJOR_NUM 100

#define SET_BUFFER_SIZE 1
#define RESET 2
#define TRACING_CONTROL 3
#define SET_FROM_EXIT 8
#define GET_COUNT_EXIT 19
#define SET_COUNT_EXIT 20
#define SET_DUMP_VMCS 22
#define CREATE_EFF_REC_BUFF 23
#define READ_EFF_REC_BUFF 24

#define KILL_GUEST_L2 29

enum {
	CDEV_NOT_USED = 0,
	CDEV_EXCLUSIVE_OPEN = 1,
};

/* RECORDING */
vmexit* buffer;
size_t exit_counter = 0;
uint8_t enable_tracing = 0;  
size_t buffer_size = BUFFER_SIZE;
size_t from_vmexit = 0; 
size_t this_exit_rec = 0; 
uint8_t is_L1 = 0; 
uint8_t enable_dump_vmcs12_rec = 0; 
struct semaphore SEM_DUMP_WAIT_rec; 
ktime_t rec_start = 0, rec_end = 0;
ktime_t* rec_cum_time; // Cumulative time buffer
uint8_t efficiency_rec = 0; 

EXPORT_SYMBOL(buffer);
EXPORT_SYMBOL(buffer_size);
EXPORT_SYMBOL(exit_counter);
EXPORT_SYMBOL(enable_tracing);
EXPORT_SYMBOL(from_vmexit);  
EXPORT_SYMBOL(this_exit_rec);
EXPORT_SYMBOL(is_L1);
EXPORT_SYMBOL(enable_dump_vmcs12_rec); 
EXPORT_SYMBOL(SEM_DUMP_WAIT_rec);
EXPORT_SYMBOL(rec_start); 
EXPORT_SYMBOL(rec_end); 
EXPORT_SYMBOL(rec_cum_time); 
EXPORT_SYMBOL(efficiency_rec); 

static ssize_t buffer_read(struct file *filp, char __user *usr_buffer, size_t count, loff_t *f_pos)
{
    size_t bytes_to_copy;

    // Calculate the number of bytes to copy based on exit_counter and count
    bytes_to_copy = min(count * sizeof(vmexit), sizeof(vmexit) * buffer_size);

    // Copy the contents of the buffer to the user space
    if (copy_to_user(usr_buffer, buffer, bytes_to_copy) != 0)
        return -EFAULT;  // Error copying data to user space

    printk(KERN_INFO "[COSMOS-REC] Buffer copied\n");

    return bytes_to_copy;
}

static int open(struct inode *inode, struct file *file)
{
    // Allocate memory for the buffer
    buffer = kvmalloc(((sizeof(vmexit) * BUFFER_SIZE + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE, GFP_KERNEL);
    if (!buffer)
        return -ENOMEM;

    exit_counter = 0; 
    enable_tracing = 0; 
    buffer_size = BUFFER_SIZE; 
    from_vmexit = 0; 
    this_exit_rec = 0;
    is_L1 = 0; 
    enable_dump_vmcs12_rec = 0; 
    sema_init(&SEM_DUMP_WAIT_rec, 0); 
    rec_start = ktime_set(0, 0);
    rec_end = ktime_set(0, 0);

    printk(KERN_INFO "[COSMOS-REC] Buffer created\n");

    return 0;
}

static int release(struct inode *inode, struct file *file)
{
    kvfree(buffer);
    if (from_vmexit > 1)
        kvfree(exit_before);

    if (efficiency_rec) { 
        kvfree (rec_cum_time); 
        efficiency_rec = 0; 
    }

    enable_tracing = 0; 
    buffer_size = BUFFER_SIZE; 
    from_vmexit = 0; 
    this_exit_rec = 0; 
    is_L1 = 0; 
    enable_dump_vmcs12_rec = 0; 

    printk(KERN_INFO "[COSMOS-REC] Buffer released\n");

    return 0;
}

void reset (void) {
    exit_counter = 0;

    printk(KERN_INFO "[COSMOS-REC] Buffer reset\n");
}

void tracing(uint8_t c) {
	enable_tracing = c;
} 

static long int ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{

    ssize_t ret;
    unsigned long bytes_copied = 0; 

    switch (cmd) {

        // SET_BUFFER_SIZE
        case SET_BUFFER_SIZE:
            if (arg <= 0)
                return -EINVAL;
 
            if (buffer != NULL)
                kvfree(buffer);  // Free existing buffer

            // Allocate memory for the new buffer
            buffer = kvmalloc(((sizeof(vmexit) * arg + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE, GFP_KERNEL);
            if (!buffer)
                return -ENOMEM;

            if (efficiency_rec) {
                kvfree(rec_cum_time);
                rec_cum_time = kvmalloc(((sizeof(ktime_t) * arg + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE, GFP_KERNEL);
                if (!rec_cum_time)
                    return -ENOMEM;
            }

            buffer_size = arg;
            reset();

	        printk(KERN_INFO "[COSMOS-REC] Buffer size changed\n");

            break;

        // RESET
        case RESET:
            reset();
            break;

        // RECORDING / TRACING CONTROL
        case TRACING_CONTROL: 
            if (arg < 0)
                return -EINVAL; 

            tracing(arg); 

            if (enable_tracing == 1)
                printk(KERN_INFO "[COSMOS-REC] tracing enabled\n");
            else
                printk(KERN_INFO "[COSMOS-REC] tracing disabled\n");  
            break; 

        case SET_FROM_EXIT: 
            if (arg < 0)
                return -EINVAL;

            from_vmexit = arg;
            /* exit_before = kvmalloc(((sizeof(Exit_Reasons_count) * 
                NUM_EXIT_REASONS + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE, GFP_KERNEL);
            init_hashmap(exit_before); */

            printk("[COSMOS-REC] From_exit set to %zu and exit_before buffer created.\n", from_vmexit);

            break; 

        case GET_COUNT_EXIT:  

            if (!can_get_count_exit)
                return -1;
            
            if(copy_to_user((void __user *)arg, &count_exit, sizeof(size_t)) != 0)
                printk("[COSMOS-REC] Error copying count_exit.\n");

            can_get_count_exit = 0; 

            break;

        case SET_COUNT_EXIT:

            if (arg < 0)
                return -EINVAL; 

            count_exit = arg; 

            break; 

        case SET_DUMP_VMCS: 

            if (arg < 0)
                return -EINVAL; 

            enable_dump_vmcs12_rec = arg; 

            break; 

        case CREATE_EFF_REC_BUFF: 

            rec_cum_time = kvmalloc(((sizeof(ktime_t) * buffer_size + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE, GFP_KERNEL);
            if (!rec_cum_time)
                return -ENOMEM;

            efficiency_rec = 1; 

            break; 

        case READ_EFF_REC_BUFF:

            int64_t* rec_cum_time_ns;

            if (efficiency_rec != 1) return -1; 

            rec_cum_time_ns = kvmalloc(((sizeof(int64_t) * buffer_size + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE, GFP_KERNEL);
            for (size_t m = 0; m < buffer_size; m++) {
                rec_cum_time_ns[m] = ktime_to_ns(rec_cum_time[m]);
            }

            bytes_copied = copy_to_user((void __user *)arg, rec_cum_time_ns, buffer_size*sizeof(int64_t));

            // Copy the contents of the buffer to the user space
            if (bytes_copied != 0)
                return -EFAULT;  // Error copying data to user space

            printk("[COSMOS-REC] rec_cum_time_ns recorded copied %lu\n", bytes_copied);

            efficiency_rec = 0;

            break; 

        case KILL_GUEST_L2: 

            if (arg < 0) return -EINVAL; 

            kill_guest_l2 = arg; 

            break; 

        default:

            printk("Invalid operation: %u.\n", cmd);
            
            break; 
        }

    return 0;
}

static struct file_operations buffer_fops = {
    .owner = THIS_MODULE,
    .read = buffer_read,
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

    printk(KERN_INFO "COSMOS-REC module initialized\n");
    return 0;
}

static void __exit buffer_exit(void)
{
    // unregister the character device
    unregister_chrdev(MAJOR_NUM, "cosmos_rec");
    printk(KERN_INFO "COSMOS-REC module exited\n");
}

module_init(buffer_init);
module_exit(buffer_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Giuseppe De Rosa");
MODULE_DESCRIPTION("COSMOS-REC for KVM");
