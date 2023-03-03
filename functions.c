#include "functions.h"

static char *merge_paths(char *a, char *b) {
    char *c;
    if (a) {
        int new_size = strlen(a) + strlen(b) + 2;
        c = calloc(new_size, sizeof(*c));
        snprintf(c, new_size, "%s/%s", a, b);
    } else {
        int new_size = strlen(b) + 1;
        c = calloc(new_size, sizeof(*c));
        snprintf(c, new_size, "%s", b);
    }
    return c;
}

static char *normalize_path(char *path) {
    char norm_path[PATH_MAX];
    if (realpath(path, norm_path) == NULL) {
        handle_fatal_error("Current path doesn't exist");
    }
    return strdup(norm_path);
}

void print_file(int file_index, int cursor, int row, char *name,
                unsigned char type, off_t file_size) {
    if (file_index == cursor) {
        attron(COLOR_PAIR(CURSOR_COLOR_PAIR));
        mvprintw(row, 0, "->");
        attroff(COLOR_PAIR(CURSOR_COLOR_PAIR));
    }
    switch (type) {
    case DT_DIR:
        attron(COLOR_PAIR(DIR_COLOR_PAIR));
        break;
    case DT_LNK:
        attron(COLOR_PAIR(SYM_LINK_COLOR_PAIR));
        break;
    case DT_FIFO:
        attron(COLOR_PAIR(FIFO_COLOR_PAIR));
        break;
    default:
        attron(COLOR_PAIR(DEFAULT_COLOR_PAIR));
        break;
    }

    if (file_index == cursor) {
        mvprintw(row, 2, "%.*s ", MAX_NAME, name);
    } else {
        mvprintw(row, 0, "%.*s ", MAX_NAME, name);
    }
    if (file_size == -1) {
        mvprintw(row, SIZE_OFFSET, "???\n");
    } else {
        mvprintw(row, SIZE_OFFSET, "%d\n", (int)file_size);
    }
    switch (type) {
    case DT_DIR:
        attroff(COLOR_PAIR(DIR_COLOR_PAIR));
        break;
    case DT_LNK:
        attroff(COLOR_PAIR(SYM_LINK_COLOR_PAIR));
        break;
    case DT_FIFO:
        attroff(COLOR_PAIR(FIFO_COLOR_PAIR));
        break;
    default:
        attroff(COLOR_PAIR(DEFAULT_COLOR_PAIR));
        break;
    }
}

void open_directory(struct window_state *state) {
    state->dir = opendir(state->cur_path);
    if (state->dir == NULL) {
        handle_error("opendir");
        return;
    }
    free(state->file_list);
    state->file_count =
        scandir(state->cur_path, &state->file_list, NULL, alphasort);

    struct dirent **new_file_list;
    int new_file_count = 0;
    for (int i = 0; i < state->file_count; ++i) {
        if (!strcmp(state->file_list[i]->d_name, ".")) {
            continue;
        }
        if (state->hide_files == 1 &&
            strcmp(state->file_list[i]->d_name, "..") &&
            state->file_list[i]->d_name[0] == '.') {
            continue;
        }
        ++new_file_count;
    }
    new_file_list = calloc(new_file_count, sizeof(*new_file_list));

    if (new_file_list == NULL) {
        handle_fatal_error("Malloc caused error");
    }
    int last_file = 0;
    for (int i = 0; i < state->file_count; ++i) {
        if (!strcmp(state->file_list[i]->d_name, ".")) {
            continue;
        }
        if (state->hide_files == 1 &&
            strcmp(state->file_list[i]->d_name, "..") &&
            state->file_list[i]->d_name[0] == '.') {
            continue;
        }
        new_file_list[last_file] = state->file_list[i];
        ++last_file;
    }

    free(state->file_list);
    state->file_list = new_file_list;
    state->file_count = new_file_count;

    if (state->cursor >= state->file_count) {
        state->cursor = state->file_count - 1;
    }
}

void print_files(struct window_state *state) {

    open_directory(state);

    attron(COLOR_PAIR(INTERFACE_COLOR_PAIR));
    mvprintw(0, 0, "%.*s ", MAX_NAME, "Name");
    mvprintw(0, SIZE_OFFSET, "%s\n", "Size");
    attroff(COLOR_PAIR(INTERFACE_COLOR_PAIR));

    int row = 1;
    int file_index = 0;
    int start = 0;

    if (state->height > 1) {
        start = (state->cursor) / (state->height - 1) * (state->height - 1);
    }

    file_index = start;

    while (file_index < state->file_count &&
           file_index < start + state->height - 1) {
        struct dirent *next_dir = state->file_list[file_index];

        char *next_path = merge_paths(state->cur_path, next_dir->d_name);
        struct stat buf;
        if (stat(next_path, &buf) < 0) {
            print_file(file_index, state->cursor, row, next_dir->d_name,
                       next_dir->d_type, -1);
        } else {
            print_file(file_index, state->cursor, row, next_dir->d_name,
                       next_dir->d_type, buf.st_size);
        }
        free(next_path);
        ++file_index;
        ++row;
    }
    closedir(state->dir);
}

