/** @file
 * Implementacja klasy przechowującej stan gracza
 *
 * @author Szymon Czyżmański 417797
 * @date 11.04.2020
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "player.h"

/**
 * Struktura przechowująca stan gracza.
 */
struct player {
    uint32_t number;      /**< Numer gracza, liczba dodatnia niewiększa od wartości
                           *   @p players z funkcji @ref gamma_new. */
    uint64_t busy_fields; /**< Liczba pól zajętych przez gracza. */
    uint32_t areas;       /**< Liczba obszarów zajętych przez gracza. */
    uint32_t perimeter;   /**< Obwód gracza, liczba wolnych pól sąsiadujących
                           *   z przynajmniej jednym polem gracza. */
    bool golden_possible; /**< Wartość @p true, jeżeli gracz nie wykonał jeszcze
                           *   złotego ruchu, a @p false w przeciwnym przypadku. */
};

player_t *player_new(uint32_t number) {
    player_t *p = malloc(sizeof(player_t));

    if (p != NULL) {
        p->number = number;
        p->busy_fields = 0;
        p->areas = 0;
        p->perimeter = 0;
        p->golden_possible = true;
    }

    return p;
}

uint32_t player_number(player_t *p) {
    return p->number;
}

uint64_t player_busy_fields(player_t *p) {
    return p->busy_fields;
}

void player_set_busy_fields(player_t *p, uint64_t busy_fields) {
    p->busy_fields = busy_fields;
}

uint32_t player_areas(player_t *p) {
    return p->areas;
}

void player_set_areas(player_t *p, uint32_t areas) {
    p->areas = areas;
}

uint32_t player_perimeter(player_t *p) {
    return p->perimeter;
}

void player_set_perimeter(player_t *p, uint32_t perimeter) {
    p->perimeter = perimeter;
}

bool player_golden_possible(player_t *p) {
    return p->golden_possible;
}

void player_set_golden_possible(player_t *p, bool golden_possible) {
    p->golden_possible = golden_possible;
}

void player_delete(player_t *p) {
    if (p != NULL) {
        free(p);
    }
}
