//
// Created by millie on 25/04/2021.
//

#ifndef SERAPHIM_FILE_H
#define SERAPHIM_FILE_H

#include "cJSON.h"

char *file_load_text(const char *filename, size_t *size);
cJSON *file_load_json(const char *filename);

#endif //SERAPHIM_FILE_H
