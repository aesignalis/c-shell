#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#pragma once

typedef struct {
    char *infile;
    char *outfile;
    int append;
} mshell_redir;

int mshell_is_redir(char *word){
    return strcmp(word, "<") == 0 || strcmp(word, ">") == 0 || strcmp(word, ">>") == 0;
}

/* this function uses a read index i and a write index w. i will scan every word and ordinary words
will be copied to position w and the redir words are skipped, so w will fall behind i.*/
int mshell_parse_redirs(char **cmd, mshell_redir *r){
    int w = 0;
    r->infile = NULL;
    r->outfile = NULL;
    r->append = 0;

    for(int i=0; cmd[i] != NULL; i++){
        if(mshell_is_redir(cmd[i])){
            char *op = cmd[i];
            char *file = cmd[i+1];

            if(file==NULL || mshell_is_redir(file)){
                return -1;
            }

            if(strcmp(op, "<") == 0){
                r->infile = file;
            }else{
                r->outfile = file;
                r->append = (strcmp(op, ">>") == 0);
            }
            i++;
        }else{
            cmd[w++] = cmd[i];
        }
    }
    cmd[w] = NULL;
    return 0;
}

void mshell_apply_redirs(mshell_redir *r){
    int fd;

    if(r->infile != NULL){
        fd = open(r->infile, O_RDONLY);
        if(fd==-1){
            perror(r->infile);
            exit(EXIT_FAILURE);
        }
        if(dup2(fd, STDIN_FILENO) == -1){
            perror("mshell");
            exit(EXIT_FAILURE);
        }
        close(fd);
    }

    if(r->outfile != NULL){
        int flags = O_WRONLY | O_CREAT | (r->append ? O_APPEND : O_TRUNC);
        fd = open(r->outfile, flags, 0644);
        if(fd == -1){
            perror(r->outfile);
            exit(EXIT_FAILURE);
        }
        if(dup2(fd, STDOUT_FILENO) == -1){
            perror("mshell");
            exit(EXIT_FAILURE);
        }
        close(fd);
    }
}