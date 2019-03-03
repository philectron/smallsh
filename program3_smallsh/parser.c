// parser.c
//
// Phi Luu
//
// Oregon State University
// CS_344_001_W2019 Operating Systems 1
// Program 3: Smallsh
//
// This module contains constants, parameters, and functions related to
// prompting user's input commands, parsing them into smallsh, and interpret
// them.

#include "parser.h"
#include "builtins.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// TODO
char* PromptUser(void) {
    // dynamically allocate memory for command line and ensure it succeeds
    char* cmdline = malloc(MAX_CMDLINE_LEN * sizeof(*cmdline));
    assert(cmdline);  // make sure allocation was successful

    do {
        // get raw input
        printf(": ");
        fflush(stdout);
        fgets(cmdline, MAX_CMDLINE_LEN, stdin);

        // remove trailing newline(s)
        size_t cmdline_len = strlen(cmdline);
        while (cmdline_len > 0 && cmdline[cmdline_len - 1] == '\n')
            cmdline[--cmdline_len] = '\0';
    } while (cmdline[0] == '\0' || cmdline[0] == '#');

    return cmdline;
}

// TODO
DynStrArr* ParseCmdLine(char* cmdline) {
    // ensure  cmdline  not NULL and not empty
    assert(cmdline && cmdline[0] != '\0');

    // a parsed command line will be split by space and put into an array
    DynStrArr* cmd_args = malloc(sizeof(*cmd_args));
    InitDynStrArr(cmd_args, INIT_CMDLINE_ARGS);

    // create a temporary copy of the command line used for splitting
    char cmdline_tmp[MAX_CMDLINE_LEN];
    strcpy(cmdline_tmp, cmdline);

    // divide the command line into tokens with " " being the delimiter
    char* cmdline_tok = strtok(cmdline_tmp, " ");  // 1st word

    // split the command line until it cannot be split anymore
    while (cmdline_tok != NULL) {
        PushBackDynStrArr(cmd_args, cmdline_tok);
        cmdline_tok = strtok(NULL, " ");
    }

    // push a NULL at the end of parsed array, signaling the end of cmd
    PushBackNullDynStrArr(cmd_args);

    return cmd_args;
}

void RunCmd(DynStrArr* cmdline, DynPidArr* children) {
    assert(cmdline && cmdline->size >= 2 && children);

    char* cmd = cmdline->strings[0];

    // check for built-in commands
    if (strcmp(cmd, "exit") == 0) {
        Exit(children);
    } else if (strcmp(cmd, "status") == 0) {
        Status();
    } else if (strcmp(cmd, "cd") == 0) {
        Cd(cmdline->strings[1]);
    } else {
        printf("Non-builtin functions\n");
    }
}
