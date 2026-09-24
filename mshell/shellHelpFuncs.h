#define _POSIX_C_SOURCE 200809L

#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#pragma once

#define MSHELL_TOK_BUFSIZE 64
#define MSHELL_TOK_DELIM " \t\r\n\a"

/* reads line from stdin. you must start with a block, so i start with
1024 and then if the user exceeds it, the function reallocates dynamically. "realloc"
can request extra memory. "heap" without the defined name, since it isn't called as such
in C. */
char *mshell_read_line(void){
    char *line = NULL;
    size_t bufsize = 0;
    ssize_t len = getline(&line, &bufsize, stdin);

    if(len == -1){
        free(line);
        if(feof(stdin)){
            printf("\n");
            exit(EXIT_SUCCESS);
        }
        perror("mshell: readline");
        exit(EXIT_FAILURE);
    }

    if(len > 0 && line[len - 1] == '\n'){
        line[len - 1] = '\0';
    }
    return line;
}


    // Obviously, mshell_read_line was updated to utilize stdio.h. 
    // here's the old implementation, though.

    /*
    int bufsize = MSHELL_RL_BUFSIZE;
    int pos = 0;
    char *buffer = malloc(sizeof(char)*bufsize);
    int c;

    if(!buffer){
        fprintf(stderr, "mshell: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while(1){
        # 1 is a way to check for EOF because EOF is actually an int
        in this loop, if it is newline or EOF, null terminate the current string and
        return it. otherwise, add the char to existing string.
        # read char
        c = getchar();

        # if end of file, replace with null char and return
        if(c == EOF || c == '\n'){
            buffer[pos] = '\0';
            return buffer;
        }else{
            buffer[pos] = c;
        }
        pos++;

        # if exceed buffer, reallocate. if char goes out of current buffer size,
        # this function can reallocate.
        if(pos >= bufsize){
            bufsize += MSHELL_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if(!buffer){
                fprintf(stderr, "mshell: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    # this function can be done with stdio.h like this to avoid manual buffer management. this is a practice shell write, so
    # the lack of stdio.h is a conscious learning decision. the same function but with stdio.h would roughly read:
    char *mshell_read_line(void){
        char *line = NULL;
        ssize_t bufsize = 0;
    
        if(getline(&line, &bufsize, stdin) == -1){
            if(feof(stdin)){
                exit(EXIT_SUCCESS);
            }else{
                perror("readline");
                exit(EXIT_FAILURE);
                }
            }
        return line;
    } */



    /* parsing the line
     use the whitespace to separate arguments from each other.
     tokenize the string using the whitespace as delimiters. then use
     strtok library to return pointers within the string we gave it, and place
     \0 bytes at the end of each token. store the pointer in the buffer array of char ptrs.
     reallocate the array of pointers if it is needed.
     uses the same strategy as read line. buffer then dynamically expand
     based on need, but using null-terminated array of ptrs instead of chars. */

char **mshell_split_line(char *line){
    int bufsize = MSHELL_TOK_BUFSIZE, pos = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;
 
    if(!tokens){
        fprintf(stderr, "mshell: allocation error.\n");
        exit(EXIT_FAILURE);
    }
 
    token = strtok(line, MSHELL_TOK_DELIM);
    while(token != NULL){
        tokens[pos] = token;
        pos++;
 
        if(pos >= bufsize){
            bufsize += MSHELL_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if(!tokens){
                fprintf(stderr, "mshell: allocation error\n");
                exit(EXIT_FAILURE);
                }
        }
 
        token = strtok(NULL, MSHELL_TOK_DELIM);
    }
    tokens[pos] = NULL;
    return tokens;
}

    /* two unix ways to start a process. one: Init. almost never applicable! two: fork(). when called,
    the OS makes a dupe process and starts them both running. the og is parent, the dupe is the child.
    fork() returns 0 to the child process, then returns to the parent the process ID num (PID) of the child.
    so, the way of starting new process is to duplicate a preexisting one. handled with exec() sys call. the OS stops your process, loads
    the new program, and starts that one in its place. the process never returns from an exec() call unless there is an error.
    parent can keep tabs on it child using sys call wait(). */

    /* mshell_launch uses our argument list, forks the process, and then saves the return value.
    once fork() returns, two processes are running concurrently. the child process will satisfy the first if condition.
    its process ID == 0. in child process, the command from user is ran. "execvp" is exec. this variant of exec
    expects a program name and an array (vector) of string arguments (first one = program name).
    the p denotes that we are not giving the full file path, but the name and the os will search for the
    program in the path. if -1 is returned, an error occured, and perror prints the error msg with program name. exit so the shell
    continues to run. the else if checks if fork executed successfully. the parent process will land here. child executes, parent waits
    for the command to finish running. waitpid() tells parent to wait for process state to change. use arguments to ensure
    that the proccess is exited or killed. then, 1 is return as a signal to prompt for input again. */

int mshell_launch(char **args){
    pid_t pid, wpid;
    int status;

    pid = fork();
    if(pid == 0){
        // child process
        if(execvp(args[0], args) == -1){
            perror("mshell");
        }
        exit(EXIT_FAILURE);
    }else if(pid < 0){
        // error forking
        perror("mshell");
    }else{
        //parent process
        do{
            wpid = waitpid(pid, &status, WUNTRACED);
        } while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int mshell_execute(char **args){
    if(args[0] == NULL){
        return 1;
    }

    for(int i=0; i < mshell_num_builtins(); i++){
        if(strcmp(args[0], builtin_str[i])==0){
            return (*builtin_func[i])(args);
        }
    }
    return mshell_launch(args);
}