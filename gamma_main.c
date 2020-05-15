#include <stdlib.h>

#include "parser.h"
#include "gamma.h"
#include "inter_mode.h"

/**
 * Początkowa długość bufora, do którego wpisywana jest linia pobierana
 * ze standardowego wejścia.
 */
#define INITIAL_BUFFER_SIZE 16

int main() {
    int exit_code = EXIT_SUCCESS;
    char *line_buffer = malloc(INITIAL_BUFFER_SIZE * sizeof(char));

    if (line_buffer == NULL) {
        exit_code = EXIT_FAILURE;
    }
    else {
        gamma_t *g = NULL;
        input_mode_t mode = PENDING_MODE;

        if (!read_lines(&g, &line_buffer, INITIAL_BUFFER_SIZE, &mode)) {
            exit_code = EXIT_FAILURE;
        }

        free(line_buffer);

        if (mode == INTERACTIVE_MODE) {
            if (!inter_mode_launch(g)) {
                exit_code = EXIT_FAILURE;
            }
        }

        gamma_delete(g);
    }

    return exit_code;
}