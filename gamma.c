/** @file
 * Implementacja klasy przechowującej stan gry gamma
 *
 * @author Szymon Czyżmański 417797
 * @date 11.04.2020
 */

#include <stdlib.h>

#include "gamma.h"
#include "field.h"

#define NEIGHBOURS 4

struct gamma {
    uint32_t width;
    uint32_t height;
    uint32_t players;
    uint32_t areas;
    uint32_t busy_fields;
    field_t ***board;
    player_t **players_arr;
};

/** @name Obszar
 * Wykorzystanie struktury Find-Union z kompresją ścieżki oraz łączeniem według
 * rangi do efektywnego utrzymania informacji o obszarach każdego z graczy.
 */
///@{

/** @brief Tworzy nowy obszar.
 * Czyni pole @p f korzeniem nowego obszaru: ustawia składową @p rank pola @p f
 * na 0 raz ustawia składową @p root pola @p f na NULL. Zwiększa o 1 wartość
 * składowej @p areas gracza @p owner będącego właścicielem pola @p f.
 * @param[in,out] f – wskaźnik na strukturę przechowującą stan pola.
 */
static void area_new(field_t *f) {
    field_set_rank(f, 0);
    field_set_parent(f, NULL);

    player_t *owner = field_owner(f);
    player_set_areas(owner, player_areas(owner) + 1);
}

/** @brief Znajduje korzeń obszaru do którego należy pole @p f.
 * Rekurencyjnie znajduje korzeń obszaru @p root do którego należy pole @p f.
 * Dokonuje kompresji ścieżki od @p f do @p root, ustawiając składową @p parent
 * w każdym polu na tej ścieżce na @p root, z wyjątkiem samego korzenia @p root.
 * @param[in,out] f – wskaźnik na strukturę przechowującą stan pola.
 * @return Wskaźnik na strukturę przechowującą stan pola będącego korzeniem
 * obszaru do którego należy pole @p f.
 */
static field_t *area_find_root(field_t *f) {
    if (field_parent(f) == NULL) {
        return f;
    }
    else {
        field_t *root = area_find_root(field_parent(f));
        field_set_parent(f, root);
        return root;
    }
}

/** @brief Łączy dwa obszary w jeden.
 * Ustawia wskaźnik @p parent korzenia @p f1_root pierwszego obszaru,
 * do którego należy pole @p f1, na korzeń @p f2_root drugiego obszaru,
 * do którego należy pole @p f2, jeśli ranga @p f1_root_rank pierwszego obszaru
 * jest mniejsza od rangi @p f2_root_rank drugiego obszaru. W przeciwnym razie,
 * ustawia wskaźnik @p parent pola @p f2_root na @p f1_root. Jeśli wartość
 * @p f1_root_rank jest równa wartości @p f2_root_rank, zwiększa rangę pierwszego
 * pola, zwiększając wartość składowej @p rank w polu @p f1_root o 1.
 * @param[in,out] f1 – wskaźnik na strukturę przechowującą stan pola
 *                     należącego do pierwszego obszaru,
 * @param[in,out] f2 – wskaźnik na strukturę przechowującą stan pola
 *                     należącego do drugiego obszaru.
 * @return Wartość @p false, gdy pola @p f1 oraz @p f2 należą do tego samego
 * obszaru i nie ma czego łączyć, a @p true w przeciwnym przypadku.
 */
static bool area_merge(field_t *f1, field_t *f2) {
    field_t *f1_root = area_find_root(f1);
    field_t *f2_root = area_find_root(f2);

    uint32_t f1_root_rank = field_rank(f1_root);
    uint32_t f2_root_rank = field_rank(f2_root);

    if (f1_root == f2_root) {
        return false;
    }
    else {
        if (f1_root_rank < f2_root_rank) {
            field_set_parent(f1_root, f2_root);
        }
        else {
            field_set_parent(f2_root, f1_root);
            if (f1_root_rank == f2_root_rank) {
                field_set_rank(f1_root, f1_root_rank + 1);
            }
        }
        return true;
    }
}

///@}

