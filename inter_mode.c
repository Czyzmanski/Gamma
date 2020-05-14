#define _GNU_SOURCE

#include <stdio.h>
#include <ctype.h>
#include <inttypes.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "gamma.h"
#include "inter_mode.h"

/**
 * Pokazuje kursor w terminalu.
 */
#define SHOW_CURSOR "\x1b[?25h"
/**
 * Ukrywa kursor w terminalu.
 */
#define HIDE_CURSOR "\x1b[?25l"
/**
 * Przesuwa kursor na podaną pozycję w terminalu.
 */
#define MOVE_CURSOR_TO "\x1b[%" PRIu32 ";%" PRIu64 "H"
/**
 * Przesuwa kursor w lewy górny róg terminala.
 */
#define MOVE_CURSOR_TO_TOP_LEFT_CORNER "\x1b[;H"
/**
 * Zamienia kolor tła z kolorem napisu.
 */
#define TURN_ON_REVERSE_VIDEO "\x1b[7m"
/**
 * Ustawia kolor napisu na czerowny.
 */
#define TURN_ON_RED_FONT "\x1b[31m"
/**
 * Przywraca domyślne ustawienia graficzne.
 */
#define RESET "\x1b[0m"

/**
 * Czyści cały ekran.
 */
#define CLEAR_SCREEN "\x1b[2J"
/**
 * Czyści wiersz, w którym obecnie znajduje się kursor, od pozycji kursora
 * do końca wiersza.
 */
#define CLEAR_LINE_FROM_CURSOR_TO_END "\x1b[0K"

/**
 * Przełącza wyjście do alternatywnego bufora, przez co w terminalu jest widoczna
 * tylko plansza oraz wiersz zachęcający gracza do wykonania ruchu.
 */
#define ENABLE_ALTERNATIVE_SCREEN_BUFFER "\x1b[?1049h"
/**
 * Przełącza wyjście do głównego bufora, przez co po zakończeniu działania
 * programu w terminalu widoczna będzie ostateczna plansza wraz z podsumowaniem.
 */
#define DISABLE_ALTERNATIVE_SCREEN_BUFFER "\x1b[?1049l"

/**
 * Wiadomość wyswietlana w przypadku, gdy rozmiar okna terminala jest zbyt mały,
 * aby poprawnie wyświetlić planszę.
 */
#define TERMINAL_TOO_SMALL "TERMINAL IS TOO SMALL FOR PLAYING\n"
/**
 * Wiadomość zachęcająca gracza do wykonania ruchu.
 */
#define PROMPT "PLAYER %*" PRIu32 " %" PRIu64 " %" PRIu64
/**
 * Wiadomość informująca gracza o możliwości wykonania złotego ruchu.
 */
#define PROMPT_GOLDEN_POSSIBLE " G"
/**
 * Wiadomość informująca o uzyskanym przez gracza wyniku.
 */
#define SUMMARY "PLAYER %*" PRIu64 " %" PRIu64 "\n"

/**
 * Kod generowany przez wciśnięcie klawisza @p Esc, zapisany heksadecymalnie.
 */
#define ESC '\x1b'
/**
 * Znak @p '[', występujący jako drugi w sekwencji trzech znaków generowanych
 * przy wciśnięciu jednego z klawiszy ze strzałkami na klawiaturze.
 */
#define LEFT_SQUARE_BRACKET '['
/**
 * Ostatni znak występujący w sekwencji trzech znaków generowanych przez wciśnięcie
 * klawisza ze strzałką w górę na klawiaturze.
 */
#define KEY_UP 'A'
/**
 * Ostatni znak występujący w sekwencji trzech znaków generowanych przez wciśnięcie
 * klawisza ze strzałką w dół na klawiaturze.
 */
#define KEY_DOWN 'B'
/**
 * Ostatni znak występujący w sekwencji trzech znaków generowanych przez wciśnięcie
 * klawisza ze strzałką w prawo na klawiaturze.
 */
#define KEY_RIGHT 'C'
/**
 * Ostatni znak występujący w sekwencji trzech znaków generowanych przez wciśnięcie
 * klawisza ze strzałką w lewo na klawiaturze.
 */
