/// file: mysh.c
/// author: Michael Kha, mxk5025@rit.edu
/// purpose: Program that acts as a terminal shell. Executes internal commands
/// and forks external commands to be executed by child processes.
#define _GNU_SOURCE // getline, strdup
#include <stdlib.h> // malloc, calloc, free
#include <stdio.h>  // print
#include <string.h> // strcat, strcmp, strcpy, strtok
#include <unistd.h> // fork, exec
#include <ctype.h> // isspace
#include <sys/types.h> // pid_t
#include <sys/wait.h> // wait
#include "mysh.h" // header file
#include "queue.h" // queue struct for history list

// Constants
const size_t ARG_BUFFER = 256;
const size_t MAX_ARGS = 10;
const size_t DEFAULT_HISTORY = 10;
const char *path = "/bin/";
const char *help_command = "help";
const char *history_command = "history";
const char *quit_command = "quit";
const char *verbose_command = "verbose";

// file-scope variables
static Queue hist;
static unsigned int vflag;
static unsigned int sequence;

// Method declarations
void run();

// Helper methods
static int is_internal(char *command);
static int is_empty(const char *s);
static void reset_arguments(int argc, char **argv);
static int set_arguments(char *line, char **argv);

// Internal commands as functions

/// Internal command that re-executes a command in the history.
/// The command is performed within this method.
/// @param argc The number of arguments
/// @param argv The arguments
/// @return The status code of the bang command
int bang(int argc, char *argv[]) {
    if (argc == 0) {
        return -1;
    }
    int status = 1;
    char *line = argv[0];
    line++;
    int comm_idx = (int) strtol(line, (char **)NULL, 10);
    int size = (int)hist->size;
    int hist_idx = comm_idx - sequence + size + 1;
    // Valid index is within the bounds of 0 and the size of the list
    if (hist_idx >= 0 && hist_idx < size) {
        // Re-execute the command
        char *line = (char *)hist->contents[hist_idx];
        char **i_argv = (char **) calloc(MAX_ARGS, sizeof(char) * ARG_BUFFER);
        int i_argc = set_arguments(line, i_argv);
        status = execute_command(i_argc, i_argv);
        free(i_argv);
    }
    // Add bang command to history
    enqueue(hist, (void *)strdup(argv[0]));
    return status;
}

/// Internal command that prints out the internal commands with descriptions.
/// @param argc The number of arguments
/// @param argv The arguments
/// @return The status code of the help command
int help(int argc, char *argv[]) {
    if (argc == 0 && strcmp(argv[0], help_command) != 0) {
        return -1;
    }
    printf("Internal Commands for MYSH:\n"
    "help             # prints a list of internal commands\n"
    "history          # prints a lit of commands executed so far, including their arguments\n"
    "!N               # re-executes the Nth command in the history list\n"
    "verbose on | off # turns the shell verbose on or off\n"
    "quit             # clean up memory and gracefully terminate the shell\n");
    return 1;
}

/// Internal command that prints out the history of commands.
/// @param argc The number of arguments
/// @param argv The arguments
/// @return The status code of the history command
int history(int argc, char *argv[]) {
    if (argc == 0 && strcmp(argv[0], history_command) != 0) {
        return -1;
    }
    size_t size = hist->size;
    char *line;
    for (size_t i = 0; i < size; i++) {
        line = (char *)hist->contents[i];
        printf("%ld: %s\n", sequence - size + i, line);
    }
    return 1;
}

/// Quit command that lets the shell know to clean up memory and terminate.
/// @param argc The number of arguments
/// @param argv The arguments
/// @return The status code of the quit command
int quit(int argc, char *argv[]) {
    if (argc == 0 && strcmp(argv[0], quit_command) != 0) {
        return -1;
    }
    // Return 0 to let caller know it is time to free memory
    return 0;
}

/// Internal command that sets the verbose mode. The mode is set using on or off.
/// @param argc The number of arguments
/// @param argv The argument that sets the verbose mode
/// @return The status code of the verbose command
int verbose(int argc, char *argv[]) {
    if (argc <= 1  && strcmp(argv[0], verbose_command) != 0) {
        fprintf(stderr, "usage: verbose on | off\n");
        return -1;
    }
    if (strcmp(argv[1], "on") == 0) {
        vflag = 1;
    }
    else if (strcmp(argv[1], "off") == 0) {
        vflag = 0;
    }
    else {
        fprintf(stderr, "usage: verbose on | off\n");
        return 1;
    }
    return 1;
}

/// Resets the arguments or the array of characters.
/// @param argc The number of arguments
/// @param argv The arguments to reset to NULL
static void reset_arguments(int argc, char **argv) {
    for (int i = 0; i < argc; i++) {
        argv[i] = NULL;
    }
}

/// Read a line of input and create arguments from that input. Arguments
/// with quotes are treated as a single argument.
/// @param line The line of input to read
/// @param argv The arguments to set from the line
/// @return The number of arguments found
static int set_arguments(char *line, char **argv) {
    int argc = 0;
    char *arg = strtok(line, " ");
    while (arg != NULL && arg[0] != '\0' && !is_empty(arg)) {
        //printf("arg: %s\n", arg);
        if (arg[0] == '\'' || arg[0] == '\"') {
            char *end = strtok(NULL, "\"\'");
            // Build quoted argument with temp
            char temp[strlen(arg) + strlen(end) + 2];
            strcpy(temp, arg);
            strcat(temp, " ");
            strcat(temp, end);
            // Remove leading quote with pointer
            char *word = temp;
            word++;
            argv[argc++] = word;
        } else {
            argv[argc++] = arg;
        }
        arg = strtok(NULL, " ");
    }
    return argc;
}

