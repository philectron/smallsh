// utility.h
//
// Phi Luu
//
// Oregon State University
// CS_344_001_W2019 Operating Systems 1
// Program 3: Smallsh
//
// This module contains constants, parameters, and functions that are useful
// for the program but might seem too general to be related to a certain module.
// They are essentially the "helpers" of the program.

#ifndef UTILITY_H_
#define UTILITY_H_

#include <sys/types.h>

// a dynamic array that can hold a command line and its arguments
typedef struct DynStrArr {
    char** strings;
    int size;
    int capacity;
} DynStrArr;

// a dynamic array that can hold children PIDs of a program
typedef struct DynPidArr {
    pid_t* pids;
    int size;
    int capacity;
} DynPidArr;

void InitDynStrArr(DynStrArr* arr, int capacity);
void PushBackDynStrArr(DynStrArr* arr, char* new_string);
void PushBackNullDynStrArr(DynStrArr* arr);
void DeleteDynStrArr(DynStrArr* arr);

void InitDynPidArr(DynPidArr* arr, int capacity);
void PushBackDynPidArr(DynPidArr* arr, pid_t new_pid);
void DeleteDynPidArr(DynPidArr* arr);

#endif  // #ifndef UTILITY_H_
