#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

#include "inter_mode.h"
#include "gamma.h"
#include "mem_alloc_check.h"

#define FREE_FIELD '.'

#define ROW_CLEAR_FROM_CURSOR_TO_END "\x1b0dK"

#define CURSOR_MOVE_TO "\x1b[%d;%luf"

typedef struct inter_mode inter_mode_t;

struct inter_mode {
    gamma_t *g;
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

static inline void inter_mode_move_cursor_below_board(inter_mode_t *im) {
    im->cursor_row = im->board_height, im->cursor_col = 0;
    inter_mode_update_cursor_on_screen(im);
}

static void inter_mode_print_prompt(inter_mode_t *im, uint32_t player) {
    uint64_t player_busy_fields = gamma_busy_fields(im->g, player);
    uint64_t player_free_fields = gamma_free_fields(im->g, player);
    bool player_golden_possible = gamma_golden_possible(im->g, player);

    printf("PLAYER %d %lu %lu", player, player_busy_fields, player_free_fields);

    if (player_golden_possible) {
        printf(" G");
    }
}

static void inter_mode_print_summary(inter_mode_t *im) {
    uint32_t num_of_players = gamma_players(im->g);

    for (uint64_t player = 1; player <= num_of_players; player++) {
        printf("PLAYER %lu %lu\n", player, gamma_busy_fields(im->g, player));
    }
}

void inter_mode_handle_input(inter_mode_t *im, uint32_t player) {

}

void inter_mode_play_gamma(inter_mode_t *im) {
    uint32_t num_of_players = gamma_players(im->g);
    bool any_player_possible_move = true, no_exit_code = true;

    while (any_player_possible_move && no_exit_code) {
        any_player_possible_move = false;

        for (uint64_t player = 1; player <= num_of_players; player++) {
            uint64_t player_free_fields = gamma_free_fields(im->g, player);
            bool player_gamma_possible = gamma_golden_possible(im->g, player);

            if (player_free_fields > 0 || player_gamma_possible) {
                any_player_possible_move = true;

                inter_mode_move_cursor_below_board(im);
                inter_mode_print_prompt(im, player);

                //TODO: move cursor back to starting position

                inter_mode_handle_input(im, player);
            }
        }
    }

    inter_mode_move_cursor_below_board(im);
    inter_mode_print_summary(im);
}

static void inter_mode_delete_board(inter_mode_t *im, uint32_t rows_to_delete) {
    for (uint32_t i = 0; i < rows_to_delete; i++) {
        free(im->board[i]);
    }
    free(im->board);
}

static void inter_mode_init(inter_mode_t *im, gamma_t *g) {
    im->g = g;
    im->cursor_row = im->cursor_col = 0;
    im->board_height = gamma_board_rows(g);
    im->board_width = gamma_board_row_len(g);
    im->board_field_width = gamma_board_field_width(g);

    char *tmp_board = gamma_board(g);
    check_for_successful_alloc(tmp_board);

    im->board = calloc(im->board_height, sizeof(char *));
    check_for_successful_alloc(im->board);

    for (uint32_t i = 0; i < im->board_height; i++) {
        im->board[i] = malloc(im->board_width * sizeof(char));

        if (im->board[i] == NULL) {
            inter_mode_delete_board(im, i);
            free(tmp_board);

            exit(EXIT_FAILURE);
        }

        strncpy(im->board[i], tmp_board + im->board_width * i, im->board_width);
        im->board[i][im->board_width - 1] = '\0';
    }

    free(tmp_board);
}

static inline void inter_mode_print_board(inter_mode_t *im) {
    for (uint32_t i = 0; i < im->board_height; i++) {
        printf("%s\n", im->board[i]);
    }
}

void inter_mode_launch(gamma_t *g) {
    inter_mode_t im;
    inter_mode_init(&im, g);

    inter_mode_print_board(&im);
    inter_mode_play_gamma(&im);

    inter_mode_delete_board(&im, im.board_height);
}
