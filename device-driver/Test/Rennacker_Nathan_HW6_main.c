/**************************************************************
 * Class:  CSC-415-03 Spring 2023
 * Name: Nathan Rennacker
 * Student ID: 921348958
 * GitHub ID: nlrennacker
 *
 * File: Rennacker_Nathan_HW6_main.c
 *
 * Description: Sample user application for testing encryption and decryption from a sample driver
 *
 **************************************************************/

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>

//Constants for passing magic numbers
#define IOCTL_SET_ENCRYPT _IOR(250, 0, int)
#define IOCTL_SET_DECRYPT _IOR(250, 1, int)

int main() {
    int fd;
    char msg[256];
    char buffer[256] = {0};

    printf("Enter a message to encrypt: ");
    fgets(msg, sizeof(msg), stdin);  // Get the message from the user
    msg[strcspn(msg, "\n")] = 0;     // Remove the newline character

    fd = open("/dev/testDriver", O_RDWR);
    if (fd < 0) {
        printf("Failed to open device.\n");
        return -1;
    }

    // Set device to encryption mode
    if (ioctl(fd, IOCTL_SET_ENCRYPT, NULL) < 0) {
        printf("Failed to set device to encryption mode.\n");
        return -1;
    }

    // Write a message to the device
    if (write(fd, msg, strlen(msg)) < 0) {
        printf("Failed to write to device.\n");
        return -1;
    }

    // Read the encrypted message back from the device
    if (read(fd, buffer, sizeof(buffer)) < 0) {
        printf("Failed to read from device.\n");
        return -1;
    }

    printf("Encrypted message: %s\n", buffer);

    // Set device to decryption mode
    if (ioctl(fd, IOCTL_SET_DECRYPT, NULL) < 0) {
        printf("Failed to set device to decryption mode.\n");
        return -1;
    }

    // Write the encrypted message to the device
    if (write(fd, buffer, strlen(buffer)) < 0) {
        printf("Failed to write to device.\n");
        return -1;
    }
    

    //Reuse buffer but clear space
    memset(buffer, 0, sizeof(buffer));

    // Read the decrypted message back from the device
    if (read(fd, buffer, sizeof(buffer)) < 0) {
        printf("Failed to read from device.\n");
        return -1;
    }

    printf("Decrypted message: %s\n", buffer);

    close(fd);

    return 0;
}