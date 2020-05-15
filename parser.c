/** @file
 * Implementacja modułu odpowiedzialnego za wczytywanie i parsowanie linii
 * oraz wykonywanie poleceń w trybie wsadowym
 *
 * @author Szymon Czyżmański 417797
 * @date 14.05.2020
 */

/**
 * Dostęp do funkcji getline.
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>
#include <errno.h>

#include "parser.h"

/**
 * Znak oznaczający początek komentarza, jeśli występuje on jako pierwszy
 * znak w linii.
 */
#define COMMENT '#'

/**
 * Znak oznaczający komendę powodującą utworzenie nowej gry za pomocą funkcji
 * @ref gamma_new i przejście do trybu wsadowego.
 */
#define BATCH 'B'
/**
 * Znak oznaczający komendę powodującą utworzenie nowej gry za pomocą funkcji
 * @ref gamma_new i przejście do trybu interaktywnego.
 */
#define INTERACTIVE 'I'

/**
 * Znak oznaczający komendę powodującą wywołanie funkcji @ref gamma_new.
 */
#define GAMMA_MOVE 'm'
/**
 * Znak oznaczający komendę powodującą wywołanie funkcji @ref gamma_golden_move.
 */
#define GAMMA_GOLDEN_MOVE 'g'
/**
 * Znak oznaczający komendę powodującą wywołanie funkcji @ref gamma_busy_fields.
 */
#define GAMMA_BUSY_FIELDS 'b'
/**
 * Znak oznaczający komendę powodującą wywołanie funkcji @ref gamma_free_fields.
 */
#define GAMMA_FREE_FIELDS 'f'
/**
 * Znak oznaczający komendę powodującą wywołanie funkcji @ref gamma_golden_possible.
 */
#define GAMMA_GOLDEN_POSSIBLE 'q'
/**
 * Znak oznaczający komendę powodującą wywołanie funkcji @ref gamma_board.
 */
#define GAMMA_BOARD 'p'

/**
 * Liczba tokenów w komendach @ref BATCH oraz @ref INTERACTIVE.
 */
#define MODE_COMMAND_TOKENS_NUM 5
/**
 * Liczba tokenów w komendach @ref GAMMA_MOVE oraz @ref GAMMA_GOLDEN_MOVE.
 */
#define MOVE_COMMAND_TOKENS_NUM 4
/**
 * Liczba tokenów w komendach @ref GAMMA_BUSY_FIELDS, @ref GAMMA_FREE_FIELDS
 * oraz @ref GAMMA_GOLDEN_POSSIBLE.
 */
#define QUERY_COMMAND_TOKENS_NUM 2
/**
 * Liczba tokenów w komendzie @ref GAMMA_BOARD.
 */
#define BOARD_COMMAND_TOKENS_NUM 1

/** @brief Wypisuje na standardowe wyjście diagnostyczne informację o błędzie.
 * Wypisuje na standardowej wyjście diagnostyczne komunikat informujący o błędzie,
 * występującym wskutek tego, że polecenie w linii o numerze @p line_num było
 * niepoprawne lub wynikiem funkcji wywołanej wskutek wykonania polecenia
 * była wartość NULL.
 * @param[in] line_num – numer linii, w której wystąpił błąd.
 */
static inline void print_error(unsigned line_num) {
    fprintf(stderr, "ERROR %u\n", line_num);
}

/** @brief Dzieli linię na tokeny.
 * Dzieli przekazaną linię na tokeny według białych znaków. Zapisuje uzyskane
 * tokeny w tablicy @p tokens.
 * @param[in,out] line   – wskaźnik na bufor zawierający linię do podzielenia,
 * @param[in,out] tokens – tablica do której mają zostać zapisane uzyskane tokeny,
 * @param[in] tokens_len – długość tablicy @p tokens, oczekiwana liczba tokenów.
 * @return Wartość @p true, jeżeli pierwszy token ma długość równą 1 oraz liczba
 * tokenów jest równa @p tokens_len, a @p false w przeciwnym przypadku.
 */
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

/** @brief Sprawdza, czy napis jest poprawnym zapisem liczby.
 * Sprawdza, czy napis wskazywany przez @p token reprezentuje poprawną liczbę,
 * to znaczy czy zawiera tylko cyfry.
 * @param[in] token – wskaźnik na napis do sprawdzenia.
 * @return Wartość @p true, jeżeli napis wskazywany przez @p token jest poprawnym
 * zapisem liczby, a @p false w przeciwnym przypadku.
 */
static inline bool token_valid_number(const char *token) {
    size_t i = 0;

    while (token[i] != '\0' && isdigit(token[i])) {
        i++;
    }

    return token[i] == '\0';
}

