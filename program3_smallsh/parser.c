// parser.c
//
// Phi Luu
//
// Oregon State University
// CS_344_001_W2019 Operating Systems 1
// Program 3: Smallsh
//
// This module contains constants, parameters, and functions related to
// prompting user's input commands and parsing them into smallsh.

#include "parser.h"
#include "builtins.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

// TODO
char* PromptUser(void) {
    // dynamically allocate memory for command line and ensure it succeeds
    char* cmdline = malloc(MAX_CMDLINE_LEN * sizeof(*cmdline));
    assert(cmdline);  // make sure allocation was successful

    do {
        // get raw input
        printf(": ");
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
    DynStrArr* cmd_args = InitDynStrArr(10);

    for (int i = 0; i < cmd_args->capacity; i++) {
        // to guarantee memory for each word, allocate max size for each
        cmd_args->strings[i] = malloc(MAX_CMDLINE_LEN *
                                      sizeof(*(cmd_args->strings[i])));
        assert(cmd_args->strings[i]);  // make sure allocation was successful
    }

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

    /* if (cmd_args->size == cmd_args->capacity) DoubleDynStrArrCapacity(cmd_args); */
    /* // free the last string before setting it NULL or else will leak the mem */
    /* free(cmd_args->strings[cmd_args->size]); */
    /* // push a NULL at the end of parsed array */
    /* cmd_args->strings[cmd_args->size++] = NULL; */

    return cmd_args;
}
