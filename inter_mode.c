#define _GNU_SOURCE

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <inttypes.h>

#include "gamma.h"
#include "inter_mode.h"
#include "inter_mode_input.h"

#define SHOW_CURSOR "\x1b[?25h"
#define HIDE_CURSOR "\x1b[?25l"
#define MOVE_CURSOR_TO "\x1b[%" PRIu32 ";%" PRIu64 "H"
#define MOVE_CURSOR_TO_TOP_LEFT_CORNER "\x1b[;H"

#define TURN_ON_REVERSE_VIDEO "\x1b[7m"
#define TURN_OFF_REVERSE_VIDEO "\x1b[0m"

#define CLEAR_SCREEN "\x1b[2J"
#define CLEAR_LINE_FROM_CURSOR_TO_END "\x1b[0K"

#define PROMPT "PLAYER %" PRIu32 " %" PRIu64 " %" PRIu64
#define PROMPT_GOLDEN_POSSIBLE " G"
#define SUMMARY "PLAYER %" PRIu64 " %" PRIu64 "\n"

typedef struct termios termios_t;

typedef struct inter_mode inter_mode_t;

typedef void (*cursor_move_fun)(inter_mode_t *imode);

typedef bool (*gamma_move_fun)(gamma_t *g, uint32_t player, uint32_t x, uint32_t y);

struct inter_mode {
    gamma_t *g;
    uint32_t board_height;
    uint64_t board_width;
    unsigned board_field_width;
    uint32_t cursor_row;
    uint64_t cursor_col;
};

static inline void inter_mode_gamma_coordinates(inter_mode_t *imode,
                                                uint32_t *x, uint32_t *y) {
    *x = imode->cursor_col / imode->board_field_width;
    *y = imode->board_height - 1 - imode->cursor_row;
}

static unsigned inter_mode_chars_to_reverse_in_current_field(inter_mode_t *imode) {
    if (imode->board_field_width == 1) {
        return 1;
    }
    else {
        uint32_t x, y;
        inter_mode_gamma_coordinates(imode, &x, &y);

        char field_repr[FIELD_MAX_WIDTH + 1];
        gamma_board_field_repr(imode->g, x, y, field_repr);

        unsigned i = 0, to_reverse = 0;

        while (isspace(field_repr[i])) {
            i++;
        }

        while (isdigit(field_repr[i]) || field_repr[i] == FREE_FIELD) {
            i++, to_reverse++;
        }

        return to_reverse;
    }
}

static inline void inter_mode_find_cursor_column(inter_mode_t *imode) {
    uint32_t x, y;
    inter_mode_gamma_coordinates(imode, &x, &y);

    char field_repr[FIELD_MAX_WIDTH + 1];
    gamma_board_field_repr(imode->g, x, y, field_repr);

    unsigned i = 0;
    imode->cursor_col = x * imode->board_field_width;

    while (isspace(field_repr[i])) {
        i++, imode->cursor_col++;
    }
}

static inline void inter_mode_move_cursor_left(inter_mode_t *imode) {
    if (imode->cursor_col >= imode->board_field_width) {
        imode->cursor_col -= imode->board_field_width;
    }
}

static inline void inter_mode_move_cursor_right(inter_mode_t *imode) {
    if (imode->cursor_col + imode->board_field_width < imode->board_width - 1) {
        imode->cursor_col += imode->board_field_width;
    }
}

static inline void inter_mode_move_cursor_up(inter_mode_t *imode) {
    if (imode->cursor_row > 0) {
        imode->cursor_row--;
    }
}

static inline void inter_mode_move_cursor_down(inter_mode_t *imode) {
    if (imode->cursor_row < imode->board_height - 1) {
        imode->cursor_row++;
    }
}

static inline void inter_mode_update_cursor_on_display(inter_mode_t *imode) {
    printf(MOVE_CURSOR_TO, imode->cursor_row + 1, imode->cursor_col + 1);
}

static inline void inter_mode_reprint_row(inter_mode_t *imode) {
    uint32_t x, y;
    inter_mode_gamma_coordinates(imode, &x, &y);
    uint32_t gamma_width = (imode->board_width - 1) / imode->board_field_width;

    while (x < gamma_width) {
        char field_repr[FIELD_MAX_WIDTH + 1];
        gamma_board_field_repr(imode->g, x, y, field_repr);

        printf("%s", field_repr);

        x++;
    }
}

static inline void inter_mode_turn_reverse_off_and_reprint_row(inter_mode_t *imode) {
    uint32_t x, y;
    inter_mode_gamma_coordinates(imode, &x, &y);

    imode->cursor_col = x * imode->board_field_width;
    inter_mode_update_cursor_on_display(imode);

    printf(CLEAR_LINE_FROM_CURSOR_TO_END);
    printf(TURN_OFF_REVERSE_VIDEO);

    inter_mode_reprint_row(imode);

    inter_mode_update_cursor_on_display(imode);
}

static void inter_mode_turn_reverse_on_and_reprint_row(inter_mode_t *imode) {
    uint32_t x, y;
    inter_mode_gamma_coordinates(imode, &x, &y);

    char field_repr[FIELD_MAX_WIDTH + 1];
    gamma_board_field_repr(imode->g, x, y, field_repr);

    imode->cursor_col = x * imode->board_field_width;
    inter_mode_update_cursor_on_display(imode);

    printf(CLEAR_LINE_FROM_CURSOR_TO_END);

    unsigned to_reverse = inter_mode_chars_to_reverse_in_current_field(imode);
    unsigned not_to_reverse = imode->board_field_width - to_reverse;

    printf("%.*s", not_to_reverse, field_repr);
    printf(TURN_ON_REVERSE_VIDEO);
    printf("%.*s", to_reverse, field_repr + not_to_reverse);
    printf(TURN_OFF_REVERSE_VIDEO);

    imode->cursor_col += imode->board_field_width;

    inter_mode_reprint_row(imode);

    imode->cursor_col -= imode->board_field_width;

    inter_mode_find_cursor_column(imode);
    inter_mode_update_cursor_on_display(imode);
}

