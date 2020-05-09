#ifndef INPUT_OUTPUT_H
#define INPUT_OUTPUT_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "gamma.h"

typedef enum input_mode input_mode_t;

enum input_mode {
    PENDING_MODE,
    BATCH_MODE,
    INTERACTIVE_MODE
};

void read_lines(gamma_t **g, char **buf, size_t buffer_size, input_mode_t *mode);

#endif // INPUT_OUTPUT_H
