#include "history.h"
#include "constants.h"
#include "write.h"
#include <unistd.h>
#include <string.h>

#define HISTORY_DEPTH 10

char history[HISTORY_DEPTH][COMMAND_LENGTH];
int command_numbers[HISTORY_DEPTH];
int history_size = 0;
int current_command_number = 1;

// shift the last (history_size - 1) entries to make space for a new entry.
// does nothing if the current history size is < 2.
static void shift_history() {
    for (int i = 1; i < history_size; ++i) {
        strcpy(history[i - 1], history[i]);
        command_numbers[i - 1] = command_numbers[i];
    }
}

void add_to_history(const char *command) {
    if (history_size < HISTORY_DEPTH) {
        strcpy(history[history_size], command);
        command_numbers[history_size] = current_command_number;
        history_size++;
    } else {
        shift_history();
        strcpy(history[HISTORY_DEPTH - 1], command);
        command_numbers[HISTORY_DEPTH - 1] = current_command_number;
    }
    current_command_number++;
}

void display_history() {
    for (int i = 0; i < history_size; ++i) {
        write_int(STDOUT_FILENO, command_numbers[i]);
        write_string(STDOUT_FILENO, "\t");
        write_string(STDOUT_FILENO, history[i]);
        write_string(STDOUT_FILENO, "\n");
    }
}

const char *get_command(int command_number) {
    const char *command = NULL;
    for (int i = 0; i < history_size; ++i) {
        if (command_numbers[i] == command_number) {
            command = history[i];
        }
    }
    return command;
}

int get_last_command_number() {
    if (history_size == 0) {
        return 0;
    }
    return command_numbers[history_size - 1];
}
