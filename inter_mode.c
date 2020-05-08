#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <inttypes.h>

#include "inter_mode.h"
#include "gamma.h"
#include "mem_alloc_check.h"

#define PLAYER_MAX_DIGITS 10

#define IS_FREE_FIELD_CHAR(c) (c == '.')

#define IS_ESC(c) (c == '\x1b')
#define IS_LEFT_SQUARE_BRACKET(c) (c == '[')
#define IS_CURSOR_UP_CHAR(c) (c == 'A')
#define IS_CURSOR_DOWN_CHAR(c) (c == 'B')
#define IS_CURSOR_RIGHT_CHAR(c) (c == 'C')
#define IS_CURSOR_MOVEMENT_CHAR(c) ('A' <= c && c <= 'D')

#define IS_MOVE_CHAR(c) (c == ' ')
#define IS_GOLDEN_MOVE_CHAR(c) (c == 'g' || c == 'G')
#define IS_QUIT_CHAR(c) (c == 'c' || c == 'C')
#define IS_END_OF_GAME_CHAR(c) (c == 4)

#define MOVE_CURSOR_TO "\x1b[%" PRIu32 ";%" PRIu64 "H"
#define MOVE_CURSOR_TO_TOP_LEFT_CORNER "\x1b[;H"

#define TURN_ON_REVERSE_VIDEO "\x1b[7m"
#define TURN_OFF_REVERSE_VIDEO "\x1b[0m"

#define CLEAR_SCREEN "\x1b[2J"
#define CLEAR_ROW_FROM_CURSOR_TO_END "\x1b[0K"

#define PROMPT "PLAYER %" PRIu32 " %" PRIu64 " %" PRIu64

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

typedef void (*cursor_move_fun)(inter_mode_t *imode);

typedef bool (*gamma_move_fun)(gamma_t *g, uint32_t player, uint32_t x, uint32_t y);

static uint32_t inter_mode_current_field_number(inter_mode_t *imode) {
    if (imode->board_field_width == 1) {
        return imode->cursor_col;
    }
    else if (imode->cursor_col <= imode->board_field_width) {
        return 0;
    }
    else {
        return 1 + (imode->cursor_col - imode->board_field_width) /
                   (imode->board_field_width + 1);
    }
}

static uint64_t inter_mode_current_field_beginning(inter_mode_t *imode) {
    uint32_t field_num = inter_mode_current_field_number(imode);

    if (imode->board_field_width == 1
        || imode->cursor_col <= imode->board_field_width) {

        return field_num;
    }
    else {
        return imode->board_field_width + 1 +
               (field_num - 1) * (imode->board_field_width + 1);
    }
}

static unsigned inter_mode_busy_characters_in_current_field(inter_mode_t *imode) {
    if (imode->board_field_width == 1) {
        return 1;
    }
    else {
        uint64_t col = imode->cursor_col;
        const char *row = imode->board[imode->cursor_row];

        if (IS_FREE_FIELD_CHAR(row[col])) {
            return 1;
        }
        else {
            while (col < imode->board_width - 1 && isdigit(row[col])) {
                col++;
            }

            return col - imode->cursor_col;
        }
    }
}

static void inter_mode_turn_reverse_off_and_reprint_row(inter_mode_t *imode) {
    printf(CLEAR_ROW_FROM_CURSOR_TO_END);
    printf(TURN_OFF_REVERSE_VIDEO);

    printf("%s", imode->board[imode->cursor_row] + imode->cursor_col);

    printf(MOVE_CURSOR_TO, imode->cursor_row + 1, imode->cursor_col + 1);
}

static void inter_mode_turn_reverse_on_and_reprint_row(inter_mode_t *imode) {
    const char *row = imode->board[imode->cursor_row];
    unsigned to_reverse = inter_mode_busy_characters_in_current_field(imode);

    printf(CLEAR_ROW_FROM_CURSOR_TO_END);
    printf(TURN_ON_REVERSE_VIDEO);

    printf("%.*s", to_reverse, row + imode->cursor_col);

    printf(TURN_OFF_REVERSE_VIDEO);

    printf("%s", row + imode->cursor_col + to_reverse);

    printf(MOVE_CURSOR_TO, imode->cursor_row + 1, imode->cursor_col + 1);
}