#define KEY_LEFT 'D'
/**
 * Znak generowany przez wciśnięcie przez gracza klawisza oznaczającego wykonanie
 * zwykłego ruchu.
 */
#define MOVE_KEY ' '
/**
 * Znak generowany przez wciśnięcie przez gracza klawisza oznaczającego wykonanie
 * złotego ruchu.
 */
#define G_MOVE_KEY 'g'
/**
 * Znak generowany przez wciśnięcie przez gracza klawisza oznaczającego rezygnację
 * z wykonania ruchu.
 */
#define QUIT_MOVE_KEY 'q'
/**
 * Znak generowany przez wciśnięcie przez gracza kombinacji klawiszy @p Ctrl-D,
 * oznaczających koniec gry.
 */
#define END_OF_GAME_KEY 4

/**
 * Typ struktury przechowującej stan trybu interaktywnego.
 */
typedef struct inter_mode inter_mode_t;

/**
 * Struktura przechowująca stan trybu interaktywnego.
 */
struct inter_mode {
    gamma_t *g;                 /**< Wskaźnik na strukturę przechowującą stan gry. */
    uint32_t cursor_row;        /**< Numer wiersza, w którym znajduje się kursor,
                                 *   liczony od góry, od 0. */
    uint64_t cursor_col;        /**< Numer kolumny, w której znajduje się kursor,
                                 *   liczonej od lewej, od 0. */
    uint32_t board_height;      /**< Wysokość wyświetlanej planszy. */
    uint64_t board_width;       /**< Szerokość wyświetlanej planszy. */
    unsigned board_field_width; /**< Szerokość pola na wyświetlanej planszy. */
    unsigned player_width;      /**< Szerokość z jaką mają być wypisywane numery
                                 *   graczy. */
};

/**
 * Typ funkcji aktualizującej położenie kursora.
 */
typedef void (*cursor_move_fun)(inter_mode_t *imode);

/**
 * Typ funkcji wykonującej ruch przez gracza.
 */
typedef bool (*gamma_move_fun)(gamma_t *g, uint32_t player, uint32_t x, uint32_t y);

