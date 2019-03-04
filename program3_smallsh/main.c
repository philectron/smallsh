#include "parser.h"
#include "builtins.h"
#include "sighandlers.h"
#include "utility.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define SHELL_PID         (pid_t)getpid()
#define INIT_CHILDREN_CAP (int)8

int main(void) {
    // trap signals
    struct sigaction SIGINT_action = {0};     // deal with SIGINT aka Ctrl-C
    SIGINT_action.sa_handler = CatchSIGINT;   // handler when caught
    sigfillset(&SIGINT_action.sa_mask);       // block all signal types
    SIGINT_action.sa_flags = 0;               // no flags
    sigaction(SIGINT, &SIGINT_action, NULL);  // register the struct

    /* struct sigaction SIGCHLD_action = {0};      // deal with SIGCHLD */
    /* SIGCHLD_action.sa_handler = CatchSIGCHLD;   // handler when caught */
    /* sigfillset(&SIGCHLD_action.sa_mask);        // block all signal types TODO? */
    /* SIGCHLD_action.sa_flags = 0;                // no flags */
    /* sigaction(SIGCHLD, &SIGCHLD_action, NULL);  // register the struct */

    struct sigaction ignore_action = {0};      // set signals to be ignored
    ignore_action.sa_handler = SIG_IGN;        // ignore signals when caught
    sigaction(SIGHUP, &ignore_action, NULL);   // register to ignore SIGHUP
    sigaction(SIGTERM, &ignore_action, NULL);  // register to ignore SIGTERM
    sigaction(SIGQUIT, &ignore_action, NULL);  // register to ignore SIGQUIT

    int quit = 0;
    int exit_status = (int)JUNK_VAL;
    while (!quit) {
        char* cmdline = PromptUser();
        DynStrArr* cmd_args = ParseCmdLine(cmdline);

        DynPidArr children;
        InitDynPidArr(&children, INIT_CHILDREN_CAP);

        // int i = 0;
        // while (1) {
        //     printf("[%d]: ", i);
        //     if (cmd_args->strings[i] == NULL) {
        //         printf("NULL\n");
        //         i++;
        //         break;
        //     } else {
        //         printf("%s\n", cmd_args->strings[i]);
        //         i++;
        //     }
        // }
        // printf("Size = %d; Cap = %d\n", cmd_args->size, cmd_args->capacity);

        quit = RunCmd(cmd_args, &children, &exit_status);
        
        /* // debug: print list of PIDs of child processes */
        /* printf("\n"); */
        /* for (int i = 0; i < children.size; i++) */
        /*     printf("%d ", (int)children.pids[i]); */
        /* printf("\n"); */

        // clean up
        free(cmdline);

        DeleteDynStrArr(cmd_args);
        free(cmd_args);

        DeleteDynPidArr(&children);
    }

    return 0;
}
