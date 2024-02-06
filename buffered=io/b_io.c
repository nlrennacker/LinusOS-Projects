/**************************************************************
 * Class:  CSC-415-0# Spring 2023
 * Name: Nathan Rennacker
 * Student ID: 921348958
 * GitHub UserID: nlrennacker
 * Project: Assignment 5 – Buffered I/O
 *
 * File: b_io.c
 *
 * Description: This program functions as a buffered reader for input and output data
 * Its operation involves setting up an array for file control blocks and populating it with files.
 * Then, the software loads and reads the file into the buffer of the file control block.  It then 
 * transfers the data from the fcb buffer into the buffer of the user who called the software.
 * And after everything is completed it closes its resources.
 *
 **************************************************************/
#include "b_io.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fsLowSmall.h"

#define MAXFCBS 20  // The maximum number of files open at one time

// This structure is all the information needed to maintain an open file
// It contains a pointer to a fileInfo strucutre and any other information
// that you need to maintain your open file.
typedef struct b_fcb {
    fileInfo *fi;  // holds the low level systems file info
    char buffer[B_CHUNK_SIZE];
    int buffer_offset;
    int buffer_size;
    int current_position;
} b_fcb;

// static array of file control blocks
b_fcb fcbArray[MAXFCBS];

// Indicates that the file control block array has not been initialized
int startup = 0;

// Method to initialize our file system / file control blocks
// Anything else that needs one time initialization can go in this routine
void b_init() {
    if (startup)
        return;

    for (int i = 0; i < MAXFCBS; i++) {
        fcbArray[i].fi = NULL;
    }

    startup = 1;
}

// Method to get a free File Control Block FCB element
b_io_fd b_getFCB() {
    for (int i = 0; i < MAXFCBS; i++) {
        if (fcbArray[i].fi == NULL) {
            fcbArray[i].fi = (fileInfo *)-2;  // used but not assigned
            return i;                         // Not thread safe but okay for this project
        }
    }

    return (-1);
}

// b_open is called by the "user application" to open a file.  This routine is
// similar to the Linux open function.
// You will create your own file descriptor which is just an integer index into an
// array of file control blocks (fcbArray) that you maintain for each open file.
// For this assignment the flags will be read only and can be ignored.

/**
 * Opens the specified file and returns a file descriptor.
 *
 * @param filename - The name of the file to open.
 * @param flags - File access flags (read-only for this assignment).
 * @return The file descriptor if successful, or a negative value if an error occurs.
 */
b_io_fd b_open(char *filename, int flags) {
    if (startup == 0) b_init();

    // Get a free FCB
    b_io_fd fd = b_getFCB();
    if (fd < 0) {
        return -1;
    }

    // Get file info, if it's NULL make sure to return while also setting fcb fd to NULL
    fileInfo *file_info = GetFileInfo(filename);
    if (!file_info) {
        fcbArray[fd].fi = NULL;
        return -1;
    }

    // init file control block
    fcbArray[fd].fi = file_info;
    fcbArray[fd].buffer_offset = 0;
    fcbArray[fd].buffer_size = 0;
    fcbArray[fd].current_position = 0;

    return fd;
}

// b_read functions just like its Linux counterpart read.  The user passes in
// the file descriptor (index into fcbArray), a buffer where thay want you to 
// place the data, and a count of how many bytes they want from the file.
// The return value is the number of bytes you have copied into their buffer.
// The return value can never be greater then the requested count, but it can
// be less only when you have run out of bytes to read.  i.e. End of File

/**
 * Reads count bytes from the file associated with the given file descriptor.
 *
 * @param fd - The file descriptor of the file to read from.
 * @param buffer - The buffer to store the read data.
 * @param count - The number of bytes to read.
 * @return The number of bytes read or a negative value if an error occurs.
 */
int b_read(b_io_fd fd, char *buffer, int count) {
    // Write buffered read function to return the data and # bytes read
    // You must use LBAread and you must buffer the data in B_CHUNK_SIZE byte chunks.

    if (startup == 0) b_init();  // Initialize our system

    // check that fd is between 0 and (MAXFCBS-1)
    if ((fd < 0) || (fd >= MAXFCBS)) {
        return (-1);  // invalid file descriptor
    }

    // and check that the specified FCB is actually in use
    if (fcbArray[fd].fi == NULL) { // File not open for this descriptor
        return -1;
    }
    int bytes_read = 0;
    b_fcb *file = &fcbArray[fd];

    // While there are still bytes left to read and we haven't reached the end of the file
    while (bytes_read < count && file->current_position < file->fi->fileSize) {
        // If the buffer is empty, read the next chunk
        if (file->buffer_offset == file->buffer_size) {
            int remaining = file->fi->fileSize - file->current_position;
            int to_read = remaining < B_CHUNK_SIZE ? remaining : B_CHUNK_SIZE;
            // Read the next chunk into the buffer
            LBAread(file->buffer, to_read / B_CHUNK_SIZE, file->fi->location + (file->current_position / B_CHUNK_SIZE));
            file->buffer_offset = 0;
            file->buffer_size = to_read;
        }

        // Copy bytes from the internal buffer to the user's buffer
        int buffer_remaining = file->buffer_size - file->buffer_offset;
        int request_remaining = count - bytes_read;
        int to_copy = buffer_remaining < request_remaining ? buffer_remaining : request_remaining;

        // Copy the data to the user's buffer
        memcpy(buffer + bytes_read, file->buffer + file->buffer_offset, to_copy);
        bytes_read += to_copy;
        file->buffer_offset += to_copy;
        file->current_position += to_copy;
    }

    return bytes_read;
}


/**
 * frees and allocated memory and places the file control block back into the unused pool of file control blocks.
 *
 * @param fd - The file descriptor of the file to close.
 * @return 0 if successful, or a negative value if fd doesn't correspond to existing.
 */
int b_close(b_io_fd fd) {
    // Validate file descriptor
    if ((fcbArray[fd].fi == NULL)) {
        return -1;
    }

    fcbArray[fd].fi = NULL;
    return 0;
}