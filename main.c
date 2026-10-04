#include "minishell.h"

DebugMode debug_mode = 0;

void sh_loop(void)
{
    char *line;
    char **args;
    int status;

    do {
        printf(COLOR_CYAN "> " COLOR_RESET);
        line = sh_read_line();
        args = sh_split_line(line);
        status = sh_execute(args);

        free(line);
        free(args);
    } while (status);
}

char *sh_read_line(void)
{
    int bufsize = BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int x;

    if (buffer == NULL) {
        fprintf(stderr, "minishell: allocation error \n");
        exit(EXIT_FAILURE);
    }

    while (TRUE) {
        x = getchar();

        if (x == EOF || x == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = x;
        }
        position++;

        if (position >= bufsize) {
            bufsize += BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (buffer == NULL) {
                fprintf(stderr, "minishell: allocatione error \n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

char **sh_split_line(char *line)
{
    int bufsize = TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *token;

    if (tokens == NULL) {
        fprintf(stderr, "minishell: allocation error");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if (tokens == NULL) {
                fprintf(stderr, "minishell: allocation error");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, TOK_DELIM); // call with NULL to keep splitting the old string
    }
    tokens[position] = NULL;
    return tokens;
}

int sh_launch(char **args, char *output_file, int bool_append)
{
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        if (output_file != NULL) {
            int fd;
            if (bool_append == 0) {
                fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            } else {
                fd = open(output_file, O_WRONLY | O_APPEND | O_CREAT, 0644);
            }

            if (fd == -1) {
                perror("minishell");
                exit(EXIT_FAILURE);
            }

            if (dup2(fd, STDOUT_FILENO) == -1) {
                perror("minishell");
                close(fd);
                exit(EXIT_FAILURE);
            }
            close(fd);
        }
        if (execvp(args[0], args) == -1) {
            perror("minishell");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("minishell");
    } else {

        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int sh_launch_pipe(char **left_args, char **right_args)
{
    pid_t pid_writer;
    pid_t pid_reader;
    int status;
    int pipefd[2]; // [0] is reader, [1] is writer
    if (pipe(pipefd) == -1) {
        perror("minishell");
        return 1;
    }

    if (debug_mode == DEBUG_ON) {
        fprintf(stderr, "[minishell debug] pipe() created\n");
    }

    pid_writer = fork();

    if (pid_writer < 0) {
        perror("minishell: fork failed");
        return 1;
    }

    if (pid_writer == 0) {
        if (debug_mode == DEBUG_ON) {
            fprintf(stderr, "[minishell debug] writer child running with PID %d\n", getpid());
        }
        close(pipefd[0]);
        if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
            perror("minishell: Pipe failed");
            exit(EXIT_FAILURE);
        }
        close(pipefd[1]);

        if (execvp(left_args[0], left_args) == -1) {
            perror("minishell: Pipe exec failed");
            exit(EXIT_FAILURE);
        }
    } else {
        if (debug_mode == DEBUG_ON) {
            fprintf(stderr, "[minishell debug] parent forked writer child PID %d\n", pid_writer);
        }
    }

    pid_reader = fork();

    if (pid_reader < 0) {
        perror("minishell: fork failed");
        return 1;
    }

    if (pid_reader == 0) {
        if (debug_mode == DEBUG_ON) {
            fprintf(stderr, "[minishell debug] reader child running with PID %d\n", getpid());
        }

        close(pipefd[1]);
        if ((dup2(pipefd[0], STDIN_FILENO) == -1)) {
            perror("minishell: Pipe failed");
            exit(EXIT_FAILURE);
        }

        if (execvp(right_args[0], right_args) == -1) {
            perror("minishell: Pipe exec failed");
            exit(EXIT_FAILURE);
        }
    } else {
        if (debug_mode == DEBUG_ON) {
            fprintf(stderr, "[minishell debug] parent forked reader child PID %d\n", pid_reader);
        }
    }

    close(pipefd[0]);
    close(pipefd[1]);

    waitpid(pid_writer, &status, 0);
    if (debug_mode == DEBUG_ON) {
        fprintf(stderr, "[minishell debug] writer child finished PID: %d\n", pid_writer);
    }

    waitpid(pid_reader, &status, 0);
    if (debug_mode == DEBUG_ON) {
        fprintf(stderr, "[minishell debug] reader child finished PID: %d\n", pid_reader);
    }

    return 1;
}

char *builtin_str[] = {"cd",   "help",    "exit",  "pwd",    "clear",
                       "echo", "version", "debug", "google", "close", "weather"};

int (*builtin_func[])(char **) = {&sh_cd,   &sh_help,    &sh_exit,  &sh_pwd,    &sh_clear,
                                  &sh_echo, &sh_version, &sh_debug, &sh_google, &sh_close, &sh_weather};

int sh_num_builtins()
{
    return sizeof(builtin_str) / sizeof(builtin_str[0]);
}
int sh_debug(char **args)
{
    if (args[1] == NULL) {
        fprintf(stderr, "minishell: Expected argument to \"debug (on/off)\" \n");
        return 1;
    } else if (strcmp(args[1], "on") == 0) {
        debug_mode = DEBUG_ON;
        printf("minishell: debug turned on, use \"debug off\" to turn off \n");
        return 1;
    } else if (strcmp(args[1], "off") == 0) {
        debug_mode = DEBUG_OFF;
        printf("minishell: debug turned off, use \"debug on\" to turn on \n");
        return 1;
    } else {
        fprintf(stderr, "minishell: Invalid argument to \"debug (on/off)\" \n");
        return 1;
    }
}
int sh_weather(char **args){
    // example curl "wttr.in/Lund?format=3"
    if(args[1] == NULL){
        fprintf(stderr,"minishell: expected location argument after command: weather");
        return 1;
    }

    char url[2048];

    char *url_part1 = "wttr.in/";
    char *url_part2 = "?format=3";
    char *location = args[1];

    sprintf(url, "%s%s%s", url_part1, location, url_part2);

    pid_t pid = fork();

    if(pid == 0){
        char *args[] = {
            "curl",
            "-s",
            url,
            NULL
        };
        execvp(args[0],args);
        perror("execvp failed");
        return 1;
    } else if(pid < 0){
        perror("minishell: fork failed");
        return 1;
    }

    return 0;
}

int sh_google(char **args)
{
    char url[2048] = "https://www.google.com/search?q=";

    for (int i = 1; args[i] != NULL; i++) {
        if (i > 1) {
            strcat(url, "+");
        }
        strcat(url, args[i]);
    }

    pid_t pid = fork();

    if (pid == 0) {
        if (debug_mode == DEBUG_OFF) { 
            int fd = open("/dev/null", O_WRONLY); // write to void to avoid writing out potential messages from the browser

            if (fd == -1) {
                perror("minishell: failed to access /dev/null");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDERR_FILENO);
            close(fd);
        }
        execlp("xdg-open", "xdg-open", url, NULL);
        perror("minishell: google");
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("minishell: fork failed");
    }

    

    return 1;
}

int sh_cd(char **args)
{
    if (args[1] == NULL) {
        fprintf(stderr, "minishell: Expected argument to \"cd\" \n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("minishell");
        }
    }
    return 1;
}

int sh_help(char **args)
{
    int i;
    printf("Unix minishell \n");
    printf("Type program names and arguments and then hit ENTER to proceed. \n");
    printf("The following are built in: \n");

    for (i = 0; i < sh_num_builtins(); i++) {
        printf(" %s\n", builtin_str[i]);
    }

    printf("Use the man command followed by the program name for instructions on usage (Not "
           "implemented yet) \n");
    return 1;
}

int sh_pwd(char **args)
{
    char cwd[1024];

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("minishell");
    }

    return 1;
}

int sh_clear(char **args)
{
    printf("\033[2J\033[H");
    return 1;
}

int sh_version(char **args)
{
    printf("%s", CURRENT_VERSION);
    printf("\n");
    return 1;
}

int sh_echo(char **args)
{
    int i = 1;

    while (args[i] != NULL) {
        printf("%s", args[i]);

        if (args[i + 1] != NULL) {
            printf(" ");
        }
        i++;
    }

    printf("\n");
    return 1;
}

int sh_exit(char **args)
{
    return 0;
}

int sh_close(char **args)
{
    return 0;
}

int sh_execute(char **args)
{
    int i = 0;
    char *output_file = NULL;
    int bool_append = 0;

    while (args[i] != NULL) {
        if (strcmp(args[i], "|") == 0) {
            args[i] = NULL;
            return sh_launch_pipe(&args[0], &args[i + 1]);
        }
        i++;
    }

    i = 0; // if no pipe was found

    while (args[i] != NULL) {
        if (strcmp(args[i], ">") == 0) {
            if (args[i + 1] == NULL) {
                fprintf(stderr, "minishell: expected filename after >\n");
                return 1;
            }

            output_file = args[i + 1];
            args[i] = NULL;
            break;
        }

        if (strcmp(args[i], ">>") == 0) {
            if (args[i + 1] == NULL) {
                fprintf(stderr, "minishell: expected filename after >>\n");
                return 1;
            }

            output_file = args[i + 1];
            bool_append = 1;
            args[i] = NULL;
            break;
        }

        i++;
    }

    if (args[0] == NULL) {
        return 1;
    }

    for (i = 0; i < sh_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            if (output_file == NULL) {
                return (*builtin_func[i])(args);
            }

            int saved_stdout = dup(STDOUT_FILENO);
            if (saved_stdout == -1) {
                perror("minishell");
                return 1;
            }
            int fd;
            if (bool_append == 0) {
                fd = open(output_file, O_WRONLY | O_TRUNC | O_CREAT, 0644);
            } else {
                fd = open(output_file, O_WRONLY | O_APPEND | O_CREAT, 0644);
            }

            if (fd == -1) {
                perror("minishell");
                return 1;
            }

            if (dup2(fd, STDOUT_FILENO) == -1) {
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

    return sh_launch(args, output_file, bool_append);
}

int main(int argc, char **argv)
{
    sh_loop();
    return EXIT_SUCCESS;
}