/** @brief Sprawdza, czy wszystkie napisy w tablicy są poprawnymi zapisami liczb.
 * Sprawdza, czy dla każdego napisu @p token z tablicy @p tokens funkcja
 * @ref token_valid_number przyjmuje wartość @p true.
 * @param[in,out] tokens – tablica zawierająca napisy do sprawdzenia,
 * @param[in] tokens_len – długość tablicy @p tokens.
 * @return Wartość @p true, jeżeli każdy element tablicy @p tokens jest poprawnym
 * zapisem liczby, a @p false w przeciwnym przypadku.
 */
static inline bool tokens_all_valid_numbers(char *tokens[], size_t tokens_len) {
    size_t i = 0;

    while (i < tokens_len && token_valid_number(tokens[i])) {
        i++;
    }

    return i == tokens_len;
}

/** @brief Konwertuje napisy do liczb, sprawdzając, czy są one dozwolonymi
 * argumentami dowolnego polecenia.
 * Konwertuje każdy z napisów będących elementami tablicy @p tokens do
 * odpowiadających im wartości liczbowej. Sprawdza w ten sposób czy napisy
 * przechowywane w tablicy @p tokens stanowią dozwolone argumenty dowolnego
 * polecenia.
 * @param[in] tokens        – tablica zawierająca napisy do skonwertowania,
 * @param[in] tokens_len    – długość tablicy @p tokens,
 * @param[in,out] converted – tablica mająca zawierać skonwertowane wartości.
 * @return Wartość @p true, jeżeli każdy element tablicy @p tokens jest poprawnym
 * zapisem liczby i został poprawnie skonwertowany do nieujemnej liczby
 * nieprzekraczającej wartości @p UINT32_MAX, a @p false w przeciwnym przypadku.
 */
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

/** @brief Wykonuje polecenie.
 * Wykonuje polecenie występujące w linii @p line_num, określone przez @p command,
 * z argumentami @p arguments.
 * Komunikaty o poprawnym wykonaniu poleceń wypisuje na standardowe wyjście,
 * a komunikaty o błędach wypisuje na standardowe wyjście diagnostyczne.
 * @param[in,out] g     – wskaźnik do wskaźnika na strukturę przechowującą stan gry,
 * @param[in] line_num  – numer linii, w której wystąpiło polecenie,
 * @param[in] command   – znak określający polecenie,
 * @param[in] arguments – argumenty polecenia,
 * @param[in,out] mode  – wskaźnik na zmienną przyjmującą jedną z wartości
 *                        zdefiniowanych w wyliczeniu @ref input_mode,
 *                        określającą, w jakim trybie aktualnie pracuje program.
 */