static inline void inter_mode_update_display(inter_mode_t *imode,
                                             cursor_move_fun cur_mv_f) {
    inter_mode_turn_reverse_off_and_reprint_row(imode);
    cur_mv_f(imode);
    inter_mode_turn_reverse_on_and_reprint_row(imode);
}

static inline void inter_mode_move_cursor_below_board(inter_mode_t *imode) {
    inter_mode_turn_reverse_off_and_reprint_row(imode);

    imode->cursor_row = imode->board_height;
    imode->cursor_col = 0;

    inter_mode_update_cursor_on_display(imode);
}

static inline void inter_mode_move_cursor_to_starting_position(inter_mode_t *imode) {
    imode->cursor_row = (imode->board_height - 1) / 2;
    imode->cursor_col = (imode->board_width - 2) / 2;

    inter_mode_find_cursor_column(imode);
    inter_mode_update_cursor_on_display(imode);
}

static inline void inter_mode_print_prompt(inter_mode_t *imode, uint32_t player) {
    uint64_t player_busy_fields = gamma_busy_fields(imode->g, player);
    uint64_t player_free_fields = gamma_free_fields(imode->g, player);
    bool player_golden_possible = gamma_golden_possible(imode->g, player);

    printf(CLEAR_LINE_FROM_CURSOR_TO_END);
    printf(PROMPT, player, player_busy_fields, player_free_fields);

    if (player_golden_possible) {
        printf(PROMPT_GOLDEN_POSSIBLE);
    }
}

static inline void inter_mode_print_summary(inter_mode_t *imode) {
    uint32_t num_of_players = gamma_players(imode->g);

    printf(CLEAR_LINE_FROM_CURSOR_TO_END);

    for (uint64_t player = 1; player <= num_of_players; player++) {
        printf(SUMMARY, player, gamma_busy_fields(imode->g, player));
    }
}

static inline bool inter_mode_gamma_move(inter_mode_t *imode, gamma_move_fun g_mv_f,
                                         uint32_t player, uint32_t x, uint32_t y) {
    if (g_mv_f(imode->g, player, x, y)) {
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
    } while (is_esc(c));

    if (is_left_square_bracket(c)) {
        c = getchar();

        if (is_cursor_movement(c)) {
            if (is_cursor_up(c)) {
                inter_mode_update_display(imode, inter_mode_move_cursor_up);
            }
            else if (is_cursor_down(c)) {
                inter_mode_update_display(imode, inter_mode_move_cursor_down);
            }
            else if (is_cursor_right(c)) {
                inter_mode_update_display(imode, inter_mode_move_cursor_right);
            }
            else {
                inter_mode_update_display(imode, inter_mode_move_cursor_left);
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
        if (is_quit(c)) {
            quit = true;
        }
        else if (is_end_of_game(c)) {
            *end_of_game_char = true;
        }
        else if (is_move(c) || is_golden_move(c)) {
            uint32_t x, y;
            inter_mode_gamma_coordinates(imode, &x, &y);

            if (is_move(c)) {
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
        else if (is_esc(c)) {
            c = inter_mode_handle_input_when_esc(imode);
        }
        else {
            c = getchar();
        }
    }
}

static void inter_mode_play_gamma(inter_mode_t *imode) {
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
                inter_mode_update_cursor_on_display(imode);

                inter_mode_turn_reverse_on_and_reprint_row(imode);

                inter_mode_handle_input(imode, player, &end_of_game_char);
            }

            player++;
        }
    }

    inter_mode_move_cursor_below_board(imode);
    inter_mode_print_summary(imode);
}

static inline bool inter_mode_set_up_terminal(termios_t *old_term,
                                              termios_t *new_term) {
    if (tcgetattr(fileno(stdin), old_term) != 0) {
        return false;
    }
    else {
        *new_term = *old_term;
        new_term->c_lflag &= ~(ICANON | ECHO);

        return tcsetattr(fileno(stdin), TCSANOW, new_term) == 0;
    }
}

static inline void inter_mode_init(inter_mode_t *imode, gamma_t *g) {
    imode->g = g;
    imode->cursor_row = imode->cursor_col = 0;
    imode->board_height = gamma_board_height(g);
    imode->board_width = gamma_board_width(g);
    imode->board_field_width = gamma_board_field_width(g);
}

bool inter_mode_launch(gamma_t *g) {
    struct termios old_term, new_term;
    bool is_terminal = isatty(fileno(stdin));

    if (is_terminal && !inter_mode_set_up_terminal(&old_term, &new_term)) {
        return false;
    }
    else {
        inter_mode_t imode;
        inter_mode_init(&imode, g);

        printf(HIDE_CURSOR);
        printf(CLEAR_SCREEN);
        printf(MOVE_CURSOR_TO_TOP_LEFT_CORNER);

        char *board = gamma_board(imode.g);

        if (board == NULL) {
            if (is_terminal) {
                tcsetattr(fileno(stdin), TCSANOW, &old_term);
            }

            return false;
        }
        else {
            printf("%s", board);

            inter_mode_move_cursor_to_starting_position(&imode);
            inter_mode_turn_reverse_on_and_reprint_row(&imode);

            inter_mode_play_gamma(&imode);

            printf(SHOW_CURSOR);

            if (is_terminal) {
                return tcsetattr(fileno(stdin), TCSANOW, &old_term) == 0;
            }
            else {
                return true;
            }
        }
    }
}
