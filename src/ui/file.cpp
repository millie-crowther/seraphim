//
// Created by millie on 25/04/2021.
//

#include "ui/file.h"

#include <stdio.h>
#include <stdlib.h>

char *file_load_text(const char *filename, size_t *size) {
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size_t _size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *string = (char *)malloc(_size + 1);
    fread(string, 1, _size, file);
    string[_size] = '\0';

    fclose(file);

    if (size != NULL) {
        *size = _size;
    }
    return string;
}

cJSON *file_load_json(const char *filename) {
    char *string = file_load_text(filename, NULL);
    cJSON *parsed_json = cJSON_Parse(string);
    free(string);
    return parsed_json;
}