/** @brief Sprawdza poprawność numeru gracza.
 * Sprawdza, czy numer gracza @p player jest liczbą całkowitą dodatnią
 * nie większą od wartości @p players z funkcji @ref gamma_new.
 * @param[in] g      – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player – numer gracza, liczba dodatnia niewiększa od wartości
 *                     @p players z funkcji @ref gamma_new.
 * @return Wartość @p true, jeśli numer gracza jest poprawny,
 * a @p false w przeciwnym przypadku.
 */
static inline bool valid_player(gamma_t *g, int64_t player) {
    return 1 <= player && player <= g->players;
}

/** @brief Sprawdza poprawność numeru kolumny.
 * Sprawdza, czy numer kolumny @p x jest liczbą całkowitą nieujemną
 * mniejszą od wartości @p width z funkcji @ref gamma_new
 * @param[in] g – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] x – numer kolumny, liczba nieujemna mniejsza od wartości
 *                @p width z funkcji @ref gamma_new,
 * @return Wartość @p true, jeśli numer kolumny jest poprawny,
 * a @p false w przeciwnym przypadku.
 */
static inline bool valid_x(gamma_t *g, int64_t x) {
    return 0 <= x && x < g->width;
}

/** @brief Sprawdza poprawność numeru wiersza.
 * Sprawdza, czy numer wiersza @p y jest liczbą całkowitą nieujemną
 * mniejszą od wartości @p height z funkcji @ref gamma_new.
 * @param[in] g – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] y – numer wiersza, liczba nieujemna mniejsza od wartości
 *                @p height z funkcji @ref gamma_new,
 * @return Wartość @p true, jeśli numer wiersza jest poprawny,
 * a @p false w przeciwnym przypadku.
 */
static inline bool valid_y(gamma_t *g, int64_t y) {
    return 0 <= y && y < g->height;
}

/** @brief Sprawdza, czy pole (@p x, @p y) jest poprawne i wolne.
 * Sprawdza, czy numer kolumny @p x jest liczbą całkowitą nieujemną
 * mniejszą od wartości @p width z funkcji @ref gamma_new.
 * Sprawdza, czy numer wiersza @p y jest liczbą całkowitą nieujemną
 * mniejszą od wartości @p height z funkcji @ref gamma_new.
 * Sprawdza, czy pole nie zostało jeszcze stworzone, tzn. czy wartość wskaźnika
 * @p g->board[y][x] jest równa NULL, lub, w przeciwnym razie, czy pole nie ma
 * przypisanego właściciela, tzn. czy wskaźnik @p owner będący składową pola
 * na które wskazuje wskaźnik @p g->board[y][x] jest równy NULL.
 * @param[in] g – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] x – numer kolumny, liczba nieujemna mniejsza od wartości
 *                @p width z funkcji @ref gamma_new,
 * @param[in] y – numer wiersza, liczba nieujemna mniejsza od wartości
 *                @p height z funkcji @ref gamma_new,
 * @return Wartość @p true, jeśli współrzędne pola są poprawne oraz pole
 * nie jest zajęte przez pewnego gracza, a @p false w przeciwnym przypadku.
 */
static inline bool valid_free_field(gamma_t *g, int64_t x, int64_t y) {
    return valid_x(g, x) && valid_y(g, y)
           && (g->board[y][x] == NULL || field_owner(g->board[y][x]) == NULL);
}

/** @brief Sprawdza, czy pole (@p x, @p y) jest poprawne i należy do gracza @p p.
 * Sprawdza, czy numer kolumny @p x jest liczbą całkowitą nieujemną
 * mniejszą od wartości @p width z funkcji @ref gamma_new.
 * Sprawdza, czy numer wiersza @p y jest liczbą całkowitą nieujemną
 * mniejszą od wartości @p height z funkcji @ref gamma_new.
 * Sprawdza, czy pole zostało już stworzone, tzn. czy wartość wskaźnika
 * @p g->board[y][x] jest różna od NULL oraz czy właścicielem pola jest gracz @p p,
 * tzn. czy wskaźnik @p owner będący składową pola na które wskazuje wskaźnik
 * @p g->board[y][x] jest równy @p p.
 * @param[in] g – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] x – numer kolumny, liczba nieujemna mniejsza od wartości
 *                @p width z funkcji @ref gamma_new,
 * @param[in] y – numer wiersza, liczba nieujemna mniejsza od wartości
 *                @p height z funkcji @ref gamma_new,
 * @return Wartość @p true, jeśli współrzędne pola są poprawne oraz pole
 * należy do gracza @p p.
 */
