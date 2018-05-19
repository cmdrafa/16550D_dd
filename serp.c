/*                                                     
 * $Id: serp.c
 */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h>   /* kmalloc() */
#include <linux/fs.h>     /* everything... */
#include <linux/errno.h>  /* error codes */
#include <linux/types.h>  /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h> /* O_ACCMODE */
#include <linux/aio.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <asm/semaphore.h>

#include "serial_reg.h"

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Rafael Kraemer");

struct dev
{
    struct cdev cdev;
    struct semaphore sem;
    char *data;
    char *devname;
    int cnt;
    int timer_state;
    dev_t uartdevice;
};

struct dev *uartdev; // Pointer to the structure

static struct timer_list read_timer; // The timer used

void configure_uart_device(void); // Prototype

int uart_open(struct inode *inodep, struct file *filep)
{
    int ret;
    struct dev *uartdev;

    uartdev = container_of(inodep->i_cdev, struct dev, cdev);
    filep->private_data = uartdev;

    ret = nonseekable_open(inodep, filep);
    printk(KERN_INFO "Device has been opened\n");

    return 0;
}

// The timer callback function
void timer_callback(unsigned long data)
{
    uartdev->timer_state = 1;
}

int uart_release(struct inode *inodep, struct file *filep)
{
    int ret;

    // Delete timer
    ret = del_timer(&read_timer);

    printk(KERN_INFO "Device has been sucessfully closed\n");
    return 0;
}

ssize_t uart_read(struct file *filep, char __user *buff, size_t count, loff_t *offp)
{
    unsigned long uncp;
    int i;
    int ret;
    unsigned char escape = 0;
    int data_read = 0;
    struct dev *uartdev = filep->private_data;

    if (down_interruptible(&uartdev->sem))
    {
        return -ERESTARTSYS;
    }

    i = 0;

    // Allocate memory for the incoming data
    uartdev->data = kmalloc(sizeof(char) * (count + 1), GFP_KERNEL);
    if (!uartdev->data)
    {
        printk(KERN_ERR "Error allocating memory for the read operation!!\n");
        return -1;
    }
    memset(uartdev->data, 0, sizeof(char) * (count + 1));

    // While timer has not ellapsed or user has not pressed enter
    while (uartdev->timer_state != 1 && escape != 10)
    {
        if (!(inb(BASE + UART_LSR) & UART_LSR_DR))
        {
            set_current_state(TASK_INTERRUPTIBLE);
            schedule_timeout(1);
        }
        else
        {
            *(uartdev->data + i) = inb(BASE + UART_RX);
            ret = mod_timer(&read_timer, jiffies + msecs_to_jiffies(2000)); // Start Timer
            escape = *(uartdev->data + i);
            i++;
            data_read++;
            if (data_read == count) // User_space buffer
            {
                printk(KERN_INFO "Userspace buffer full, returning only %d bytes to userspace\n", count);
                break;
            }
            msleep_interruptible(1);
        }
    }
    uartdev->timer_state = 0;

    // Send data do the user
    uncp = copy_to_user(buff, uartdev->data, count);

    kfree(uartdev->data);

    if (uncp == 0)
    {
        up(&uartdev->sem);
        if (data_read > 0)
        {
            printk(KERN_INFO "Bytes sent to user %d\n", data_read);
            return data_read;
        }
        else
        {
            return 0;
        }
    }
    // Check the better error message to send
    else
    {
        up(&uartdev->sem);
        return -uncp;
    }
}

