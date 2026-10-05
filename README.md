# minishell

A small Unix shell written in C.

`minishell` is just a fun shell project. Could it be considered a subset of the shell I run it in? Maybe. Was it fun to make? Yes. 

## Features

* Runs external programs using `fork()`, `execvp()`, `execlp()`, and `waitpid()`
* Built-in commands:

  * `cd`
  * `pwd`
  * `echo`
  * `clear`
  * `help`
  * `version`
  * `exit`
  * `google`
  * `debug`
  * `weather`
* Output redirection:

  * `>` overwrite a file
  * `>>` append to a file
* Supports one pipe between two commands:

  * `ls | grep main`
  * `echo hello | wc -c`
* Dynamically reads input and tokenizes commands
  
* Debug mode for showing what the shell does internally
* `google` command for opening a search query in the browser
* `weather` command for getting the weather of a city 

## Build and run

This project is designed for Linux systems and should be compiled and run in a Linux or POSIX-compatible environment.

```bash
gcc -Wall -Wextra -std=c11 main.c -o minishell
./minishell
```
## Requirements
* A POSIX-compatible environment

* GCC or another C compiler

* `curl` for the `weather` command

* `xdg-open` for the `google` command

## Example usage

```text
> pwd
/home/user/minishell

> echo Hello from minishell
Hello from minishell

> ls > files.txt

> echo another line >> files.txt

> ls | grep main
main
main.c

> google cute pictures of cats

This opens a Google search for "cute pictures of cats" in the host system's default browser.

> weather Lund
Lund: 🌦 +8°C

> cat main.c | grep fork
pid = fork();
pid_writer = fork();
pid_reader = fork();
```

## How it works

The shell follows the usual Unix process model:

1. Read a command from standard input.
2. Split the command into arguments.
3. Check whether it is a built-in command.
4. For external commands:

   * create a child process with `fork()`
   * redirect input or output when needed with `dup2()`
   * replace the child with the requested program using `execvp()`
   * wait for the child using `waitpid()`

For pipelines, the shell creates a pipe with `pipe()`, redirects the first command's standard output to the pipe, and redirects the second command's standard input from it.

## Current limitations

This is a learning project, not a full shell. It currently does not support:

* Multiple pipes, such as `a | b | c`
* Input redirection with `<`
* Combining pipes and file redirection
* Quoted arguments, such as `echo "hello world"`
* Background processes with `&`
* Environment-variable expansion
* Command history

## Possible next steps

* Add multiple-pipe support
* Add `<` input redirection
* Support quoted strings
* Add command history with arrow-key navigation
* Add better error handling for malformed commands such as `ls |`
* Split the project into multiple `.c` and `.h` files

## What I learned

This project helped me practice:

* Unix processes and process creation
* File descriptors
* Standard input, output, and error
* Pipes and inter-process communication
* Dynamic memory allocation
* Parsing command-line input
* POSIX system calls in C
* Making HTTP/HTTPS requests using `curl`
