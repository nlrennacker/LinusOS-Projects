/**************************************************************
 * Class:  CSC-415-03 Fall 2023
 * Name: Nathan Rennacker
 * Student ID: 921348958
 * GitHub UserID: nlrennacker
 * Project: Assignment 2 – Stucture in Memory and Buffering
 *
 * File: Rennacker_Nathan_HW2_main.c
 *
 * Description: This assignment's purpose is to work with a structure defined in a header file
 * that contains varied information: from student info (name, ID, gradeLevel, etc), to a message
 * that needs to be populated with information from the command line. Then a series of strings
 * need to be moved from calling getNext() on a char * string to a buffer, which needs to be 
 * committed to a different, unknown data structure while ensuring that the buffer doesn't overflow
 * and we are committing exactly 256 bytes each time (except for the last commit)
 *
 **************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assignment2.h"

int main(int argc, char *argv[]) {
    // initial struct allocation, no data populated
    struct personalInfo *myInfo = malloc(sizeof *myInfo);

    // first and last name take first two arguments from the command line to populate data fields
    myInfo->firstName = argv[1];
    myInfo->lastName = argv[2];
    myInfo->studentID = 921348958;
    myInfo->level = SENIOR;

    // using or operators to populate language field since there will be no overlapping bits and
    // resulting binary addition is easy
    myInfo->languages = KNOWLEDGE_OF_C | KNOWLEDGE_OF_JAVA | KNOWLEDGE_OF_JAVASCRIPT | KNOWLEDGE_OF_PYTHON | KNOWLEDGE_OF_CPLUSPLUS | KNOWLEDGE_OF_SQL | KNOWLEDGE_OF_HTML | KNOWLEDGE_OF_ARM_ASSEMBLER;

    // using sizeof to know how much to copy into the message -- 1 char = 1 byte
    // checking for length of the 4th argument, if its less than the size of message we only
    // want to copy enough so that the entire string is put in, otherwise we'll be stepping out of
    // string bounds
    if (strlen(argv[3]) < sizeof(myInfo->message)) {
        strncpy(myInfo->message, argv[3], strlen(argv[3]));
    } else {
        strncpy(myInfo->message, argv[3], sizeof(myInfo->message));
    }

    // writePersonalInfo returns 0 if it succeeds
    if (writePersonalInfo(myInfo) != 0) {
        printf("Writing personal info failed! \n");
    }

    const char *next = getNext();
    char *bufferBlock = malloc(BLOCK_SIZE);
    char *temp;

    int length = 0;
    int bytePos = 0;

    // loop while there are still strings to pull from Lincoln's famous speech -- in this case
    // (or whatever large string getNext() is pulling from -- in other cases)
    while (next != NULL) {
        length = strlen(next);
        int spaceLeft = BLOCK_SIZE - bytePos;

        // memcpy if length isnt larger than space left in buffer
        // else memcpy first part, commit the full buffer, then memcpy second half
        if (length <= spaceLeft) {
            memcpy(bufferBlock + bytePos, next, length);
            bytePos += length;
        } else {
            memcpy(bufferBlock + bytePos, next, spaceLeft);
            commitBlock(bufferBlock);
            memcpy(bufferBlock, next + spaceLeft, length - spaceLeft);
            bytePos = length - spaceLeft;
        }

        next = getNext();

        // FIRST ITERATION NESTED LOOP

        // length is variable depending of the size of string from getNext()
        // so loop through, copying it over to bufferblock, but if the end of bufferBlock is reached
        // commit it and start copying from 0 again
        //  for (int i = 0; i < length; i++) {
        //      bufferBlock[bytePos++] = next[i];

        //     // reached end of bufferBlock size: commit the block, reset the pos
        //     if (bytePos == 256) {
        //         commitBlock(bufferBlock);
        //         bytePos = 0;
        //     }
        // }
    }

    // if there's anything left in the bufferBlock -> commit
    if (strlen(bufferBlock) != 0) {
        commitBlock(bufferBlock);
    }

    int check = checkIt();

    // making sure to free malloc mem and set pointers to null;
    free(myInfo);
    free(bufferBlock);

    myInfo = NULL;
    bufferBlock = NULL;

    return check;
}