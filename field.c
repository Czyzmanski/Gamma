#include <stdlib.h>
#include <stdint.h>

#include "field.h"

struct field {
    uint32_t x;
    uint32_t y;
    player_t *owner;
    field_t *parent;
    uint32_t rank;
    state_t state;
};

field_t *field_new(uint32_t x, uint32_t y, player_t *owner) {
    field_t *f = malloc(sizeof(field_t));

    f->x = x;
    f->y = y;
    f->owner = owner;
    f->parent = NULL;
    f->rank = 0;
    f->state = UNCHECKED;

    return f;
}

uint32_t field_x(field_t *f) {
    return f->x;
}

uint32_t field_y(field_t *f) {
    return f->y;
}

player_t *field_owner(field_t *f) {
    return f->owner;
}

void field_set_owner(field_t *f, player_t *owner) {
    f->owner = owner;
}

field_t *field_parent(field_t *f) {
    return f->parent;
}

void field_set_parent(field_t *f, field_t *parent) {
    f->parent = parent;
}

uint32_t field_rank(field_t *f) {
    return f->rank;
}

void field_set_rank(field_t *f, uint32_t rank) {
    f->rank = rank;
}

state_t field_state(field_t *f) {
    return f->state;
}

void field_set_state(field_t *f, state_t state) {
    f->state = state;
}

void field_delete(field_t *f) {
    if (f != NULL) {
        free(f);
    }
}