static inline void inter_mode_update_board_display(inter_mode_t *imode,
                                                   cursor_move_fun cur_mv_f) {
    inter_mode_turn_reverse_off_and_reprint_row(imode);
    cur_mv_f(imode);
    inter_mode_turn_reverse_on_and_reprint_row(imode);
}

static void inter_mode_move_cursor_left(inter_mode_t *imode) {
    if (imode->cursor_col > 0) {
        const char *row = imode->board[imode->cursor_row];
        uint64_t col = imode->cursor_col - 1;

        while (col > 0 && isspace(row[col])) {
            col--;
        }

        if (imode->board_field_width > 1) {
            while (col > 0 && isdigit(row[col])) {
                col--;
            }
        }

        if (col > 0) {
            imode->cursor_col = col + isspace(row[col]);
        }
        else if (imode->board_field_width == 1) {
            imode->cursor_col = col;
        }

        printf(MOVE_CURSOR_TO, imode->cursor_row + 1, imode->cursor_col + 1);
    }
}

static void inter_mode_move_cursor_right(inter_mode_t *imode) {
    if (imode->cursor_col < imode->board_width - 1) {
        const char *row = imode->board[imode->cursor_row];
        uint64_t col = imode->cursor_col + 1;

        if (imode->board_field_width > 1) {
            while (col < imode->board_width - 1 && isdigit(row[col])) {
                col++;
            }
        }

        while (col < imode->board_width - 1 && isspace(row[col])) {
            col++;
        }

        if (col < imode->board_width - 1) {
            imode->cursor_col = col;
        }

        printf(MOVE_CURSOR_TO, imode->cursor_row + 1, imode->cursor_col + 1);
    }
}

static void inter_mode_move_cursor_vertically_find_column(inter_mode_t *imode) {
    const char *row = imode->board[imode->cursor_row];
    uint64_t col = imode->cursor_col;

    if (isspace(row[col])) {
        do {
            col++;
        } while (isspace(row[col]));
    }
    else if (imode->board_field_width > 1) {
        while (col > 0 && isdigit(row[col])) {
            col--;
        }
    }

    imode->cursor_col = col + isspace(row[col]);
}

static void inter_mode_move_cursor_up(inter_mode_t *imode) {
    if (imode->cursor_row > 0) {
        imode->cursor_row--;
        inter_mode_move_cursor_vertically_find_column(imode);

        printf(MOVE_CURSOR_TO, imode->cursor_row + 1, imode->cursor_col + 1);
    }
}

static void inter_mode_move_cursor_down(inter_mode_t *imode) {
    if (imode->cursor_row < imode->board_height - 1) {
        imode->cursor_row++;
        inter_mode_move_cursor_vertically_find_column(imode);

        printf(MOVE_CURSOR_TO, imode->cursor_row + 1, imode->cursor_col + 1);
    }
}

static inline void inter_mode_move_cursor_below_board(inter_mode_t *imode) {
    inter_mode_turn_reverse_off_and_reprint_row(imode);
    imode->cursor_row = imode->board_height, imode->cursor_col = 0;
    printf(MOVE_CURSOR_TO, imode->cursor_row + 1, imode->cursor_col + 1);
}

static void inter_mode_move_cursor_to_starting_position(inter_mode_t *imode) {
    imode->cursor_row = imode->board_height / 2;
    imode->cursor_col = imode->board_width / 2;
    imode->cursor_col = inter_mode_current_field_beginning(imode);

    while (isspace(imode->board[imode->cursor_row][imode->cursor_col])) {
        imode->cursor_col++;
    }

    printf(MOVE_CURSOR_TO, imode->cursor_row + 1, imode->cursor_col + 1);
}

