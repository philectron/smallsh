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
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO
void Exit(DynPidArr* children) {
    printf("Exit()\n");
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
    if (chdir(processed_path) == -1) {
        perror("chdir() failure\n");
        return;
    }

    /* // debug: print cwd */
    /* char cwd[PATH_MAX]; */
    /* if (getcwd(cwd, sizeof(cwd)) == NULL) { */
    /*     perror("getcwd() error\n"); */
    /*     return; */
    /* } */
    /* printf("cwd = %s\n", cwd); */
}

// TODO
void Status(void) {
    printf("Status()\n");
}
