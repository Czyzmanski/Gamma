#ifndef INTER_MODE_INPUT_H
#define INTER_MODE_INPUT_H

#include <stdbool.h>

static inline bool is_esc(int c) {
    return c == '\x1b';
}

static inline bool is_left_square_bracket(int c) {
    return c == '[';
}

static inline bool is_cursor_up(int c) {
    return c == 'A';
}

static inline bool is_cursor_down(int c) {
    return c == 'B';
}

static inline bool is_cursor_right(int c) {
    return c == 'C';
}

static inline bool is_cursor_movement(int c) {
    return 'A' <= c && c <= 'D';
}

static inline bool is_move(int c) {
    return c == ' ';
}

static inline bool is_golden_move(int c) {
    return c == 'g' || c == 'G';
}

static inline bool is_quit(int c) {
    return c == 'c' || c == 'C';
}

static inline bool is_end_of_game(int c) {
    return c == 4;
}

#endif // INTER_MODE_INPUT_H
