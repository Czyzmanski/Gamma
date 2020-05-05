#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <errno.h>

#include "input_output.h"
#include "mem_alloc_check.h"

#define COMMENT '#'

#define BATCH 'B'
#define INTERACTIVE 'I'

#define GAMMA_MOVE 'm'
#define GAMMA_GOLDEN_MOVE 'g'
#define GAMMA_BUSY_FIELDS 'b'
#define GAMMA_FREE_FIELDS 'f'
#define GAMMA_GOLDEN_POSSIBLE 'q'
#define GAMMA_BOARD 'p'

#define MODE_COMMAND_TOKENS 5
#define MOVE_COMMAND_TOKENS 4
#define QUERY_COMMAND_TOKENS 2
#define BOARD_COMMAND_TOKENS 1

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

void interactive_mode_launch(gamma_t *g) {
    uint64_t board_width;
    uint32_t board_height, board_field_width;
    char **board = interactive_mode_new_board(g, &board_width, &board_height,
                                              &board_field_width);

    free(board);
}

static inline void error_print(unsigned line_num) {
    fprintf(stderr, "ERROR %d\n", line_num);
}

static bool line_split_into_tokens(char *line, char *tokens[], size_t tokens_len) {
    char delimiters[] = " \t\v\f\r\n";

    tokens[0] = strtok(line, delimiters);
    if (strlen(tokens[0]) != 1) {
        return false;
    }
    else {
        char *token;
        size_t split = 1;
        while (split < tokens_len && (token = strtok(NULL, delimiters)) != NULL) {
            tokens[split] = token;
            split++;
        }

        return split == tokens_len && strtok(NULL, delimiters) == NULL;
    }
}

static bool token_valid_number(const char *token) {
    size_t i = 0;
    while (token[i] != '\0' && isdigit(token[i])) {
        i++;
    }

    return token[i] == '\0';
}

static bool tokens_all_valid_numbers(char *tokens[], size_t tokens_len) {
    size_t i = 0;
    while (i < tokens_len && token_valid_number(tokens[i])) {
        i++;
    }

    return i == tokens_len;
}

static bool tokens_valid_arguments(char *tokens[], size_t tokens_len,
                                   unsigned long converted[]) {
    if (tokens_all_valid_numbers(tokens, tokens_len)) {
        for (size_t i = 0; i < tokens_len; i++) {
            errno = 0;
            converted[i] = strtoul(tokens[i], NULL, 10);
            if (errno == ERANGE || converted[i] > UINT32_MAX) {
                return false;
            }
        }

        return true;
    }
    else {
        return false;
    }
}

static void command_execute(gamma_t **g, unsigned line_num, char command,
                            const unsigned long arguments[], input_mode_t *mode) {
    switch (command) {
        case BATCH:
        case INTERACTIVE: {
            uint32_t width = arguments[0], height = arguments[1];
            uint32_t players = arguments[2], areas = arguments[3];
            *g = gamma_new(width, height, players, areas);
            if (*g == NULL) {
                error_print(line_num);
            }
            else {
                *mode = command == BATCH ? BATCH_MODE : INTERACTIVE_MODE;
                printf("OK %d\n", line_num);
            }
            break;
        }
        case GAMMA_MOVE: {
            uint32_t player = arguments[0], x = arguments[1], y = arguments[2];
            printf("%d\n", gamma_move(*g, player, x, y) ? 1 : 0);
            break;
        }
        case GAMMA_GOLDEN_MOVE: {
            uint32_t player = arguments[0], x = arguments[1], y = arguments[2];
            printf("%d\n", gamma_golden_move(*g, player, x, y) ? 1 : 0);
            break;
        }
        case GAMMA_BUSY_FIELDS: {
            uint32_t player = arguments[0];
            printf("%lu\n", gamma_busy_fields(*g, player));
            break;
        }
        case GAMMA_FREE_FIELDS: {
            uint32_t player = arguments[0];
            printf("%lu\n", gamma_free_fields(*g, player));
            break;
        }
        case GAMMA_GOLDEN_POSSIBLE: {
            uint32_t player = arguments[0];
            printf("%d\n", gamma_golden_possible(*g, player) ? 1 : 0);
            break;
        }
        default: {
            printf("%s", gamma_board(*g));
        }
    }
}

static void command_handle_line(gamma_t **g, char *line, unsigned line_num,
                                size_t tokens_len, input_mode_t *mode) {
    char *tokens[tokens_len];
    if (line_split_into_tokens(line, tokens, tokens_len)) {
        unsigned long converted[tokens_len - 1];
        if (tokens_valid_arguments(tokens + 1, tokens_len - 1, converted)) {
            command_execute(g, line_num, tokens[0][0], converted, mode);
        }
        else {
            error_print(line_num);
        }
    }
    else {
        error_print(line_num);
    }
}

static void batch_mode_handle_line(gamma_t *g, char *line,
                                   unsigned line_num, input_mode_t *mode) {
    switch (line[0]) {
        case GAMMA_MOVE:
        case GAMMA_GOLDEN_MOVE:
            command_handle_line(&g, line, line_num, MOVE_COMMAND_TOKENS, mode);
            break;
        case GAMMA_BUSY_FIELDS:
        case GAMMA_FREE_FIELDS:
        case GAMMA_GOLDEN_POSSIBLE:
            command_handle_line(&g, line, line_num, QUERY_COMMAND_TOKENS, mode);
            break;
        case GAMMA_BOARD:
            command_handle_line(&g, line, line_num, BOARD_COMMAND_TOKENS, mode);
            break;
        default:
            error_print(line_num);
    }
}

static void pending_mode_handle_line(gamma_t **g, char *line,
                                     unsigned line_num, input_mode_t *mode) {
    switch (line[0]) {
        case BATCH:
        case INTERACTIVE:
            command_handle_line(g, line, line_num, MODE_COMMAND_TOKENS, mode);
            break;
        default:
            error_print(line_num);
    }
}

void read_lines(gamma_t **g, char *buffer, size_t buffer_size, input_mode_t *mode) {
    ssize_t line_len;
    unsigned line_num = 0;
    while ((line_len = getline(&buffer, &buffer_size, stdin)) != -1) {
        line_num++;

        if (buffer[0] != '\n' && buffer[0] != COMMENT) {
            if (buffer[line_len - 1] != '\n') {
                error_print(line_num);
            }
            else if (*mode == BATCH_MODE) {
                batch_mode_handle_line(*g, buffer, line_num, mode);
            }
            else {
                pending_mode_handle_line(g, buffer, line_num, mode);
            }
        }

        if (*mode == INTERACTIVE_MODE) {
            break;
        }
    }
}
