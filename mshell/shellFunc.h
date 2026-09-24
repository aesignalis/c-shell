#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#pragma once

//main shell function
void mshell_loop(void){
    //init
    //** = variable name represents pointer to a pointer to a char*/
    char *line;
    char **args;
    int status;

    do{
        printf("> ");
        line = mshell_read_line();
        args = mshell_split_line(line);
        status = mshell_execute(args);

        free(line);
        free(args);
    } while(status);
}