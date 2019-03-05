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

static DynPidArr bg_children;  // array of PIDs of background children
static pid_t fg_childpid;      // PID of current foreground child
static int fg_exit_status;     // exit status of the latest foreground child
static bool is_fg_only_mode;   // toggle for SIGTSTP

// Catches SIGINT (i.e. INTerrupt i.e. Ctrl-C) and let the current foreground
// child process terminate itself instead of exiting from the shell.
void CatchSIGINT(int signo) {
    write(STDOUT_FILENO, "\n", 1);
    fflush(stdout);
}

// Catches SIGINT (i.e. INTerrupt i.e. Ctrl-C) inside  fork()  and let the
// current foreground child process terminate itself instead of exiting from the
// shell.
void CatchChildSIGINT(int signo) {
    write(STDOUT_FILENO, "\nChild received SIGINT\n", 23);
    fflush(stdout);
}

// Catches SIGTSTP (i.e. Terminal SToP i.e. Ctrl-Z) and toggle the
// foreground-only mode.
void CatchSIGTSTP(int signo) {
    if (is_fg_only_mode) {
        is_fg_only_mode = false;
        write(STDOUT_FILENO, "\nExiting foreground-only mode\n", 30);
    } else {
        is_fg_only_mode = true;
        write(STDOUT_FILENO,
              "\nEntering foreground-only mode (& is now ignored)\n", 50);
    }
    fflush(stdout);
}

