#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    char* cmdline = PromptUser();

    DynStrArr* cmd_args = ParseCmdLine(cmdline);

    int i = 0;
    do {
        char* arg = cmd_args->strings[i];
        if (!arg) {
            printf("[%d] = NULL\n", i);
            break;
        } else {
            printf("[%d] = %s\n", i, arg);
        }
        i++;
    } while (1);

    // clean up
    free(cmdline);

    DeleteDynStrArr(cmd_args);
    free(cmd_args);

    return 0;
}
