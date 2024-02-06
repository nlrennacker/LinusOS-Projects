/**************************************************************
 * Class:  CSC-415-03 Spring 2023
 * Name: Nathan Rennacker
 * Student ID: 921348958
 * GitHub ID: nlrennacker
 *
 * File: testDriver.c
 *
 * Description: Sample linux kernel module which functions as a character device driver. 
 * It registers a device which can be written to or read from by user programs, 
 * and can also receive specific commands via ioctl calls.
 *
 **************************************************************/

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "testDriver"  // device will show up as dev/testDeviceDrive
#define MY_MAJOR 250
#define MY_MINOR 0
#define KEY 0xAB  // Simple XOR key for encryption and decryption.
#define BUFFER_SIZE 256
// constants for passing magic numbers
#define IOCTL_SET_ENCRYPT _IOR(MY_MAJOR, 0, int)
#define IOCTL_SET_DECRYPT _IOR(MY_MAJOR, 1, int)

// module metadata
MODULE_LICENSE("GPL");  // license type
MODULE_AUTHOR("Nathan");
MODULE_DESCRIPTION("A Sample Device Driver");

static int deviceCount;                  // Track how many times device is opened.
static bool isEncryptMode = true;        // Track current mode (encrypt or decrypt) default mode is TRUE
static char message[BUFFER_SIZE] = {0};  // Buffer to store message from user.
static int messageLength;                // length of message
struct cdev my_cDev;                     // Character device structure.

// Function prototypes
static int myOpen(struct inode *, struct file *);
static int myRelease(struct inode *, struct file *);
static ssize_t myRead(struct file *, char *, size_t, loff_t *);
static ssize_t myWrite(struct file *, const char *, size_t, loff_t *);
static long myIoCtl(struct file *fs, unsigned int command, unsigned long data);

// File operations structure, this defines which functions are called when a user program
// interacts with the device file
static struct file_operations fops = {
    .open = myOpen,
    .read = myRead,
    .write = myWrite,
    .release = myRelease,
    .unlocked_ioctl = myIoCtl,
    .owner = THIS_MODULE
};

// Module initialization function This is called when the module is loaded into the kernel
static int __init myInit(void) {
    int result, registers;
    dev_t devNo = MKDEV(MY_MAJOR, MY_MINOR);                    // Create device number using major and minor numbers
    registers = register_chrdev_region(devNo, 1, DEVICE_NAME);  // Register device region
    printk(KERN_INFO "Register chrdev succeeded 1: %d\n", registers);
    cdev_init(&my_cDev, &fops);   // initialize character device structure
    my_cDev.owner = THIS_MODULE;  // Set owner of the device

    // Add character device into the system
    result = cdev_add(&my_cDev, devNo, 1);
    return result;
}

// Module exit function this is called when the module is removed from the kernel
static void __exit myExit(void) {
    dev_t devNo = MKDEV(MY_MAJOR, MY_MINOR);  // Create device number using major and minor numbers.
    unregister_chrdev_region(devNo, 1);       // Unregister device region.
    cdev_del(&my_cDev);                       // Delete character device from the system.
    printk(KERN_INFO "Exiting module...\n");
}

static int myOpen(struct inode *inode, struct file *fs) {
    deviceCount++;
    printk(KERN_INFO "Device #%d is now opened\n", deviceCount);
    return 0;
}

static ssize_t myRead(struct file *fs, char *buffer, size_t len, loff_t *offset) {
    int error = 0;                                 // Error code.
    if (len > messageLength) len = messageLength;  // Set len to message length if it's greater than message length.
    // Copy the message to user buffer.
    error = copy_to_user(buffer, message, len);
    if (error == 0) {                                                           // If no error occurred.
        printk(KERN_INFO "Sent %zu characters to the user\n", len);             // Log the read operation.
        return len;                                                             // Return the length of data read.
    } else {                                                                    // If an error occurred.
        printk(KERN_INFO "Failed to send %d characters to the user\n", error);  // Log the error.
        return -EFAULT;                                                         // Return error code.
    }
}

static ssize_t myWrite(struct file *fs, const char *buffer, size_t len, loff_t *offset) {
    int i;
    // Set message length. If len is greater than BUFFER_SIZE, limit it to BUFFER_SIZE
    messageLength = len > BUFFER_SIZE ? BUFFER_SIZE : len;
    // Copy data from user buffer to the message buffer
    if (copy_from_user(message, buffer, messageLength)) {
        return -EFAULT;  // If error occurred while copying, return error code
    }

    // Perform XOR cipher encryption or decryption on the message
    for (i = 0; i < messageLength; i++) {
        message[i] ^= KEY;
    }

    printk(KERN_INFO "Received %zu characters from the user\n", len);  // Log the write operation
    return len;                                                        // Return the length of data written
}

static long myIoCtl(struct file *fs, unsigned int command, unsigned long data) {
    switch (command) {             // Switch based on the ioctl command received
        case IOCTL_SET_ENCRYPT:    // If command to set encryption mode
            isEncryptMode = true;  // Set mode to encryption
            break;
        case IOCTL_SET_DECRYPT:     // If command to set decryption mode
            isEncryptMode = false;  // Set mode to decryption
            break;
        default:             // If unknown command
            return -EINVAL;  // Return invalid argument error code
    }
    return 0;  // Return 0 to indicate successful ioctl operation
}

// called when the device file is closed
static int myRelease(struct inode *inode, struct file *fs) {
    deviceCount--;
    printk(KERN_INFO "Device successfully closed\n");
    return 0;
}

module_init(myInit);
module_exit(myExit);