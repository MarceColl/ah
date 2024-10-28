#include "helpers.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

buffer_t* read_file(char *path) {
    FILE *fp;
    buffer_t *buffer;
    size_t file_size;

    fp = fopen(path, "rb");
    if (!fp) {
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    buffer = malloc(sizeof(buffer_t) + file_size);
    if (!buffer) {
        fclose(fp);
        return NULL;
    }

    buffer->size = file_size;

    if (fread(buffer->data, 1, file_size, fp) != file_size) {
        free(buffer);
        fclose(fp);
        return NULL;
    }

    fclose(fp);
    return buffer;
}
