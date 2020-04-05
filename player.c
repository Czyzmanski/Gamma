#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "player.h"

struct player {
    uint32_t number;
    uint32_t busy_fields;
    uint32_t areas;
    uint32_t borders;
    bool golden_possible;
};

player_t *player_new(uint32_t number, bool golden_possible) {
    player_t *p = malloc(sizeof(player_t));
    //TODO: check malloc

    p->number = number;
    p->busy_fields = 0;
    p->areas = 0;
    p->borders = 0;
    p->golden_possible = golden_possible;

    return p;
}

uint32_t player_number(player_t *p) {
    return p->number;
}

uint32_t player_busy_fields(player_t *p) {
    return p->busy_fields;
}

void player_set_busy_fields(player_t *p, uint32_t busy_fields) {
    p->busy_fields = busy_fields;
}

uint32_t player_areas(player_t *p) {
    return p->areas;
}

void player_set_areas(player_t *p, uint32_t areas) {
    p->areas = areas;
}

uint32_t player_borders(player_t *p) {
    return p->borders;
}

void player_set_borders(player_t *p, uint32_t borders) {
    p->borders = borders;
}

bool player_golden_possible(player_t *p) {
    return p->golden_possible;
}

void player_set_golden_possible(player_t *p, bool golden_possible) {
    p->golden_possible = golden_possible;
}

void player_delete(player_t *p) {
    if (p != NULL) {
        free(p);
    }
}
