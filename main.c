#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 1024
#define TRUE 1
#define TOK_BUFSIZE 64
#define TOK_DELIM " \t\r\n\a"

int main(int argc, char **argv){


    lsh_loop();

    return EXIT_SUCCESS;
}


void sh_loop(void){
    char *line;
    char **args;
    int status;

    do{
        print("> "):
        line = sh_read_line();
        args = sh_split_line(line);
        status = sh_execute(args);

        free(line);
        free(args);
    } while (status);
}


char *sh_read_line(void){
    int bufsize = BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int x;

    if(buffer == NULL){
        fprintf(stderr, "minishell: allocation error \n");
        exit(EXIT_FAILURE);
    }

    while(TRUE){
        x = getchar();

        if(x == EOF || x == '\n'){
            buffer[position] = '\0';
            return buffer; 
        } else{
            buffer[position] = x;
        } 
        position++;


        if(position >= bufsize){
            bufsize += BUFSIZE;
            buffer = realloc(buffer,bufsize)
            if(buffer == NULL){
                fprinf(stderr, "minishell: allocatione error \n");
                exit(EXIT_FAILURE);
            }
        }
    } 


}

char **sh_split_line(char *line){
    int bufsize = TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if(tokens == NULL){
        fprinf(stderr, "minishell: allocation error");
        exit(EXIT_FAILURE);
    }

    token = strtok(line,TOK_DELIM);
    while( token != NULL){
        tokens[position] = token;
        position++;

        if(position >= bufsize){
            bufsize += TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if(tokens == NULL){
                fprinf(stderr, "minishell: allocation error");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;

    
}