/** @file
 * Interfejs klasy przechowującej stan pola na planszy
 *
 * @author Szymon Czyżmański 417797
 * @date 11.04.2020
 */

#ifndef FIELD_H
#define FIELD_H

#include <stdio.h>

#include "player.h"

/**
 * Znak reprezentujący wolne pole.
 */
#define FREE_FIELD '.'
/**
 * Maksymalna długość, jaką może mieć tekstowa reprezentacja pola na planszy,
 * równa liczbie cyfr potrzebnych do zapisania największego numeru gracza mogącego
 * brać udział w jakiejkolwiek rozgrywce, dodać jeden na odstęp między kolumnami
 * kiedy jest więcej niż dziewięciu graczy.
 */
#define FIELD_MAX_WIDTH 11

/**
 * Typ wyliczeniowy pozwalający na przechowywanie informacji o statusie pola.
 */
typedef enum status status_t;

/**
 * Wyliczenia pozwalajace na przechowywanie informacji o statusie
 * pola, zmieniającym się w wyniku wykonywania przeszukiwania w głąb (DFS)
 * sprawdzającym legalność złotego ruchu.
 */
enum status {
    UNCHECKED, /**< Domyślny status pola. */
    COUNTED,   /**< Pole zostało sprawdzone podczas liczenia niepołączonych
                *   składowych danego obszaru gracza, któremu jest zabierane
                *   pole, mogących powstać w wyniku wykonywania złotego ruchu
                *   przez przeciwnika. */
    MODIFIED   /**< Pole zostało uwzględnione podczas modyfikowania obszaru
                *   z którego zostało zabrane graczowi pole w wyniku wykonania
                *   złotego ruchu przez przeciwnika. */
};

/**
 * Typ struktury przechowującej stan pola (@p x, @p y).
 */
typedef struct field field_t;

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

/** @brief Inicjuje strukturę przechowującą stan pola (@p x, @p y).
 * Inicjuje strukturę wskazywaną przez @p f tak, aby reprezentowała początkowy
 * stan pola.
 * @param[in,out] f     – wskaźnik na strukturę mającą przechowywać stan
 *                        pola (@p x, @p y),
 * @param[in] x         – numer kolumny, liczba nieujemna mniejsza od wartości
 *                        @p width z funkcji @ref gamma_new
 * @param[in] y         – numer wiersza, liczba nieujemna mniejsza od wartości
 *                        @p height z funkcji @ref gamma_new,
 */
static inline void field_init(field_t *f, uint32_t x, uint32_t y) {
    f->x = x;
    f->y = y;
    f->owner = NULL;
    f->parent = NULL;
    f->rank = 0;
    f->status = UNCHECKED;
}

/** @brief Podaje współrzędną @p x pola (@p x, @p y).
 * Podaje numer kolumny @p x na którym znajduje się pole wskazywane przez @p f.
 * @param[in] f         – wskaźnik na strukturę przechowującą stan pola.
 * @return Numer kolumny @p x na którym znajduje się pole wskazywane przez @p f.
 */
static inline uint32_t field_x(field_t *f) {
    return f->x;
}

/** @brief Podaje współrzędną @p y pola (@p x, @p y).
 * Podaje numer wiersza @p y na którym znajduje się pole wskazywane przez @p f.
 * @param[in] f         – wskaźnik na strukturę przechowującą stan pola.
 * @return Numer wiersza @p y na którym znajduje się pole wskazywane przez @p f.
 */
static inline uint32_t field_y(field_t *f) {
    return f->y;
}

/** @brief Podaje wskaźnik do właściciela pola.
 * Podaje wskaźnik @p owner na strukturę gracza mającego pionek na polu
 * wskazywanym przez @p f.
 * @param[in] f         – wskaźnik na strukturę przechowującą stan pola.
 * @return Wskaźnik @p owner na strukturę gracza mającego pionek na polu
 * wskazywanym przez @p f.
 */
static inline player_t *field_owner(field_t *f) {
    return f->owner;
}

/** @brief Aktualizuje właściciela pola.
 * Przypisuje polu właściciela, czyli gracza, którego pionek stoi na polu
 * wskazywanym przez @p f.
 * Przypisuje wskaźnikowi @p owner, będącego składową struktury pola wskazywanej
 * przez @p f, wartość wskaźnika @p owner będącego parametrem procedury.
 * @param[in,out] f     – wskaźnik na strukturę przechowującą stan pola,
 * @param[in] owner     – wskaźnik na strukturę przechowującą stan gracza,
 *                        którego pionek znajduje się na polu @p f.
 */
