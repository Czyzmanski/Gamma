//
// Created by szymon on 06.05.2020.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>

#include "inter_mode.h"
#include "gamma.h"

#define FREE_FIELD '.'

#define ROW_CLEAR_FROM_CURSOR_TO_END "\x1b0dK"

#define CURSOR_MOVE_TO "\x1b[%d;%df"

typedef struct cursor cursor_t;

struct cursor {
    uint32_t row;
    uint64_t col;
};



static uint64_t cursor_move_left_next_column(const char *row, uint64_t curr_col) {
    if (curr_col == 0) {
        return curr_col;
    }
    else {
        uint64_t i = curr_col - 1;

        while (i > 0 && isspace(row[i])) {
            i--;
        }

        if (i > 0) {
            return i;
        }
        else {
            return isspace(row[i]) ? curr_col : i;
        }
    }
}

static uint64_t cursor_move_right_next_column(const char *row, uint64_t curr_col) {
    uint64_t i = curr_col + 1;

    while (row[i] != '\0' && isdigit(row[i])) {
        i++;
    }

    if (row[i] == '\0') {
        return curr_col;
    }
    else {
        while (isspace(row[i])) {
            i++;
        }

        return i;
    }
}

static uint64_t cursor_vertical_move_next_column(const char *row,
                                                 uint64_t curr_col) {
    if (isspace(row[curr_col])) {
        do {
            curr_col++;
        } while (isspace(row[curr_col]));
    }
    else {
        while (curr_col > 0 && isdigit(row[curr_col])) {
            curr_col--;
        }

        if (isspace(row[curr_col])) {
            curr_col++;
        }
    }

    return curr_col;
}

static void cursor_move_up_next_coordinates(char **board, const cursor_t *cur,
                                            uint32_t *next_row, uint64_t *next_col) {
    if (cur->row == 0) {
        *next_row = cur->row, *next_col = cur->col;
    }
    else {
        *next_row = cur->row - 1;
        *next_col = cursor_vertical_move_next_column(board[*next_row], cur->col);
    }
}

static void cursor_move_down_next_coordinates(char **board, uint32_t board_height,
                                              const cursor_t *cur,
                                              uint32_t *next_row,
                                              uint64_t *next_col) {
    if (cur->row == board_height - 1) {
        *next_row = cur->row, *next_col = cur->col;
    }
    else {
        *next_row = cur->row + 1;
        *next_col = cursor_vertical_move_next_column(board[*next_row], cur->col);
    }
}

static inline void cursor_move_to(cursor_t *cur, int row, int col) {
    cur->row = row, cur->col = col;
    printf(CURSOR_MOVE_TO, row, col);
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
