#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#pragma once

//Function Declarations for builtins

int mshell_cd(char **args);
int mshell_help(char **args);
int mshell_exit(char **args);

char *builtin_str[] = {
    "cd",
    "help",
    "exit"
};

/* frankly nonsensical C declaration of a function that is also named as an array of function pointers.
wowie... */
int (*builtin_func[]) (char **) = {
    &mshell_cd,
    &mshell_help,
    &mshell_exit
};

int mshell_num_builtins(){
    return sizeof(builtin_str) / sizeof(char *);
}

//builtin function imps.
//we all know cd but this calls chdir() and checks for errors and returns.
//prints an error message is the second arg does not exist
int mshell_cd(char **args){
    if(args[1]==NULL){
        fprintf(stderr, "mshell: expected argument to \"cd\"\n");
    }else{
        if(chdir(args[1])!=0){
            perror("mshell");
        }
    }
    return 1;
}

//basic help! based on my work on Go CLI flags!!
int mshell_help(char **args){
    printf("aesignalis personal project: mshell. based on Stephen Brennan's LSH guide! ˚.⋆꒰১ ໒꒱⋆.˚\n");
    printf("Type program names and arguments, then hit enter.\n");
    printf("The following are built in:\n");

    for(int i=0; i<mshell_num_builtins(); i++){
        printf(" %s\n", builtin_str[i]);
    }

    printf("use the man command for information on other programs.\n");
    return 1;
}

int mshell_exit(char **args){
    return 0;
}