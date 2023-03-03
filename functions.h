#pragma once
#include "colors.h"
#include "errors.h"
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

struct extension_handler {
    char *extension;
    char *programm;
    int arg_count;
    char **args;
};

enum buffer_state {
    CUT,
    COPY,
};

enum constants {
    BUF_SIZE = 4096,
};

static char *merge_paths(char *a, char *b);

static char *normalize_path(char *path);

void print_file(int file_index, int cursor, int row, char *name,
                unsigned char type, off_t file_size);

void open_directory(struct window_state *state);

void print_files(struct window_state *state);

void select_file(struct window_state *state);

void delete_file(struct window_state *state);

void init_buffer(struct window_state *state, int buffer_state);

void insert_buffer(struct window_state *state);

void toggle_hidden_files(struct window_state *state);
