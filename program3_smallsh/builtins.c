// builtins.c
//
// Phi Luu
//
// Oregon State University
// CS_344_001_W2019 Operating Systems 1
// Program 3: Smallsh
//
// This module contains constants, parameters, and functions related to the
// built-in commands of the shell. Commands that are not in this module will
// be executed via  exec() .

#include "builtins.h"
#include "utility.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <assert.h>
#include <string.h>

// Kills all background child processes, flushes stdout, and then terminates
// the shell.
//
// Argument:
//   bg_children  a dynamic array of PIDs containing the PIDs of bg children
void Exit(DynPidArr* bg_children) {
    assert(bg_children);

    // kill all child processes
    pid_t* childpid;
    int child_exit_status;
    while ((childpid = PopBackDynPidArr(bg_children))) {
        kill(SIGKILL, *childpid);
        assert(*childpid == waitpid(*childpid, &child_exit_status, 0));
    }
}

// Changes the current working directory to a directory specified by  path .
void Cd(char* path) {
    char* processed_path;

    if (!path || path[0] == '\0') {
        // cd  with no arg, default to ${HOME}
        processed_path = getenv("HOME");
    } else {
        // cd  with a single arg
        processed_path = path;
    }

    // try going to the directory & handle errors if any
    if (chdir(processed_path) == -1)
        fprintf(stderr, "%s: no such file or directory\n", processed_path);
}

// Displays the exit code (or signal termination code) of previously terminated
// process.
void Status(int child_exit_status) {
    if (WIFEXITED(child_exit_status) != 0) {
        // if terminated normally, get the exit status via macro
        printf("exit value %d\n", (int)WEXITSTATUS(child_exit_status));
        fflush(stdout);
    } else if (WIFSIGNALED(child_exit_status) != 0) {
        // if terminated by a signal, get the exit signal via macro
        printf("terminated by signal %d\n", (int)WTERMSIG(child_exit_status));
        fflush(stdout);
    } else {
        // let's hope it never gets here
        perror("Status() failure\n");
        exit(1);
    }
}
