#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define ANSI_RESET      "\e[0m"
#define ANSI_GREEN      "\e[1;32m"
#define ANSI_RED        "\e[1;31m"
#define LINE_MAX        4096
#define PATH_MAX        2048

/**
 * Read a single line from stdin. Allocates memory so don't
 * forget to free it.
 * @param buff the string to write to.
 * @param max the number of characters to read plus one extra character for null byte.
*/
int readline(char buff[], size_t max)
{
    int c, read = 0;
    while (read < max - 1 && (c = getchar()) != EOF && c != '\n') {
        buff[read++] = c;
    }
    buff[read] = '\0';
    return read;
}

/**
 * Parse a line into a structure containing argc and argv.
 * @param line the line from which to parse argv.
*/
char **parse_args(char *line)
{   
    int block_size = 16, block = block_size;
    int argc = 0;
    char **argv = calloc(block + 1, sizeof(char*));
    if (argv == NULL) {
        return NULL;
    }

    // Split the input line into an argument vector.
    line = strtok(line, " ");
    while (line != NULL) {
        if (argc == block) {
            block += block_size;
            argv = realloc(argv, sizeof(char*) * block);
        }

        argv[argc++] = line;
        line = strtok(NULL, " ");
    }

    argv[argc] = NULL;
    return argv;
}

/**
 * Starts a process and passes command line arguments to it.
 * @param name the name of the process
 * @param argv the array of command line arguments
*/
void run_process(char name[], char *argv[])
{
    pid_t proc = fork();
    if (proc == 0) {
        execvp(name, argv);
        // Execution will only reach this point if the process fails.
        printf(ANSI_RED"%s "ANSI_RESET"is not a valid command or script.\n", argv[0]);
        exit(1);
    } else if (proc < 0) {
        // If the process fails to start
        fprintf(stderr, "Failed to start process.\n");
    } else {
        // If running from the parent process, wait for the child process to finish.
        waitpid(proc, NULL, 0);   
    }
}

int main()
{
    char line[LINE_MAX];
    char cwd[PATH_MAX];

    while (1) {
        getcwd(cwd, PATH_MAX);
        printf(ANSI_GREEN"~%s"ANSI_RESET" $ ", cwd);
        if (readline(line, LINE_MAX) == 0) {
            // Failed to read line
        }

        char **argv = parse_args(line);
        if (argv == NULL) {
            fprintf(stderr, ANSI_RED"OUT OF MEMORY\n"ANSI_RESET);
            exit(-1);
        }

        if (argv[0] == NULL) {
            ;
        } else if (strcmp(argv[0], "exit") == 0) {
            // Exit the shell.
            free(argv);
            exit(0);
        } else if (strcmp(argv[0], "cd") == 0) {
            // Change the current working directory.
            if (argv[1] != NULL && chdir(argv[1]) != 0) {
                fprintf(stderr, ANSI_RED"%s is not a directory.\n"ANSI_RESET, argv[1]);
            }
        } else {
            // Run the given command.
            run_process(argv[0], argv);
        }

        free(argv);
    }

    return 0;
}