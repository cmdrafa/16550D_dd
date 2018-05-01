/*                                                     
 * $Id: echo.c,v 1.5 2004/10/26 03:32:21 corbet Exp $ 
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
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include "serial_reg.h"

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Rafael Kraemer");

dev_t uartdevice;

struct dev
{
    struct cdev cdev;
    char *data;
    char *devname;
    int cnt;
};

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

int uart_release(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "Device has been sucessfully closed\n");
    return 0;
}

ssize_t uart_write(struct file *filep, const char __user *buff, size_t count, loff_t *offp)
{
    unsigned long uncp; // Variable that stores the return from copy_from_user (uncopied data)
    int data_checker;   // Increment until end of char arr
    int to_write;       // data to write
    int to_write_bits;  // Data to write Bits
    char b_data;        // char that stores data to write (1 byte)
    int i;              // Array index of char data
    struct dev *uartdev = filep->private_data;

    data_checker = 0;
    to_write = 0;
    to_write_bits = 0;
    i = 0;

    uartdev->data = kmalloc(sizeof(char) * (count + 1), GFP_KERNEL);
    if (!uartdev->data)
    {
        printk(KERN_ERR "Error aloccating memory for data!!\n");
    }
    memset(uartdev->data, 0, sizeof(char) * (count + 1));

    uncp = copy_from_user(uartdev->data, buff, count);
    //printk(KERN_INFO "Kernel received: %s\n", uartdev->data);
    to_write = count - uncp;
    to_write_bits = to_write * 8;

    while (data_checker != to_write_bits)
    {

        if (inb((BASE + UART_LSR) & UART_LSR_THRE) == 0) // check for THRE emptyness
        {

            b_data = *(uartdev->data + i);
            printk(KERN_INFO "b_data: %c \n", b_data);

            outb(b_data, BASE + UART_TX); // write something to it
            i++;
            data_checker += 8;
        }
    }

    kfree(uartdev->data);

    if (uncp == 0)
    {
        uartdev->cnt += count;
        printk(KERN_INFO "Bytes Read %d\n", count);
        return count;
    }
    else
    {
        printk(KERN_ALERT "Unable to copy %lu bytes\n", uncp);
        return uncp;
    }
}

struct file_operations uart_fops = {
    .owner = THIS_MODULE,
    .llseek = no_llseek,
    .open = uart_open,
    //.read = uart_read,
    .write = uart_write,
    .release = uart_release,
};

struct dev *uartdev;

//initialize the UART with the following communication parameters:
//8-bit chars, 2 stop bits, parity even, and 1200 bps.
//Because, we will not use interrupts,
//make sure that the UART is configured not to generate interrupts
//send one character via the UART
static int uart_init(void)
{
    int ret, Major, Minor, reg, count;
    unsigned char lcr = 0;

    uartdev = kmalloc(sizeof(struct dev), GFP_KERNEL);
    if (!uartdev)
    {
        printk(KERN_ERR "Failed allocating memory por the device structure\n");
        return -1;
    }
    memset(uartdev, 0, sizeof(struct dev));
    uartdev->devname = "serp";

    if (!request_region(BASE, 8, uartdev->devname))
    {
        printk(KERN_ERR "Request_region failed !!\n");
        return -1;
    }

    outb(0, BASE + UART_IER); // Disable interrupt
    //lcr = UART_LCR_WLEN8 | UART_LCR_EPAR;
    outb(UART_LCR_WLEN8, BASE + UART_LCR); //Set len to 8
    outb(UART_LCR_EPAR, BASE + UART_LCR);  // Even parity
    outb(UART_LCR_DLAB, BASE + UART_LCR);  // Acess dlab
    outb(UART_DIV_1200, BASE + UART_DLL);  // 1200bps br
    outb(0, BASE + UART_DLM);              //
    outb(0, BASE + UART_LCR);              // set it to zero

    //  Allocate Major Numbers
    ret = alloc_chrdev_region(&uartdevice, 0, 1, uartdev->devname);
    if (ret < 0)
    {
        printk(KERN_ALERT "Major number allocation failed\n");
        return ret;
    }
    Major = MAJOR(uartdevice);
    Minor = MINOR(uartdevice);
    printk(KERN_INFO "Allocated Major number: %d\n", Major);

    //Register after allocation
    cdev_init(&uartdev->cdev, &uart_fops);
    uartdev->cdev.owner = THIS_MODULE;
    uartdev->cdev.ops = &uart_fops;

    reg = cdev_add(&uartdev->cdev, uartdevice, 1);
    if (reg < 0)
    {
        printk(KERN_ERR "Error in cdev_add\n");
    }

    return 0;
}

static void uart_exit(void)
{
    int Major;
    Major = MAJOR(uartdevice);
    cdev_del(&uartdev->cdev);
    kfree(uartdev);
    release_region(BASE, 8);
    unregister_chrdev_region(uartdevice, 1);
    printk(KERN_INFO "Major number: %d unloaded\n", Major);
}
module_init(uart_init);
module_exit(uart_exit);