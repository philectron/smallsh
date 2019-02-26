// utility.c
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

#include "utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

// Dynamically allocates an array of strings structure with the given capacity.
//
// Argument:
//   capacity  the initial capacity of the array
//
// Returns:
//   A newly allocated structure of dynamic array of strings
DynStrArr* InitDynStrArr(int capacity) {
    assert(capacity > 0);

    // allocate dynamic memory for the structure
    DynStrArr* arr = malloc(sizeof(*arr));
    assert(arr);  // make sure allocation was successful

    // allocate dynamic memory for the array
    arr->strings = malloc(capacity * sizeof(*(arr->strings)));
    assert(arr->strings);  // make sure allocation was succesful

    // init the array's size and capacity
    arr->size = 0;
    arr->capacity = capacity;

    return arr;
}

// Doubles the capacity of a dynamic array of strings.
//
// Argument:
//   arr  the structure of dynamic array of strings, whose capacity to double
//
// The array of strings inside  arr  will have a double capacity.
void DoubleDynStrArrCapacity(DynStrArr* arr) {
    assert(arr && arr->strings);

    // init a new array of strings with double the capacity
    char** new_strings = malloc(arr->capacity * 2 * sizeof(*new_strings));
    assert(new_strings);  // make sure allocation was successful

    // move all strings to the new array
    for (int i = 0; i < arr->size; i++)
        // no need to copy the strings since they're all pointers
        new_strings[i] = arr->strings[i];

    // destroy the old array and set pointer to the new one
    free(arr->strings);
    arr->strings = new_strings;

    // update the capacity
    arr->capacity *= 2;
}

// Pushes a string to the back of the dynamic array (double the capacity if
// needed).
//
// Arguments:
//   arr         the structure of dynamic array of strings
//   new_string  the string to be pushed to the back of  arr->strings
//
// The array of strings inside  arr  will have an additional string at its back.
// The capacity of the array will be doubled if necessary.
void PushBackDynStrArr(DynStrArr* arr, char* new_string) {
    assert(arr && arr->strings && new_string);

    // before adding, must double the capacity if array is full
    if (arr->size == arr->capacity) DoubleDynStrArrCapacity(arr);

    // copy the new string to the back of the array
    strcpy(arr->strings[arr->size++], new_string);
}

void DeleteDynStrArr(DynStrArr* arr) {
    // do nothing if the structure or the array is NULL
    if (!arr || !arr->strings) return;

    // clean up the strings if needed
    for (int i = 0; i < arr->size; i++)
        if (arr->strings[i]) free(arr->strings[i]);

    // clean up the array and reset parameters
    free(arr->strings);
    arr->size = 0;
    arr->capacity = 0;
}
