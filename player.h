#ifndef PLAYER_H
#define PLAYER_H

typedef struct player player_t;

player_t *player_new(uint32_t number, bool golden_possible);

uint32_t player_number(player_t *p);

uint64_t player_busy_fields(player_t *p);

void player_set_busy_fields(player_t *p, uint64_t busy_fields);

uint32_t player_areas(player_t *p);

void player_set_areas(player_t *p, uint32_t areas);

uint32_t player_borders(player_t *p);

void player_set_borders(player_t *p, uint32_t borders);

bool player_golden_possible(player_t *p);

void player_set_golden_possible(player_t *p, bool golden_possible);;

void player_delete(player_t *p);

#endif // GAMMA_PLAYER_H
