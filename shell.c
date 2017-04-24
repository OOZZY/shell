#include "constants.h"
#include "write.h"
#include "history.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

// custom signal handler for SIGINT.
void handle_SIGINT() {
    write_string(STDOUT_FILENO, "\n");
    display_history();
}

// replaces all spaces in buff with the null terminator.
// assumes given length is the length of buff.
void replace_spaces_with_null_terminator(char *buff, int length) {
    for (int i = 0; i < length; ++i) {
        if (buff[i] == ' ') {
            buff[i] = '\0';
        }
    }
}

// modifies the beginning elements in the tokens array to point to the tokens
// in buff. then modifies the next element in the tokens array to NULL to
// signify the end of the tokens.
// assumes tokens in buff are separated by null terminators.
// assumes given length is the length of buff.
int tokenize_command(char *buff, int length, char *tokens[]) {
    int token_count = 0;

    // if buff starts with a token, get the token
    if (buff[0] != '\0') {
        tokens[token_count] = buff;
        token_count++;
    }

    // get more tokens in buff
    for (int i = 0; i < length; ++i) {
        // if current character is a null terminator and the next one is not,
        // then the next one is the start of a new token
        if ((buff[i] == '\0') && (i+1 < length) && (buff[i + 1] != '\0')) {
            tokens[token_count] = buff + i + 1;
            token_count++;
        }
    }

    tokens[token_count] = NULL;
    return token_count;
}

// from the given tokens, extracts whether the command should run in the
// background. stores the value in the bool pointed to by in_background.
// if the last token is "&", removes that token from the tokens array.
// assumes token_count is the number of elements/tokens in the tokens array.
void extract_ampersand(char *tokens[], int token_count, _Bool *in_background) {
    if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) {
        *in_background = true;
        tokens[token_count - 1] = NULL;
    }
}

// following variable is used to store the original/unchanged user input.
char current_command[COMMAND_LENGTH];

// reads the user input into buff. separates each token in buff with null
// terminators. modifies tokens array to point to tokens in buff. modifies
// the bool pointed to by in_background to whether the user command should
// be run in the background.
void read_command(char *buff, char *tokens[], _Bool *in_background) {
    *in_background = false;

    // read input
    int length = read(STDIN_FILENO, buff, COMMAND_LENGTH - 1);

    // if read fails but NOT because it was interrupted by a signal
    if ((length < 0) && (errno != EINTR)) {
        write_string(STDERR_FILENO, "Unable to read command. Terminating.\n");
        exit(EXIT_FAILURE); // terminate with error
    }

    // if read fails because it was interrupted by a signal
    if ((length < 0) && (errno == EINTR)) {
        // clear out input buffer and tokens so that shell will behave as if
        // the user didn't input anything. if we don't do this, the previous
        // command will still be in the input buffer and tokens and so the
        // shell will incorrectly execute the previous command
        buff[0] = '\0';
        current_command[0] = '\0';
        tokens[0] = NULL;
        return;
    }

    // null terminate and strip \n
    buff[length] = '\0';
    if (buff[strlen(buff) - 1] == '\n') {
        buff[strlen(buff) - 1] = '\0';
    }

    memcpy(current_command, buff, length); // save original command
    replace_spaces_with_null_terminator(buff, length);

    // tokenize (saving original command string)
    int token_count = tokenize_command(buff, length, tokens);

    // extract if running in background:
    extract_ampersand(tokens, token_count, in_background);
}

// writes the given tokens to standard output
void print_tokens(char *tokens[]) {
    for (int i = 0; i < NUM_TOKENS && tokens[i] != NULL; ++i) {
        write_string(STDOUT_FILENO, "|");
        write_string(STDOUT_FILENO, tokens[i]);
        write_string(STDOUT_FILENO, "|");
    }
    write_string(STDOUT_FILENO, "\n");
}

