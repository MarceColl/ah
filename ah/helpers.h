#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct buffer {
    size_t size;
    uint8_t data[];
} buffer_t;

buffer_t* read_file(char *path);
