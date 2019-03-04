#include "parser.h"
#include "builtins.h"
#include "utility.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define INIT_CHILDREN_CAP (int)8

static DynPidArr bg_children;
static pid_t fg_childpid;

// Catches SIGINT (i.e. Ctrl-C) and stops the current foregrounded child process
// instead of exiting from the shell.
void CatchSIGINT(int signo) {
    /* if (signo == SIGINT && fg_childpid != (int)JUNK_VAL) { */
    /*     int child_exit_status = (int)JUNK_VAL; */
    /*     write(STDOUT_FILENO, "Parent received SIGINT, sending SIGINT to child\n", 48); */
    /*     fflush(stdout); */
    /*     // reset fg_childpid */
    /*     fg_childpid = (int)JUNK_VAL; */
    /*     int tmp = kill(fg_childpid, SIGINT); */
    /*     if (tmp == 0) { */
    /*         write(STDOUT_FILENO, "Parent sent SIGINT successfully\n", 32); */
    /*         fflush(stdout); */
    /*         waitpid(fg_childpid, &child_exit_status, 0); */
    /*     } else { */
    /*         write(STDOUT_FILENO, "Parent failed to send SIGINT\n", 29); */
    /*         fflush(stdout); */
    /*     } */
    /* } else { */
    /*     write(STDERR_FILENO, "Something went wrong with Parent's SIGINT handler\n", 50); */
    /*     fflush(stdout); */
    /* } */

    int child_exit_status = (int)JUNK_VAL;

    write(STDOUT_FILENO, "Parent received SIGINT, sending SIGKILL to child\n", 49);
    fflush(stdout);
    if (kill(fg_childpid, SIGKILL)) {
        write(STDOUT_FILENO, "Parent sent SIGKILL successfully\n", 33);
        fflush(stdout);
        waitpid(fg_childpid, &child_exit_status, 0);
    } else {
        write(STDOUT_FILENO, "Parent failed to send SIGKILL\n", 30);
        fflush(stdout);
    }
}

int main(void) {
    // trap signals
    struct sigaction SIGINT_action = {0};     // deal with SIGINT aka Ctrl-C
    SIGINT_action.sa_handler = CatchSIGINT;   // handler when caught
    sigfillset(&SIGINT_action.sa_mask);       // block all signal types
    SIGINT_action.sa_flags = 0;               // no flags
    sigaction(SIGINT, &SIGINT_action, NULL);  // register the struct

    /* struct sigaction SIGCHLD_action = {0};      // deal with SIGCHLD */
    /* SIGCHLD_action.sa_handler = CatchSIGCHLD;   // handler when caught */
    /* sigfillset(&SIGCHLD_action.sa_mask);        // no flags */
    /* sigaction(SIGCHLD, &SIGCHLD_action, NULL);  // register the struct */

    struct sigaction ignore_action = {0};      // set signals to be ignored
    ignore_action.sa_handler = SIG_IGN;        // ignore signals when caught
    /* sigaction(SIGINT, &ignore_action, NULL);   // register to ignore SIGINT */
    sigaction(SIGHUP, &ignore_action, NULL);   // register to ignore SIGHUP
    sigaction(SIGTERM, &ignore_action, NULL);  // register to ignore SIGTERM
    sigaction(SIGQUIT, &ignore_action, NULL);  // register to ignore SIGQUIT

    InitDynPidArr(&bg_children, INIT_CHILDREN_CAP);

    int done = 0;
    int exit_status = (int)JUNK_VAL;
    fg_childpid = JUNK_VAL;
    while (!done) {
        char* cmdline = PromptUser();
        DynStrArr* cmd_args = ParseCmdLine(cmdline);

        done = RunCmd(cmd_args, &bg_children, &exit_status, &fg_childpid);

        // check all background PIDs
        for (int i = 0; i < bg_children.size; i++) {
            // check if any bg process has completed, return 0 if none has
            int childpid = waitpid(bg_children.pids[i], &exit_status, WNOHANG);

            // if there is a completed bg process
            if (childpid == bg_children.pids[i]) {
                // pop from array
                PopDynPidArrAt(&bg_children, i);

                printf("background pid %d is done: ", (int)childpid);
                fflush(stdout);
                Status(exit_status);
            }
        }

        // clean up
        free(cmdline);

        DeleteDynStrArr(cmd_args);
        free(cmd_args);
    }

    DeleteDynPidArr(&bg_children);

    return 0;
}
