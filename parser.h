/** @file
 * Interfejs modułu odpowiedzialnego za wczytywanie i parsowanie linii
 * oraz wykonywanie poleceń w trybie wsadowym
 *
 * @author Szymon Czyżmański 417797
 * @date 14.05.2020
 */

#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "gamma.h"

/**
 * Typ wyliczeniowy pozwalający na przechowywanie informacji w jakim trybie pracy
 * znajduje się program.
 */
typedef enum input_mode input_mode_t;

/**
 * Wyliczenia pozwalające na przechowywanie informacji w jakim trybie pracy
 * znajduje się program.
 */
enum input_mode {
    PENDING_MODE,    /**< Domyślny tryb pracy programu, w którym oczekuje on
                      *   na polecenie @ref BATCH lub @ref INTERACTIVE. */
    BATCH_MODE,      /**< Tryb wsadowy, do którego program przechodzi w wyniku
                      *   poprawnego wykonania polecenia @ref BATCH. W trybie tym
                      *   program oczekuje poleceń, każde w osobnym wierszu. Rodzaj
                      *   polecenia jest determinowany pierwszym znakiem wiersza. */
    INTERACTIVE_MODE /**< Tryb interaktywny, do którego program przechodzi w wyniku
                      *   poprawnego wykonania polecenia @ref INTERACTIVE. W trybie
                      *   tym program wyświetla planszę, a pod nią wiersz zachęcający
                      *   gracza do wykonania ruchu. */
};

/** @brief Wczytuje kolejne linie ze standardowego wejścia i je interpretuje.
 * Wczytuje kolejne linie ze standardowego wejścia i dokonuje ich interpretacji,
 * komunikaty o poprawnym wykonaniu poleceń wypisując na standardowe wyjście,
 * a komunikaty o błędach wypisując na standardowe wyjście diagnostyczne.
 * @param[in,out] g       – wskaźnik na strukturę przechowującą stan gry,
 * @param[in,out] buf     – wskaźnik do wskaźnika na bufor, do którego mają być
 *                          wczytywane linie,
 * @param[in] buffer_size – rozmiar bufora wskazywanego przez wskaźnik, na który
 *                          to wskaźnik wskazuje wskaźnik @p buf,
 * @param[in,out] mode    – wskaźnik na zmienną przyjmującą jedną z wartości
 *                          zdefiniowanych w wyliczeniu @ref input_mode,
 *                          określającą, w jakim trybie aktualnie pracuje program.
 * @return Wartość @p false, gdy podczas wczytywania linii wystąpił błąd krytyczny
 * uniemożliwiający dalsze działania programu, na przykład spowodowany brakiem
 * pamięci, a @p true w przeciwnym przypadku.
 */
bool read_lines(gamma_t **g, char **buf, size_t buffer_size, input_mode_t *mode);

#endif // PARSER_H