int main(void) {
    // signal handler registration
    /* struct sigaction SIGINT_action = {0};     // deal with SIGINT (i.e Ctrl-C) */
    /* SIGINT_action.sa_handler = CatchSIGINT;   // handler when caught */
    /* sigfillset(&SIGINT_action.sa_mask);       // block all signal types */
    /* SIGINT_action.sa_flags = 0;               // no flags */
    /* sigaction(SIGINT, &SIGINT_action, NULL);  // register the struct */

    struct sigaction SIGTSTP_action = {0};      // deal with SIGTSTP
    SIGTSTP_action.sa_handler = CatchSIGTSTP;   // handler when caught
    sigfillset(&SIGTSTP_action.sa_mask);        // block all signal types
    SIGTSTP_action.sa_flags = 0;                // no flags
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);  // register the struct

    struct sigaction ignore_action = {0};      // set signals to be ignored
    ignore_action.sa_handler = SIG_IGN;        // ignore signals when caught
    sigaction(SIGHUP, &ignore_action, NULL);   // ignore SIGHUP
    sigaction(SIGTERM, &ignore_action, NULL);  // ignore SIGTERM
    sigaction(SIGQUIT, &ignore_action, NULL);  // ignore SIGQUIT

    // initialization of static global var
    InitDynPidArr(&bg_children, INIT_CHILDREN_CAP);
    fg_childpid = JUNK_VAL;
    fg_exit_status = 0;
    is_fg_only_mode = false;

    // start smallsh and keep it running in a loop
    while (true) {
        // cmdline  contains the whole command line in a C string
        char* cmdline = PromptUser();
        // if blank line or line starting with #, ignore the rest
        if (cmdline[0] == '\0' || cmdline[0] == '#') continue;

        // cmdwords  contains the whole command line in an array of C strings,
        //           whose each element is a word
        DynStrArr* cmdwords = SplitCmdLineToWords(cmdline);

        // no need to parse for  execvp()  if command is one of the built-ins
        if (strcmp(cmdwords->strings[0], "exit") == 0) {
            // clean up before exiting
            free(cmdline);
            DeleteDynStrArr(cmdwords);
            free(cmdwords);

            // clean up all background children and exit with code 0
            Exit(&bg_children, 0);
        }
        if (strcmp(cmdwords->strings[0], "status") == 0) {
            Status(fg_exit_status);

            // clean up before ignoring everything below
            free(cmdline);
            DeleteDynStrArr(cmdwords);
            free(cmdwords);
            continue;
        } else if (strcmp(cmdwords->strings[0], "cd") == 0) {
            // if no arg, pass  ${HOME}  in as path, otherwise pass  arg[1]
            Cd((cmdwords->size == 1) ? getenv("HOME") : cmdwords->strings[1]);

            // clean up before ignoring everything below
            free(cmdline);
            DeleteDynStrArr(cmdwords);
            free(cmdwords);
            continue;
        }

        // execvp_argv  contains the command line words from the beginning to
        //              the word before  < ,  > ,  and/or  &  symbols. This is
        //              an  execvp() -compatible array of C strings
        int stdin_redir_idx = -1;
        int stdout_redir_idx = -1;
        bool is_bg = false;
        int execvp_argc = 0;
        char* execvp_argv[cmdwords->size + 1];  // +1 for NULL at the end
        ParseCmdWords(cmdwords, execvp_argv, &execvp_argc, &stdin_redir_idx,
                      &stdout_redir_idx, &is_bg);

        int bg_exit_status = (int)JUNK_VAL;

        // fork off a child
        pid_t spawnpid = JUNK_VAL;
        spawnpid = fork();
        if (spawnpid == -1) {
            fprintf(stderr, "fork() failure\n");

            // clean up
            free(cmdline);
            DeleteDynStrArr(cmdwords);
            free(cmdwords);

            // clean up all children and exit with code 1
            Exit(&bg_children, 1);
        } else if (spawnpid == 0) {  // child
            // redirect stdin to file if the  <  symbol was found
            if (stdin_redir_idx != -1) {
                char* file = cmdwords->strings[stdin_redir_idx + 1];
                RedirectFileDescriptor(0, file, O_RDONLY, 0);

                /* // open a new file descriptor */
                /* int target_fd = open(file, O_RDONLY); */
                /* // make sure  open()  was successful */
                /* if (target_fd == -1) { */
                /*     fprintf(stderr, "cannot open %s for input\n", file); */
                /*     exit(1); */
                /* } */
                /*  */
                /* // redirect stdin to this new file descriptor */
                /* int dup2_status = dup2(target_fd, 0);  // stdin has fd 0 */
                /* // make sure  dup2()  was successful */
                /* if (dup2_status == -1) { */
                /*     fprintf(stderr, "dup2() from stdin failure\n"); */
                /*     exit(1); */
                /* } */
                /*  */
                /* // close on  exec() */
                /* fcntl(target_fd, F_SETFD, FD_CLOEXEC); */
            } else if (is_bg) {
                // if the process is to run on background, suppress all stdin
                // if no input file is specified
                RedirectFileDescriptor(0, "/dev/null", O_RDONLY, 0);

                /* // open a new file descriptor */
                /* int target_fd = open("/dev/null", O_RDONLY); */
                /* // make sure  open()  was successful */
                /* if (target_fd == -1) { */
                /*     fprintf(stderr, "cannot open /dev/null for input\n"); */
                /*     exit(1); */
                /* } */
                /*  */
                /* // redirect stdout to this new file descriptor */
                /* int dup2_status = dup2(target_fd, 0);  // stdin has fd 0 */
                /* // make sure  dup2()  was successful */
                /* if (dup2_status == -1) { */
                /*     fprintf(stderr, "dup2() from stdin failure\n"); */
                /*     exit(1); */
                /* } */
                /*  */
                /* // close on  exec() */
                /* fcntl(target_fd, F_SETFD, FD_CLOEXEC); */
            }

            // redirect stdout to file if the  >  symbol was found
            if (stdout_redir_idx != -1) {
                char* file = cmdwords->strings[stdout_redir_idx + 1];
                RedirectFileDescriptor(1, file, O_WRONLY | O_CREAT | O_TRUNC,
                                       0644);

                /* // open a new file descriptor */
                /* int target_fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644); */
                /* // make sure  open()  was successful */
                /* if (target_fd == -1) { */
                /*     fprintf(stderr, "cannot open %s for output\n", file); */
                /*     exit(1); */
                /* } */
                /*  */
                /* // redirect stdout to this new file descriptor */
                /* int dup2_status = dup2(target_fd, 1);  // stdout has fd 1 */
                /* // make sure  dup2()  was successful */
                /* if (dup2_status == -1) { */
                /*     fprintf(stderr, "dup2() from stdout failure\n"); */
                /*     exit(1); */
                /* } */
                /*  */
                /* // close on  exec() */
                /* fcntl(target_fd, F_SETFD, FD_CLOEXEC); */
            } else if (is_bg && !is_fg_only_mode) {
                // if the process is to run on background, suppress all stdout
                // if no output file is specified
                RedirectFileDescriptor(1, "/dev/null", O_WRONLY, 0);

                /* // open a new file descriptor */
                /* int target_fd = open("/dev/null", O_WRONLY); */
                /* // make sure  open()  was successful */
                /* if (target_fd == -1) { */
                /*     fprintf(stderr, "cannot open /dev/null for output\n"); */
                /*     exit(1); */
                /* } */
                /*  */
                /* // redirect stdout to this new file descriptor */
                /* int dup2_status = dup2(target_fd, 1);  // stdout has fd 1 */
                /* // make sure  dup2()  was successful */
                /* if (dup2_status == -1) { */
                /*     fprintf(stderr, "dup2() from stdout failure\n"); */
                /*     exit(1); */
                /* } */
                /*  */
                /* // close on  exec() */
                /* fcntl(target_fd, F_SETFD, FD_CLOEXEC); */
            }

            // signal handler registration

            // deal with SIGINT as a child
            struct sigaction SIGINT_child_action = {0};
            // handler when caught
            SIGINT_child_action.sa_handler = CatchChildSIGINT;
            // block all signal types
            sigfillset(&SIGINT_child_action.sa_mask);
            // no flags
            SIGINT_child_action.sa_flags = 0;
            // register the struct
            sigaction(SIGINT, &SIGINT_child_action, NULL);

            struct sigaction ignore_action = {0};  // set signals to be ignored
            ignore_action.sa_handler = SIG_IGN;  // ignore signals when caught
            sigaction(SIGTSTP, &ignore_action, NULL);  // ignore SIGTSTP

            // use the  exec()  family for non-builtin commands
            execvp(execvp_argv[0], execvp_argv);

            // if execvp() fails, handle it
            fprintf(stderr, "%s: no such file or directory\n", execvp_argv[0]);
            exit(1);
        }

        // no  fork()  error + not child => must be the parent

        // add the child's PID to the children array if it's bg and continue on
        if (is_bg && !is_fg_only_mode) {
            PushBackDynPidArr(&bg_children, spawnpid);
            printf("background pid is %d\n", (int)spawnpid);
            fflush(stdout);

        } else {
            // otherwise, block the shell until the child exits (either normally
            // or by a signal)
            fg_childpid = spawnpid;
            pid_t waitpid_status = waitpid(spawnpid, &fg_exit_status, 0);

            if (waitpid_status == -1) {
                // if  waitpid()  returns -1, the child was signal-terminated
                Status(fg_exit_status);
            } else {
                // otherwise, make sure  waitpid()  returns same PID as spawnpid
                assert(spawnpid == waitpid_status);
            }
        }

        // check all background child PIDs
        for (int i = 0; i < bg_children.size; i++) {
            // check if the current child PID has completed, return 0
            // immediately if it hasn't
            pid_t bg_childpid = waitpid(bg_children.pids[i], &bg_exit_status,
                                        WNOHANG);  // non-blocking

            // if this child is indeed finished, print status
            if (bg_childpid == bg_children.pids[i]) {
                PopDynPidArrAt(&bg_children, i);

                printf("background pid %d is done: ", (int)bg_childpid);
                fflush(stdout);
                Status(bg_exit_status);
            }
        }

        // clean up heap used for this loop
        free(cmdline);
        DeleteDynStrArr(cmdwords);
        free(cmdwords);
    }

    return 0;
}
