// parser.h
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

#ifndef PARSER_H_
#define PARSER_H_

#include "utility.h"

#define MAX_CMDLINE_LEN   (int)(2048 + 2)  // +2 for  \n  and  \0
#define MAX_CMDLINE_ARGS  (int)(512 + 2)   // + 2 for the command itself and  \0
#define INIT_CMDLINE_ARGS (int)8           // initial number of cmd-line args

char* PromptUser(void);

DynStrArr* ParseCmdLine(char* cmdline);

void RunCmd(DynStrArr* cmdline, DynPidArr* children);

#endif  // #ifndef PARSER_H_
