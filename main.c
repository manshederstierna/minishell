#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>


#define BUFSIZE 1024
#define TRUE 1
#define TOK_BUFSIZE 64
#define TOK_DELIM " \t\r\n\a"
#define CURRENT_VERSION "1.0.0"


void sh_loop(void);
char *sh_read_line(void);
char **sh_split_line(char *line);

int sh_launch(char **args, char *output_file, int bool_append);
int sh_execute(char **args);
int sh_launch_pipe(char **left_args, char **right_args);
int sh_cd(char **args);
int sh_help(char **args);
int sh_exit(char **args);
int sh_num_builtins(void);
int sh_pwd(char **args);
int sh_clear(char **args);
int sh_echo(char **args);
int sh_version(char **args);

void sh_loop(void){
    char *line;
    char **args;
    int status;

    do{
        printf("> ");
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
            buffer = realloc(buffer,bufsize);
            if(buffer == NULL){
                fprintf(stderr, "minishell: allocatione error \n");
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
        fprintf(stderr, "minishell: allocation error");
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

int sh_launch(char **args, char *output_file, int bool_append){
    pid_t pid,wpid; 
    int status;

    pid = fork();
    if(pid == 0){
        if(output_file != NULL){
			int fd;
			if(bool_append == 0){
				fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			} else {
				fd = open(output_file, O_WRONLY | O_APPEND | O_CREAT, 0644);
			}
			
            if(fd == -1){
                perror("minishell");
                exit(EXIT_FAILURE);
            }
            
            if(dup2(fd, STDOUT_FILENO) == -1){
                perror("minishell");
                close(fd);
                exit(EXIT_FAILURE);
            }
            close(fd);

        }
        if(execvp(args[0],args) == -1){
            perror("minishell");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid < 0){
        perror("minishell");
    } else {

        do{
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int sh_launch_pipe(char **left_args, char **right_args){
	pid_t pid_writer;
	pid_t pid_reader;
	int status;
	int pipefd[2]; // [0] is reader, [1] is writer
	if(pipe(pipefd) == -1){
		perror("minishell");
	}
	
	pid_writer = fork();
	
	if(pid_writer == 0){
			close(pipefd[0]);
			if(dup2(pipefd[1], STDOUT_FILENO) == -1){
				perror("minishell: ");
				exit(EXIT_FAILURE);
			}
			close(pipefd[1]);
			
		    if(execvp(left_args[0],left_args) == -1){
				perror("minishell");
				exit(EXIT_FAILURE);
			}	
	}
	
	pid_reader = fork();
	
	if(pid_reader == 0){
		close(pipefd[1]);
		if((dup2(pipefd[0],STDIN_FILENO) == -1)){
			perror("minishell: ");
			exit(EXIT_FAILURE);
		}
		
		if(execvp(right_args[0],right_args) == -1){
			perror("minishell: ");
			exit(EXIT_FAILURE);
		}	
	}
	
	close(pipefd[0]);
	close(pipefd[1]);
	
	
	waitpid(pid_writer, &status, 0);
	waitpid(pid_reader, &status, 0);
	
	return 1;
}


char *builtin_str[] = {
  "cd",
  "help",
  "exit",
  "pwd",
  "clear",
  "echo",
  "version"
};

int (*builtin_func[]) (char **) = {
    &sh_cd,
    &sh_help,
    &sh_exit,
    &sh_pwd,
    &sh_clear,
    &sh_echo,
    &sh_version
};

int sh_num_builtins(){
    return sizeof(builtin_str) / sizeof(builtin_str[0]);
}

int sh_cd(char **args){
    if (args[1] == NULL){
        fprintf(stderr, "minishell: Expected argument to \"cd\" \n");
    } else {
        if (chdir(args[1]) != 0){
            perror("minishell");
        }
    }
    return 1;
}

int sh_help(char **args){
    int i;
    printf("Unix minishell \n");
    printf("Type program names and arguments and then hit ENTER to proceed. \n");
    printf("The following are built in: \n");

    for(i = 0; i < sh_num_builtins(); i++){
        printf(" %s\n", builtin_str[i]);
    }

    printf("Use the man command followed by the program name for instructions on usage \n");
    return 1;
}

int sh_pwd(char **args){
    char cwd[1024];

    if(getcwd(cwd,sizeof(cwd)) != NULL){
        printf("%s\n",cwd);
    } else{
        perror("minishell");
    }

    return 1;
}

int sh_clear(char **args){
    printf("\033[2J\033[H");
    return 1;
}


int sh_version(char **args){
    printf("%s", CURRENT_VERSION);
    printf("\n");
    return 1;
}

int sh_echo(char **args){
    int i = 1;

    while(args[i] != NULL){
        printf("%s",args[i]);

        if(args[i+1] != NULL){
            printf(" ");
        }
        i++;
    }

    printf("\n");
    return 1;
    
}

int sh_exit(char **args){
    return 0;
}

int sh_execute(char **args){
    int i = 0;
    char *output_file = NULL;
	int bool_append = 0;
	int bool_has_pipe = 0;
	
	while(args[i] != NULL){
		if(strcmp(args[i], "|") == 0	){
			bool_has_pipe = 1;
			args[i] = NULL;
			return 	sh_launch_pipe(&args[0],&args[i+1]); 

		}
		i++;
	}
	
	i = 0; // if no pipe was found
	
    while(args[i] != NULL){
        if(strcmp(args[i],">") == 0){
            if(args[i+1] == NULL){
                fprintf(stderr, "minishell: expected filename after >\n");
                return 1;
            }

            output_file = args[i+1];
            args[i] = NULL;
            break;
        }
		
		if(strcmp(args[i],">>") == 0){
			    if(args[i+1] == NULL){
                fprintf(stderr, "minishell: expected filename after >>\n");
                return 1;
				}
				
				output_file = args[i+1];
				bool_append  = 1;
				args[i] = NULL;
				break;
		}
		
        i++;
    }

    if(args[0] == NULL){
        return 1;
    }

    for(i=0; i < sh_num_builtins(); i++){
        if(strcmp(args[0], builtin_str[i]) == 0){
            if(output_file == NULL){
                return (*builtin_func[i])(args);
            }

            int saved_stdout = dup(STDOUT_FILENO);
            if(saved_stdout == -1){
                perror("minishell");
                return 1;
            }
			int fd;
			if(bool_append == 0){
				fd = open(output_file, O_WRONLY | O_TRUNC | O_CREAT, 0644);
			} else{
				fd = open(output_file, O_WRONLY | O_APPEND | O_CREAT, 0644);
			}

            if(fd == -1){
                perror("minishell");
                return 1;
            }

            if(dup2(fd,STDOUT_FILENO) == -1){
                perror("minishell");
                close(fd);
                close(saved_stdout);
                return 1;
            }

            close(fd);

            int result = (*builtin_func[i])(args);

            fflush(stdout);

            dup2(saved_stdout, STDOUT_FILENO);
            close(saved_stdout);

            return result;  
        }
    }

    return sh_launch(args,output_file,bool_append);
}

int main(int argc, char **argv){
    sh_loop();
    return EXIT_SUCCESS;
}

