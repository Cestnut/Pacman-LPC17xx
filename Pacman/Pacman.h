#ifndef __PACMAN_H
#define __PACMAN_H

#include "LPC17xx.h"
#include "GLCD/GLCD.h"

#define LABYRINTH_X_SIZE 24
#define LABYRINTH_Y_SIZE 29
#define TILE_X_SIZE 10
#define TILE_Y_SIZE 10
#define STANDARD_PILLS_NUMBER 240
#define POWER_PILLS_NUMBER 6
#define MAX_TIME 90

typedef enum entity {SPAWN, EMPTY, PACMAN, GHOST, WALL, STANDARD_PILL, POWER_PILL} entity;
typedef enum player_status {NORMAL, SUPER} player_status;
typedef enum player_direction {STILL, LEFT, RIGHT, UP, DOWN} player_direction;
typedef enum game_state_enum {GAME_OVER, PAUSED, RUNNING, VICTORY} game_state_enum;

typedef struct player_struct{
	uint16_t x,y;
	uint16_t lives;
	uint16_t score;
	player_status status;
	player_direction direction;
} player_struct;

typedef struct ghost_struct{
	int x,y;
	entity hovering_entity;
} ghost_struct;


void init_board();
void draw_tile(uint16_t x, uint16_t y, char tile[TILE_X_SIZE][TILE_Y_SIZE]);
void draw_board();

void generate_standard_pills();
void generate_power_pill();

void init_player();
void init_ghost();

void move_player();
void move_ghost();


#endif /* end __PACMAN_H */
