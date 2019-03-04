// sighandlers.c
//
// Phi Luu
//
// Oregon State University
// CS_344_001_W2019 Operating Systems 1
// Program 3: Smallsh
//
// This module contains constants, parameters, and functions related to signal
// handling.

#include "sighandlers.h"
#include <unistd.h>
#include <stdlib.h>

// Catches SIGINT (i.e. Ctrl-C) and stops the current child process instead
// of exiting from the shell.
void CatchSIGINT(int signo) {
    write(STDOUT_FILENO, "\n", 1);
}

// Catches SIGCHLD and do something TODO
void CatchSIGCHLD(int signo) {
    write(STDOUT_FILENO, "Some child exited\n", 18);
}
