#ifndef MEM_ALLOC_CHECK_H
#define MEM_ALLOC_CHECK_H

#include <stdlib.h>

#define NO_MEMORY_EXIT_CODE 1

static inline void check_for_successful_alloc(void *object) {
    if (object == NULL) {
        exit(NO_MEMORY_EXIT_CODE);
    }
}

#endif // MEM_ALLOC_CHECK_H
