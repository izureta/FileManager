#include "errors.h"
#include <errno.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

void handle_error(char *func) {
    erase();
    printw("%s: %s\n", func, strerror(errno));
    while (getch() == ERR) {
        ;
    }
}

void handle_custom_error(char *msg) {
    erase();
    printw("%s\n", msg);
    while (getch() == ERR) {
        ;
    }
}

void handle_fatal_error(char *msg) {
    erase();
    printw("%s\n", msg);
    while (getch() == ERR) {
        ;
    }
    endwin();
    exit(1);
}
