/** @file
 * Interfejs modułu obsługującego tryb interaktywny
 *
 * @author Szymon Czyżmański 417797
 * @date 14.05.2020
 */

#ifndef INTER_MODE_H
#define INTER_MODE_H

#include "gamma.h"

/** @brief Uruchamia tryb interaktywny.
 * @param[in,out] g – wskaźnik na strukturę przechowującą stan gry.
 * @return Wartość @p true, jeżeli poprawnie udało się uruchomić tryb interaktywny
 * i przeprowadzić rozgrywkę oraz w jej trakcie nie nastąpił żaden krytyczny błąd,
 * a @p false w przeciwnym przypadku.
 */
bool inter_mode_launch(gamma_t *g);

#endif // INTER_MODE_H
