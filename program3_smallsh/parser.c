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
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

// Displays the prompt and prompts the user for a smallsh command.
//
// Assumes a command is formatted as
//
//   command [arg1 arg2 ...] [< input_file] [> output_file] [&]
//
// where items in square brackets are optional.
//
// Assumes commands are made up of words separated by spaces. The special
// symbols-- < ,  > , and  & --are recognized, but they must be surrounded by
// spaces like other words.
//
// Assumes the syntax of the command is valid.
//
// Returns:
//   A new dynamically allocated C string containing the whole command line
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

// Parses the command line by splitting them into words. Puts each of the words
// into a dynamic array of C strings.
//
// Argument:
//   cmdline  a pointer to the string that contains the command line
//
// Returns:
//   A new dynamically allocated dynamic array structure of strings containing
//   the parsed command line
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

    return cmd_args;
}

// Runs the command after it has been parsed. Commands will be further broken
// down based on the three special symbols:  < ,  > , and  & .
//
// Built-in commands will be run accordingly. Non-built-in commands will be run
// using the  exec()  family in a child process created by  fork() .
//
// Arguments:
//   cmdline  a pointer to the dynamic array of structure of strings containing
//            the parsed command line
//   bg_children  a pointer to the dynamic array of structure of PIDs containing
//                the PIDs of children that are run in the background
//   child_exit_status  a pointer to the latest exit status of a child process
//   fg_childpid  a pointer to the PID of the current foreground child process
//
// Returns:
//   0  if the user wants the shell to keep going after running this command
//      successfully
//   1  if the user wants the shell to perform clean up and then terminate
int RunCmd(DynStrArr* cmdline, DynPidArr* bg_children, int* child_exit_status,
           pid_t* fg_childpid) {
    assert(cmdline && cmdline->size >= 1 && bg_children && child_exit_status
           && fg_childpid);

    char* cmd = cmdline->strings[0];

    // check for built-in commands
    if (strcmp(cmd, "exit") == 0) {
        Exit(bg_children);
        return 1;  // return 1 to let the shell clean up itself in main()
    }
    if (strcmp(cmd, "status") == 0) {
        Status(*child_exit_status);
        return 0;  // return 0 to continue the while loop in main()
    }
    if (strcmp(cmd, "cd") == 0) {
        Cd((cmdline->size == 1) ? NULL : cmdline->strings[1]);
        return 0;  // return 0 to continue the while loop in main()
    }

    // find the  <  or  >  or  &  symbols.  &  must appear at the end to be
    // considered
    int execvp_len = 0;
    bool execvp_len_done = false;
    int stdin_redir_idx = -1, stdout_redir_idx = -1;
    bool is_bg = false;
    for (int i = 0; i < cmdline->size; i++) {
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
        if (strcmp(cmdline->strings[i], "&") == 0 && i == cmdline->size - 1) {
            is_bg = true;
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

    // fork off a child
    pid_t spawnpid = JUNK_VAL;
    spawnpid = fork();
    switch (spawnpid) {
    case -1:
        perror("fork() failure\n");
        exit(1);
    case 0:  // child
        // redirect stdin to file if the  <  symbol was found
        if (stdin_redir_idx != -1) {
            char* file = cmdline->strings[stdin_redir_idx + 1];

            // open a new file descriptor
            int target_fd = open(file, O_RDONLY);
            // make sure  open()  was successful
            if (target_fd == -1) {
                fprintf(stderr, "cannot open %s for input\n", file);
                exit(1);
            }

            // redirect stdin to this new file descriptor
            int dup2_status = dup2(target_fd, 0);  // stdin has fd 0
            // make sure  dup2()  was successful
            if (dup2_status == -1) {
                perror("dup2() from stdin failure\n");
                exit(1);
            }

            // close on exec()
            fcntl(target_fd, F_SETFD, FD_CLOEXEC);
        }

        // redirect stdout to file if the  >  symbol was found
        if (stdout_redir_idx != -1) {
            char* file = cmdline->strings[stdout_redir_idx + 1];

            // open a new file descriptor
            int target_fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            // make sure  open()  was successful
            if (target_fd == -1) {
                fprintf(stderr, "cannot open %s for output\n", file);
                exit(1);
            }

            // redirect stdout to this new file descriptor
            int dup2_status = dup2(target_fd, 1);  // stdout has fd 1
            // make sure  dup2()  was successful
            if (dup2_status == -1) {
                perror("dup2() from stdout failure\n");
                exit(1);
            }

            // close on exec()
            fcntl(target_fd, F_SETFD, FD_CLOEXEC);
        }
        
        signal(SIGINT, CatchChildSIGINT);
        /* // trap signals */
        /* struct sigaction SIGINT_action = {0};     // deal with SIGINT aka Ctrl-C */
        /* SIGINT_action.sa_handler = CatchChildSIGINT;   // handler when caught */
        /* sigfillset(&SIGINT_action.sa_mask);       // block all signal types */
        /* SIGINT_action.sa_flags = 0;               // no flags */
        /* sigaction(SIGINT, &SIGINT_action, NULL);  // register the struct */
        /*  */
        /* printf("CHILD: PID = %d\n", (int)getpid()); */
        /* fflush(stdout); */

        // use the  exec()  family for non-builtin commands
        execvp(cmd, execvp_arr);

        // if execvp() fails, handle it
        fprintf(stderr, "%s: no such file or directory\n", execvp_arr[0]);
        exit(1);
    default:  // parent
        // add the child's PID to the children array if it's bg and continue on
        if (is_bg) {
            PushBackDynPidArr(bg_children, spawnpid);
            printf("background pid is %d\n", (int)spawnpid);
            fflush(stdout);
        } else {
            *fg_childpid = spawnpid;
            pid_t waitpid_ret = waitpid(spawnpid, child_exit_status, 0);
            if (waitpid_ret == -1) {
                // if waitpid() returns -1, the child was signal-terminated
                Status(*child_exit_status);
            } else {
                // otherwise, make sure waitpid() returns same PID as spawnpid
                assert(spawnpid == waitpid_ret);
            }
        }
        return 0;  // return 0 to continue the while loop in main()
    }

    return 0;  // return 0 to continue the while loop in main()
}

// Catches SIGINT (i.e. Ctrl-C) and stops the current fg child process instead
void CatchChildSIGINT(int signo) {
    if (signo == SIGINT) {
        write(STDOUT_FILENO, "Child received SIGINT\n", 22);
    } else {
        write(STDERR_FILENO, "Something went wrong with Child's SIGINT handler\n", 49);
    }
}
