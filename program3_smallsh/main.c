#include "parser.h"
#include "utility.h"
/* #include <sys/types.h> */
/* #include <sys/wait.h> */
/* #include <unistd.h> */
/* #include <signal.h> */
#include <stdio.h>
#include <stdlib.h>

#define JUNK_VAL (pid_t)-7

int main(void) {
    /* pid_t spawnpid = JUNK_VAL; */
    /* int child_exit_method = JUNK_VAL; */
    /*  */
    /* spawnpid = fork(); */
    /*  */
    /* if (spawnpid == -1) { */
    /*     perror("Something went wrong.\n"); */
    /*     exit(1); */
    /* } else if (spawnpid == 0) { */
    /*     printf("CHILD: pid = %d, exiting...\n", spawnpid); */
    /*     exit(0); */
    /* } */
    /*  */
    /* printf("PARENT: pid = %d, waiting...\n", spawnpid); */
    /* waitpid(spawnpid, &child_exit_method, 0); */
    /* printf("PARENT: Child process terminated, exiting...\n"); */
    /*  */
    /* return 0; */

    char* cmdline = PromptUser();
    DynStrArr* cmd_args = ParseCmdLine(cmdline);

    int i = 0;
    while (1) {
        printf("[%d]: ", i);
        if (cmd_args->strings[i] == NULL) {
            printf("NULL\n");
            i++;
            break;
        } else {
            printf("%s\n", cmd_args->strings[i]);
            i++;
        }
    }

    printf("Size = %d; Cap = %d\n", cmd_args->size, cmd_args->capacity);

    /* RunCmd(cmd_args); */

    // clean up
    free(cmdline);

    DeleteDynStrArr(cmd_args);
    free(cmd_args);

    return 0;
}
