#ifndef MINISHELL_H
#define MINISHELL_H

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFSIZE 1024
#define TRUE 1
#define TOK_BUFSIZE 64
#define TOK_DELIM " \t\r\n\a"
#define CURRENT_VERSION "1.0.0"

#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN "\033[36m"
#define COLOR_WHITE "\033[37m"

typedef enum {
    DEBUG_OFF,
    DEBUG_ON
} DebugMode;

extern DebugMode debug_mode; // can be useful if main.c get split into multiple files

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
int sh_debug(char **args);
int sh_google(char **args);
int sh_close(char **args);

#endif