/** @file
 * Interfejs klasy przechowującej stan gry gamma
 *
 * @author Marcin Peczarski <marpe@mimuw.edu.pl>, Szymon Czyżmański 417797
 * @copyright Uniwersytet Warszawski
 * @date 18.03.2020
 */

#ifndef GAMMA_H
#define GAMMA_H

#include <stdbool.h>
#include <stdint.h>

#include "field.h"

/**
 * Struktura przechowująca stan gry.
 */
typedef struct gamma gamma_t;

/** @brief Tworzy strukturę przechowującą stan gry.
 * Alokuje pamięć na nową strukturę przechowującą stan gry.
 * Inicjuje tę strukturę tak, aby reprezentowała początkowy stan gry.
 * @param[in] width   – szerokość planszy, liczba dodatnia,
 * @param[in] height  – wysokość planszy, liczba dodatnia,
 * @param[in] players – liczba graczy, liczba dodatnia,
 * @param[in] areas   – maksymalna liczba obszarów,
 *                      jakie może zająć jeden gracz, liczba dodatnia.
 * @return Wskaźnik na utworzoną strukturę lub NULL, gdy nie udało się
 * zaalokować pamięci lub któryś z parametrów jest niepoprawny.
 */
gamma_t *gamma_new(uint32_t width, uint32_t height,
                   uint32_t players, uint32_t areas);

/** @brief Usuwa strukturę przechowującą stan gry.
 * Usuwa z pamięci strukturę wskazywaną przez @p g.
 * Nic nie robi, jeśli wskaźnik ten ma wartość NULL.
 * @param[in] g       – wskaźnik na usuwaną strukturę.
 */
void gamma_delete(gamma_t *g);

/** @brief Wykonuje ruch.
 * Ustawia pionek gracza @p player na polu (@p x, @p y).
 * @param[in,out] g   – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new,
 * @param[in] x       – numer kolumny, liczba nieujemna mniejsza od wartości
 *                      @p width z funkcji @ref gamma_new,
 * @param[in] y       – numer wiersza, liczba nieujemna mniejsza od wartości
 *                      @p height z funkcji @ref gamma_new.
 * @return Wartość @p true, jeśli ruch został wykonany, a @p false,
 * gdy ruch jest nielegalny lub któryś z parametrów jest niepoprawny.
 */
bool gamma_move(gamma_t *g, uint32_t player, uint32_t x, uint32_t y);

/** @brief Wykonuje złoty ruch.
 * Ustawia pionek gracza @p player na polu (@p x, @p y) zajętym przez innego
 * gracza, usuwając pionek innego gracza.
 * @param[in,out] g   – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new,
 * @param[in] x       – numer kolumny, liczba nieujemna mniejsza od wartości
 *                      @p width z funkcji @ref gamma_new,
 * @param[in] y       – numer wiersza, liczba nieujemna mniejsza od wartości
 *                      @p height z funkcji @ref gamma_new.
 * @return Wartość @p true, jeśli ruch został wykonany, a @p false,
 * gdy gracz wykorzystał już swój złoty ruch, ruch jest nielegalny
 * lub któryś z parametrów jest niepoprawny.
 */
bool gamma_golden_move(gamma_t *g, uint32_t player, uint32_t x, uint32_t y);

/** @brief Podaje liczbę pól zajętych przez gracza.
 * Podaje liczbę pól zajętych przez gracza @p player.
 * @param[in] g       – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new.
 * @return Liczba pól zajętych przez gracza lub zero,
 * jeśli któryś z parametrów jest niepoprawny.
 */
uint64_t gamma_busy_fields(gamma_t *g, uint32_t player);

/** @brief Podaje liczbę pól, jakie jeszcze gracz może zająć.
 * Podaje liczbę wolnych pól, na których w danym stanie gry gracz @p player może
 * postawić swój pionek w następnym ruchu.
 * @param[in] g       – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new.
 * @return Liczba pól, jakie jeszcze może zająć gracz lub zero,
 * jeśli któryś z parametrów jest niepoprawny.
 */
uint64_t gamma_free_fields(gamma_t *g, uint32_t player);

/** @brief Sprawdza, czy gracz może wykonać złoty ruch.
 * Sprawdza, czy gracz @p player jeszcze nie wykonał w tej rozgrywce złotego
 * ruchu oraz istnieje takie pole zajęte przez przeciwnika, że po zajęciu
 * go przez gracza zarówno liczba obszarów zajętych przez gracza, jak
 * i liczba obszarów zajętych przez przeciwnika nie przekroczy maksymalnej
 * liczby zajętych obszarów.
 * @param[in] g       – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new.
 * @return Wartość @p true, jeśli gracz jeszcze nie wykonał w tej rozgrywce
 * złotego ruchu i istnieje takie pole zajęte przez przeciwnika, że po zajęciu
 * go przez gracza zarówno liczba obszarów zajętych przez gracza, jak
 * i liczba obszarów zajętych przez przeciwnika nie przekroczy maksymalnej
 * liczby zajętych obszarów.
 */
