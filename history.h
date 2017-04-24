#ifndef HISTORY_H
#define HISTORY_H

// adds the given command string to the history.
void add_to_history(const char *command);

// writes the history to standard output.
// writes nothing if the history is empty
void display_history();

// returns the command string in the history with the given command number.
// returns NULL if there is no command with the given command number in the
// history.
const char *get_command(int command_number);

// returns the command number of the last/latest command in the history.
// returns 0 if the history is empty.
int get_last_command_number();

#endif
