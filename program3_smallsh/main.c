#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    char* cmdline = PromptUser();
    DynStrArr* cmd_args = ParseCmdLine(cmdline);

    RunCmd(cmd_args);

    // clean up
    free(cmdline);

    DeleteDynStrArr(cmd_args);
    free(cmd_args);

    return 0;
}
