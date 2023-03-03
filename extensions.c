#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct extenstion_handler {
    char *extension;
    char *programm;
    int arg_count;
    char **args;
};

struct extenstion_handler handlers[] = {
    {".txt", "nano", 1, NULL},
    {".gz", "gunzip", 1, NULL},
};

void init_extensions() {
    for (int i = 0; i < sizeof(handlers); ++i) {
        handlers[i].args = calloc(1, sizeof(*handlers[0].args));
        handlers[i].args[0] = handlers[i].programm;
    }
}

struct extenstion_handler *get_extension_handler(char *file_name) {
    char *extension = strrchr(file_name, '.');
    if (!extension) {
        return NULL;
    }
    for (int i = 0; i < sizeof(handlers) / sizeof(handlers[0]); i++) {
        if (!strcmp(extension, handlers[i].extension)) {
            return &handlers[i];
        }
    }
    return NULL;
}