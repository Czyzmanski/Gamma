#include <stdlib.h>

#include "pend_batch_mode.h"
#include "mem_alloc_check.h"
#include "gamma.h"
#include "inter_mode.h"

#define INITIAL_BUFFER_SIZE 16

int main() {
    char *line_buffer = malloc(INITIAL_BUFFER_SIZE * sizeof(char));
    check_for_successful_alloc(line_buffer);

    gamma_t *g = NULL;
    input_mode_t mode = PENDING_MODE;

    read_lines(&g, line_buffer, INITIAL_BUFFER_SIZE, &mode);

    free(line_buffer);

    if (mode == INTERACTIVE_MODE) {
        if (!inter_mode_launch(g)) {
            gamma_delete(g);
            exit(EXIT_FAILURE);
        }
    }

    gamma_delete(g);

    return 0;
}