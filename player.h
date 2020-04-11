/**
 * @file
 * Interfejs klasy przechowującej stan gracza
 *
 * @author Szymon Czyżmański 417797
 * @date 11.04.2020
 */

#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>

/**
 * Struktura przechowująca stan gracza.
 */
typedef struct player player_t;

/** @brief Tworzy strukturę przechowującą stan gracza.
 * Alokuje pamięć na nową strukturę przechowującą stan gracza.
 * Inicjuje tę strukturę tak, aby reprezentowała początkowy stan gracza.
 * @param[in] number          – numer gracza, liczba dodatnia niewiększa od
 *                              wartości @p players z funkcji @ref gamma_new.
 * @return Wskaźnik na utworzoną strukturę lub NULL, gdy nie udało się
 * zaalokować pamięci.
 */
player_t *player_new(uint32_t number);

/** @brief Podaje numer gracza.
 * Podaje numer gracza wskazywanego przez @p p.
 * @param[in] p               – wskaźnik na strukturę przechowującą stan gracza.
 * @return Numer gracza wskazywanego przez @p p, liczba dodatnia niewiększa
 * od wartości @p players z funkcji @ref gamma_new.
 */
uint32_t player_number(player_t *p);

/** @brief Podaje liczbę pól zajętych przez gracza.
 * Podaje liczbę pól zajętych przez gracza wskazywanego przez @p p.
 * @param[in] p               – wskaźnik na strukturę przechowującą stan gracza.
 * @return Liczba pól zajętych przez gracza wskazywanego przez @p p.
 */
uint64_t player_busy_fields(player_t *p);

/** @brief Przypisuje graczowi liczbę zajętych przez niego pól.
 * Przypisuje składowej @p busy_fields gracza wskazywanego przez @p p wartość
 * zmiennej @p busy_fields będącej paramatrem procedury.
 * @param[in,out] p           – wskaźnik na strukturę przechowującą stan gracza,
 * @param[in] busy_fields     – liczba pól zajętych przez gracza.
 */
void player_set_busy_fields(player_t *p, uint64_t busy_fields);

/** @brief Podaje liczbę obszarów zajętych przez gracza.
 * Podaje liczbę obszarów zajętych przez gracza wskazywanego przez @p p.
 * @param[in] p               – wskaźnik na strukturę przechowującą stan gracza.
 * @return Liczba obszarów zajętych przez gracza wskazywanego przez @p p.
 */
uint32_t player_areas(player_t *p);

/** @brief Przypisuje graczowi liczbę zajętych przez niego obszarów.
 * Przypisuje składowej @p areas gracza wskazywanego przez @p p wartość
 * zmiennej @p areas będącej paramatrem procedury.
 * @param[in,out] p           – wskaźnik na strukturę przechowującą stan gracza,
 * @param[in] areas           – liczba obszarów zajętych przez gracza.
 */
void player_set_areas(player_t *p, uint32_t areas);

/** @brief Podaje obwód gracza.
 * Podaje obwód gracza wskazywanego przez @p p, czyli liczbę wolnych pól
 * sąsiadujących z co najmniej jednym polem zajętym przez tego gracza.
 * @param[in] p               – wskaźnik na strukturę przechowującą stan gracza.
 * @return Liczba wolnych pól sąsiadujących z co najmniej jednym polem zajętym
 * przez gracza wskazywanego przez @p p.
 */
uint32_t player_perimeter(player_t *p);

/** @brief Przypisuje graczowi obwód.
 * Przypisuje składowej @p perimeter gracza wskazywanego przez @p p wartość
 * zmiennej @p perimeter będącej paramatrem procedury.
 * @param[in,out] p           – wskaźnik na strukturę przechowującą stan gracza,
 * @param[in] perimeter       – liczba wolnych pól sąsiadujących z co najmniej
 *                              jednym polem zajętym przez gracza wskazywanego
 *                              przez @p p.
 */
void player_set_perimeter(player_t *p, uint32_t perimeter);

/** @brief Sprawdza, czy gracz nie wykonał jeszcze złotego ruchu.
 * Sprawdza, czy gracz wskazywany przez @p p nie wykonał w tej rozgrywce
 * złotego ruchu.
 * @param[in] p               – wskaźnik na strukturę przechowującą stan gracza.
 * @return Wartość @p true, jeśli gracz jeszcze nie wykonał w tej rozgrywce
 * złotego ruchu, a @p false w przeciwnym przypadku.
 */
bool player_golden_possible(player_t *p);

/** @brief Przypisuje graczowi możliwość wykonania złotego ruchu.
 * Przypisuje składowej @p golden_possible gracza wskazywanego przez @p p
 * wartość zminnej @p golden_possible będącej parametrem procedury.
 * @param[in,out] p           – wskaźnik na strukturę przechowującą stan gracza,
 * @param[in] golden_possible – wartość @p true wskazująca, że gracz może wykonać
 *                              złoty ruch w danej rozgrywce, a @p false wskazująca
 *                              przeciwnie.
 */
void player_set_golden_possible(player_t *p, bool golden_possible);

/** @brief Usuwa strukturę przechowującą stan gracza.
 * Usuwa z pamięci strukturę wskazywaną przez @p p.
 * Nic nie robi, jeśli wskaźnik ten ma wartość NULL.
 * @param[in] p        – wskaźnik na usuwaną strukturę.
 */
void player_delete(player_t *p);

#endif // GAMMA_PLAYER_H
