#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define ANSI_RESET      "\e[0m"
#define ANSI_GREEN      "\e[1;32m"
#define ANSI_RED        "\e[1;31m"
#define MIN_LINE_SIZE   32

/**
 * Read a single line from stdin. Allocates memory so don't
 * forget to free it.
*/
char *readline()
{
    int line_delta = 16;
    int line_size = MIN_LINE_SIZE;
    char *line = calloc(line_size + 1, sizeof(char));

    int c, r = 0;
    while ((c = getchar()) != EOF && c != '\n') {
        if (r == line_size) {
            line_size += line_delta;
            line = realloc(line, line_size + 1);
        }
        line[r++] = c;
    }

    line[r] = '\0';
    return line;
}

/**
 * Parse a line into a structure containing argc and argv.
 * @param line the line from which to parse argv.
*/
char **parse_args(char *line)
{   
    int block_size = 16, block = block_size;
    int argc = 0;
    char **argv = malloc(sizeof(char*) * block);

    char *tok = strtok(line, " ");
    while (tok != NULL) {
        if (argc == block) {
            block += block_size;
            argv = realloc(argv, sizeof(char*) * block);
        }

        argv[argc++] = tok;
        tok = strtok(NULL, " ");
    }

    return argv;
}

int main()
{
    char *cwd = getenv("PWD");

    while (1) {
        printf(ANSI_GREEN"%s"ANSI_RESET" $ ", cwd);
        char *line = readline();
        char **argv = parse_args(line);

        if (strcmp(line, "exit") == 0) {
            free(argv);
            free(line);
            break;
        }

        pid_t proc = fork();
        if (proc == 0) {
            execvp(line, argv);
            printf(ANSI_RED"%s "ANSI_RESET"is not a valid command or script.\n", argv[0]);
        }

        waitpid(proc, NULL, 0);

        free(argv);
        free(line);
    }

    return 0;
}