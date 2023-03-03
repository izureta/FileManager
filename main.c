#include "colors.h"
#include "errors.h"
#include "functions.h"
#include "window.h"
#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

    struct window_state state;

    init_window(&state);

    if (argc == 2) {
        state.cur_path = argv[1];
    } else {
        state.cur_path = calloc(2, sizeof(*state.cur_path));
        state.cur_path[0] = '.';
        state.cur_path[1] = '\0';
    }
    state.cursor = 0;
    state.hide_files = 0;
    state.file_count = 0;
    state.file_list = NULL;

    state.lib_handle = dlopen("libextensions.so", RTLD_LAZY);
    if (!state.lib_handle) {
        handle_fatal_error(dlerror());
    }
    void (*init_extensions)() = dlsym(state.lib_handle, "init_extensions");
    init_extensions();

    int key = 0;

    do {
        erase();
        print_files(&state);
        switch (key) {
        case KEY_DOWN:
            ++state.cursor;
            break;
        case KEY_UP:
            if (state.cursor > 0) {
                --state.cursor;
            }
            break;
        case 'D':
        case 'd':
            delete_file(&state);
            break;
        case 'X':
        case 'x':
            init_buffer(&state, CUT);
            break;
        case 'C':
        case 'c':
            init_buffer(&state, COPY);
            break;
        case 'V':
        case 'v':
            insert_buffer(&state);
            break;
        case 'h':
            toggle_hidden_files(&state);
            break;
        case '\n':
            select_file(&state);
            break;
        case ERR:
            break;
        }
    } while ((key = getch()) != 'q');

    dlclose(state.lib_handle);

    endwin();

    free(state.cur_path);
    free(state.buffer);

    return 0;
}