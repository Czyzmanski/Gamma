/** @file
 * Definicja funkcji sprawdzającej pomyślność alokacji pamięci
 *
 * @author Szymon Czyżmański 417797
 * @date 13.04.2020
 */

#ifndef MEM_ALLOC_CHECK_H
#define MEM_ALLOC_CHECK_H

#include <stdlib.h>

/**
 * Kod wyjścia informujący, że podczas działania programu zabrakło pamięci.
 */
#define NO_MEMORY_EXIT_CODE 1

/** @brief Sprawdza, czy udało się zaalokować pamięć.
 * Sprawdza, czy wartość wskaźnika @p object jest równa NULL.
 * Jeśli tak, kończy działanie programu z kodem wyjścia @p NO_MEMORY_EXIT_CODE,
 * sygnalizującym, że zabrakło pamięci.
 * @param[in] object – wskaźnik na zaalokowany obiekt w pamięci.
 */
static inline void check_for_successful_alloc(void *object) {
    if (object == NULL) {
        exit(NO_MEMORY_EXIT_CODE);
    }
}

#endif // MEM_ALLOC_CHECK_H
