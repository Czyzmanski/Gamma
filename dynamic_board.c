/** @file
 * Implementacja klasy reprezentującej dynamiczną tablicę zawierającą napis
 * opisujący aktualny stan planszy
 *
 * @author Szymon Czyżmański 417797
 * @date 13.04.2020
 */

#include <string.h>
#include <stdlib.h>

#include "dynamic_board.h"

#define GROWTH_FACTOR 2

#define PLAYER_MAX_DIGITS 10

struct dynamic_board {
    char *array;
    size_t size;
    size_t capacity;
};

dyn_board_t *dynamic_board_new(size_t capacity) {
    dyn_board_t *board = malloc(sizeof(dyn_board_t));

    if (board != NULL) {
        board->array = malloc(capacity * sizeof(char));
        if (board->array == NULL) {
            free(board);
            board = NULL;
        }
        else {
            board->size = 0;
            board->capacity = capacity;
        }
    }

    return board;
}

static bool dynamic_board_ensure_capacity(dyn_board_t *board) {
    if (board->size < board->capacity) {
        return true;
    }
    else {
        board->capacity = board->capacity * GROWTH_FACTOR;
        board->array = realloc(board->array, board->capacity * sizeof(char));
        return board->array != NULL;
    }
}

bool dynamic_board_add_char(dyn_board_t *board, char c) {
    if (!dynamic_board_ensure_capacity(board)) {
        return false;
    }
    else {
        board->array[board->size] = c;
        board->size++;
        return true;
    }
}

bool dynamic_board_add_player(dyn_board_t *board, uint32_t player) {
    uint8_t player_digits[PLAYER_MAX_DIGITS];
    uint8_t digits = 0;

    while (player > 0) {
        player_digits[digits] = player % 10;
        digits++;
        player /= 10;
    }

    bool added = dynamic_board_add_char(board, '[');
    for (int8_t i = digits - 1; i >= 0 && added; i--) {
        added = dynamic_board_add_char(board, player_digits[i] + '0');
    }
    added = dynamic_board_add_char(board, ']');

    return added;
}

char *dynamic_board_fitted_array(dyn_board_t *board) {
    char *fitted = malloc(board->size * sizeof(char));

    if (fitted != NULL) {
        memcpy(fitted, board->array, board->size);
    }

    return fitted;
}

size_t dynamic_board_size(dyn_board_t *board) {
    return board->size;
}

void dynamic_board_delete(dyn_board_t *board) {
    if (board != NULL) {
        free(board->array);
        free(board);
    }
}