static bool player_valid_field(gamma_t *g, player_t *p, int64_t x, int64_t y) {
    if (!valid_x(g, x) || !valid_y(g, y)) {
        return false;
    }
    else {
        field_t *f = g->board[y][x];
        return f != NULL && field_owner(f) == p;
    }
}

static uint8_t player_adjacent_fields(gamma_t *g, player_t *p,
                                      uint32_t x, uint32_t y) {
    uint8_t fields = 0;

    fields += player_valid_field(g, p, x - 1, y);
    fields += player_valid_field(g, p, x + 1, y);
    fields += player_valid_field(g, p, x, y - 1);
    fields += player_valid_field(g, p, x, y + 1);

    return fields;
}

static bool player_move_legal(gamma_t *g, player_t *p, uint32_t x, uint32_t y) {
    if (!valid_free_field(g, x, y)) {
        return false;
    }
    else if (p == NULL || player_areas(p) < g->areas) {
        return true;
    }
    else {
        return player_adjacent_fields(g, p, x, y) > 0;
    }
}

static bool unique_neighbour(player_t *p, player_t **neighbours, uint8_t added) {
    uint8_t i = 0;

    while (i < added && p != neighbours[i]) {
        i++;
    }

    return i == added;
}

static bool neighbour_valid_unique_field(gamma_t *g, int64_t x, int64_t y,
                                         player_t **neighbours, uint8_t added) {
    if (!valid_x(g, x) || !valid_y(g, y)) {
        return false;
    }
    else {
        field_t *f = g->board[y][x];
        return f != NULL && unique_neighbour(field_owner(f), neighbours, added);
    }
}

static uint8_t add_neighbour_if_unique(gamma_t *g, int64_t x, int64_t y,
                                       player_t **neighbours, uint8_t added) {
    if (neighbour_valid_unique_field(g, x, y, neighbours, added)) {
        neighbours[added] = field_owner(g->board[y][x]);
        added++;
    }
    return added;
}

static player_t **unique_neighbours(gamma_t *g, uint32_t x, uint32_t y) {
    uint8_t added = 0;
    player_t **neighbours = calloc(NEIGHBOURS, sizeof(player_t *));

    added = add_neighbour_if_unique(g, x - 1, y, neighbours, added);
    added = add_neighbour_if_unique(g, x + 1, y, neighbours, added);
    added = add_neighbour_if_unique(g, x, y - 1, neighbours, added);
    add_neighbour_if_unique(g, x, y + 1, neighbours, added);

    return neighbours;
}

static void neighbours_update_perimeter(gamma_t *g, field_t *f) {
    player_t *owner = field_owner(f);
    player_t **neighbours = unique_neighbours(g, field_x(f), field_y(f));

    for (uint8_t i = 0; i < NEIGHBOURS; i++) {
        if (neighbours[i] != NULL && neighbours[i] != owner) {
            player_set_perimeter(neighbours[i], player_perimeter(neighbours[i]) - 1);
        }
    }

    free(neighbours);
}

static inline bool player_valid_free_single_field(gamma_t *g, player_t *owner,
                                                  int64_t x, int64_t y) {
    return valid_free_field(g, x, y) && player_adjacent_fields(g, owner, x, y) == 0;
}

static uint8_t player_adjacent_free_single_fields(gamma_t *g, player_t *owner,
                                                  uint32_t x, uint32_t y) {
    uint8_t fields = 0;

    fields += player_valid_free_single_field(g, owner, x - 1, y);
    fields += player_valid_free_single_field(g, owner, x + 1, y);
    fields += player_valid_free_single_field(g, owner, x, y - 1);
    fields += player_valid_free_single_field(g, owner, x, y + 1);

    return fields;
}

