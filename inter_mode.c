#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>

#include "inter_mode.h"
#include "gamma.h"

#define FREE_FIELD '.'

#define ROW_CLEAR_FROM_CURSOR_TO_END "\x1b0dK"

#define CURSOR_MOVE_TO "\x1b[%d;%luf"

typedef struct inter_mode inter_mode_t;

struct inter_mode {
    char **board;
    uint32_t board_height;
    uint64_t board_width;
    unsigned board_field_width;
    uint32_t cursor_row;
    uint64_t cursor_col;
};

static void inter_mode_move_cursor_left(inter_mode_t *im) {
    if (im->cursor_col > 0) {
        const char *row = im->board[im->cursor_row];
        uint64_t col = im->cursor_col - 1;

        /* Znajdź koniec pierwszego pola z lewej. */
        while (col > 0 && isspace(row[col])) {
            col--;
        }

        /* Znajdź początek pierwszego pola z lewej. */
        while (col > 0 && isdigit(row[col]) && im->board_field_width > 1) {
            col--;
        }

        if (col > 0) {
            im->cursor_col = col + isspace(row[col]);
        }
        else if (im->board_field_width == 1) {
            im->cursor_col = col;
        }
    }
}

static void inter_mode_move_cursor_right(inter_mode_t *im) {
    if (im->cursor_col < im->board_width - 1) {
        const char *row = im->board[im->cursor_row];
        uint64_t col = im->cursor_col + 1;

        /* Znajdź koniec obecnego pola. */
        while (col < im->board_width
               && isdigit(row[col])
               && im->board_field_width > 1) {
            col++;
        }

        /* Znajdź początek pierwszego pola z prawej. */
        while (col < im->board_width && isspace(row[col])) {
            col++;
        }

        if (col < im->board_width) {
            im->cursor_col = col;
        }
    }
}

static void inter_mode_move_cursor_vertically_find_proper_column(inter_mode_t *im) {
    const char *row = im->board[im->cursor_row];
    uint64_t col = im->cursor_col;

    if (isspace(row[col])) {
        do {
            col++;
        } while (isspace(row[col]));
    }
    else if (im->board_field_width > 1) {
        while (col > 0 && isdigit(row[col])) {
            col--;
        }
    }

    im->cursor_col = col + isspace(row[col]);
}

static void inter_mode_move_cursor_up(inter_mode_t *im) {
    if (im->cursor_row > 0) {
        im->cursor_row--;
        inter_mode_move_cursor_vertically_find_proper_column(im);
    }
}

static void inter_mode_move_cursor_down(inter_mode_t *im) {
    if (im->cursor_row < im->board_height - 1) {
        im->cursor_row++;
        inter_mode_move_cursor_vertically_find_proper_column(im);
    }
}

static inline void inter_mode_update_cursor_on_screen(inter_mode_t *im) {
    printf(CURSOR_MOVE_TO, im->cursor_row, im->cursor_col);
}

static char **interactive_mode_new_board(gamma_t *g, uint64_t *board_width,
                                         uint32_t *board_height,
                                         unsigned *board_field_width) {
    *board_width = gamma_board_row_len(g);
    *board_height = gamma_board_rows(g);
    *board_field_width = gamma_board_field_width(g);

    char *tmp_board = gamma_board(g);
    check_for_successful_alloc(tmp_board);
    char **new_board = calloc(*board_height, sizeof(char *));
    check_for_successful_alloc(new_board);

    for (uint32_t i = 0; i < *board_height; i++) {
        new_board[i] = malloc((*board_width) * sizeof(char));
        check_for_successful_alloc(new_board[i]);

        strncpy(new_board[i], tmp_board + (*board_width) * i, *board_width);
        new_board[i][*board_width - 1] = '\0';
    }

    free(tmp_board);

    return new_board;
}

void interactive_mode_print_board(char **board, uint32_t board_height) {
    for (uint32_t i = 0; i < board_height; i++) {
        printf("%s\n", board[i]);
    }
}

void interactive_mode_print_prompt(uint32_t player, uint64_t player_busy_fields,
                                   uint32_t player_free_fields,
                                   bool player_gamma_possible) {
    printf("PLAYER %d %lu %d", player, player_busy_fields, player_free_fields);

    if (player_gamma_possible) {
        printf(" G");
    }
}

void interactive_mode_print_players_results(gamma_t *g) {
    uint32_t num_of_players = gamma_players(g);
    for (uint64_t player = 1; player <= num_of_players; player++) {
        printf("PLAYER %lu %lu\n", player, gamma_busy_fields(g, player));
    }
}

void interactive_mode_handle_player_actions(gamma_t *g, char **board,
                                            uint64_t board_width,
                                            uint32_t board_height,
                                            unsigned board_field_width,
                                            uint32_t player) {

}

void interactive_mode_play_game(gamma_t *g, char **board, uint64_t board_width,
                                uint32_t board_height, unsigned board_field_width) {
    cursor_t cur;
    uint32_t num_of_players = gamma_players(g);
    bool any_player_possible_move = true, no_exit_code = true;

    while (any_player_possible_move && no_exit_code) {
        any_player_possible_move = false;

        for (uint64_t player = 1; player <= num_of_players; player++) {
            uint64_t player_busy_fields = gamma_busy_fields(g, player);
            uint64_t player_free_fields = gamma_free_fields(g, player);
            bool player_gamma_possible = gamma_golden_possible(g, player);

            if (player_free_fields > 0 || player_gamma_possible) {
                any_player_possible_move = true;

                /* Move cursor below board. */
                cursor_move_to(&cur, board_height, 0);

                interactive_mode_print_prompt(player, player_busy_fields,
                                              player_free_fields,
                                              player_gamma_possible);

                //TODO: move cursor back to starting position

                //TODO: handle gamer's actions
            }
        }
    }

    /* Move cursor below board. */
    cursor_move_to(&cur, board_height, 0);

    interactive_mode_print_players_results(g);
}

void interactive_mode_launch(gamma_t *g) {
    uint64_t board_width;
    uint32_t board_height, board_field_width;
    char **board = interactive_mode_new_board(g, &board_width, &board_height,
                                              &board_field_width);

    interactive_mode_print_board(board, board_height);

    interactive_mode_play_game(g, board, board_width,
                               board_height, board_field_width);

    free(board);
}
