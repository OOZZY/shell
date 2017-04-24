# shell

A simple Unix shell.

## Features

* External programs can be executed in the background
    * `<command> &` will execute the given command in the background
* Shell builtins
    * `exit` will exit the shell session
    * `pwd` will print the current working directory
    * `cd <directory>` will change the current working directory to the given directory
    * `history` will print the command history
        * The command history can also be printed by pressing Control-C
* Command history
    * Remembers only the last 10 commands of the current shell session
* History expansion
    * `!n` will execute the nth command in the command history
    * `!!` will execute the previous command

## Build Requirements

* GNU make
* C99 compiler (Makefile uses `gcc` by default)
* Unix-like environment (Linux, FreeBSD, Cygwin, etc.)

## Clone, Build, and Run

Clone into shell directory.

```
$ git clone --branch develop <url/to/shell.git>
```

Build.

```
$ cd shell
$ make
```

Run.

```
$ ./shell
```
