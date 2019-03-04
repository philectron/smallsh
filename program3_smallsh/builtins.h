// builtins.h
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

#ifndef BUILTINS_H_
#define BUILTINS_H_

#include "utility.h"

void Exit(DynPidArr* children);

void Cd(char* path);

void Status(int child_exit_status);

#endif  // #ifndef BUILTINS_H_