static void command_execute(gamma_t **g, unsigned line_num, char command,
                            unsigned long arguments[], input_mode_t *mode) {
    switch (command) {
        case BATCH:
        case INTERACTIVE: {
            uint32_t width = arguments[0], height = arguments[1];
            uint32_t players = arguments[2], areas = arguments[3];
            *g = gamma_new(width, height, players, areas);

            if (*g == NULL) {
                print_error(line_num);
            }
            else {
                *mode = command == BATCH ? BATCH_MODE : INTERACTIVE_MODE;

                if (command == BATCH) {
                    printf("OK %u\n", line_num);
                }
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
            printf("%" PRIu64 "\n", gamma_busy_fields(*g, player));
            break;
        }
        case GAMMA_FREE_FIELDS: {
            uint32_t player = arguments[0];
            printf("%" PRIu64 "\n", gamma_free_fields(*g, player));
            break;
        }
        case GAMMA_GOLDEN_POSSIBLE: {
            uint32_t player = arguments[0];
            printf("%d\n", gamma_golden_possible(*g, player) ? 1 : 0);
            break;
        }
        case GAMMA_BOARD: {
            char *board = gamma_board(*g);

            if (board == NULL) {
                print_error(line_num);
            }
            else {
                printf("%s", board);
                free(board);
            }

            break;
        }
        default: {
            print_error(line_num);
        }
    }
}

/** @brief Parsuje linię pod kątem konkretnego typu polecenia, determinowanego
 * oczekiwaną liczbą tokenów.
 * Parsuje linię i jeśli otrzymane tokeny reprezentują poprawne polecenie,
 * wykonuje je, a w przeciwnym przypadku wypisuje komunikat o błędzie
 * na standardowe wyjście diagnostyczne.
 * @param[in,out] g      – wskaźnik do wskaźnika na strukturę przechowującą stan gry,
 * @param[in,out] line   – wskaźnik do bufora zawierającego linię do interpretacji,
 * @param[in] line_num   – numer linii,
 * @param[in] tokens_len – oczekiwana liczba tokenów,
 * @param[in,out] mode   – wskaźnik na zmienną przyjmującą jedną z wartości
 *                         zdefiniowanych w wyliczeniu @ref input_mode,
 *                         określającą, w jakim trybie aktualnie pracuje program.
 */
static void command_parse_line(gamma_t **g, char *line, unsigned line_num,
                               size_t tokens_len, input_mode_t *mode) {
    char *tokens[tokens_len];

    if (line_split_into_tokens(line, tokens, tokens_len)) {
        unsigned long converted[tokens_len - 1];

        if (tokens_valid_arguments(tokens + 1, tokens_len - 1, converted)) {
            command_execute(g, line_num, tokens[0][0], converted, mode);
        }
        else {
            print_error(line_num);
        }
    }
    else {
        print_error(line_num);
    }
}

/** @brief Parsuje linię w trybie wsadowym.
 * Funkcja ta jest wywoływana w momencie, kiedy program przeszedł już w tryb
 * wsadowy, czyli jeżeli wykonano już poprawnie polecenie @ref BATCH.
 * Parsuje linię i jeśli otrzymane tokeny reprezentują poprawne polecenie
 * w trybie wsadowym, wykonuje je, a w przeciwnym przypadku wypisuje komunikat
 * o błędzie na standardowe wyjście diagnostyczne.
 * @param[in,out] g      – wskaźnik na strukturę przechowującą stan gry,
 * @param[in,out] line   – wskaźnik do bufora zawierającego linię do interpretacji,
 * @param[in] line_num   – numer linii,
 * @param[in,out] mode   – wskaźnik na zmienną przyjmującą jedną z wartości
 *                         zdefiniowanych w wyliczeniu @ref input_mode,
 *                         określającą, w jakim trybie aktualnie pracuje program,
 *                         zakłada się, że w momencie wywołania funkcji wartość
 *                         zmiennej jest równa @ref input_mode::BATCH_MODE.
 */
static void batch_mode_parse_line(gamma_t *g, char *line,
                                  unsigned line_num, input_mode_t *mode) {
    switch (line[0]) {
        case GAMMA_MOVE:
        case GAMMA_GOLDEN_MOVE:
            command_parse_line(&g, line, line_num, MOVE_COMMAND_TOKENS_NUM, mode);
            break;
        case GAMMA_BUSY_FIELDS:
        case GAMMA_FREE_FIELDS:
        case GAMMA_GOLDEN_POSSIBLE:
            command_parse_line(&g, line, line_num, QUERY_COMMAND_TOKENS_NUM, mode);
            break;
        case GAMMA_BOARD:
            command_parse_line(&g, line, line_num, BOARD_COMMAND_TOKENS_NUM, mode);
            break;
        default:
            print_error(line_num);
    }
}

/** @brief Parsuje linię w trybie oczekującym na polecenie @ref BATCH
 * lub @ref INTERACTIVE.
 * Funkcja ta jest wywoływana w momencie, kiedy program pracuje w trybie
 * oczekującym, to znaczy nie przeszedł jeszcze ani w tryb wsadowy, ani w tryb
 * interaktywny.
 * Parsuje linię i jeśli otrzymane tokeny reprezentują poprawne polecenie
 * @ref BATCH lub @ref INTERACTIVE, wykonuje je, a w przeciwnym przypadku
 * wypisuje komunikat o błędzie na standardowe wyjście diagnostyczne.
 * @param[in,out] g      – wskaźnik na strukturę przechowującą stan gry,
 * @param[in,out] line   – wskaźnik do bufora zawierającego linię do interpretacji,
 * @param[in] line_num   – numer linii,
 * @param[in,out] mode   – wskaźnik na zmienną przyjmującą jedną z wartości
 *                         zdefiniowanych w wyliczeniu @ref input_mode,
 *                         określającą, w jakim trybie aktualnie pracuje program,
 *                         zakłada się, że w momencie wywołania funkcji wartość
 *                         tej zmiennej jest równa @ref input_mode::PENDING_MODE.
 */
static inline void pending_mode_parse_line(gamma_t **g, char *line,
                                           unsigned line_num, input_mode_t *mode) {
    switch (line[0]) {
        case BATCH:
        case INTERACTIVE:
            command_parse_line(g, line, line_num, MODE_COMMAND_TOKENS_NUM, mode);
            break;
        default:
            print_error(line_num);
    }
}

bool read_lines(gamma_t **g, char **buf, size_t buffer_size, input_mode_t *mode) {
    ssize_t line_len;
    unsigned line_num = 0;
    errno = 0;

    while (*mode != INTERACTIVE_MODE
           && (line_len = getline(buf, &buffer_size, stdin)) != -1) {

        line_num++;
        char *buffer = *buf;

        if (errno == ENOMEM || errno == EINVAL) {
            return false;
        }

        if (buffer[0] != '\n' && buffer[0] != COMMENT) {
            if (buffer[line_len - 1] != '\n') {
                print_error(line_num);
            }
            else if (*mode == BATCH_MODE) {
                batch_mode_parse_line(*g, buffer, line_num, mode);
            }
            else {
                pending_mode_parse_line(g, buffer, line_num, mode);
            }
        }

        errno = 0;
    }

    return true;
}
