/** @file
 * Interfejs klasy przechowującej stan pola na planszy
 */

#ifndef FIELD_H
#define FIELD_H

#include "player.h"

typedef struct field field_t;

enum state {
    UNCHECKED,
    COUNTED,
    MODIFIED
};

typedef enum state state_t;

/** @brief Tworzy strukturę przechowującą stan pola (@p x, @p y).
 * Alokuje pamięć na nową strukturę przechowującą stan pola (@p x, @p y).
 * Inicjuje tę strukturę tak, aby reprezentowała początkowy stan pola.
 * @param[in] x     – numer kolumny, liczba nieujemna mniejsza od wartości
 *                  @p width z funkcji @ref gamma_new
 * @param[in] y     – numer wiersza, liczba nieujemna mniejsza od wartości
 *                  @p height z funkcji @ref gamma_new,
 * @param[in] owner – wskaźnik na strukturę przechowującą stan gracza,
 *                  który postawił pionek na polu (@p x, @p y).
 * @return Wskaźnik na utworzoną strukturę lub NULL, gdy nie udało się
 * zaalokować pamięci lub któryś z parametrów jest niepoprawny.
 */
field_t *field_new(uint32_t x, uint32_t y, player_t *owner);

/** @brief Zwraca numer kolumny na której znajduje się pole @p f.
 * @param[in] f     – wskaźnik na strukturę przechowującą stan pola.
 * @return Numer kolumny na której znajduje się pole @p f.
 */
uint32_t field_x(field_t *f);

/** @brief Zwraca numer wiersza na którym znajduje się pole @p f.
 * @param[in] f     – wskaźnik na strukturę przechowującą stan pola.
 * @return Numer wiersza na którym znajduje się pole @p f.
 */
uint32_t field_y(field_t *f);

/** @brief Zwraca wskaźnik na strukturę gracza mającego pionek na polu @p f.
 * @param[in] f     – wskaźnik na strukturę przechowującą stan pola.
 * @return Wskaźnik na strukturę gracza mającego pionek na polu @p f.
 */
player_t *field_owner(field_t *f);

void field_set_owner(field_t *f, player_t *owner);

/** @brief Zwraca wskaźnik na strukturę pola będącego rodzicem pola @p f.
 * @param[in] f     – wskaźnik na strukturę przechowującą stan pola.
 * @return Wskaźnik na strukturę pola będącego rodzicem pola @p f.
 */
field_t *field_parent(field_t *f);

void field_set_parent(field_t *f, field_t *parent);

/** @brief Zwraca rangę pola @p f.
 * Zwraca wartość składowej @ref field.rank pola @p f.
 * @param[in] f     – wskaźnik na strukturę przechowującą stan pola.
 * @return Wartość rangi pola @p f.
 */
uint32_t field_rank(field_t *f);

void field_set_rank(field_t *f, uint32_t rank);

/** @brief Zwraca stan pola.
 * Zwraca wartość składowej @p state struktury wskazywanej przez @p f.
 * @param[in] f     – wskaźnik na strukturę przechowującą stan pola.
 * @return Wartość składowej @p state struktury wskazywanej przez @p f.
 */
state_t field_state(field_t *f);

void field_set_state(field_t *f, state_t state);

/** @brief Usuwa strukturę przechowującą stan pola.
 * Usuwa z pamięci strukturę wskazywaną przez @p f.
 * Nic nie robi, jeśli wskaźnik ten ma wartość NULL.
 * @param[in] f       – wskaźnik na usuwaną strukturę.
 */
void field_delete(field_t *f);

#endif // FIELD_H
