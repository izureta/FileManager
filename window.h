#pragma once
#include "colors.h"
#include "errors.h"
#include <dirent.h>
#include <ncurses.h>

enum window {
    MAX_NAME = 30,
    MAX_FILE_SIZE = 10,
    SIZE_OFFSET = 35,
    MIN_WIDTH = 47,
};

struct window_state {
    char *cur_path;
    char *buffer;
    int cursor;
    int width;
    int height;
    DIR *dir;
    struct dirent **file_list;
    int file_count;
    int buffer_state;
    int hide_files;
    void *lib_handle;
};

void init_window(struct window_state *state);
