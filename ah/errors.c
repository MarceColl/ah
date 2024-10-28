#include "errors.h"

#include <stdio.h>

char *error_msg;

void print_error(char *extra) {
    printf("%s: %s\n", extra, error_msg);
}

void set_error(char *err) {
    error_msg = err;
}
