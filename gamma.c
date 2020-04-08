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

static void area_new(field_t *f) {
    field_set_rank(f, 0);
    field_set_parent(f, NULL);

    player_t *owner = field_owner(f);
    player_set_areas(owner, player_areas(owner) + 1);
}

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

static bool merge_areas(field_t *f1, field_t *f2) {
    field_t *f1_root = area_find_root(f1);
    field_t *f2_root = area_find_root(f2);

    uint32_t f1_root_rank = field_rank(f1_root);
    uint32_t f2_root_rank = field_rank(f2_root);

    if (f1_root == f2_root) {
        return false;
    }
    else {
        if (f1_root_rank < f2_root_rank) {
            field_set_parent(f1, f2_root);
        }
        else if (f1_root_rank > f2_root_rank) {
            field_set_parent(f2, f1_root);
        }
        else {
            field_set_parent(f2, f1_root);
            field_set_rank(f1_root, f1_root_rank + 1);
        }
        return true;
    }
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
            for (uint32_t i = 0; i < g->players; i++) {
                player_delete(g->players_arr[i]);
            }
            free(g->players_arr);
        }
        free(g);
    }
}

static inline bool valid_player(gamma_t *g, int64_t player) {
    return 1 <= player && player <= g->players;
}

static inline bool valid_x(gamma_t *g, int64_t x) {
    return 0 <= x && x < g->width;
}

static inline bool valid_y(gamma_t *g, int64_t y) {
    return 0 <= y && y < g->height;
}

static inline bool valid_free_field(gamma_t *g, int64_t x, int64_t y) {
    return valid_x(g, x) && valid_y(g, y)
           && (g->board[y][x] == NULL || field_owner(g->board[y][x]) == NULL);
}

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

static void neighbours_update_borders(gamma_t *g, field_t *f) {
    player_t *owner = field_owner(f);
    player_t **neighbours = unique_neighbours(g, field_x(f), field_y(f));

    for (uint8_t i = 0; i < NEIGHBOURS; i++) {
        if (neighbours[i] != NULL && neighbours[i] != owner) {
            player_set_borders(neighbours[i], player_borders(neighbours[i]) - 1);
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
    uint32_t fields = 0;

    fields += player_valid_free_single_field(g, owner, x - 1, y);
    fields += player_valid_free_single_field(g, owner, x + 1, y);
    fields += player_valid_free_single_field(g, owner, x, y - 1);
    fields += player_valid_free_single_field(g, owner, x, y + 1);

    return fields;
}

static void player_update_borders(gamma_t *g, field_t *f, bool golden_move) {
    uint32_t x = field_x(f);
    uint32_t y = field_y(f);
    player_t *owner = field_owner(f);
    uint32_t borders = player_borders(owner);

    field_set_owner(f, NULL);

    if (!golden_move && player_adjacent_fields(g, owner, x, y) > 0) {
        borders--;
    }

    borders += player_valid_free_single_field(g, owner, x - 1, y);
    borders += player_valid_free_single_field(g, owner, x + 1, y);
    borders += player_valid_free_single_field(g, owner, x, y - 1);
    borders += player_valid_free_single_field(g, owner, x, y + 1);

    field_set_owner(f, owner);
    player_set_borders(owner, borders);
}

static void player_merge_adjacent_areas(gamma_t *g, field_t *f,
                                        int64_t x, int64_t y) {
    player_t *owner = field_owner(f);
    if (player_valid_field(g, owner, x, y) && merge_areas(f, g->board[y][x])) {
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
    neighbours_update_borders(g, f);
    player_update_borders(g, f, false);
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
                        int64_t x, int64_t y, state_t desired) {
    if (!player_valid_search_field(g, owner, x, y)) {
        return false;
    }
    else if (field_state(g->board[y][x]) == desired) {
        return false;
    }
    else {
        field_set_state(g->board[y][x], desired);

        area_search(g, owner, x - 1, y, desired);
        area_search(g, owner, x + 1, y, desired);
        area_search(g, owner, x, y - 1, desired);
        area_search(g, owner, x, y + 1, desired);

        return true;
    }
}

static uint32_t victim_new_areas(gamma_t *g, player_t *victim,
                                 uint32_t x, uint32_t y) {
    field_set_state(g->board[y][x], COUNTED);

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
        && field_state(g->board[y][x]) == COUNTED) {

        field_t *f = g->board[y][x];

        field_set_rank(f, 0);
        field_set_parent(f, parent);
        field_set_state(f, MODIFIED);

        area_update_parent_and_rank(g, owner, x - 1, y, parent);
        area_update_parent_and_rank(g, owner, x + 1, y, parent);
        area_update_parent_and_rank(g, owner, x, y - 1, parent);
        area_update_parent_and_rank(g, owner, x, y + 1, parent);
    }
}

static uint32_t area_set_component(gamma_t *g, player_t *old_owner,
                                   uint32_t x, uint32_t y, uint32_t areas) {
    if (player_valid_search_field(g, old_owner, x, y)) {
        if (field_state(g->board[y][x]) != MODIFIED) {
            areas++;

            field_t *root = g->board[y][x];
            field_set_state(root, MODIFIED);
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
    field_set_state(f, UNCHECKED);

    player_update_areas(g, f);
    player_update_borders(g, f, true);
    player_set_golden_possible(new_owner, false);
    player_set_busy_fields(new_owner, player_busy_fields(new_owner) + 1);

    old_owner_update_areas(g, old_owner, x, y);
    player_set_busy_fields(old_owner, player_busy_fields(old_owner) - 1);
    player_set_borders(old_owner,
                       player_borders(old_owner) -
                       player_adjacent_free_single_fields(g, old_owner, x, y));
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
            return player_borders(p);
        }
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

static void fill_board(gamma_t *g, char *board) {
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

char *gamma_board(gamma_t *g) {
    if (g == NULL) {
        return NULL;
    }
    else {
        char *board = malloc(((g->width + 1) * g->height + 1) * sizeof(char));

        if (board != NULL) {
            fill_board(g, board);
        }

        return board;
    }
}
