#ifndef WRITE_H
#define WRITE_H

// writes the given string to the file referred to by the given file descriptor
void write_string(int fd, const char *string);

// writes the given int to the file referred to by the given file descriptor
void write_int(int fd, int num);

#endif