/// Check if the command is an internal command.
/// @param command The command to check
/// @return If the command is an internal command
static int is_internal(char *command) {
    if (strcmp(command, help_command) == 0 ||
            strcmp(command, history_command) == 0 ||
            strcmp(command, quit_command) == 0 ||
            strcmp(command, verbose_command) == 0) {
        return 1;
    }
    // Bang command
    if (command[0] == '!') {
        return 1;
    }
    return 0;
}

/// Processes an internal command by calling the correct command to run.
/// If no command matches, returns 0.
/// @param argc The number of arguments of an internal command
/// @param argv The arguments of an internal command
/// @return The status code of the internal command run
int process_internal(int argc, char *argv[]) {
    if (argc == 0) {
        return -1;
    }
    else if (strcmp(argv[0], help_command) == 0) {
        return help(argc, argv);
    }
    else if (strcmp(argv[0], history_command) == 0) {
        return history(argc, argv);
    }
    else if (strcmp(argv[0], quit_command) == 0) {
        return quit(argc, argv);
    }
    else if (strcmp(argv[0], verbose_command) == 0) {
        return verbose(argc, argv);
    }
    else if (argv[0][0] == '!') {
        return bang(argc, argv);
    }
    return 0;
}

/// Processes an external command. The main process forks a child
/// process to execute the command.
/// @param argc The number of arguments of an external command
/// @param argv The arguments of an external command
/// @return The status code of the external command run
int process_external(int argc, char *argv[]) {
    if (argc == 0) {
        return -1;
    }
    int wait_status;
    char *command = argv[0];
    char file[strlen(path) + strlen(command) + 1];
    strcpy(file, path);
    strcat(file, argv[0]);
    // make a fork...store pid_t
    pid_t pid = fork();
    if (pid == 0) {
        if (vflag) {
            printf("\twait for pid %d: %s\n", getpid(), command);
            printf("\texecvp: %s\n", command);
        }
        // execute the file with the arguments
        int exec_status = execvp(file, argv);
        // Prints if error occurred and did not terminate
        if (exec_status == -1) {
            fprintf(stderr, "%s: No such file or directory\n", command);
        }
        fprintf(stderr, "command status: 1\n");
        return 0;
    }
    // check status of child through wait
    else {
        if (wait(&wait_status) < 0) {
            perror("Fatal error occurred");
            return 0;
        }
        // If the child did not exit normally
        if (!(WIFEXITED(wait_status) && WEXITSTATUS(wait_status) == 0)) {
            fprintf(stderr, "command status: 2\n");
        }
    }
    return 1;
}

/// Executes an internal or external command.
/// @param argc The number of arguments of the command
/// @param argv The arguments of the command
int execute_command(int argc, char *argv[]) {
    if (argc == 0) {
        return -1;
    }
    // Process an internal or external command
    return is_internal(argv[0]) ? process_internal(argc, argv) :
            process_external(argc, argv);
}

/// Check if the string is only white space or not.
/// @param s The string to check each character for
/// @return If the string does not have real characters
static int is_empty(const char *s) {
    while (*s != '\0') {
        if (!isspace((unsigned char)*s))
            return 0;
        s++;
    }
    return 1;
}

/// Run the loop for the shell program. The program continues to read user
/// input and executes commands. Commands may be internal or external. An
/// external command is run by forking a process to execute the command.
/// The main process then waits for the child process to finish executing
/// before continuing.
void run() {
    // Initialize input variables
    ssize_t nread;
    size_t BUFFER = ARG_BUFFER;
    char *line = malloc(sizeof(char) * BUFFER);
    int i_argc;
    char **i_argv = (char **) calloc(MAX_ARGS, sizeof(char) * ARG_BUFFER);
    while (1) {
        printf("mysh[%d]> ", sequence++);
        nread = getline(&line, &BUFFER, stdin);
        // Control-D was read; exit
        if (nread == -1) {
            printf("\n");
            break;
        }
        // New line was read; continue
        else if (nread == 1) {
            continue;
        }
        // Strip new lines
        line = strtok(line, "\n");
        // Add to the history for all command except bang
        if (line[0] != '!') {
            enqueue(hist, (void *)strdup(line));
        }
        // Build i_argc and i_argv from input
        i_argc = set_arguments(line, i_argv);
        // Print information if verbose is on
        if (vflag) {
            printf("\tinput command tokens:\n");
            for (int i = 0; i < i_argc; i++) {
                printf("\t%d: %s\n", i, i_argv[i]);
            }
        }
        // Execute the command
        int status = execute_command(i_argc, i_argv);
        if (status == 0) {
            break;
        }
        // Reset for next iteration
        reset_arguments(i_argc, i_argv);
    }
    free(line);
    free(i_argv);
    destroy_queue(hist);
}

/// Initializes the program state and then begin running the loop.
/// Reads possible command line option flags and then creates the history queue.
/// @param argc Number of command line arguments
/// @param argv Optional flags to run the shell with
int main(int argc, char *argv[]) {
    // Read optional flags
    const char* optstring = "vh:";
    size_t history_capacity = DEFAULT_HISTORY;
    int opt;
    vflag = 0;
    while ((opt = getopt(argc, argv, optstring)) != -1) {
        switch (opt) {
            case 'v':
                vflag = 1;
                break;
            case 'h':
                history_capacity = (int) strtol(optarg, (char **)NULL, 10);
                if (history_capacity <= 0) {
                    fprintf(stderr, "usage: mysh [-v] [-h pos_num]\n");
                    return 0;
                }
                break;
        }
    }
    // Initialize history state
    hist = create_queue(history_capacity);
    sequence = 1;
    // Run the program loop
    run();
    return 0;
}