void select_file(struct window_state *state) {

    open_directory(state);

    if (state->file_list[state->cursor]->d_type == DT_DIR) {
        char *not_normalized_next_path = merge_paths(
            state->cur_path, state->file_list[state->cursor]->d_name);
        char *next_path = normalize_path(not_normalized_next_path);
        free(not_normalized_next_path);
        DIR *next_dir = opendir(next_path);
        if (next_dir == NULL) {
            free(next_dir);
            closedir(state->dir);
            handle_error("opendir");
            return;
        }
        free(state->cur_path);
        state->cur_path = next_path;
        state->cursor = 0;
    } else if (state->file_list[state->cursor]->d_type == DT_REG ||
               state->file_list[state->cursor]->d_type == DT_LNK) {

        struct extension_handler *(*get_extension_handler)(char *) =
            dlsym(state->lib_handle, "get_extension_handler");
        if (!get_extension_handler) {
            handle_custom_error(dlerror());
            return;
        }

        char *not_normalized_next_path = merge_paths(
            state->cur_path, state->file_list[state->cursor]->d_name);
        char *next_path = normalize_path(not_normalized_next_path);
        free(not_normalized_next_path);

        struct extension_handler *handler = get_extension_handler(next_path);

        if (handler) {
            char **args = calloc(handler->arg_count + 2, sizeof(*args));

            for (int i = 0; i < handler->arg_count; ++i) {
                args[i] = handler->args[i];
            }

            args[handler->arg_count] = next_path;
            args[handler->arg_count + 1] = NULL;

            if (handler->programm != NULL) {
                pid_t pid = fork();
                int return_value;
                if (pid < 0) {
                    handle_error("fork");
                } else if (pid == 0) {
                    execvp(args[0], args);
                    handle_error("execlp");
                } else {
                    wait(&return_value);
                    endwin();
                    init_window(state);
                }
            }
            free(args);
        } else {
            handle_custom_error("Such extensions are not supported");
        }
    } else {
        handle_custom_error("Such files are not supported");
    }

    closedir(state->dir);
}

void delete_file(struct window_state *state) {

    open_directory(state);

    if (state->file_list[state->cursor]->d_type == DT_DIR) {
        handle_custom_error("Deleting directories is not supported");
    } else {
        char *next_path = merge_paths(state->cur_path,
                                      state->file_list[state->cursor]->d_name);
        if (unlink(next_path) == -1) {
            handle_error("unlink");
        }
        free(next_path);
    }
    closedir(state->dir);
}

void init_buffer(struct window_state *state, int buffer_state) {

    open_directory(state);

    state->buffer_state = buffer_state;

    char *next_path =
        merge_paths(state->cur_path, state->file_list[state->cursor]->d_name);
    struct stat stat_buf;
    if (stat(next_path, &stat_buf) < 0) {
        closedir(state->dir);
        free(next_path);
        handle_error("stat");
        return;
    }
    if (S_ISDIR(stat_buf.st_mode)) {
        closedir(state->dir);
        free(next_path);
        handle_custom_error("Copying/Cutting directories is not supported");
        return;
    }

    free(state->buffer);
    state->buffer = next_path;

    closedir(state->dir);
}

void insert_buffer(struct window_state *state) {
    if (state->buffer == NULL) {
        handle_custom_error("Nothing to insert");
        return;
    }
    int source_fd = open(state->buffer, O_RDONLY);
    if (source_fd < 0) {
        handle_error("open");
        return;
    }

    char *new_path = merge_paths(state->cur_path, basename(state->buffer));
    if (access(new_path, F_OK) == 0) {
        free(new_path);
        close(source_fd);
        handle_custom_error("File already exists");
        return;
    }

    int destination_fd =
        open(new_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    free(new_path);
    if (destination_fd < 0) {
        close(source_fd);
        handle_error("open");
        return;
    }
    char buf[BUF_SIZE];
    int count = 0;
    while (count = read(source_fd, buf, BUF_SIZE)) {
        if (count < 0) {
            close(source_fd);
            close(destination_fd);
            handle_error("read");
            return;
        } else {
            int res = write(destination_fd, buf, count);
            if (res != count) {
                close(source_fd);
                close(destination_fd);
                handle_error("write");
                return;
            }
        }
    }

    close(source_fd);
    close(destination_fd);

    if (state->buffer_state == CUT) {
        if (unlink(state->buffer) == -1) {
            handle_error("unlink");
        }
        free(state->buffer);
    }
}

void toggle_hidden_files(struct window_state *state) {
    state->hide_files = (state->hide_files + 1) % 2;
}