ssize_t uart_write(struct file *filep, const char __user *buff, size_t count, loff_t *offp)
{
    unsigned long uncp; // Variable that stores the return from copy_from_user (uncopied data)
    int data_checker;   // Increment until end of char arr
    int to_write;       // data to write
    char b_data;        // char that stores data to write (1 byte)
    int i;              // Array index of char data
    struct dev *uartdev = filep->private_data;

    if (down_interruptible(&uartdev->sem))
    {
        return -ERESTARTSYS;
    }

    data_checker = 0;
    to_write = 0;
    i = 0;

    uartdev->data = kmalloc(sizeof(char) * (count + 1), GFP_KERNEL);
    if (!uartdev->data)
    {
        printk(KERN_ERR "Error aloccating memory for write operation!!\n");
        return -1;
    }
    memset(uartdev->data, 0, sizeof(char) * (count + 1));

    uncp = copy_from_user(uartdev->data, buff, count);
    to_write = count - uncp;

    while (data_checker != to_write)
    {
        if ((inb(BASE + UART_LSR) & UART_LSR_THRE) != 0) // check for THRE emptyness
        {
            b_data = *(uartdev->data + i);
            outb(b_data, BASE + UART_TX); // write something to it
            i++;
            data_checker++;
        }
        else
        {
            set_current_state(TASK_INTERRUPTIBLE);
            schedule_timeout(1);
        }
    }

    kfree(uartdev->data);

    if (uncp == 0)
    {
        uartdev->cnt += count;
        up(&uartdev->sem);
        printk(KERN_INFO "Bytes written %d\n", count);
        return count;
    }
    else // Check for better error condition
    {
        up(&uartdev->sem);
        printk(KERN_ALERT "Unable to copy %lu bytes\n", uncp);
        return -uncp;
    }
}

struct file_operations uart_fops = {
    .owner = THIS_MODULE,
    .llseek = no_llseek,
    .open = uart_open,
    .read = uart_read,
    .write = uart_write,
    .release = uart_release,
};

static int uart_init(void)
{
    int ret, Major, Minor, reg;

    // Allocate memory for the structure
    uartdev = kmalloc(sizeof(struct dev), GFP_KERNEL);
    if (!uartdev)
    {
        printk(KERN_ERR "Failed allocating memory por the device structure\n");
        return -1;
    }
    memset(uartdev, 0, sizeof(struct dev));
    uartdev->devname = "serp";

    // Initialize the semaphore
    init_MUTEX(&uartdev->sem);

    // Request the region - Start: 0x3F8 End: 0x3FF
    if (!request_region(BASE, 8, uartdev->devname))
    {
        printk(KERN_ERR "Request_region failed !!\n");
        return -1;
    }

    // Configure serial port with defaul parameters
    configure_uart_device();

    //  Allocate Major Numbers

    ret = alloc_chrdev_region(&uartdev->uartdevice, 0, 1, uartdev->devname);
    if (ret < 0)
    {
        printk(KERN_ALERT "Major number allocation failed\n");
        return ret;
    }
    Major = MAJOR(uartdev->uartdevice);
    Minor = MINOR(uartdev->uartdevice);
    printk(KERN_INFO "Allocated Major number: %d\n", Major);

    //Register after allocation
    cdev_init(&uartdev->cdev, &uart_fops);
    uartdev->cdev.owner = THIS_MODULE;
    uartdev->cdev.ops = &uart_fops;

    reg = cdev_add(&uartdev->cdev, uartdev->uartdevice, 1);
    if (reg < 0)
    {
        printk(KERN_ERR "Error in cdev_add\n");
    }

    setup_timer(&read_timer, timer_callback, 0);
    uartdev->timer_state = 0;

    return 0;
}

void configure_uart_device()
{
    unsigned char lcr = 0;

    outb(0, BASE + UART_IER);                             // Disable interrupts
    lcr = UART_LCR_WLEN8 | UART_LCR_EPAR | UART_LCR_STOP; //Set len to 8, Even parity and 2 stop bits
    outb(lcr, BASE + UART_LCR);
    lcr |= UART_LCR_DLAB;                 // Select d_dlab
    outb(lcr, BASE + UART_LCR);           // Acess dlab
    outb(UART_DIV_1200, BASE + UART_DLL); // 1200bps br
    outb(0, BASE + UART_DLM);             //
    lcr &= ~UART_LCR_DLAB;
    outb(lcr, BASE + UART_LCR); // set it to zero
}

static void uart_exit(void)
{
    int Major;

    Major = MAJOR(uartdev->uartdevice);
    cdev_del(&uartdev->cdev);
    unregister_chrdev_region(uartdev->uartdevice, 1);
    kfree(uartdev);
    release_region(BASE, 8);

    printk(KERN_INFO "Major number: %d unloaded\n", Major);
}

module_init(uart_init);
module_exit(uart_exit);