static void inter_mode_print_prompt(inter_mode_t *imode, uint32_t player) {
    uint64_t player_busy_fields = gamma_busy_fields(imode->g, player);
    uint64_t player_free_fields = gamma_free_fields(imode->g, player);
    bool player_golden_possible = gamma_golden_possible(imode->g, player);

    printf(CLEAR_ROW_FROM_CURSOR_TO_END);
    printf(PROMPT, player, player_busy_fields, player_free_fields);

    if (player_golden_possible) {
        printf(" G");
    }
}

static void inter_mode_print_summary(inter_mode_t *imode) {
    uint32_t num_of_players = gamma_players(imode->g);

    printf(CLEAR_ROW_FROM_CURSOR_TO_END);

    for (uint64_t player = 1; player <= num_of_players; player++) {
        printf("PLAYER %lu %lu\n", player, gamma_busy_fields(imode->g, player));
    }
}

static inline void inter_mode_print_board(inter_mode_t *imode) {
    for (uint32_t i = 0; i < imode->board_height; i++) {
        printf("%s\n", imode->board[i]);
    }
}

static bool inter_mode_gamma_move(inter_mode_t *imode, gamma_move_fun g_mv_f,
                                  uint32_t player, uint32_t x, uint32_t y) {
    if (g_mv_f(imode->g, player, x, y)) {
        char *row = imode->board[imode->cursor_row];
        uint64_t field_beg = inter_mode_current_field_beginning(imode);

        if (imode->board_field_width == 1) {
            row[field_beg] = (player % 10) + '0';
        }
        else {
            char digits[PLAYER_MAX_DIGITS + 1];

            sprintf(digits, "%*" PRIu32, imode->board_field_width, player);
            strncpy(row + field_beg, digits, imode->board_field_width);
        }

        inter_mode_turn_reverse_off_and_reprint_row(imode);

        imode->cursor_col = field_beg;

        while (isspace(row[imode->cursor_col])) {
            imode->cursor_col++;
        }

        printf(MOVE_CURSOR_TO, imode->cursor_row + 1, imode->cursor_col + 1);
        inter_mode_turn_reverse_on_and_reprint_row(imode);

        return true;
    }
    else {
        return false;
    }
}

static int inter_mode_handle_input_when_esc(inter_mode_t *imode) {
    int c;
    do {
        c = getchar();
    } while (IS_ESC(c));

    if (IS_LEFT_SQUARE_BRACKET(c)) {
        c = getchar();

        if (IS_CURSOR_MOVEMENT_CHAR(c)) {
            if (IS_CURSOR_UP_CHAR(c)) {
                inter_mode_update_board_display(imode, inter_mode_move_cursor_up);
            }
            else if (IS_CURSOR_DOWN_CHAR(c)) {
                inter_mode_update_board_display(imode, inter_mode_move_cursor_down);
            }
            else if (IS_CURSOR_RIGHT_CHAR(c)) {
                inter_mode_update_board_display(imode, inter_mode_move_cursor_right);
            }
            else {
                inter_mode_update_board_display(imode, inter_mode_move_cursor_left);
            }

            c = getchar();
        }
    }

    return c;
}

static void inter_mode_handle_input(inter_mode_t *imode, uint32_t player,
                                    bool *end_of_game_char) {
    int c = getchar();
    bool quit = false, player_valid_move = false;
    bool player_golden_possible = gamma_golden_possible(imode->g, player);

    while (!player_valid_move && !quit && !(*end_of_game_char)) {
        if (IS_QUIT_CHAR(c)) {
            quit = true;
        }
        else if (IS_END_OF_GAME_CHAR(c)) {
            *end_of_game_char = true;
        }
        else if (IS_MOVE_CHAR(c) || IS_GOLDEN_MOVE_CHAR(c)) {
            uint32_t x = inter_mode_current_field_number(imode);
            uint32_t y = imode->board_height - 1 - imode->cursor_row;

            if (IS_MOVE_CHAR(c)) {
                player_valid_move = inter_mode_gamma_move(imode, gamma_move,
                                                          player, x, y);
            }
            else if (player_golden_possible) {
                player_valid_move = inter_mode_gamma_move(imode, gamma_golden_move,
                                                          player, x, y);
            }

            if (!player_valid_move) {
                c = getchar();
            }
        }
        else if (IS_ESC(c)) {
            c = inter_mode_handle_input_when_esc(imode);
        }
        else {
            c = getchar();
        }
    }
}

