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

typedef struct DynStrArr {
    char** strings;
    int size;
    int capacity;
} DynStrArr;

DynStrArr* InitDynStrArr(int capacity);

void DoubleDynStrArrCapacity(DynStrArr* arr);

void PushBackDynStrArr(DynStrArr* arr, char* new_string);

void DeleteDynStrArr(DynStrArr* arr);

#endif  // #ifndef UTILITY_H_
