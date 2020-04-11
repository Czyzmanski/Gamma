/** @file
 * Interfejs klasy przechowującej stan pola na planszy
 *
 * @author Szymon Czyżmański
 * @date 11.04.2020
 */

#ifndef FIELD_H
#define FIELD_H

#include "player.h"

/**
 * Struktura przechowująca stan pola.
 */
typedef struct field field_t;

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
 * Typ wyliczeniowy pozwalający na przechowywanie informacji o statusie pola.
 */
typedef enum status status_t;

/** @brief Tworzy strukturę przechowującą stan pola (@p x, @p y).
 * Alokuje pamięć na nową strukturę przechowującą stan pola (@p x, @p y).
 * Inicjuje tę strukturę tak, aby reprezentowała początkowy stan pola.
 * @param[in] x         – numer kolumny, liczba nieujemna mniejsza od wartości
 *                        @p width z funkcji @ref gamma_new
 * @param[in] y         – numer wiersza, liczba nieujemna mniejsza od wartości
 *                        @p height z funkcji @ref gamma_new,
 * @param[in] owner     – wskaźnik na strukturę przechowującą stan gracza,
 *                        który posiada pionek na polu (@p x, @p y).
 * @return Wskaźnik na utworzoną strukturę lub NULL, gdy nie udało się
 * zaalokować pamięci.
 */
field_t *field_new(uint32_t x, uint32_t y, player_t *owner);

/** @brief Podaje współrzędną @p x pola (@p x, @p y).
 * Podaje numer kolumny @p x na którym znajduje się pole wskazywane przez @p f.
 * @param[in] f         – wskaźnik na strukturę przechowującą stan pola.
 * @return Numer kolumny @p x na którym znajduje się pole wskazywane przez @p f.
 */
uint32_t field_x(field_t *f);

/** @brief Podaje współrzędną @p y pola (@p x, @p y).
 * Podaje numer wiersza @p y na którym znajduje się pole wskazywane przez @p f.
 * @param[in] f         – wskaźnik na strukturę przechowującą stan pola.
 * @return Numer wiersza @p y na którym znajduje się pole wskazywane przez @p f.
 */
uint32_t field_y(field_t *f);

/** @brief Podaje wskaźnik do właściciela pola.
 * Podaje wskaźnik @p owner na strukturę gracza mającego pionek na polu
 * wskazywanym przez @p f.
 * @param[in] f         – wskaźnik na strukturę przechowującą stan pola.
 * @return Wskaźnik @p owner na strukturę gracza mającego pionek na polu
 * wskazywanym przez @p f.
 */
player_t *field_owner(field_t *f);

/** @brief Przypisuje polu właściciela.
 * Przypisuje polu właściciela, czyli gracza, którego pionek stoi na polu
 * wskazywanym przez @p f.
 * Przypisuje wskaźnikowi @p owner, będącego składową struktury pola wskazywanej
 * przez @p f, wartość wskaźnika @p owner będącego parametrem procedury.
 * @param[in,out] f     – wskaźnik na strukturę przechowującą stan pola,
 * @param[in] owner     – wskaźnik na strukturę przechowującą stan gracza,
 *                        którego pionek znajduje się na polu @p f.
 */
void field_set_owner(field_t *f, player_t *owner);

/** @brief Podaje wskaźnik do rodzica pola.
 * Podaje wskaźnik @p parent na strukturę pola będącego rodzicem pola
 * wskazywanego przez @p f.
 * @param[in] f         – wskaźnik na strukturę przechowującą stan pola.
 * @return Wskaźnik @p parent na strukturę pola będącego rodzicem pola
 * wskazywanego przez @p f.
 */
field_t *field_parent(field_t *f);

/** @brief Przypisuje polu rodzica.
 * Przypisuje wskaźnikowi @p parent, będącego składową struktury pola wskazywanej
 * przez @p f, wartość wskaźnika @p owner będącego parametrem procedury.
 * @param[in,out] f     – wskaźnik na strukturę przechowującą stan pola,
 * @param[in] owner     – wskaźnik na strukturę przechowującą stan gracza,
 *                        którego pionek znajduje się na polu @p f.
 */
void field_set_parent(field_t *f, field_t *parent);

/** @brief Podaje rangę pola.
 * Podaje wartość @p rank pola wskazywanego przez @p f.
 * @param[in] f         – wskaźnik na strukturę przechowującą stan pola.
 * @return Wartość @p rank pola wskazywanego przez @p f.
 */
uint32_t field_rank(field_t *f);

/** @brief Przypisuje polu rangę.
 * Przypisuje składowej @p rank pola wskazywanego przez @p f wartość zmiennej
 * @p rank będącej paramatrem procedury.
 * @param[in,out] f     – wskaźnik na strukturę przechowującą stan pola,
 * @param[in] rank      – ranga, liczba całkowita nieujemna.
 */
void field_set_rank(field_t *f, uint32_t rank);

/** @brief Podaje status pola.
 * Podaje wartość @p status pola wskazywanego przez @p f.
 * @param[in] f         – wskaźnik na strukturę przechowującą stan pola.
 * @return Wartość @p status pola wskazywanego przez @p f, jedna z wartości
 * wyliczenia @ref status.
 */
status_t field_status(field_t *f);

/** @brief Przypisuje polu status.
 * Przypisuje składowej @p status pola wskazywanego przez @p f wartość zmiennej
 * @p status będącej paramatrem procedury.
 * @param[in,out] f     – wskaźnik na strukturę przechowującą stan pola,
 * @param[in] status    – status pola, jedna z wartości wyliczenia @ref status.
 */
void field_set_status(field_t *f, status_t status);

/** @brief Usuwa strukturę przechowującą stan pola.
 * Usuwa z pamięci strukturę wskazywaną przez @p f.
 * Nic nie robi, jeśli wskaźnik ten ma wartość NULL.
 * @param[in] f        – wskaźnik na usuwaną strukturę.
 */
void field_delete(field_t *f);

#endif // FIELD_H