static void player_update_perimeter(gamma_t *g, field_t *f, bool golden_move) {
    uint32_t x = field_x(f);
    uint32_t y = field_y(f);
    player_t *owner = field_owner(f);
    uint32_t perimeter = player_perimeter(owner);

    field_set_owner(f, NULL);

    if (!golden_move && player_adjacent_fields(g, owner, x, y) > 0) {
        perimeter--;
    }

    perimeter += player_valid_free_single_field(g, owner, x - 1, y);
    perimeter += player_valid_free_single_field(g, owner, x + 1, y);
    perimeter += player_valid_free_single_field(g, owner, x, y - 1);
    perimeter += player_valid_free_single_field(g, owner, x, y + 1);

    field_set_owner(f, owner);
    player_set_perimeter(owner, perimeter);
}

static void player_merge_adjacent_areas(gamma_t *g, field_t *f,
                                        int64_t x, int64_t y) {
    player_t *owner = field_owner(f);

    if (player_valid_field(g, owner, x, y) && area_merge(f, g->board[y][x])) {
        player_set_areas(owner, player_areas(owner) - 1);
    }
}

static void player_update_areas(gamma_t *g, field_t *f) {
    area_new(f);

    uint32_t x = field_x(f);
    uint32_t y = field_y(f);

    player_merge_adjacent_areas(g, f, x - 1, y);
    player_merge_adjacent_areas(g, f, x + 1, y);
    player_merge_adjacent_areas(g, f, x, y - 1);
    player_merge_adjacent_areas(g, f, x, y + 1);
}

static inline bool valid_busy_field(gamma_t *g, int64_t x, int64_t y) {
    return valid_x(g, x) && valid_y(g, y) && g->board[y][x] != NULL;
}

static bool player_golden_move_legal(gamma_t *g, player_t *p,
                                     uint32_t x, uint32_t y) {
    if (p == NULL) {
        return true;
    }
    else if (field_owner(g->board[y][x]) == p) {
        return false;
    }
    else if (player_areas(p) < g->areas) {
        return true;
    }
    else {
        return player_adjacent_fields(g, p, x, y) > 0;
    }
}

static inline bool player_valid_search_field(gamma_t *g, player_t *owner,
                                             int64_t x, int64_t y) {
    return valid_busy_field(g, x, y) && field_owner(g->board[y][x]) == owner;
}

static bool area_search(gamma_t *g, player_t *owner,
                        int64_t x, int64_t y, status_t desired) {
    if (!player_valid_search_field(g, owner, x, y)) {
        return false;
    }
    else if (field_status(g->board[y][x]) == desired) {
        return false;
    }
    else {
        field_set_status(g->board[y][x], desired);

        area_search(g, owner, x - 1, y, desired);
        area_search(g, owner, x + 1, y, desired);
        area_search(g, owner, x, y - 1, desired);
        area_search(g, owner, x, y + 1, desired);

        return true;
    }
}

static uint32_t victim_new_areas(gamma_t *g, player_t *victim,
                                 uint32_t x, uint32_t y) {
    field_set_status(g->board[y][x], COUNTED);

    uint32_t areas = player_areas(victim) - 1;

    areas += area_search(g, victim, x - 1, y, COUNTED);
    areas += area_search(g, victim, x + 1, y, COUNTED);
    areas += area_search(g, victim, x, y - 1, COUNTED);
    areas += area_search(g, victim, x, y + 1, COUNTED);

    return areas;
}

static bool victim_golden_move_legal(gamma_t *g, uint32_t x, uint32_t y) {
    player_t *victim = field_owner(g->board[y][x]);
    uint8_t mx_new_areas = player_adjacent_fields(g, victim, x, y) - 1;

    if (player_areas(victim) + mx_new_areas <= g->areas) {
        return true;
    }
    else if (victim_new_areas(g, victim, x, y) <= g->areas) {
        return true;
    }
    else {
        area_search(g, victim, x, y, UNCHECKED);
        return false;
    }
}

static void area_update_parent_and_rank(gamma_t *g, player_t *owner,
                                        int64_t x, int64_t y, field_t *parent) {
    if (player_valid_search_field(g, owner, x, y)
        && field_status(g->board[y][x]) != MODIFIED) {

        field_t *f = g->board[y][x];

        field_set_rank(f, 0);
        field_set_parent(f, parent);
        field_set_status(f, MODIFIED);

        area_update_parent_and_rank(g, owner, x - 1, y, parent);
        area_update_parent_and_rank(g, owner, x + 1, y, parent);
        area_update_parent_and_rank(g, owner, x, y - 1, parent);
        area_update_parent_and_rank(g, owner, x, y + 1, parent);
    }
}

