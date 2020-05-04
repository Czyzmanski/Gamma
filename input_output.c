#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <errno.h>

#include "input_output.h"

#define BATCH 'B'
#define INTERACTIVE 'I'

#define GAMMA_MOVE 'm'
#define GAMMA_GOLDEN_MOVE 'g'
#define GAMMA_BUSY_FIELDS 'b'
#define GAMMA_FREE_FIELDS 'f'
#define GAMMA_GOLDEN_POSSIBLE 'q'
#define GAMMA_BOARD 'p'

#define MOVE_COMMAND_TOKENS 4
#define QUERY_COMMAND_TOKENS 2
#define BOARD_COMMAND_TOKENS 1

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

static void batch_mode_command_execute(gamma_t *g, char command,
                                       const unsigned long arguments[]) {
    switch (command) {
        case GAMMA_MOVE: {
            uint32_t player = arguments[0], x = arguments[1], y = arguments[2];
            printf("%d\n", gamma_move(g, player, x, y) ? 1 : 0);
            break;
        }
        case GAMMA_GOLDEN_MOVE: {
            uint32_t player = arguments[0], x = arguments[1], y = arguments[2];
            printf("%d\n", gamma_golden_move(g, player, x, y) ? 1 : 0);
            break;
        }
        case GAMMA_BUSY_FIELDS: {
            uint32_t player = arguments[0];
            printf("%lu\n", gamma_busy_fields(g, player));
            break;
        }
        case GAMMA_FREE_FIELDS: {
            uint32_t player = arguments[0];
            printf("%lu\n", gamma_free_fields(g, player));
            break;
        }
        case GAMMA_GOLDEN_POSSIBLE: {
            uint32_t player = arguments[0];
            printf("%d\n", gamma_golden_possible(g, player) ? 1 : 0);
            break;
        }
        default: {
            printf("%s", gamma_board(g));
        }
    }
}

static void batch_mode_command_handle_line(gamma_t *g, char *line,
                                           unsigned line_num, size_t tokens_len) {
    char *tokens[tokens_len];
    if (line_split_into_tokens(line, tokens, tokens_len)) {
        unsigned long converted[tokens_len - 1];
        if (tokens_valid_arguments(tokens + 1, tokens_len - 1, converted)) {
            batch_mode_command_execute(g, tokens[0][0], converted);
        }
        else {
            error_print(line_num);
        }
    }
    else {
        error_print(line_num);
    }
}

static void batch_mode_handle_line(gamma_t *g, char *line, unsigned line_num) {
    switch (line[0]) {
        case GAMMA_MOVE:
        case GAMMA_GOLDEN_MOVE:
            batch_mode_command_handle_line(g, line, line_num, MOVE_COMMAND_TOKENS);
            break;
        case GAMMA_BUSY_FIELDS:
        case GAMMA_FREE_FIELDS:
        case GAMMA_GOLDEN_POSSIBLE:
            batch_mode_command_handle_line(g, line, line_num, QUERY_COMMAND_TOKENS);
            break;
        case GAMMA_BOARD:
            batch_mode_command_handle_line(g, line, line_num, BOARD_COMMAND_TOKENS);
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
        if (buffer[0] != '\n' && buffer[0] != COMMENT_BEGINNING) {
            if (*mode == BATCH_MODE) {
                batch_mode_handle_line(*g, buffer, line_len);
            }
            else {
                pending_mode_handle_line(g, buffer, line_len, mode);
            }
        }
        if (*mode == INTERACTIVE_MODE) {
            break;
        }
    }
}