void inter_mode_play_gamma(inter_mode_t *imode) {
    uint32_t num_of_players = gamma_players(imode->g);
    bool any_player_possible_move = true, end_of_game_char = false;

    while (any_player_possible_move && !end_of_game_char) {
        any_player_possible_move = false;
        uint64_t player = 1;

        while (player <= num_of_players && !end_of_game_char) {
            uint64_t player_free_fields = gamma_free_fields(imode->g, player);
            bool player_gamma_possible = gamma_golden_possible(imode->g, player);

            if (player_free_fields > 0 || player_gamma_possible) {
                any_player_possible_move = true;

                uint32_t cursor_row = imode->cursor_row;
                uint64_t cursor_col = imode->cursor_col;

                inter_mode_move_cursor_below_board(imode);
                inter_mode_print_prompt(imode, player);

                imode->cursor_row = cursor_row, imode->cursor_col = cursor_col;
                printf(MOVE_CURSOR_TO, imode->cursor_row + 1, imode->cursor_col + 1);

                inter_mode_turn_reverse_on_and_reprint_row(imode);

                inter_mode_handle_input(imode, player, &end_of_game_char);
            }

            player++;
        }
    }

    inter_mode_move_cursor_below_board(imode);
    inter_mode_print_summary(imode);
}

static void inter_mode_delete_board(inter_mode_t *imode, uint32_t rows_to_delete) {
    for (uint32_t i = 0; i < rows_to_delete; i++) {
        free(imode->board[i]);
    }
    free(imode->board);
}

static void inter_mode_init(inter_mode_t *imode, gamma_t *g) {
    imode->g = g;
    imode->cursor_row = imode->cursor_col = 0;
    imode->board_height = gamma_board_rows(g);
    imode->board_width = gamma_board_row_len(g);
    imode->board_field_width = gamma_board_field_width(g);

    char *tmp_board = gamma_board(g);
    check_for_successful_alloc(tmp_board);

    imode->board = calloc(imode->board_height, sizeof(char *));
    check_for_successful_alloc(imode->board);

    for (uint32_t i = 0; i < imode->board_height; i++) {
        imode->board[i] = malloc(imode->board_width * sizeof(char));

        if (imode->board[i] == NULL) {
            inter_mode_delete_board(imode, i);
            free(tmp_board);

            exit(EXIT_FAILURE);
        }

        strncpy(imode->board[i],
                tmp_board + imode->board_width * i, imode->board_width);

        imode->board[i][imode->board_width - 1] = '\0';
    }

    free(tmp_board);
}

void inter_mode_launch(gamma_t *g) {
    struct termios old_term, new_term;

    if (tcgetattr(STDIN_FILENO, &old_term) != 0) {
        exit(EXIT_FAILURE);
    }

    new_term = old_term;
    new_term.c_lflag &= ~(ICANON | ECHO);

    if (tcsetattr(STDIN_FILENO, TCSANOW, &new_term) != 0) {
        exit(EXIT_FAILURE);
    }


    inter_mode_t imode;
    inter_mode_init(&imode, g);

    printf(CLEAR_SCREEN);
    printf(MOVE_CURSOR_TO_TOP_LEFT_CORNER);

    system("setterm -cursor off");

    inter_mode_print_board(&imode);

    inter_mode_move_cursor_to_starting_position(&imode);
    inter_mode_turn_reverse_on_and_reprint_row(&imode);

    inter_mode_play_gamma(&imode);

    tcsetattr(STDIN_FILENO, TCSANOW, &old_term);

    system("setterm -cursor on");

    inter_mode_delete_board(&imode, imode.board_height);
}
