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
#include <unistd.h>
#include <stdio.h>
#include <string.h>

// TODO
void Exit(void) {
    printf("Exit()\n");
}

// TODO
void Cd(char* path) {
    printf("Cd()\n");

    // if  path  is NULL, that means  cd  was called with no argument
    if (!path) {
        printf("path is NULL\n");
    } else if (path[0] == '\0') {
        printf("path is empty\n");
    } else {
        printf("path = %s\n", path);
    }
}

// TODO
void Status(void) {
    printf("Status()\n");
}