// if buff and tokens indicate a history command (that starts with !), this
// function will overwrite buff, tokens, and in_background to their appropriate
// values based on the command string stored in the history.
// returns 0 if no error occurs.
// returns -1 if an error occurs.
int handle_history_command(char *buff, char *tokens[], _Bool *in_background) {
    if (tokens[0] != NULL && tokens[0][0] == '!') {
        int command_number;

        if (tokens[0][1] == '!') {
            command_number = get_last_command_number();
        } else {
            command_number = atoi(tokens[0] + 1);
        }

        if (command_number < 1) {
            write_string(STDERR_FILENO, "invalid command number\n");
            return -1;
        }

        const char *command = get_command(command_number);
        if (!command) {
            write_string(STDERR_FILENO, "invalid command number\n");
            return -1;
        }

        // overwrite current command with the one from history
        strcpy(current_command, command);

        // overwrite input buffer with the one from history. then separate
        // each token in the buffer with null terminators
        strcpy(buff, command);
        int length = strlen(buff);
        replace_spaces_with_null_terminator(buff, length);

        // overwrite tokens array and overwrite in_background
        int token_count = tokenize_command(buff, length, tokens);
        extract_ampersand(tokens, token_count, in_background);

        // write the command from history so the user can see what command they
        // are actually running
        write_string(STDOUT_FILENO, command);
        write_string(STDOUT_FILENO, "\n");
    }

    return 0;
}

// execute the command indicated by buff and tokens.
// assumes cwd stores the current working directory.
// assumes in_background stores whether the command should run in the
// background.
void process_command(char *cwd, char *buff, char *tokens[], _Bool *in_background) {
    int error = handle_history_command(buff, tokens, in_background);

    if (!error) {
        _Bool save_to_history = true;
        if (tokens[0] == NULL) { // empty input
            save_to_history = false;
        } else if (strcmp(tokens[0], "exit") == 0) {
            exit(EXIT_SUCCESS);
        } else if (strcmp(tokens[0], "pwd") == 0) {
            write_string(STDOUT_FILENO, cwd);
            write_string(STDOUT_FILENO, "\n");
        } else if (strcmp(tokens[0], "cd") == 0) {
            int error = chdir(tokens[1]);
            if (error) {
                write_string(STDERR_FILENO, "cd failed\n");
            }
        } else if (strcmp(tokens[0], "history") == 0) {
            save_to_history = false;
            add_to_history(current_command);
            display_history();
        } else { // non-internal commands
            pid_t pid = fork();
            if (pid >= 0) { // fork succeeded
                if (pid == 0) { // child
                    int error = execvp(tokens[0], tokens);
                    if (error) {
                        write_string(STDERR_FILENO, "execvp failed\n");
                        exit(EXIT_FAILURE);
                    }
                } else { // parent
                    if (!*in_background) {
                        int status;
                        waitpid(pid, &status, 0);
                    }
                }
            } else {
                write_string(STDERR_FILENO, "fork failed\n");
                exit(EXIT_FAILURE);
            }
        }

        if (save_to_history) {
            add_to_history(current_command);
        }
    }

    // cleanup any previously exited background child processes (zombies)
    while (waitpid(-1, NULL, WNOHANG) > 0) {}
}

#define CWD_CAPACITY 4096

int main() {
    // register custom signal handler for SIGINT
    struct sigaction handler;
    handler.sa_handler = handle_SIGINT;
    handler.sa_flags = 0;
    sigemptyset(&handler.sa_mask);
    sigaction(SIGINT, &handler, NULL);

    // shell's read/process loop
    char cwd[CWD_CAPACITY];
    _Bool in_background;
    char input_buffer[COMMAND_LENGTH];
    char *tokens[NUM_TOKENS];
    while (true) {
        getcwd(cwd, CWD_CAPACITY);
        write_string(STDOUT_FILENO, cwd);
        write_string(STDOUT_FILENO, "> ");
        in_background = false;
        read_command(input_buffer, tokens, &in_background);
        // print_tokens(tokens);
        process_command(cwd, input_buffer, tokens, &in_background);
    }

    return 0;
}
