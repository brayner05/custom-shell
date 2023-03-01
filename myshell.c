#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define ANSI_RESET      "\e[0m"
#define ANSI_GREEN      "\e[1;32m"
#define ANSI_RED        "\e[1;31m"
#define LINE_MAX        4096

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
 * @param argv the array of command line arguments.
*/
void run_process(char name[], char *argv[])
{
    pid_t proc = fork();
    if (proc == 0) {
        execvp(name, argv);
        printf(ANSI_RED"%s "ANSI_RESET"is not a valid command or script.\n", argv[0]);
        exit(1);
    } else if (proc < 0) {
        fprintf(stderr, "Failed to start process.\n");
    } else {
        waitpid(proc, NULL, 0);   
    }
}

int main()
{
    char *cwd = getenv("PWD");
    char line[LINE_MAX];

    while (1) {
        printf(ANSI_GREEN"%s"ANSI_RESET" $ ", cwd);
        readline(line, LINE_MAX);
        char **argv = parse_args(line);

        if (strcmp(line, "exit") == 0) {
            free(argv);
            break;
        }

        run_process(argv[0], argv);
        free(argv);
    }

    return 0;
}