/** @file
 * Interfejs klasy przechowującej stan gracza
 *
 * @author Szymon Czyżmański 417797
 * @date 11.04.2020
 */

#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>

/**
 * Typ struktury przechowującej stan gracza.
 */
typedef struct player player_t;

/**
 * Struktura przechowująca stan gracza.
 */
struct player {
    uint32_t number;      /**< Numer gracza, liczba dodatnia niewiększa od wartości
                           *   @p players z funkcji @ref gamma_new. */
    uint64_t busy_fields; /**< Liczba pól zajętych przez gracza. */
    uint32_t areas;       /**< Liczba obszarów zajętych przez gracza. */
    uint64_t perimeter;   /**< Obwód gracza, liczba wolnych pól sąsiadujących
                           *   z przynajmniej jednym polem gracza. */
    bool golden_possible; /**< Wartość @p true, jeżeli gracz nie wykonał jeszcze
                           *   złotego ruchu, a @p false w przeciwnym przypadku. */
};

/** @brief Inicjuje strukturę przechowującą stan gracza.
 * Inicjuje strukturę wskazywaną przez @p p tak, aby reprezentowała początkowy
 * stan gracza.
 * @param[in] p               – wskaźnik na strukturę przechowującą stan gracza,
 * @param[in] number          – numer gracza, liczba dodatnia niewiększa od
 *                              wartości @p players z funkcji @ref gamma_new.
 */
static inline void player_init(player_t *p, uint32_t number) {
    p->number = number;
    p->busy_fields = 0;
    p->areas = 0;
    p->perimeter = 0;
    p->golden_possible = true;
}

/** @brief Podaje numer gracza.
 * Podaje numer gracza wskazywanego przez @p p.
 * @param[in] p               – wskaźnik na strukturę przechowującą stan gracza.
 * @return Numer gracza wskazywanego przez @p p, liczba dodatnia niewiększa
 * od wartości @p players z funkcji @ref gamma_new.
 */
static inline uint32_t player_number(player_t *p) {
    return p->number;
}

/** @brief Podaje liczbę pól zajętych przez gracza.
 * Podaje liczbę pól zajętych przez gracza wskazywanego przez @p p.
 * @param[in] p               – wskaźnik na strukturę przechowującą stan gracza.
 * @return Liczba pól zajętych przez gracza wskazywanego przez @p p.
 */
static inline uint64_t player_busy_fields(player_t *p) {
    return p->busy_fields;
}

/** @brief Aktualizuje liczbę zajętych przez gracza pól.
 * Przypisuje składowej @p busy_fields gracza wskazywanego przez @p p wartość
 * zmiennej @p busy_fields będącej paramatrem procedury.
 * @param[in,out] p           – wskaźnik na strukturę przechowującą stan gracza,
 * @param[in] busy_fields     – liczba pól zajętych przez gracza.
 */
static inline void player_set_busy_fields(player_t *p, uint64_t busy_fields) {
    p->busy_fields = busy_fields;
}

/** @brief Podaje liczbę obszarów zajętych przez gracza.
 * Podaje liczbę obszarów zajętych przez gracza wskazywanego przez @p p.
 * @param[in] p               – wskaźnik na strukturę przechowującą stan gracza.
 * @return Liczba obszarów zajętych przez gracza wskazywanego przez @p p.
 */
static inline uint32_t player_areas(player_t *p) {
    return p->areas;
}

/** @brief Aktualizuje liczbę zajętych przez gracza obszarów.
 * Przypisuje składowej @p areas gracza wskazywanego przez @p p wartość
 * zmiennej @p areas będącej paramatrem procedury.
 * @param[in,out] p           – wskaźnik na strukturę przechowującą stan gracza,
 * @param[in] areas           – liczba obszarów zajętych przez gracza.
 */
static inline void player_set_areas(player_t *p, uint32_t areas) {
    p->areas = areas;
}

/** @brief Podaje obwód gracza.
 * Podaje obwód gracza wskazywanego przez @p p, czyli liczbę wolnych pól
 * sąsiadujących z co najmniej jednym polem zajętym przez tego gracza.
 * @param[in] p               – wskaźnik na strukturę przechowującą stan gracza.
 * @return Liczba wolnych pól sąsiadujących z co najmniej jednym polem zajętym
 * przez gracza wskazywanego przez @p p.
 */
static inline uint64_t player_perimeter(player_t *p) {
    return p->perimeter;
}

/** @brief Aktualizuje obwód gracza.
 * Przypisuje składowej @p perimeter gracza wskazywanego przez @p p wartość
 * zmiennej @p perimeter będącej paramatrem procedury.
 * @param[in,out] p           – wskaźnik na strukturę przechowującą stan gracza,
 * @param[in] perimeter       – liczba wolnych pól sąsiadujących z co najmniej
 *                              jednym polem zajętym przez gracza wskazywanego
 *                              przez @p p.
 */
static inline void player_set_perimeter(player_t *p, uint64_t perimeter) {
    p->perimeter = perimeter;
}

/** @brief Sprawdza, czy gracz nie wykonał jeszcze złotego ruchu.
 * Sprawdza, czy gracz wskazywany przez @p p nie wykonał w tej rozgrywce
 * złotego ruchu.
 * @param[in] p               – wskaźnik na strukturę przechowującą stan gracza.
 * @return Wartość @p true, jeśli gracz jeszcze nie wykonał w tej rozgrywce
 * złotego ruchu, a @p false w przeciwnym przypadku.
 */
static inline bool player_golden_possible(player_t *p) {
    return p->golden_possible;
}

/** @brief Aktualizuje możliwość wykonania złotego ruchu przez gracza.
 * Przypisuje składowej @p golden_possible gracza wskazywanego przez @p p
 * wartość zminnej @p golden_possible będącej parametrem procedury.
 * @param[in,out] p           – wskaźnik na strukturę przechowującą stan gracza,
 * @param[in] golden_possible – wartość @p true wskazująca, że gracz może wykonać
 *                              złoty ruch w danej rozgrywce, a @p false wskazująca
 *                              przeciwnie.
 */
static inline void player_set_golden_possible(player_t *p, bool golden_possible) {
    p->golden_possible = golden_possible;
}

#endif // PLAYER_H
