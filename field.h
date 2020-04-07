#ifndef FIELD_H
#define FIELD_H

#include "player.h"

typedef struct field field_t;

enum state {
    UNCHECKED,
    COUNTED,
    MODIFIED
};

typedef enum state state_t;

field_t *field_new(uint32_t x, uint32_t y, player_t *owner);

uint32_t field_x(field_t *f);

uint32_t field_y(field_t *f);

player_t *field_owner(field_t *f);

void field_set_owner(field_t *f, player_t *owner);

field_t *field_parent(field_t *f);

void field_set_parent(field_t *f, field_t *parent);

uint32_t field_rank(field_t *f);

void field_set_rank(field_t *f, uint32_t rank);

state_t field_state(field_t *f);

void field_set_state(field_t *f, state_t state);

void field_delete(field_t *f);

#endif // FIELD_H
