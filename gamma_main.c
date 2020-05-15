/** @file
 * Plik zawierający funkcję @ref main, rozpoczynającą i kończącą program
 *
 * @author Szymon Czyżmański 417797
 * @date 15.05.2020
 */

#include <stdlib.h>

#include "parser.h"
#include "gamma.h"
#include "inter_mode.h"

/**
 * Początkowa długość bufora, do którego wpisywana jest linia pobierana
 * ze standardowego wejścia.
 */
#define INITIAL_BUFFER_SIZE 16

/** @brief Rozpoczyna i kończy działanie programu
 * Alokuje pamięć na bufor przechowujący wczytywane linie, wczytuje dane ze
 * standardowego strumienia wejścia i uruchamia tryb wsadowy lub interaktywny.
 * Zwalnia pamięć po buforze i strukturze przechowującej stan gry.
 * @return Wartość @p EXIT_SUCCESS, jeśli w trakcie działania programu nie wystąpił
 * żaden krytyczny błąd, a @p EXIT_FAILURE w przeciwnym przypadku.
 */
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