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

static void make_new_area(field_t *f) {
    field_set_rank(f, 0);
    field_set_parent(f, NULL);

    player_t *owner = field_owner(f);
    player_set_areas(owner, player_areas(owner) + 1);
}

static field_t *find_root(field_t *f) {
    if (field_parent(f) == NULL) {
        return f;
    }
    else {
        field_t *root = find_root(field_parent(f));
        field_set_parent(f, root);
        return root;
    }
}

static bool merge_areas(field_t *f1, field_t *f2) {
    field_t *f1_root = find_root(f1);
    field_t *f2_root = find_root(f2);

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
        for (uint32_t i = 0; i < height && !mem_error; i++) {
            board[i] = calloc(width, sizeof(field_t **));
            if (board[i] == NULL) {
                mem_error = true;
                board_remove_rows(board, width, i);
            }
        }
        if (mem_error) {
            free(board);
            return NULL;
        }
        else {
            return board;
        }
    }
}

gamma_t *gamma_new(uint32_t width, uint32_t height,
                   uint32_t players, uint32_t areas) {
    if (width <= 0 || height <= 0 || players <= 0 || areas <= 0) {
        return NULL;
    }
    else {
        gamma_t *g = malloc(sizeof(gamma_t));
        if (g == NULL) {
            return NULL;
        }
        else {
            g->board = board_new(width, height);
            if (g->board == NULL) {
                free(g);
                return NULL;
            }
            else {
                g->players_arr = calloc(players + 1, sizeof(player_t *));
                if (g->players_arr == NULL) {
                    free(g);
                    return NULL;
                }
                else {
                    g->width = width;
                    g->height = height;
                    g->players = players;
                    g->areas = areas;
                    g->busy_fields = 0;
                    return g;
                }
            }
        }
    }
}

void gamma_delete(gamma_t *g) {
    if (g != NULL) {
        board_remove_rows(g->board, g->width, g->height);
        free(g->board);

        for (uint32_t i = 0; i < g->players; i++) {
            player_delete(g->players_arr[i]);
        }
        free(g->players_arr);

        free(g);
    }
}

static inline bool valid_player(gamma_t *g, uint32_t player) {
    return 1 <= player && player <= g->players;
}

static inline bool valid_x(gamma_t *g, uint32_t x) {
    return 0 <= x && x < g->width;
}

static inline bool valid_y(gamma_t *g, uint32_t y) {
    return 0 <= y && y < g->height;
}

static inline bool valid_free_field(gamma_t *g, uint32_t x, uint32_t y) {
    return valid_x(g, x) && valid_y(g, y) && g->board[y][x] == NULL;
}

static bool valid_owner_field(gamma_t *g, player_t *owner, uint32_t x, uint32_t y) {
    if (!valid_x(g, x) || !valid_y(g, y)) {
        return false;
    }
    else {
        field_t *f = g->board[y][x];
        return f != NULL && field_owner(f) == owner;
    }
}

static uint8_t adjacent_player_fields(gamma_t *g, player_t *owner,
                                      uint32_t x, uint32_t y) {
    uint8_t c = 0;

    c += valid_owner_field(g, owner, x - 1, y);
    c += valid_owner_field(g, owner, x + 1, y);
    c += valid_owner_field(g, owner, x, y - 1);
    c += valid_owner_field(g, owner, x, y + 1);

    return c;
}

static bool move_possible(gamma_t *g, player_t *p, uint32_t x, uint32_t y) {
    if (!valid_free_field(g, x, y)) {
        return false;
    }
    else if (p == NULL || player_areas(p) < g->areas) {
        return true;
    }
    else {
        return adjacent_player_fields(g, p, x, y) > 0;
    }
}

static bool unique_neighbour(player_t *p, player_t **neighbours, uint8_t added) {
    uint8_t i = 0;

    while (i < added && p != neighbours[i]) {
        i++;
    }

    return i == added;
}

