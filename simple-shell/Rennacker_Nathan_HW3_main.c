/**************************************************************
 * Class:  CSC-415-03 Spring 2023
 * Name: Nathan Rennacker
 * Student ID: 921348958
 * GitHub ID: nlrennacker
 * Project: Assignment 3 – Simple Shell
 *
 * File: Rennacker_Nathan_HW3_main.c
 *
 * Description: The purpose of this program is to act as a simple shell which accepts commands, uses execvp on
 * child processes, and then returns their status to the command line once they finish executing or fail. 
 * A custom command line prompt is also accepted for the shell.
 *
 **************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_INPUT_LENGTH 102
#define MAX_ARGC 5

/**
 * reads input into buffer, if input is from a file, print commands to terminal
 * returns 1 if eof or 0 if success
*/
int readInput(char* buffer, int isFileInput) {

    //get input of maxinputlength, if it's null (eof) then return 1 meaning eof
    if (fgets(buffer, MAX_INPUT_LENGTH, stdin) == NULL) {
        return 1;
    }
    // print before null terminator if input is from file to show commands on prompt lines and include newline char
    if (isFileInput) {
        printf("%s", buffer);
    }
    int len = strlen(buffer);
    // Check if we got the whole line
    if (buffer[len - 1] == '\n') {
        // entire line read into input
    } else {
        // missing some of the line (didn't fit in the 102 size buffer, discard rest)
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {
            // discard character
        }
    }
    // change newline (or 102nd char) to null term
    buffer[len - 1] = '\0';
    return 0;
}

/**
 * split input (buffer) along space boundaries until null terminator is hit and store in args
 * returns 1 if number of args is exceeded or empty command
 * 0 if success
*/
int parseInput(char* buffer, char** args) {
    int i = 0;
    args[i] = strtok(buffer, " ");
    while (args[i] != NULL) {
        i++;
        if (i >= 5) {
            printf("Error: too many arguments\n");
            return 1;
        }
        args[i] = strtok(NULL, " ");
    }
    if (i == 0) {
        return 1;
    }
    return 0;
}

void runCommand(char** args) {
    // fork and execute command
    pid_t pid = fork();
    if (pid == -1) {
        printf("Error: failed to fork\n");
    } else if (pid == 0) {
        // child process
        execvp(args[0], args);
        // if process fails to execute, then and only then will this line be reached because of the way execvp works
        printf("Error: command not found\n");
        exit(1);
    } else {
        // parent process
        int status;
        //wait for child to die
        waitpid(pid, &status, 0);
        printf("Child %d, exited with %d\n", pid, WEXITSTATUS(status));
    }
}

int main(int argc, char* argv[]) {
    int isFileInput = 1;
    // checks if STDIN is tty, if it is, set fileInput "boolean" so that file commands are printed to terminal
    if (isatty(STDIN_FILENO)) {
        isFileInput = 0;
    }
    // check for prompt argument and set prompt string
    char* prompt;
    if (argc > 1) {
        prompt = argv[1];
    } else {
        prompt = "> ";
    }

    //malloc for future fgets()
    char* buffer = malloc(MAX_INPUT_LENGTH);
    char* args[MAX_ARGC];

    // loop to read and execute commands
    while (1) {
        // print prompt and read input
        printf("%s", prompt);
        if (readInput(buffer, isFileInput)) {
            printf("\nEnd of File reached... Thank you for using Simple Shell™\n");
            break;
        }
        if (parseInput(buffer, args)) {
            continue;
        }
        if (strcmp(args[0], "exit") == 0) {
            printf("Exiting... Thank you for using Simple Shell™\n");
            break;
        }
        runCommand(args);
    }
    free(buffer);
    buffer = NULL;
    return 0;
}
