#include <stdlib.h>

#include "pend_batch_mode.h"
#include "mem_alloc_check.h"
#include "gamma.h"
#include "inter_mode.h"

#define INITIAL_BUFFER_SIZE 16

int main() {
    gamma_t *g = NULL;

    char *line_buffer = malloc(INITIAL_BUFFER_SIZE * sizeof(char));
    check_for_successful_alloc(line_buffer);

    input_mode_t mode = PENDING_MODE;
    size_t line_buffer_size = INITIAL_BUFFER_SIZE;
    read_lines(&g, line_buffer, line_buffer_size, &mode);

    free(line_buffer);

    if (mode == INTERACTIVE_MODE) {
        inter_mode_launch(g);
    }

    gamma_delete(g);

    return 0;
}