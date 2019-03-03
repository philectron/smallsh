#include "parser.h"
#include "utility.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define INIT_NUM_CHILDREN (int)8
#define JUNK_VAL (pid_t)-7

int main(void) {
    /* pid_t spawnpid = JUNK_VAL; */
    /* int child_exit_status = JUNK_VAL; */
    /*  */
    /* char* cmdline = PromptUser(); */
    /* DynStrArr* cmd_args = ParseCmdLine(cmdline); */
    /*  */
    /* spawnpid = fork(); */
    /* switch (spawnpid) { */
    /* case -1: */
    /*     perror("Something went wrong\n"); */
    /*     exit(1); */
    /*     break; */
    /* case 0: */
    /*     printf("CHILD(getpid = %d, spawnpid = %d): Sleeping for 1 second\n", */
    /*            (int)getpid(), spawnpid); */
    /*     sleep(1); */
    /*     printf("CHILD(getpid = %d, spawnpid = %d): Converting into '", */
    /*            (int)getpid(), spawnpid); */
    /*     for (int i = 0; i < cmd_args->size; i++) { */
    /*         if (0 < i && i < cmd_args->size - 1) printf(" "); */
    /*         if (cmd_args->strings[i]) { */
    /*             printf("%s", cmd_args->strings[i]); */
    /*         } else { */
    /*             printf("'\n"); */
    /*         } */
    /*     } */
    /*  */
    /*     execvp(cmd_args->strings[0], cmd_args->strings); */
    /*     perror("CHILD: execvp() failure\n"); */
    /*     exit(2); */
    /*      */
    /*     break; */
    /* default: */
    /*     printf("PARENT(getpid = %d, spawnpid = %d): Sleeping for 2 seconds\n", */
    /*            (int)getpid(), spawnpid); */
    /*     sleep(2); */
    /*     printf("PARENT(getpid = %d, spawnpid = %d): Wait()ing for child(%d) to terminate\n", */
    /*            (int)getpid(), spawnpid, spawnpid); */
    /*     pid_t actual_pid = waitpid(spawnpid, &child_exit_status, 0); */
    /*     printf("PARENT(getpid = %d, spawnpid = %d): Child(%d) terminated. Exiting...\n", */
    /*            (int)getpid(), spawnpid, actual_pid); */
    /*     break; */
    /* } */
    /*  */
    /* free(cmdline); */
    /*  */
    /* DeleteDynStrArr(cmd_args); */
    /* free(cmd_args); */
    /*  */
    /* return 0; */

    /* -----------------------</TEST>-------------------------- */

    // clear screen
    system("clear");

    // while (1) {
        char* cmdline = PromptUser();
        DynStrArr* cmd_args = ParseCmdLine(cmdline);

        DynPidArr children;
        InitDynPidArr(&children, INIT_NUM_CHILDREN);

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

        RunCmd(cmd_args, &children);

        // clean up
        free(cmdline);

        DeleteDynStrArr(cmd_args);
        free(cmd_args);

        DeleteDynPidArr(&children);
    // }

    return 0;
}
