/** @file
 * Przykładowe użycie silnika gry gamma
 *
 * @author Marcin Peczarski <marpe@mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 18.03.2020
 */

// CMake w wersji release wyłącza asercje.
#ifdef NDEBUG
#undef NDEBUG
#endif

#include "gamma.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Tak ma wyglądać plansza po wykonaniu wszystkich testów.
 */
static const char board[] =
        "1.........\n"
        "..........\n"
        "..........\n"
        "......2...\n"
        ".....2....\n"
        "..........\n"
        "..........\n"
        "1.........\n"
        "1221......\n"
        "1.........\n";

/** @brief Testuje silnik gry gamma.
 * Przeprowadza przykładowe testy silnika gry gamma.
 * @return Zero, gdy wszystkie testy przebiegły poprawnie,
 * a w przeciwnym przypadku kod zakończenia programu jest kodem błędu.
 */
int main() {
    gamma_t* board = gamma_new(11, 4, 7, 3);
    assert( board != NULL );


    assert( gamma_move(board, 1, 0, 10) == 0 );
    assert( gamma_move(board, 1, 3, 1) == 1 );
    assert( gamma_busy_fields(board, 1) == 1 );
    assert( gamma_move(board, 2, 1, 7) == 0 );
    assert( gamma_move(board, 2, 1, 2) == 1 );
    assert( gamma_move(board, 3, 3, 10) == 0 );
    assert( gamma_move(board, 3, 4, 3) == 1 );
    assert( gamma_move(board, 4, 2, 9) == 0 );
    assert( gamma_move(board, 4, 1, 1) == 1 );
    assert( gamma_move(board, 5, 1, 10) == 0 );
    assert( gamma_move(board, 6, 3, 0) == 1 );
    assert( gamma_move(board, 6, 10, 1) == 1 );
    assert( gamma_move(board, 7, 1, 9) == 0 );
    assert( gamma_move(board, 7, 3, 2) == 1 );
    assert( gamma_move(board, 1, 0, 6) == 0 );
    assert( gamma_move(board, 2, 3, 7) == 0 );
    assert( gamma_move(board, 3, 0, 8) == 0 );
    assert( gamma_move(board, 4, 0, 1) == 1 );
    assert( gamma_move(board, 4, 2, 2) == 1 );
    assert( gamma_move(board, 5, 3, 7) == 0 );
    assert( gamma_move(board, 6, 7, 2) == 1 );
    assert( gamma_move(board, 7, 0, 10) == 0 );
    assert( gamma_move(board, 1, 3, 8) == 0 );
    assert( gamma_move(board, 1, 0, 1) == 0 );
    assert( gamma_move(board, 2, 2, 5) == 0 );
    assert( gamma_move(board, 2, 6, 1) == 1 );
    assert( gamma_move(board, 3, 7, 3) == 1 );
    assert( gamma_move(board, 4, 1, 7) == 0 );
    assert( gamma_move(board, 5, 1, 2) == 0 );
    assert( gamma_move(board, 6, 0, 6) == 0 );
    assert( gamma_move(board, 6, 2, 1) == 0 );
    assert( gamma_free_fields(board, 6) == 8 );
    assert( gamma_move(board, 7, 2, 4) == 0 );
    assert( gamma_move(board, 7, 10, 0) == 1 );
    assert( gamma_move(board, 1, 10, 0) == 0 );
    assert( gamma_move(board, 2, 2, 4) == 0 );
    assert( gamma_move(board, 2, 0, 2) == 1 );
    assert( gamma_busy_fields(board, 2) == 3 );
    assert( gamma_move(board, 3, 3, 1) == 0 );
    assert( gamma_busy_fields(board, 3) == 2 );
    assert( gamma_move(board, 4, 2, 4) == 0 );
    assert( gamma_move(board, 4, 8, 0) == 1 );
    assert( gamma_move(board, 5, 9, 2) == 1 );
    assert( gamma_move(board, 6, 3, 10) == 0 );
    assert( gamma_move(board, 7, 2, 2) == 0 );
    assert( gamma_move(board, 7, 6, 3) == 1 );
    assert( gamma_busy_fields(board, 7) == 3 );
    assert( gamma_move(board, 1, 1, 9) == 0 );
    assert( gamma_free_fields(board, 1) == 27 );
    assert( gamma_move(board, 2, 3, 10) == 0 );
    assert( gamma_move(board, 2, 5, 1) == 1 );
    assert( gamma_move(board, 3, 9, 3) == 1 );
    assert( gamma_move(board, 4, 1, 7) == 0 );
    assert( gamma_move(board, 4, 5, 3) == 0 );
    assert( gamma_move(board, 5, 2, 4) == 0 );
    assert( gamma_move(board, 5, 8, 0) == 0 );
    assert( gamma_move(board, 6, 0, 5) == 0 );
    assert( gamma_move(board, 6, 5, 3) == 0 );
    assert( gamma_move(board, 7, 0, 1) == 0 );
    assert( gamma_move(board, 7, 4, 0) == 0 );
    assert( gamma_move(board, 1, 2, 0) == 1 );
    assert( gamma_busy_fields(board, 1) == 2 );
    assert( gamma_move(board, 2, 0, 1) == 0 );
    assert( gamma_move(board, 2, 6, 0) == 1 );
    assert( gamma_move(board, 3, 1, 8) == 0 );
    assert( gamma_move(board, 3, 8, 2) == 0 );
    assert( gamma_move(board, 4, 0, 7) == 0 );
    assert( gamma_move(board, 4, 10, 3) == 0 );
    assert( gamma_move(board, 5, 3, 10) == 0 );
    assert( gamma_golden_move(board, 5, 1, 10) == 0 );
    assert( gamma_move(board, 6, 2, 8) == 0 );
    assert( gamma_move(board, 6, 5, 2) == 0 );
    assert( gamma_move(board, 7, 2, 8) == 0 );
    assert( gamma_golden_possible(board, 7) == 1 );
    assert( gamma_move(board, 1, 2, 5) == 0 );
    assert( gamma_busy_fields(board, 1) == 2 );
    assert( gamma_golden_move(board, 1, 2, 9) == 0 );
    assert( gamma_move(board, 2, 2, 0) == 0 );
    assert( gamma_move(board, 2, 6, 1) == 0 );
    assert( gamma_move(board, 4, 3, 0) == 0 );
    assert( gamma_move(board, 4, 6, 2) == 0 );
    assert( gamma_busy_fields(board, 4) == 4 );
    assert( gamma_move(board, 5, 10, 0) == 0 );
    assert( gamma_move(board, 5, 7, 2) == 0 );
    assert( gamma_move(board, 6, 1, 9) == 0 );
    assert( gamma_move(board, 6, 6, 3) == 0 );
    assert( gamma_move(board, 7, 6, 0) == 0 );
    assert( gamma_move(board, 7, 4, 1) == 0 );
    assert( gamma_golden_move(board, 7, 0, 2) == 0 );
    assert( gamma_move(board, 1, 1, 9) == 0 );
    assert( gamma_busy_fields(board, 1) == 2 );
    assert( gamma_move(board, 2, 3, 10) == 0 );
    assert( gamma_move(board, 2, 0, 1) == 0 );
    assert( gamma_busy_fields(board, 2) == 5 );
    assert( gamma_move(board, 3, 3, 1) == 0 );
    assert( gamma_golden_possible(board, 3) == 1 );


    char* board605281148 = gamma_board(board);
    assert( board605281148 != NULL );
    assert( strcmp(board605281148,
                   "....3.73.3.\n"
                   "2247...6.5.\n"
                   "44.1.22...6\n"
                   "..16..2.4.7\n") == 0);
    free(board605281148);
    board605281148 = NULL;
    assert( gamma_move(board, 4, 5, 0) == 0 );
    assert( gamma_move(board, 5, 3, 8) == 0 );
    assert( gamma_move(board, 6, 0, 9) == 0 );
    assert( gamma_move(board, 7, 1, 9) == 0 );
    assert( gamma_move(board, 1, 3, 2) == 0 );
    assert( gamma_golden_possible(board, 1) == 1 );
    assert( gamma_move(board, 2, 7, 0) == 1 );
    assert( gamma_move(board, 2, 3, 1) == 0 );
    assert( gamma_move(board, 3, 0, 3) == 0 );
    assert( gamma_move(board, 4, 2, 8) == 0 );
    assert( gamma_move(board, 5, 1, 4) == 0 );
    assert( gamma_move(board, 5, 5, 2) == 1 );
    assert( gamma_golden_possible(board, 5) == 1 );
    assert( gamma_move(board, 6, 3, 2) == 0 );
    assert( gamma_move(board, 6, 0, 0) == 0 );
    assert( gamma_move(board, 7, 0, 5) == 0 );
    assert( gamma_busy_fields(board, 7) == 3 );
    assert( gamma_move(board, 1, 3, 2) == 0 );
    assert( gamma_move(board, 2, 3, 8) == 0 );
    assert( gamma_move(board, 2, 1, 1) == 0 );
    assert( gamma_move(board, 3, 10, 1) == 0 );
    assert( gamma_free_fields(board, 3) == 5 );
    assert( gamma_golden_possible(board, 3) == 1 );
    assert( gamma_move(board, 5, 4, 0) == 1 );
    assert( gamma_move(board, 6, 2, 6) == 0 );
    assert( gamma_move(board, 6, 3, 0) == 0 );
    assert( gamma_move(board, 7, 2, 3) == 0 );
    assert( gamma_move(board, 7, 3, 0) == 0 );
    assert( gamma_move(board, 1, 0, 1) == 0 );
    assert( gamma_golden_move(board, 1, 3, 9) == 0 );
    assert( gamma_move(board, 2, 1, 2) == 0 );
    assert( gamma_move(board, 3, 2, 0) == 0 );
    assert( gamma_move(board, 3, 3, 1) == 0 );
    assert( gamma_move(board, 5, 3, 5) == 0 );
    assert( gamma_move(board, 5, 5, 3) == 1 );
    assert( gamma_move(board, 6, 3, 0) == 0 );
    assert( gamma_busy_fields(board, 6) == 3 );
    assert( gamma_golden_possible(board, 6) == 1 );
    assert( gamma_move(board, 7, 0, 2) == 0 );
    assert( gamma_move(board, 7, 1, 0) == 0 );
    assert( gamma_move(board, 1, 3, 8) == 0 );
    assert( gamma_golden_move(board, 1, 1, 5) == 0 );
    assert( gamma_golden_possible(board, 2) == 1 );


    char* board493797172 = gamma_board(board);
    assert( board493797172 != NULL );
    assert( strcmp(board493797172,
                   "....3573.3.\n"
                   "2247.5.6.5.\n"
                   "44.1.22...6\n"
                   "..165.224.7\n") == 0);
    free(board493797172);
    board493797172 = NULL;
    assert( gamma_move(board, 3, 3, 8) == 0 );
    assert( gamma_move(board, 3, 8, 1) == 0 );
    assert( gamma_move(board, 4, 1, 8) == 0 );
    assert( gamma_move(board, 4, 8, 2) == 0 );
    assert( gamma_golden_possible(board, 4) == 1 );
    assert( gamma_move(board, 5, 2, 8) == 0 );
    assert( gamma_move(board, 6, 3, 2) == 0 );
    assert( gamma_move(board, 6, 4, 3) == 0 );
    assert( gamma_busy_fields(board, 6) == 3 );


    char* board585856818 = gamma_board(board);
    assert( board585856818 != NULL );
    assert( strcmp(board585856818,
                   "....3573.3.\n"
                   "2247.5.6.5.\n"
                   "44.1.22...6\n"
                   "..165.224.7\n") == 0);
    free(board585856818);
    board585856818 = NULL;
    assert( gamma_move(board, 7, 9, 3) == 0 );
    assert( gamma_move(board, 7, 7, 3) == 0 );
    assert( gamma_golden_possible(board, 7) == 1 );
    assert( gamma_move(board, 1, 1, 8) == 0 );
    assert( gamma_move(board, 2, 6, 1) == 0 );
    assert( gamma_move(board, 3, 4, 2) == 1 );


    gamma_delete(board);




    return 0;
}
