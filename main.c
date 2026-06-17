#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>


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
                fprintf(stderr, "minishell: allocation error");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, TOK_DELIM); // call with NULL to keep splitting the old string
    }
    tokens[position] = NULL;
    return tokens;
}

int sh_launch(char **args){
    pid_t pid, wpid;
    int status;

    pid = fork();
    if(pid ==0){
        if(execvp(args[0],args) == -1){
            perror("minishell");
        }
        exit(EXIT_FAILURE):
    }
        exit(EXIT_FAILURE):
    else if(pid < 0){
        perror("minishell");
    } else {

        do{
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int sh_cd(char **args);
int sh_help(char **args);
int sh_exit(char **args);


char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
    &sh_cd,
    &sh_help,
    &sh_exit
};

int sh_num_builtins(){
    return sizeof(builtin_str / sizeof(char*));
}

int sh_cd(char **args){
    if (args[0] == NULL){
        fprinf(stderr, "minishell: Expected argument to \"cd\" \n");
    } else {
        if (chdir(args[1]) != 0){
            perror("minishell");
        }
    }
    return 1;
}

int sh_help(char **args){
    int i;
    printf("unix minishell");
    printf("Type program names and arguments and then hit ENTER to proceed. \n");
    printf("The following are built in: \n");

    for(i = 0; i < sh_num_builtins(); i++){
        printf(" %s\n", builtin_str[i]);
    }

    printf("Use the man command followed by the program name for instructions on usage");
    return 1;
}

int sh_exit(char **args){
    return 0;
}


int sh_execute(char **args){
    int i;

    if(args[0] == NULL){
        return 1;
    }

    for(i=0; i < sh_num_builtins; i++){
        if(strcmp(args[0], builtin_str[i]) == 0){
            return (*builtin_func[i])(args);
        }
    }

    return sh_launch(args);
}