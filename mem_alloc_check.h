/** @file
 * Definicja funkcji sprawdzającej pomyślność alokacji pamięci
 *
 * @author Szymon Czyżmański 417797
 * @date 13.04.2020
 */

#ifndef MEM_ALLOC_CHECK_H
#define MEM_ALLOC_CHECK_H

#include <stdlib.h>

/** @brief Sprawdza, czy udało się zaalokować pamięć.
 * Sprawdza, czy wartość wskaźnika @p object jest równa NULL.
 * Jeśli tak, kończy działanie programu z kodem wyjścia @p EXIT_FAILURE.
 * @param[in] object – wskaźnik na zaalokowany obiekt w pamięci.
 */
static inline void check_for_successful_alloc(void *object) {
    if (object == NULL) {
        exit(EXIT_FAILURE);
    }
}

#endif // MEM_ALLOC_CHECK_H