static uint32_t area_set_component(gamma_t *g, player_t *old_owner,
                                   uint32_t x, uint32_t y, uint32_t areas) {
    if (player_valid_search_field(g, old_owner, x, y)) {
        if (field_status(g->board[y][x]) != MODIFIED) {
            areas++;

            field_t *root = g->board[y][x];
            field_set_status(root, MODIFIED);
            field_set_rank(root, 0);
            field_set_parent(root, NULL);

            area_update_parent_and_rank(g, old_owner, x - 1, y, root);
            area_update_parent_and_rank(g, old_owner, x + 1, y, root);
            area_update_parent_and_rank(g, old_owner, x, y - 1, root);
            area_update_parent_and_rank(g, old_owner, x, y + 1, root);
        }
    }

    return areas;
}

static void old_owner_update_areas(gamma_t *g, player_t *old_owner,
                                   uint32_t x, uint32_t y) {
    uint32_t areas = player_areas(old_owner) - 1;

    areas = area_set_component(g, old_owner, x - 1, y, areas);
    areas = area_set_component(g, old_owner, x + 1, y, areas);
    areas = area_set_component(g, old_owner, x, y - 1, areas);
    areas = area_set_component(g, old_owner, x, y + 1, areas);

    player_set_areas(old_owner, areas);

    area_search(g, old_owner, x - 1, y, UNCHECKED);
    area_search(g, old_owner, x + 1, y, UNCHECKED);
    area_search(g, old_owner, x, y - 1, UNCHECKED);
    area_search(g, old_owner, x, y + 1, UNCHECKED);
}

static void board_remove_rows(field_t ***board, uint32_t width,
                              uint32_t num_of_rows) {
    for (uint32_t i = 0; i < num_of_rows; i++) {
        for (uint32_t j = 0; j < width; j++) {
            field_delete(board[i][j]);
        }
        free(board[i]);
    }
}

static field_t ***board_new(uint32_t width, uint32_t height) {
    field_t ***board = malloc(height * sizeof(field_t ***));

    if (board == NULL) {
        return NULL;
    }
    else {
        bool mem_error = false;
        uint32_t added_rows = 0;

        for (uint32_t i = 0; i < height && !mem_error; i++, added_rows++) {
            board[i] = calloc(width, sizeof(field_t **));
            if (board[i] == NULL) {
                mem_error = true;
                board_remove_rows(board, width, added_rows);
            }
        }

        if (!mem_error) {
            return board;
        }
        else {
            free(board);
            return NULL;
        }
    }
}

static void board_fill(gamma_t *g, char *board) {
    uint64_t filled = 0;

    for (int64_t y = g->height - 1; y >= 0; y--, filled++) {
        for (uint32_t x = 0; x < g->width; x++, filled++) {
            if (g->board[y][x] == NULL) {
                board[filled] = '.';
            }
            else {
                board[filled] = player_number(field_owner(g->board[y][x])) + '0';
            }
        }
        board[filled] = '\n';
    }

    board[filled] = '\0';
}

static bool gamma_init(gamma_t *g, uint32_t width, uint32_t height,
                       uint32_t players, uint32_t areas) {
    g->board = board_new(width, height);

    if (g->board == NULL) {
        return false;
    }
    else {
        g->players_arr = calloc(players + 1, sizeof(player_t *));

        if (g->players_arr == NULL) {
            return false;
        }
        else {
            g->width = width;
            g->height = height;
            g->players = players;
            g->areas = areas;
            g->busy_fields = 0;
            return true;
        }
    }
}

static void gamma_move_update(gamma_t *g, uint32_t player, uint32_t x, uint32_t y) {
    if (g->players_arr[player] == NULL) {
        g->players_arr[player] = player_new(player);
    }

    g->board[y][x] = field_new(x, y, g->players_arr[player]);

    player_t *p = g->players_arr[player];
    field_t *f = g->board[y][x];

    g->busy_fields++;
    player_set_busy_fields(p, player_busy_fields(p) + 1);

    player_update_areas(g, f);
    neighbours_update_perimeter(g, f);
    player_update_perimeter(g, f, false);
}

