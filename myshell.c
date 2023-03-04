#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <ctype.h>
#include "toml.h"

#define ANSI_RESET      "\e[0m"
#define ANSI_GREEN      "\e[1;32m"
#define ANSI_RED        "\e[1;31m"
#define LINE_MAX        4096
#define PATH_MAX        2048
#define ARGS_MAX        LINE_MAX

typedef const char *Colour;

/**
 * Prints an error message in red and returns an exit code for failure.
 * @param fmt the format to follow.
*/
int error(char fmt[], ...)
{
    va_list args;
    va_start(args, fmt);

    fprintf(stderr, ANSI_RED);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, ANSI_RESET);

    va_end(args);
    return -1;
}

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
    if (read >= max - 1) fflush(stdin);
    buff[read] = '\0';
    return read;
}

/**
 * Parse a line into a structure containing argc and argv.
 * @param line the line from which to parse argv.
*/
void parse_args(char *line, char *argv[ARGS_MAX + 1])
{
    int argc = 0;
    line = strtok(line, " ");
    while (line != NULL) {
        argv[argc++] = line;
        line = strtok(NULL, " ");
    }

    argv[argc] = NULL;
}

/**
 * Starts a process and passes command line arguments to it.
 * @param name the name of the process
 * @param argv the array of command line arguments
*/
void run_process(char name[], char *argv[ARGS_MAX + 1])
{
    pid_t proc = fork();
    if (proc == 0) {
        execvp(name, argv);
        // Execution will only reach this point if the process fails.
        error("%s "ANSI_RESET"is not a valid command or script.\n", argv[0]);
        exit(1);
    } else if (proc < 0) {
        // If the process fails to start
        error("Failed to start process.\n");

    } else {
        // If running from the parent process, wait for the child process to finish.
        waitpid(proc, NULL, 0);   
    }
}

/**
 * Loads the config data from the users computer. The config data will include
 * things such as prompt colour and more.
 * @param config_path the path to the config file
*/
toml_table_t *load_config(const char *config_path)
{
    FILE *config_fp = fopen(config_path, "r");
    if (config_fp == NULL) {
        return NULL;
    }

    toml_table_t *conf = toml_parse_file(config_fp, NULL, 0);
    fclose(config_fp);
    
    return conf;
}

/**
 * Search the config file for a prompt colour and return that colour.
 * @param conf the TOML table containing the colour data.
*/
Colour get_prompt_colour(toml_table_t *conf)
{
    toml_table_t *colour_data = toml_table_in(conf, "colours");
    if (colour_data == NULL) {
        return NULL;
    }

    Colour prompt_colour = NULL;
    toml_datum_t prompt_colour_data = toml_string_in(colour_data, "prompt");

    if (strcmp(prompt_colour_data.u.s, "green") == 0) {
        prompt_colour = ANSI_GREEN;
    } else if (strcmp(prompt_colour_data.u.s, "red") == 0) {
        prompt_colour = ANSI_RED;
    }

    free(prompt_colour_data.u.s);
    return prompt_colour;
}

/**
 * Parse the path to the user's config file using the USER 
 * environment variable.
 * @param config_path the buffer in which to store the path
*/
void parse_config_path(char config_path[PATH_MAX + 1])
{
    snprintf(
        config_path, PATH_MAX,
        "/home/%s/.config/myshell/myshellconfig.toml",
        getenv("USER")
    );
}

/**
 * Converts a line to a command an arguments, and then runs it.
 * @param line the raw line entered by the user
*/
void process_line(char *line)
{   
    char *argv[ARGS_MAX + 1];
    memset(argv, 0, (ARGS_MAX + 1) * sizeof(char*));
    parse_args(line, argv);

    if (argv == NULL) {
        exit(error("OUT OF MEMORY"));
    }

    // If a blank line is passed.
    if (argv[0] == NULL) return;

    if (strcmp(argv[0], "exit") == 0) {
        exit(0);
    } else if (strcmp(argv[0], "cd") == 0) {
        if (argv[1] != NULL && chdir(argv[1]) != 0) {
            error("%s "ANSI_RESET"is not a directory.\n", argv[1]);
        }
    } else {
        run_process(argv[0], argv);
    }
}

int main()
{
    char line[LINE_MAX + 1];
    char cwd[PATH_MAX + 1];

    Colour prompt_colour = ANSI_RED;

    char config_path[PATH_MAX + 1];
    parse_config_path(config_path);

    toml_table_t *conf = load_config(config_path);
    if (conf != NULL) {
        Colour c = get_prompt_colour(conf);
        prompt_colour = c ? c : ANSI_GREEN;
        toml_free(conf);
    }

    while (1) {
        getcwd(cwd, PATH_MAX + 1);
        printf("%s~%s"ANSI_RESET" $ ", prompt_colour, cwd);
        readline(line, LINE_MAX + 1);
        process_line(line);
    }

    return 0;
}