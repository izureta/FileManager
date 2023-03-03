#include "window.h"

void init_window(struct window_state *state) {
    initscr();
    if (!has_colors()) {
        handle_fatal_error("Your terminal doesn't support colors");
    }

    nodelay(stdscr, true);

    getmaxyx(stdscr, state->height, state->width);

    if (state->height < 2 || state->width < MIN_WIDTH) {
        handle_fatal_error("The window of your terminal is too small");
    }

    start_color();
    cbreak();
    noecho();

    init_colors();

    curs_set(0);
    keypad(stdscr, true);
    nl();
}