static inline void inter_mode_gamma_coordinates(inter_mode_t *imode,
                                                uint32_t *x, uint32_t *y) {
    *x = imode->cursor_col / imode->board_field_width;
    *y = imode->board_height - 1 - imode->cursor_row;
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

static inline void inter_mode_print_row(inter_mode_t *imode) {
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

static inline void inter_mode_print_board(inter_mode_t *imode) {
    printf(CLEAR_SCREEN);
    printf(MOVE_CURSOR_TO_TOP_LEFT_CORNER);
    imode->cursor_row = imode->cursor_col = 0;

    while (imode->cursor_row < imode->board_height) {
        inter_mode_print_row(imode);
        printf("\n");

        imode->cursor_row++;
    }
}

static inline void inter_mode_turn_reverse_off_and_reprint_row(inter_mode_t *imode) {
    uint32_t x, y;
    inter_mode_gamma_coordinates(imode, &x, &y);

    imode->cursor_col = x * imode->board_field_width;
    inter_mode_update_cursor_on_display(imode);

    printf(CLEAR_LINE_FROM_CURSOR_TO_END);
    printf(RESET);

    inter_mode_print_row(imode);

    inter_mode_update_cursor_on_display(imode);
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
    printf(RESET);

    imode->cursor_col += imode->board_field_width;

    inter_mode_print_row(imode);

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
    printf(PROMPT, imode->player_width, player,
           player_busy_fields, player_free_fields);

    if (player_golden_possible) {
        printf(PROMPT_GOLDEN_POSSIBLE);
    }
}

static inline void inter_mode_print_summary(inter_mode_t *imode) {
    uint32_t num_of_players = gamma_players(imode->g);

    printf(CLEAR_LINE_FROM_CURSOR_TO_END);

    for (uint64_t player = 1; player <= num_of_players; player++) {
        printf(SUMMARY, imode->player_width, player,
               gamma_busy_fields(imode->g, player));
    }
}

/** @name Rozgrywka
 * Zarządzanie rozgrywką oraz obsługa akcji wykonywanych przez graczy.
 */
///@{

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
    } while (c == ESC);

    if (c == LEFT_SQUARE_BRACKET) {
        c = getchar();

        if (c == KEY_UP || c == KEY_DOWN || c == KEY_RIGHT || c == KEY_LEFT) {
            if (c == KEY_UP) {
                inter_mode_update_display(imode, inter_mode_move_cursor_up);
            }
            else if (c == KEY_DOWN) {
                inter_mode_update_display(imode, inter_mode_move_cursor_down);
            }
            else if (c == KEY_RIGHT) {
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
                                    bool *end_of_game_key_pressed) {
    int c = getchar();
    bool quit_key_pressed = false, player_valid_move = false;
    bool player_golden_possible = gamma_golden_possible(imode->g, player);

    while (!player_valid_move && !quit_key_pressed && !(*end_of_game_key_pressed)) {
        if (c == QUIT_MOVE_KEY || toupper(c) == QUIT_MOVE_KEY) {
            quit_key_pressed = true;
        }
        else if (c == END_OF_GAME_KEY) {
            *end_of_game_key_pressed = true;
        }
        else if (c == MOVE_KEY || c == G_MOVE_KEY || toupper(c) == G_MOVE_KEY) {
            uint32_t x, y;
            inter_mode_gamma_coordinates(imode, &x, &y);

            if (c == MOVE_KEY) {
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
        else if (c == ESC) {
            c = inter_mode_handle_input_when_esc(imode);
        }
        else {
            c = getchar();
        }
    }
}

static void inter_mode_play_gamma(inter_mode_t *imode) {
    uint32_t num_of_players = gamma_players(imode->g);
    bool any_player_possible_move = true, end_of_game_key_pressed = false;

    while (any_player_possible_move && !end_of_game_key_pressed) {
        any_player_possible_move = false;
        uint64_t player = 1;

        while (player <= num_of_players && !end_of_game_key_pressed) {
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

                inter_mode_handle_input(imode, player, &end_of_game_key_pressed);
            }

            player++;
        }
    }
}

///@}

static inline bool inter_mode_set_up_terminal(struct termios *old_term,
                                              struct termios *new_term) {
    if (tcgetattr(fileno(stdin), old_term) != 0) {
        return false;
    }
    else {
        *new_term = *old_term;
        new_term->c_lflag &= ~(ICANON | ECHO);

        return tcsetattr(fileno(stdin), TCSANOW, new_term) == 0;
    }
}

static inline bool inter_mode_check_terminal_size(inter_mode_t *imode) {
    struct winsize term_size;

    return ioctl(fileno(stdin), TIOCGWINSZ, &term_size) == 0
           && imode->board_width <= term_size.ws_col
           && imode->board_height + 1 <= term_size.ws_row;
}

static inline void inter_mode_init(inter_mode_t *imode, gamma_t *g) {
    imode->g = g;
    imode->cursor_row = imode->cursor_col = 0;
    imode->board_height = gamma_board_height(g);
    imode->board_width = gamma_board_width(g);
    imode->board_field_width = gamma_board_field_width(g);
    imode->player_width = imode->board_field_width == 1 ?
                          imode->board_field_width : imode->board_field_width - 1;
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

        if (is_terminal && !inter_mode_check_terminal_size(&imode)) {
            fprintf(stderr, TURN_ON_RED_FONT);
            fprintf(stderr, TERMINAL_TOO_SMALL);
            fprintf(stderr, RESET);

            tcsetattr(fileno(stdin), TCSANOW, &old_term);

            return false;
        }
        else {
            printf(HIDE_CURSOR);
            printf(ENABLE_ALTERNATIVE_SCREEN_BUFFER);

            inter_mode_print_board(&imode);
            inter_mode_move_cursor_to_starting_position(&imode);
            inter_mode_turn_reverse_on_and_reprint_row(&imode);

            inter_mode_play_gamma(&imode);

            printf(SHOW_CURSOR);
            printf(DISABLE_ALTERNATIVE_SCREEN_BUFFER);

            inter_mode_print_board(&imode);
            inter_mode_print_summary(&imode);

            if (is_terminal) {
                return tcsetattr(fileno(stdin), TCSANOW, &old_term) == 0;
            }
            else {
                return true;
            }
        }
    }
}
