#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "shellCommands.h"
#include "shellHelpFuncs.h"
#include "shellFunc.h" 

int main(int argc, char **argv){

    mshell_loop();

    return EXIT_SUCCESS;
}