static void gamma_golden_move_update(gamma_t *g, uint32_t player,
                                     uint32_t x, uint32_t y) {
    if (g->players_arr[player] == NULL) {
        g->players_arr[player] = player_new(player);
    }

    player_t *new_owner = g->players_arr[player];
    field_t *f = g->board[y][x];
    player_t *old_owner = field_owner(f);

    field_set_owner(f, new_owner);
    field_set_status(f, UNCHECKED);

    player_update_areas(g, f);
    player_update_perimeter(g, f, true);
    player_set_golden_possible(new_owner, false);
    player_set_busy_fields(new_owner, player_busy_fields(new_owner) + 1);

    old_owner_update_areas(g, old_owner, x, y);
    player_set_busy_fields(old_owner, player_busy_fields(old_owner) - 1);
    player_set_perimeter(old_owner,
                       player_perimeter(old_owner) -
                       player_adjacent_free_single_fields(g, old_owner, x, y));
}

gamma_t *gamma_new(uint32_t width, uint32_t height,
                   uint32_t players, uint32_t areas) {
    if (width == 0 || height == 0 || players == 0 || areas == 0) {
        return NULL;
    }
    else {
        gamma_t *g = malloc(sizeof(gamma_t));
        if (g == NULL) {
            return NULL;
        }
        else if (gamma_init(g, width, height, players, areas)) {
            return g;
        }
        else {
            gamma_delete(g);
            return NULL;
        }
    }
}

void gamma_delete(gamma_t *g) {
    if (g != NULL) {
        if (g->board != NULL) {
            board_remove_rows(g->board, g->width, g->height);
            free(g->board);
        }

        if (g->players_arr != NULL) {
            for (uint32_t i = 0; i <= g->players; i++) {
                player_delete(g->players_arr[i]);
            }
            free(g->players_arr);
        }

        free(g);
    }
}

bool gamma_move(gamma_t *g, uint32_t player, uint32_t x, uint32_t y) {
    if (g == NULL || !valid_player(g, player)) {
        return false;
    }
    else if (!player_move_legal(g, g->players_arr[player], x, y)) {
        return false;
    }
    else {
        gamma_move_update(g, player, x, y);
        return true;
    }
}

bool gamma_golden_possible(gamma_t *g, uint32_t player) {
    if (g == NULL || !valid_player(g, player)) {
        return false;
    }
    else if (g->players_arr[player] == NULL) {
        return g->busy_fields > 0;
    }
    else {
        player_t *p = g->players_arr[player];
        return player_golden_possible(p) && player_busy_fields(p) < g->busy_fields;
    }
}

bool gamma_golden_move(gamma_t *g, uint32_t player, uint32_t x, uint32_t y) {
    if (!gamma_golden_possible(g, player)) {
        return false;
    }
    else if (!valid_busy_field(g, x, y)) {
        return false;
    }
    else if (!player_golden_move_legal(g, g->players_arr[player], x, y)) {
        return false;
    }
    else if (!victim_golden_move_legal(g, x, y)) {
        return false;
    }
    else {
        gamma_golden_move_update(g, player, x, y);
        return true;
    }
}

uint64_t gamma_busy_fields(gamma_t *g, uint32_t player) {
    if (g == NULL || !valid_player(g, player) || g->players_arr[player] == NULL) {
        return 0;
    }
    else {
        return player_busy_fields(g->players_arr[player]);
    }
}

uint64_t gamma_free_fields(gamma_t *g, uint32_t player) {
    if (g == NULL || !valid_player(g, player)) {
        return 0;
    }
    else {
        player_t *p = g->players_arr[player];

        if (p == NULL || player_areas(p) < g->areas) {
            return g->width * g->height - g->busy_fields;
        }
        else {
            return player_perimeter(p);
        }
    }
}

char *gamma_board(gamma_t *g) {
    if (g == NULL) {
        return NULL;
    }
    else {
        char *board = malloc(((g->width + 1) * g->height + 1) * sizeof(char));

        if (board != NULL) {
            board_fill(g, board);
        }

        return board;
    }
}
