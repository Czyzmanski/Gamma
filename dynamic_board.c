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

/**
 * Tyle razy zostanie powiększony bufor @ref dynamic_board::array, jeśli dojdzie do
 * jego pełnego zapełnienia i trzeba będzie dodać nowy znak.
 */
#define GROWTH_FACTOR 2

/**
 * Maksymalna liczba cyfr, jakie może posiadać numer gracza.
 * Maksymalna wartość, jaką może przyjmować numer gracza jest równa 2^32 - 1.
 * Sufit z logarytmu dziesiętnego z tej wartości jest równy 10.
 */
#define PLAYER_MAX_DIGITS 10

/**
 * Struktura przechowująca stan dynamicznego bufora zawierającego napis
 * opisujący aktualny stan planszy.
 */
struct dynamic_board {
    char *array;       /**< Bufor zawierający opis aktualnego stanu planszy. */
    uint64_t size;     /**< Długość napisu reprezentującego tesktowy opis
                        *   aktualnego stanu planszy, równy liczbie dodanych
                        *   znaków do bufora @p array. */
    uint64_t capacity; /**< Pojemność bufora @p array. */
};

dyn_board_t *dynamic_board_new(uint64_t capacity) {
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

/** @brief Zapewnia, aby można było dodać znak do bufora.
 * Sprawdza, czy w buforze @ref dynamic_board::array jest wystarczająco miejsca,
 * aby można było dodać nowy znak i jeżeli nie, to powiększa ten bufor tak,
 * aby dodanie nowego znaku było możliwe.
 * @param board[in,out] – wskaźnik na strukturę przechowującą napis zawierający
 *                        opis aktualnego stanu planszy.
 * @return Wartość @p true, jeżeli w buforze @ref dynamic_board::array jest
 * miejsce na dodanie nowego znaku lub jeżeli tego miejsca nie było, ale pomyślnie
 * udało się powiększyć bufor, a @p false, jeżeli w buforze nie było miejsca
 * na dodanie nowego znaku i próba jego powiększenia zakończyła się niepowodzeniem
 * z powodu braku pamięci.
 */
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

    bool added = true;
    for (int8_t i = digits - 1; i >= 0 && added; i--) {
        added = dynamic_board_add_char(board, player_digits[i] + '0');
    }

    return added;
}

char *dynamic_board_fitted_array(dyn_board_t *board) {
    char *fitted = malloc(board->size * sizeof(char));

    if (fitted != NULL) {
        memcpy(fitted, board->array, board->size);
    }

    return fitted;
}

void dynamic_board_delete(dyn_board_t *board) {
    if (board != NULL) {
        free(board->array);
        free(board);
    }
}
