/**************************************************************
 * Class:  CSC-415-03 Spring 2023
 * Name: Nathan Rennacker
 * Student ID: 921348958
 * GitHub ID: nlrennacker
 * Project: Assignment 4 – Word Blast
 *
 * File: Rennacker_Nathan_HW4_main.c
 *
 * Description: This program reads the entirety of War and Peace using an optional number of threads
 * to break up the txt file in order to make it run faster. After counting the occurences of every word,
 * the top 10 are printed to the terminal.
 *
 **************************************************************/
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

// You may find this Useful
char* delim = "\"\'.“”‘’?:;-,—*($%)! \t\n\x0A\r";

// number of unique words in War and Peace (plus a little extra)
#define TABLE_SIZE 20700

// c structure for word *string* and its respective count
typedef struct wordAndCount {
    char* word;
    int count;
} wordAndCount;

// struct object to hold array of words
struct wordAndCount mostFrequentWords[TABLE_SIZE];

// global variables for thread function
int totalCount = 0;
int blockSize;
int txtFile;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/**
 *
 * Counts the frequency of words in a block of text from a given file.
 *
 * @param args Pointer to an integer indicating the buffer end.
 */
void* countWords(void* args);

/**
 * Sorts the array of most frequently occurring words in ascending order.
 * The selection sort algorithm is used to accomplish this.
 */
void sortWords();

/**
 * Prints the top ten most frequently occurring words from a sorted wordAndCount array.
 *
 * @param fileName A "string" representing the name of the file.
 * @param threads The number of threads used to process the file.
 */
void printTopTen(char* fileName, int threads);

int main(int argc, char* argv[]) {
    // initialize struct data allocating mem for char* and setting integers to 0 for incrementing
    for (int i = 0; i < TABLE_SIZE; i++) {
        mostFrequentWords[i].word = calloc(20, sizeof(char));
        mostFrequentWords[i].count = 0;
    }
    // initialize mutex lock and check for errors
    int mutexError;
    if ((mutexError = pthread_mutex_init(&lock, NULL)) != 0) {
        printf("ERROR: Mutex init failed [%d]\n", mutexError);
    }

    // open file from given argument
    txtFile = open(argv[1], O_RDONLY);
    // lseek will return file size
    int txtFileSize = lseek(txtFile, 0, SEEK_END);
    // set file position back to start
    lseek(txtFile, 0, SEEK_SET);

    // need number of threads as integer to pass to thread creation function
    int threads = atoi(argv[2]);
    // block size depends on number of threads
    blockSize = txtFileSize / threads;
    pthread_t tid[threads];

    //**************************************************************
    // DO NOT CHANGE THIS BLOCK
    // Time stamp start
    struct timespec startTime;
    struct timespec endTime;

    clock_gettime(CLOCK_REALTIME, &startTime);
    //**************************************************************

    // loop through number of threads creating one for each
    for (int i = 0; i < threads; i++) {
        int* arg = malloc(sizeof(*arg));
        *arg = i;
        // thread error checking
        int threadError;
        if (threadError = pthread_create(&tid[i], NULL, countWords, (void*)arg)) {
            printf("ERROR: Thread creation failed [%d]\n", threadError);
            exit(EXIT_FAILURE);
        }
    }
    // join all threads, making main wait to finish until all threads finish first
    for (int i = 0; i < threads; i++) {
        pthread_join(tid[i], NULL);
    }

    sortWords();

    printTopTen(argv[1], threads);

    //**************************************************************
    // DO NOT CHANGE THIS BLOCK
    // Clock output
    // clock_gettime(CLOCK_REALTIME, &endTime);
    clock_gettime(CLOCK_REALTIME, &endTime);
    time_t sec = endTime.tv_sec - startTime.tv_sec;
    long n_sec = endTime.tv_nsec - startTime.tv_nsec;
    if (endTime.tv_nsec < startTime.tv_nsec) {
        --sec;
        n_sec = n_sec + 1000000000L;
    }

    printf("Total Time was %ld.%09ld seconds\n", sec, n_sec);
    //**************************************************************

    // self explanatory
    close(txtFile);
    // lock destroyed
    pthread_mutex_destroy(&lock);

    for (int i = 0; i < TABLE_SIZE; i++) {
        free(mostFrequentWords[i].word);
    }
    return 0;
}

void* countWords(void* args) {
    // Initialize variables
    char buffer[blockSize];
    char* savePtr;
    int result;
    int bufferEnd = *(int*)args;
    int bufferStartOffset = bufferEnd * blockSize;

    // read text from file based on which thread is reading, (offset calculated)
    pread(txtFile, buffer, blockSize, bufferStartOffset);

    // Loop through tokens in buffer
    char* token = strtok_r(buffer, delim, &savePtr);
    while (token != NULL) {
        // Check if string length is greater than 5
        if ((int)strlen(token) > 5) {
            // Check if token is in mostFrequentWords array
            for (int i = 0; i < TABLE_SIZE; i++) {
                if ((result = strcasecmp(mostFrequentWords[i].word, token)) == 0) {
                    // Increment count of mostFrequentWords[i]
                    pthread_mutex_lock(&lock);
                    mostFrequentWords[i].count++;
                    pthread_mutex_unlock(&lock);
                    break;
                }
            }

            // If token is not in mostFrequentWords array, add it
            if (result != 0 && *token != '\0') {
                pthread_mutex_lock(&lock);
                strcpy(mostFrequentWords[totalCount].word, token);
                mostFrequentWords[totalCount].count++;
                totalCount++;
                pthread_mutex_unlock(&lock);
            }
        }
        // update token with next string
        token = strtok_r(NULL, delim, &savePtr);
    }

    // Free memory and exit thread
    free(args);
}

void sortWords() {
    // selection sort orders in ascending order so we need to flip the order
    // without changing the index number for end reporting (by simply
    // multiplying each value by -1)
    for (int i = 0; i < TABLE_SIZE; i++) {
        mostFrequentWords[i].count *= -1;
    }

    wordAndCount temporaryStorage;

    // SELECTION SORT (simple c implemenation)
    //  One by one move boundary of unsorted subarray
    for (int i = 0; i < TABLE_SIZE; i++) {
        // Find the minimum element in rest of unsorted array
        int mindex = i;
        for (int j = i + 1; j < TABLE_SIZE; j++) {
            if (mostFrequentWords[j].count < mostFrequentWords[mindex].count) {
                mindex = j;
            }
        }
        // Swap the found minimum element with the first element
        if (mindex != i) {
            temporaryStorage = mostFrequentWords[mindex];
            mostFrequentWords[mindex] = mostFrequentWords[i];
            mostFrequentWords[i] = temporaryStorage;
        }
    }
}

void printTopTen(char* fileName, int threads) {
    // prints word count header with command line argument printf formatting
    printf("\n\nWord Frequency Count on %s with %d threads\n", fileName, threads);
    printf("Printing top 10 words 6 characters or more.\n");

    // loops through most frequent word array (multiplying the count by -1 to flip the negative)
    for (int i = 0; i < 10; i++) {
        printf("Number %d is %s with a count of %d\n", i + 1, mostFrequentWords[i].word, (mostFrequentWords[i].count * -1));
    }
}