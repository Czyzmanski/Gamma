/** @file
 * Interfejs klasy reprezentującej dynamiczną tablicę zawierającą napis
 * opisujący aktualny stan planszy
 *
 * @author Szymon Czyżmański 417797
 * @date 13.04.2020
 */

#ifndef DYNAMIC_BOARD_H
#define DYNAMIC_BOARD_H

#include <stdint.h>
#include <stdbool.h>

/**
 * Struktura przechowująca stan dynamicznego bufora zawierającego napis
 * opisujący aktualny stan planszy.
 */
typedef struct dynamic_board dyn_board_t;

dyn_board_t *dynamic_board_new(uint64_t capacity);

bool dynamic_board_add_char(dyn_board_t *board, char c);

bool dynamic_board_add_player(dyn_board_t *board, uint32_t player);

char *dynamic_board_fitted_array(dyn_board_t *board);

uint64_t dynamic_board_size(dyn_board_t *board);

void dynamic_board_delete(dyn_board_t *board);

#endif // DYNAMIC_BOARD_H
