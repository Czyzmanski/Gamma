/**
 * @file
 * Implementacja klasy przechowującej stan pola na planszy
 *
 * @author Szymon Czyżmański
 * @date 11.04.2020
 */

#include <stdlib.h>
#include <stdint.h>

#include "field.h"

/**
 * Struktura przechowująca stan pola (@p x, @p y).
 */
struct field {
    uint32_t x;      /**< Numer kolumny, liczba nieujemna mniejsza od wartości
                      *   @p width z funkcji @ref gamma_new. */
    uint32_t y;      /**< Numer wiersza, liczba nieujemna mniejsza od wartości
                      *   @p height z funkcji @ref gamma_new. */
    player_t *owner; /**< Wskaźnik do właściciela pola, gracza posiadającego pionek
                      *   na tym polu. */
    field_t *parent; /**< Wskaźnik do rodzica, pewnego pola znajdującego się w tym
                      *   samym obszarze co pole (@p x, @p y) lub NULL, jeśli
                      *   pole (@p x, @p y) jest korzeniem obszaru gracza
                      *   wskazywanego przez @p owner. Pozwala na implementację
                      *   operacji na obszarach zajętych przez tego gracza przy
                      *   pomocy struktury Find-Union. */
    uint32_t rank;   /**< Ranga pola, wartość wykorzystywana przy operacji łączenia
                      *   dwóch obszarów zajętych przez gracza wskazywanego przez
                      *   @p owner w jeden. Pozwala na implementację operacji na
                      *   obszarach zajętych przez tego gracza przy pomocy struktury
                      *   Find-Union. */
    status_t status; /**< Status pola, jedna z wartości zdefiniowanych w wyliczeniu
                      *   @ref status. Pozwala na odróżnienie, które pola zostały
                      *   już uwzględnione w trakcie przeszukiwania w głąb (DFS)
                      *   w implementacji funkcji @ref gamma_golden_move. */
};

field_t *field_new(uint32_t x, uint32_t y, player_t *owner) {
    field_t *f = malloc(sizeof(field_t));

    f->x = x;
    f->y = y;
    f->owner = owner;
    f->parent = NULL;
    f->rank = 0;
    f->status = UNCHECKED;

    return f;
}

uint32_t field_x(field_t *f) {
    return f->x;
}

uint32_t field_y(field_t *f) {
    return f->y;
}

player_t *field_owner(field_t *f) {
    return f->owner;
}

void field_set_owner(field_t *f, player_t *owner) {
    f->owner = owner;
}

field_t *field_parent(field_t *f) {
    return f->parent;
}

void field_set_parent(field_t *f, field_t *parent) {
    f->parent = parent;
}

uint32_t field_rank(field_t *f) {
    return f->rank;
}

void field_set_rank(field_t *f, uint32_t rank) {
    f->rank = rank;
}

status_t field_status(field_t *f) {
    return f->status;
}

void field_set_status(field_t *f, status_t status) {
    f->status = status;
}

void field_delete(field_t *f) {
    if (f != NULL) {
        free(f);
    }
}
