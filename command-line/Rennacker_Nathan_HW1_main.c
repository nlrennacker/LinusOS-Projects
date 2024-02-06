/**************************************************************
 * Class:  CSC-415-03 Spring 2023
 * Name: Nathan Rennacker
 * Student ID: 921348958
 * GitHub Name: nlrennacker
 * Project: Assignment 1 – Command Line Arguments
 *
 * File: Rennacker_Nathan_HW1_main.c
 *
 * Description: This purpose of this file is to accept any number of command line arguments and
 * list them out ordered by input with each associated number. It also implements some basic text
 * coloring and formatting to format the text within the linux terminal.
 *
 **************************************************************/
#include <stdio.h>

// ANSI escape sequences for linux coloring (also sets the font weight to bold with "1m")
// Using bright variations of colors since linux terminal supports up to 16 might be 256 I wasn't sure found conflicting info!
#define GREEN "\x001b[32;1m"  //\x001b == \e -- escape character
#define YELLOW "\x001b[33;1m"
#define WHITE "\x001b[37;1m"
#define RESET "\x001b[0m"

int main(int argc, char* argv[]) {

    // argc is the number of command line arguments
    // argv[] is where the command line arguments are stored
    printf(YELLOW "\n There were %d arguments found on the command line:\n\n", argc);

    //loops through args printing out each one in format:
    /**
     * Argument 00:   xxx
     * Argument 01:   xxx
     * ...
     * Argument nn:   xxx
    */
    //left side is colored white, right side green
    for (int i = 0; i < argc; i++) {
        printf(WHITE "    Argument %02d:   ", i); //%02d causes numbers with single digits to have a 0 padded in front
        printf(GREEN "%s\n", argv[i]);
    }

    // Printing with YELLOW and then using RESET coloring just in case some text was being printed after
    // in a different command (not sure if it carries over)
    printf(YELLOW "\n Thank you for using Argument Counter™ \n          Have a nice day! \n\n" RESET);
    return 0;
}