bool gamma_golden_possible(gamma_t *g, uint32_t player);

/** @brief Daje napis opisujący stan planszy.
 * Alokuje w pamięci bufor, w którym umieszcza napis zawierający tekstowy
 * opis aktualnego stanu planszy. Przykład znajduje się w pliku gamma_test.c.
 * Funkcja wywołująca musi zwolnić ten bufor.
 * @param[in] g       – wskaźnik na strukturę przechowującą stan gry.
 * @return Wskaźnik na zaalokowany bufor zawierający napis opisujący stan
 * planszy lub NULL, jeśli nie udało się zaalokować pamięci.
 */
char *gamma_board(gamma_t *g);

/** @brief Podaje wysokość planszy w jej tekstowym opisie.
 * Podaje wysokość planszy w napisie otrzymywanym w wyniku wywołania funkcji
 * @ref gamma_board.
 * @param[in] g       – wskaźnik na strukturę przechowującą stan gry.
 * @return Wysokość planszy w napisie otrzymywanym w wyniku wywołania funkcji
 * @ref gamma_board lub 0, gdy wskaźnik @p g jest równy NULL.
 */
uint32_t gamma_board_height(gamma_t *g);

/** @brief Podaje szerokość planszy w jej tekstowym opisie.
 * Podaje szerokość planszy w napisie otrzymywanym w wyniku wywołania funkcji
 * @ref gamma_board.
 * @param[in] g       – wskaźnik na strukturę przechowującą stan gry.
 * @return Szerokość planszy w napisie otrzymywanym w wyniku wywołania funkcji
 * @ref gamma_board lub 0, gdy wskaźnik @p g jest równy NULL.
 */
uint64_t gamma_board_width(gamma_t *g);

/** @brief Podaje szerokość pola na planszy w jej tekstowym opisie.
 * Podaje szerokość pola na planszy w napisie otrzymywanym w wyniku wywołania
 * funkcji @ref gamma_board.
 * @param[in] g       – wskaźnik na strukturę przechowującą stan gry.
 * @return Szerokość pola na planszy w napisie otrzymywanym w wyniku wywołania
 * funkcji @ref gamma_board lub 0, gdy wskaźnik @p g jest równy NULL.
 */
unsigned gamma_board_field_width(gamma_t *g);

/** @brief Daje napis reprezentujący pole (@p x, @p y).
 * Wpisuje do bufora długości @p FIELD_MAX_WIDTH + 1 wskazywanego przez
 * @p repr tekstową reprezentację pola (@p x, @p y).
 * Znak null jest dopisywany na końcu wpisanej reprezentacji tego pola.
 * Nic nie robi, jeżeli wskaźnik @p g jest równy NULL lub któraś ze współrzędnych
 * @p x, @p y jest niepoprawna.
 * @param[in] g        – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] x        – numer kolumny, liczba nieujemna mniejsza od wartości
 *                       @p width z funkcji @ref gamma_new,
 * @param[in] y        – numer wiersza, liczba nieujemna mniejsza od wartości
 *                       @p height z funkcji @ref gamma_new.
 * @param[in,out] repr – wskaźnik na bufor długości @p FIELD_MAX_WIDTH + 1,
 *                       do którego ma zostać wpisana tekstowa reprezentacja pola.
 */
void gamma_board_field_repr(gamma_t *g, uint32_t x, uint32_t y,
                            char repr[FIELD_MAX_WIDTH + 1]);

/** @brief Podaje liczbę graczy.
 * Podaje liczbę graczy biorących udział w rozgrywce.
 * @param[in] g       – wskaźnik na strukturę przechowującą stan gry.
 * @return Liczba graczy biorących udział w rozgrywce lub 0,
 * gdy wskaźnik @p g jest równy NULL.
 */
uint32_t gamma_players(gamma_t *g);

/** @brief Podaje maksymalną liczbę pól zajętych przez jednego gracza.
 * Podaje maksymalną liczbę pól, jaka została zajęta przez jednego gracza.
 * @param[in] g       – wskaźnik na strukturę przechowującą stan gry.
 * @return Maksymalna liczba pól, jaka została zajęta przez jednego gracza
 * lub 0, gdy wskaźnik @p g jest równy NULL.
 */
uint64_t gamma_max_busy_fields(gamma_t *g);

#endif // GAMMA_H
