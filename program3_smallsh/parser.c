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
#include "utility.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

int num_fork = 0;

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

// TODO
int RunCmd(DynStrArr* cmdline, DynPidArr* children, int* child_exit_status) {
    assert(cmdline && cmdline->size >= 2 && children && child_exit_status);

    char* cmd = cmdline->strings[0];

    // check for built-in commands
    if (strcmp(cmd, "exit") == 0) {
        Exit(children);
        return 1;  // return 1 to let the shell clean up itself in main()
    }

    if (strcmp(cmd, "status") == 0) {
        Status(*child_exit_status);
        return 0;  // return 0 to continue the while loop in main()
    }

    if (strcmp(cmd, "cd") == 0) {
        Cd(cmdline->strings[1]);
        return 0;  // return 0 to continue the while loop in main()
    }

    // find the  <  or  >  or  &  symbols.  &  must appear at the end to be
    // considered
    int execvp_len = 0;
    bool execvp_len_done = false;
    int stdin_redir_idx = -1;
    int stdout_redir_idx = -1;
    bool is_backgrounded = false;
    for (int i = 0; i < cmdline->size - 1; i++) {  // -1 due to NULL
        // if matches stdin redirection symbol, done with counting words
        if (strcmp(cmdline->strings[i], "<") == 0) {
            stdin_redir_idx = i;
            execvp_len_done = true;
        }
        // if matches stdout redirection symbol, done with counting words
        if (strcmp(cmdline->strings[i], ">") == 0) {
            stdout_redir_idx = i;
            execvp_len_done = true;
        }
        // if matches background symbol at the end (and only at the end),
        // done with counting words
        if (strcmp(cmdline->strings[i], "&") == 0 && i == cmdline->size - 2) {
            is_backgrounded = true;
            execvp_len_done = true;
        }
        // otherwise, increase the len of the array to be passed to execvp()
        if (!execvp_len_done) execvp_len++;
    }

    execvp_len++;  // need 1 more slot to contain NULL
    char* execvp_arr[execvp_len];
    for (int i = 0; i < execvp_len - 1; i++)
        execvp_arr[i] = cmdline->strings[i];  // temp pointer, not hard copy
    execvp_arr[execvp_len - 1] = NULL;

    /* // debug */
    /* for (int i = 0; i < execvp_len; i++) { */
    /*     if (!execvp_arr[i]) { */
    /*         printf("[%d]: NULL\n", i); */
    /*     } else { */
    /*         printf("[%d]: %s\n", i, execvp_arr[i]); */
    /*     } */
    /* } */
    /* printf("stdin_redir_idx = %d; stdout_redir_idx = %d; ", */
    /*        stdin_redir_idx, stdout_redir_idx); */
    /* is_backgrounded ? printf("is backgrounded\n") : printf("is foregrounded\n"); */

    // fork off a child
    if (num_fork >= 20) {  // fail-safe for testing TODO delete when done
        abort();
    }

    pid_t spawnpid = JUNK_VAL;
    spawnpid = fork();
    num_fork++;
    switch (spawnpid) {
    case -1:
        perror("Could not fork() a child\n");
        exit(1);
    case 0:  // child process
        // use the  exec()  family for non-builtin commands
        execvp(cmd, execvp_arr);

        // if execvp() fails, handle it
        fprintf(stderr, "%d: ", (int)getpid());
        perror("execvp() failure\n");
        exit(1);
    default:  // parent process
        // add the child's PID to the children array if it's bg and continue on
        if (is_backgrounded) {
            PushBackDynPidArr(children, spawnpid);
        } else {
            // otherwise (fg process), wait for child to finish
            // and makes sure  waitpid()  succeeds
            assert(spawnpid == waitpid(spawnpid, child_exit_status, 0));

            /* printf("PARENT(%d): Child(%d) terminated\n", (int)getpid(), ret_childpid); */
        }
        break;
    }

    return 0;  // return 0 to continue the while loop in main()
}
