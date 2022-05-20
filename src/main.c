#include "cli.h"
#include "debug.h"
#include "serial.h"
#include "util.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <unistd.h>

// TODO:
// This file desparately needs to be refactored and cleaned.
// The UI leading up to the launch of the actual debugger could
// use some improvement as well.
//
// Also, the port should probably be initialized with some sort of structure
// instead of globals.

void usage(char *msg);
void parse_args(int argc, char *argv[], char **path);
void start_debugger(char *path);
void autodetect_and_start();

int main(int argc, char *argv[]) {
    char *term_path;

    parse_args(argc, argv, &term_path);

    if (term_path == NULL) {
        usage("Specify device");
    } else {
        start_debugger(term_path);
    }
}

void usage(char *msg) {
    if (msg != NULL)
        fprintf(stderr, "%s\n", msg);

    fprintf(stderr, "Usage: rvdb [serial port]\n");
    exit(EXIT_FAILURE);
}

void parse_args(int argc, char *argv[], char **path) {

    if (argc == 1) {
        *path = NULL;
    }

    else if (argc > 2) {
        usage("Error: too many arguments");
        exit(EXIT_FAILURE);
    }

    else if (match_strs(argv[1], "-h") || match_strs(argv[1], "--help")) {
        printf(HELP_MSG);
        exit(EXIT_SUCCESS);
    }

    else
        *path = argv[1];
}

void start_debugger(char *path) {
    int serial_port = -1;

    if (open_serial(path, &serial_port)) {
        fprintf(stderr, "Error: could not open serial port\n");
        exit(EXIT_FAILURE);
    }

    if (connection_test(serial_port, 16, 0, 0)) {
        fprintf(stderr, "Error: could not open a stable connection\n");
        exit(EXIT_FAILURE);
    }

    printf(
        "\nA stable connection has been established. Launching debugger...\n");

    // launch debug cli on device at serial_port
    debug_cli(path, serial_port);
    close(serial_port);
    restore_term(serial_port);
}

int try_open(char *path) {
    int serial_port;

    if (open_serial(path, &serial_port))
        return 0;
    else if (connection_test(serial_port, 1, 0, 1)) {
        restore_term(serial_port);
        close(serial_port);
        return 0;
    } else {
        close(serial_port);
        return 1;
    }
}
