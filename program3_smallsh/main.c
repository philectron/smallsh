#include "parser.h"
#include "builtins.h"
#include "utility.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#define INIT_CHILDREN_CAP (int)8
#define JUNK_VAL (int)-7

static DynPidArr bg_children;
static pid_t fg_childpid;
static int child_exit_status;

// Catches SIGINT (i.e. Ctrl-C) and stops the current foregrounded child process
// instead of exiting from the shell.
void CatchSIGINT(int signo) {
    if (signo == SIGINT && fg_childpid != (int)JUNK_VAL) {
        int child_exit_status = (int)JUNK_VAL;
        /* write(STDOUT_FILENO, "Parent received SIGINT, sending SIGINT to child\n", 48); */
        /* fflush(stdout); */
        int kill_ret = kill(fg_childpid, SIGINT);
        if (kill_ret == 0) {
            /* write(STDOUT_FILENO, "Parent sent SIGINT successfully\n", 32); */
            /* fflush(stdout); */
            waitpid(fg_childpid, &child_exit_status, 0);
            fg_childpid = (int)JUNK_VAL;
        } else {
            write(STDOUT_FILENO, "Parent failed to send SIGINT\n", 29);
            fflush(stdout);
        }
    }

    /* int child_exit_status = (int)JUNK_VAL; */
    /* write(STDOUT_FILENO, "Parent received SIGINT, sending SIGKILL to child\n", 49); */
    /* write(STDOUT_FILENO, "Parent received SIGINT\n", 23); */
    /* fflush(stdout); */
    /* if (kill(fg_childpid, SIGKILL)) { */
    /*     write(STDOUT_FILENO, "Parent sent SIGKILL successfully\n", 33); */
    /*     fflush(stdout); */
    /*     waitpid(fg_childpid, &child_exit_status, 0); */
    /* } else { */
    /*     write(STDOUT_FILENO, "Parent failed to send SIGKILL\n", 30); */
    /*     fflush(stdout); */
    /* } */
}

int main(void) {
    // trap signals
    struct sigaction SIGINT_action = {0};     // deal with SIGINT aka Ctrl-C
    SIGINT_action.sa_handler = CatchSIGINT;   // handler when caught
    sigemptyset(&SIGINT_action.sa_mask);       // block all signal types
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

    child_exit_status = (int)JUNK_VAL;
    fg_childpid = JUNK_VAL;
    while (1) {
        char* cmdline = PromptUser();
        DynStrArr* cmdwords = SplitCmdLineToWords(cmdline);

        int stdin_redir_idx = -1;
        int stdout_redir_idx = -1;
        bool is_bg = false;
        char** execvp_argv = ParseCmdWords(cmdwords, &stdin_redir_idx,
                                           &stdout_redir_idx, &is_bg);

        // check for built-in function
        if (!execvp_argv) {
            char* cmd = cmdwords->strings[0];
            if (strcmp(cmd, "exit") == 0) {
                Exit(&bg_children);
                break;
            }
            if (strcmp(cmd, "status") == 0) {
                Status(child_exit_status);
                continue;
            } else if (strcmp(cmd, "cd") == 0) {
                Cd((cmdwords->size == 1) ? NULL : cmdwords->strings[1]);
                continue;
            }
        }

        // fork off a child
        pid_t spawnpid = JUNK_VAL;
        spawnpid = fork();
        if (spawnpid == -1) {
            perror("fork() failure\n");
            exit(1);
        } else if (spawnpid == 0) {  // child
            // redirect stdin to file if the  <  symbol was found
            if (stdin_redir_idx != -1) {
                char* file = cmdwords->strings[stdin_redir_idx + 1];

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

                // close on  exec()
                fcntl(target_fd, F_SETFD, FD_CLOEXEC);
            }

            // redirect stdout to file if the  >  symbol was found
            if (stdout_redir_idx != -1) {
                char* file = cmdwords->strings[stdout_redir_idx + 1];

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

                // close on  exec()
                fcntl(target_fd, F_SETFD, FD_CLOEXEC);
            }

            // if the process is to run on background, suppress all stdout if no
            // output file is specified
            if (is_bg && stdout_redir_idx == -1) {
                // open a new file descriptor
                int target_fd = open("/dev/null", O_WRONLY);
                // make sure  open()  was successful
                if (target_fd == -1) {
                    fprintf(stderr, "cannot open /dev/null for output\n");
                    exit(1);
                }

                // redirect stdout to this new file descriptor
                int dup2_status = dup2(target_fd, 1);  // stdout has fd 1
                // make sure  dup2()  was successful
                if (dup2_status == -1) {
                    perror("dup2() from stdout failure\n");
                    exit(1);
                }

                // close on  exec()
                fcntl(target_fd, F_SETFD, FD_CLOEXEC);
            }

            /* signal(SIGINT, CatchChildSIGINT); */
            /* // trap signals */
            /* struct sigaction SIGINT_action = {0};     // deal with SIGINT aka Ctrl-C */
            /* SIGINT_action.sa_handler = CatchChildSIGINT;   // handler when caught */
            /* sigemptyset(&SIGINT_action.sa_mask);       // block all signal types */
            /* SIGINT_action.sa_flags = 0;               // no flags */
            /* sigaction(SIGINT, &SIGINT_action, NULL);  // register the struct */

            /* printf("CHILD: PID = %d\n", (int)getpid()); */
            /* fflush(stdout); */

            // use the  exec()  family for non-builtin commands
            execvp(execvp_argv[0], execvp_argv);

            // if execvp() fails, handle it
            fprintf(stderr, "%s: no such file or directory\n", execvp_argv[0]);
            exit(1);
        }

        // no  fork()  error + not child => must be the parent
        // add the child's PID to the children array if it's bg and continue on
        if (is_bg) {
            PushBackDynPidArr(&bg_children, spawnpid);
            printf("background pid is %d\n", (int)spawnpid);
            fflush(stdout);
        } else {
            fg_childpid = spawnpid;
            pid_t waitpid_status = waitpid(spawnpid, &child_exit_status, 0);

            if (waitpid_status == -1) {
                // if  waitpid()  returns -1, the child was signal-terminated
                /* printf("waitpid() returned -1\n"); */
                /* fflush(stdout); */
                Status(child_exit_status);
            } else {
                // otherwise, make sure  waitpid()  returns same PID as spawnpid
                printf("waitpid() returned %d\n", waitpid_status);
                fflush(stdout);
                assert(spawnpid == waitpid_status);
            }
        }

        // check all background PIDs
        for (int i = 0; i < bg_children.size; i++) {
            // check if any bg process has completed, return 0 if none has
            int childpid = waitpid(bg_children.pids[i], &child_exit_status,
                                   WNOHANG);

            // if there is a completed bg process
            if (childpid == bg_children.pids[i]) {
                // pop from array
                PopDynPidArrAt(&bg_children, i);

                printf("background pid %d is done: ", (int)childpid);
                fflush(stdout);
                Status(child_exit_status);
            }
        }

        // clean up
        free(cmdline);

        DeleteDynStrArr(cmdwords);
        free(cmdwords);

        free(execvp_argv);
    }

    DeleteDynPidArr(&bg_children);

    return 0;
}
