/// file: mysh.h
/// author: Michael Kha, mxk5025@rit.edu
/// purpose: Defines the interface of an interactive shell

#ifndef SHELL
#define SHELL

/// Internal command that re-executes a command in the history.
/// @param argc The number of arguments
/// @param argv The arguments
/// @return The status code of the bang command
int bang(int argc, char *argv[]);

/// Internal command that prints out the internal commands with descriptions.
/// @param argc The number of arguments
/// @param argv The arguments
/// @return The status code of the help command
int help(int argc, char *argv[]);

/// Internal command that prints out the history of commands.
/// @param argc The number of arguments
/// @param argv The arguments
/// @return The status code of the history command
int history(int argc, char *argv[]);

/// Internal command that prints out the history of commands.
/// @param argc The number of arguments
/// @param argv The arguments
/// @return The status code of the quit command
int quit(int argc, char *argv[]);

/// Internal command that sets the verbose mode.
/// @param argc The number of arguments
/// @param argv The argument that sets the verbose mode
/// @return The status code of the verbose command
int verbose(int argc, char *argv[]);

/// Processes an internal command.
/// @param argc The number of arguments of an internal command
/// @param argv The arguments of an internal command
/// @return The status code of the internal command run
int process_internal(int argc, char *argv[]);

/// Processes an external command.
/// @param argc The number of arguments of an external command
/// @param argv The arguments of an external command
/// @return The status code of the external command run
int process_external(int argc, char *argv[]);

/// Executes an internal or external command.
/// @param argc The number of arguments of the command
/// @param argv The arguments of the command
/// @return The status code of the executed command
int execute_command(int argc, char *argv[]);

#endif