static inline void field_set_owner(field_t *f, player_t *owner) {
    f->owner = owner;
}

/** @brief Sprawdza czy pole jest wolne.
 * Sprawdza, czy pole wskazywane przez @p f jest wolne, to znaczy czy wartość
 * składowej @ref field::owner jest równa NULL.
 * @param[in,out] f     – wskaźnik na strukturę przechowującą stan pola.
 */
static inline bool field_is_free(field_t *f) {
    return f->owner == NULL;
}

/** @brief Podaje wskaźnik do rodzica pola.
 * Podaje wskaźnik @p parent na strukturę pola będącego rodzicem pola
 * wskazywanego przez @p f.
 * @param[in] f         – wskaźnik na strukturę przechowującą stan pola.
 * @return Wskaźnik @p parent na strukturę pola będącego rodzicem pola
 * wskazywanego przez @p f.
 */
static inline field_t *field_parent(field_t *f) {
    return f->parent;
}

/** @brief Aktualizuje rodzica pola.
 * Przypisuje wskaźnikowi @p parent, będącego składową struktury pola wskazywanej
 * przez @p f, wartość wskaźnika @p parent będącego parametrem procedury.
 * @param[in,out] f     – wskaźnik na strukturę przechowującą stan pola,
 * @param[in] parent    – wskaźnik na strukturę przechowującą stan pola,
 *                        które ma się stać rodzicem pola wskazywanego przez @p f.
 */
static inline void field_set_parent(field_t *f, field_t *parent) {
    f->parent = parent;
}

/** @brief Podaje rangę pola.
 * Podaje wartość @p rank pola wskazywanego przez @p f.
 * @param[in] f         – wskaźnik na strukturę przechowującą stan pola.
 * @return Wartość @p rank pola wskazywanego przez @p f.
 */
static inline uint32_t field_rank(field_t *f) {
    return f->rank;
}

/** @brief Aktualizuje rangę pola.
 * Przypisuje składowej @p rank pola wskazywanego przez @p f wartość zmiennej
 * @p rank będącej paramatrem procedury.
 * @param[in,out] f     – wskaźnik na strukturę przechowującą stan pola,
 * @param[in] rank      – ranga, liczba całkowita nieujemna.
 */
static inline void field_set_rank(field_t *f, uint32_t rank) {
    f->rank = rank;
}

/** @brief Podaje status pola.
 * Podaje wartość @p status pola wskazywanego przez @p f.
 * @param[in] f         – wskaźnik na strukturę przechowującą stan pola.
 * @return Wartość @p status pola wskazywanego przez @p f, jedna z wartości
 * wyliczenia @ref status.
 */
static inline status_t field_status(field_t *f) {
    return f->status;
}

/** @brief Aktualizuje status pola.
 * Przypisuje składowej @p status pola wskazywanego przez @p f wartość zmiennej
 * @p status będącej paramatrem procedury.
 * @param[in,out] f     – wskaźnik na strukturę przechowującą stan pola,
 * @param[in] status    – status pola, jedna z wartości wyliczenia @ref status.
 */
static inline void field_set_status(field_t *f, status_t status) {
    f->status = status;
}

/** @brief Daje napis reprezentujący pole.
 * Wpisuje do bufora długości @p FIELD_MAX_WIDTH + 1 wskazywanego przez
 * @p repr tekstową reprezentację pola.
 * Długość tej reprezentacji określona jest przez @p field_width,
 * wartość nie większą od @p FIELD_MAX_WIDTH.
 * Znak null jest dopisywany na końcu wpisanej reprezentacji pola.
 * @param[in] f           – wskaźnik na strukturę przechowującą stan pola,
 * @param[in,out] repr    – wskaźnik na bufor długości @p FIELD_MAX_WIDTH + 1,
 *                          do którego ma zostać wpisana tekstowa reprezentacja pola,
 * @param[in] field_width – długość, jaką ma mieć tekstowa reprezentacja pola.
 */
static inline void field_repr(field_t *f,
                              char repr[FIELD_MAX_WIDTH + 1], unsigned field_width) {
    if (field_is_free(f)) {
        repr[field_width] = '\0';
        repr[field_width - 1] = FREE_FIELD;

        char padding = ' ';

        for (int i = field_width - 2; i >= 0; i--) {
            repr[i] = padding;
        }
    }
    else {
        sprintf(repr, "%*" PRIu32, field_width, player_number(field_owner(f)));
    }
}

#endif // FIELD_H
