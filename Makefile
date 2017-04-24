## This is a simple Makefile with lost of comments
## Check Unix Programming Tools handout for more info.

# Define what compiler to use and the flags.
CC=gcc
CXX=g++
CCFLAGS=-std=c99 -pedantic -Wall -Wextra -Werror -g -D_XOPEN_SOURCE

all: shell

shell: write.o history.o shell.o
	$(CC) -o shell *.o $(CCFLAGS)

# Compile all .c files into .o files
# % matches all (like * in a command)
# $< is the source file (.c file)
%.o: %.c
	$(CC) -c $(CCFLAGS) $<

clean:
	rm -f *.o shell