static bool valid_unique_neighbour_field(gamma_t *g, uint32_t x, uint32_t y,
                                         player_t **neighbours, uint8_t added) {
    if (!valid_x(g, x) || !valid_y(g, y)) {
        return false;
    }
    else {
        field_t *f = g->board[y][x];
        return f != NULL && unique_neighbour(field_owner(f), neighbours, added);
    }
}

static uint8_t add_neighbour_if_unique(gamma_t *g, uint32_t x, uint32_t y,
                                       player_t **neighbours, uint8_t added) {
    if (valid_unique_neighbour_field(g, x, y, neighbours, added)) {
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

static void update_neighbours_borders(gamma_t *g, field_t *f) {
    player_t *owner = field_owner(f);
    player_t **neighbours = unique_neighbours(g, field_x(f), field_y(f));

    for (uint8_t i = 0; i < NEIGHBOURS; i++) {
        if (neighbours[i] != NULL && neighbours[i] != owner) {
            player_set_borders(neighbours[i], player_borders(neighbours[i]) - 1);
        }
    }

    free(neighbours);
}

static inline bool valid_free_single_field(gamma_t *g, player_t *owner,
                                           uint32_t x, uint32_t y) {
    return valid_free_field(g, x, y) && adjacent_player_fields(g, owner, x, y) == 1;
}

static void update_player_borders(gamma_t *g, field_t *f) {
    uint32_t x = field_x(f);
    uint32_t y = field_y(f);
    player_t *owner = field_owner(f);
    uint32_t borders = player_borders(owner);

    if (adjacent_player_fields(g, owner, x, y) > 0) {
        borders--;
    }

    borders += valid_free_single_field(g, owner, x - 1, y);
    borders += valid_free_single_field(g, owner, x + 1, y);
    borders += valid_free_single_field(g, owner, x, y - 1);
    borders += valid_free_single_field(g, owner, x, y + 1);

    player_set_borders(owner, borders);
}

static void merge_adjacent_player_areas_if_different(gamma_t *g, field_t *f,
                                                     uint32_t x, uint32_t y) {
    player_t *owner = field_owner(f);
    if (valid_owner_field(g, owner, x, y)) {
        if (merge_areas(f, g->board[y][x])) {
            player_set_areas(owner, player_areas(owner) - 1);
        }
    }
}

static void update_player_areas(gamma_t *g, field_t *f) {
    make_new_area(f);

    uint32_t x = field_x(f);
    uint32_t y = field_y(f);

    merge_adjacent_player_areas_if_different(g, f, x - 1, y);
    merge_adjacent_player_areas_if_different(g, f, x + 1, y);
    merge_adjacent_player_areas_if_different(g, f, x, y - 1);
    merge_adjacent_player_areas_if_different(g, f, x, y + 1);
}

bool gamma_move(gamma_t *g, uint32_t player, uint32_t x, uint32_t y) {
    if (g == NULL || !valid_player(g, player)) {
        return false;
    }
    else if (!move_possible(g, g->players_arr[player], x, y)) {
        return false;
    }
    else {
        if (g->players_arr[player] == NULL) {
            g->players_arr[player] = player_new(player, true);
        }

        g->board[y][x] = field_new(x, y, g->players_arr[player]);

        player_t *p = g->players_arr[player];
        field_t *f = g->board[y][x];

        g->busy_fields++;
        player_set_busy_fields(p, player_busy_fields(p) + 1);

        update_player_areas(g, f);
        update_neighbours_borders(g, f);
        update_player_borders(g, f);

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

char* gamma_board(gamma_t *g) {
    if (g == NULL) {
        return NULL;
    }
    else {
        char *board = malloc((g->width + 1) * g->height);

        if (board != NULL) {
            uint64_t index = 0;

            for (uint32_t y = g->height - 1; y >= 0; y--, index++) {
                for (uint32_t x = 0; x < g->width; x++, index++) {
                    board[index] = player_number(g->board[y][x]) - '0';
                }
                board[index] = '\n';
            }
        }
        
        return board;
    }
}
