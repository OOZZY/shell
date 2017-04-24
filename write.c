#include "write.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>

void write_string(int fd, const char *string) {
    write(fd, string, strlen(string));
}

#define NUMBER_CAPACITY 1024

void write_int(int fd, int num) {
    char number[NUMBER_CAPACITY];
    sprintf(number, "%d", num);
    write(fd, number, strlen(number));
}
