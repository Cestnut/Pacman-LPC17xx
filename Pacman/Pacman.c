#include "Pacman/Pacman.h"
#include "GLCD/GLCD.h"
#include <stdlib.h>     
#include "config/config.h"

player_struct player;
ghost_struct ghost;
game_state_enum game_state;
int eaten_pills = 0;
extern int toggle_lives;
extern int toggle_score;	
extern int toggle_power_pill;	
extern int ticks;
extern int power_pill_tick;
extern int ghost_death_tick; 
int board_drawn_sound = 0, pill_eaten_sound = 0;

char char_board[LABYRINTH_X_SIZE][LABYRINTH_Y_SIZE] = {	
			"#############-###############",
			"#P--------------------##----#",
			"#-###-##-##-#-####-##-##-##-#",			
			"#-###-##-##-#-####-##----##-#",			
			"#-###-##-##-#-####-#####-##-#",			
			"#------------------------##-#",			
			"#-###-#######-##-#-########-#",			
			"#-###----#---------##----##-#",
			"#-###-##-#-#####-#-##-##-##-#",
			"#-###-##-#-##@@#-#-##-##-##-#",			
			"#-----##---##@@#-#----##----#",			
			"#####-####-@@@@#-####-#####-#",			
			"#####-####-@@G@#-####-#####-#",			
			"#-----##---##@@#-#----##----#",			
			"#-###-##-#-##@@#-#-##-##-##-#",			
			"#-###-##-#-#####-#-##-##-##-#",
			"#-###----#---------##----##-#",
			"#-###-#######-##-#-########-#",			
			"#------------------------##-#",			
			"#-###-##-##-#-####-#####-##-#",			
			"#-###-##-##-#-####-##----##-#",			
			"#-###-##-##-#-####-##-##-##-#",			
			"#---------------------##----#",
			"#############-###############"
};

char wall_pattern[TILE_X_SIZE][TILE_Y_SIZE] = {"BBBBBBBBBB",
			"BBBBBBBBBB",
			"BBBBBBBBBB",
			"BBBBBBBBBB",
			"BBBBBBBBBB",
			"BBBBBBBBBB",
			"BBBBBBBBBB",
			"BBBBBBBBBB",
			"BBBBBBBBBB",
			"BBBBBBBBBB"
};

char standard_pill_pattern[TILE_X_SIZE][TILE_Y_SIZE] = {
			"bbbbbbbbbb",
			"bbbbbbbbbb",
			"bbbbYYbbbb",
			"bbbYYYYbbb",
			"bbYYYYYYbb",
			"bbYYYYYYbb",
			"bbbYYYYbbb",
			"bbbbYYbbbb",
			"bbbbbbbbbb",
			"bbbbbbbbbb",
};

char power_pill_pattern[TILE_X_SIZE][TILE_Y_SIZE] = {
			"bbbbbbbbbb",
			"bbbYYYYbbb",
			"bbYYYYYYbb",
			"bYYYYYYYYb",
			"bYYYYYYYYb",
			"bYYYYYYYYb",
			"bYYYYYYYYb",
			"bbYYYYYYbb",
			"bbbYYYYbbb",
			"bbbbbbbbbb",
};

char pacman_pattern[TILE_X_SIZE][TILE_Y_SIZE] = {
			"bbbbbbbbbb",
			"bbbYYYYbbb",
			"bbYYYYYYbb",
			"bYYYYYYYbb",
			"bYYYYbbbbb",
			"bYYYYbbbbb",
			"bYYYYYYYbb",
			"bbYYYYYYbb",
			"bbbYYYYbbb",
			"bbbbbbbbbb",
};

char ghost_chasing_pattern[TILE_X_SIZE][TILE_Y_SIZE] = {
			"bbbRRRRbbb",
			"bbRRRRRRRb",
			"bRWWRRRWWb",
			"RWWBBRWWBB",
			"RWWBBRWWBB",
			"RWWWWRWWWW",
			"RRWWRRRWWR",
			"RRRRRRRRRR",
			"RRbRRRRbRR",
			"RbbbRRbbbR"
};

char ghost_scared_pattern[TILE_X_SIZE][TILE_Y_SIZE] = {
			"bbbBBBBbbb",
			"bbBBBBBBBb",
			"bBWWBBBWWb",
			"BWWBBBWWBB",
			"BWWBBBWWBB",
			"BWWWWBWWWW",
			"BBWWBBBWWB",
			"BBBBBBBBBB",
			"BBbBBBBbBB",
			"BbbbBBbbbB"
};

char empty_pattern[TILE_X_SIZE][TILE_Y_SIZE] = {
			"bbbbbbbbbb",
			"bbbbbbbbbb",
			"bbbbbbbbbb",
			"bbbbbbbbbb",
			"bbbbbbbbbb",
			"bbbbbbbbbb",
			"bbbbbbbbbb",
			"bbbbbbbbbb",
			"bbbbbbbbbb",
			"bbbbbbbbbb"
};

tile_type board[LABYRINTH_X_SIZE][LABYRINTH_Y_SIZE] = {{EMPTY}};

typedef struct coordinates{
	int x, y;
} coordinates;

typedef struct tiles_list{
	coordinates tiles[400];
	int16_t index;
} tiles_list;
tiles_list empty_tiles, standard_pill_tiles;

typedef struct node{
	coordinates coordinates;
	int father;
	int8_t depth;
} node;

typedef struct node_queue{
	int head, tail;
	node nodes[400];
} node_queue;
node_queue frontier_queue;


void init_board(){
		int x,y;
		empty_tiles.index = 0;
		for(x=0; x<LABYRINTH_X_SIZE; x++){
			for(y=0; y<LABYRINTH_Y_SIZE; y++){
				if(char_board[x][y] == '#') board[x][y] = WALL;
				else if(char_board[x][y] == 'P') board[x][y] = PACMAN;
				else if(char_board[x][y] == '@') board[x][y] = SPAWN;
				else if(char_board[x][y] == 'G') board[x][y] = GHOST;
				else if(char_board[x][y] == '-'){
					board[x][y] = EMPTY;
					empty_tiles.tiles[empty_tiles.index].x = x;
					empty_tiles.tiles[empty_tiles.index].y = y;
					empty_tiles.index++;
				}
				
			}
		}
		game_state = PAUSED;
}

void generate_standard_pills(){
		//Select STANDARD_PILLS_NUMBER random positions from empty_tiles to make them STANDARD_PILLS
		uint16_t i, inserted_pills=0, inserted_power_pills=0, x, y;
		for(i=0; i<empty_tiles.index; i++){
			//Using Selection Sampling
			if(((STANDARD_PILLS_NUMBER - inserted_pills)*100)/(empty_tiles.index-i) > (rand()%100)){
				x = empty_tiles.tiles[i].x;
				y = empty_tiles.tiles[i].y;
				board[x][y] = STANDARD_PILL;
				standard_pill_tiles.tiles[inserted_pills].x = x;
				standard_pill_tiles.tiles[inserted_pills].y = y;
				draw_tile(x,y,standard_pill_pattern);	
				inserted_pills++;
				//whenever a new pill is added repeat the algorithm to insert POWER PILL
			if(inserted_pills == STANDARD_PILLS_NUMBER) break;
			}	
		}
}

void generate_power_pill(){
/*This function selects a random standard pill and makes it a power pill, redrawing the tile*/
	uint16_t pill_index, x, y;
	do{
		pill_index = rand()%STANDARD_PILLS_NUMBER;	
		x = standard_pill_tiles.tiles[pill_index].x;
		y = standard_pill_tiles.tiles[pill_index].y;
	}while(board[x][y] == POWER_PILL || board[x][y] == EMPTY); /*To choose another pill in case it has already been transformed or eaten*/

	board[x][y] = POWER_PILL;
	draw_tile(x,y,power_pill_pattern);	
}

void draw_tile(uint16_t x, uint16_t y, char tile[TILE_X_SIZE][TILE_Y_SIZE]){
		uint16_t drawing_x, drawing_y, color;
		for(drawing_x=0; drawing_x<TILE_X_SIZE; drawing_x++){
			for(drawing_y=0; drawing_y<TILE_Y_SIZE; drawing_y++){
				switch(tile[drawing_y][drawing_x]){
					case 'B': color = Blue; break;
					case 'Y': color = Yellow; break;
					case 'b': color = Black; break;
					case 'R': color = Red; break;
					case 'W': color = White; break;
				}
				LCD_SetPoint((x*TILE_X_SIZE)+drawing_x,(y*TILE_Y_SIZE)+drawing_y,color);
			}
		}
}

void draw_board(){
	uint16_t x,y;
	
	LCD_Clear(Black);

	
	for(x=0; x<LABYRINTH_X_SIZE; x++){
		for(y=0; y<LABYRINTH_Y_SIZE; y++){
				switch(board[x][y]){
					case EMPTY: break;
					case PACMAN: draw_tile(x,y, pacman_pattern); break;
					case GHOST: draw_tile(x,y, ghost_chasing_pattern); break;
					case WALL: draw_tile(x,y, wall_pattern); break;
					case STANDARD_PILL: draw_tile(x,y, standard_pill_pattern); break;
					case POWER_PILL: draw_tile(x,y, power_pill_pattern); break;
					case SPAWN: break;
				}
		}		
	}
	board_drawn_sound = 1;
}

void init_player(){
	player.x = 1;
	player.y = 1;
	board[player.x][player.y] = PACMAN;
	player.lives = 1;
	player.score = 0;
	player.status = NORMAL;
	player.direction = STILL;
}

void init_ghost(){
	ghost.x = 12;
	ghost.y = 13;
	ghost.hovering_tile_type = SPAWN;
	board[ghost.x][ghost.y] = GHOST;
	ghost.status = ALIVE;

}

void move_player(){
	int16_t new_x=player.x, new_y=player.y, moved_flag=0, new_points=0;
	switch(player.direction){
		case STILL: break;
		case RIGHT: new_x=(player.x + 1)%LABYRINTH_X_SIZE; break;
		case LEFT: 	new_x=(player.x - 1); 
								if(new_x == -1) new_x = LABYRINTH_X_SIZE - 1;
								break;
		case UP: 		new_y=(player.y - 1); break;
		case DOWN: 	new_y=(player.y + 1); break;
	}

	switch(board[new_x][new_y]){
		case PACMAN:
		case SPAWN:
		case WALL: player.direction = STILL; break;
		case STANDARD_PILL:
				increase_score(STANDARD_PILL_VALUE);
				moved_flag = 1;
				eaten_pills++;
				pill_eaten_sound = 1;
				break;
		case POWER_PILL:
				increase_score(POWER_PILL_VALUE);
				moved_flag = 1;
				eaten_pills++;
				player.status = SUPER;
				power_pill_tick = ticks;
				pill_eaten_sound = 1;
				break;
		case EMPTY: 
				moved_flag = 1;
				break;
		case GHOST: 
				if(player.status == SUPER){
					increase_score(GHOST_VALUE);
					init_ghost(); /*Puts ghost back to starting position*/
					moved_flag = 1;
					ghost.status = DEAD;
					ghost_death_tick = ticks;
					player.status = NORMAL;
				}
				else{
					player.lives--;
					toggle_lives = 1;
					if(player.lives == 0){
						game_state = GAME_OVER;
					}
					else{
						draw_tile(ghost.x, ghost.y, empty_pattern);
						init_ghost();
						init_player();
						draw_tile(player.x, player.y, pacman_pattern);
						return;
					}
				};
				break;
	}
	
	if(moved_flag == 1){
		board[new_x][new_y] = PACMAN;
		board[player.x][player.y] = EMPTY;
		
		draw_tile(new_x,new_y, pacman_pattern);
		draw_tile(player.x,player.y, empty_pattern);
		
		player.x = new_x; 
		player.y = new_y;
	}
}

void move_ghost(){
	if(ghost.status == DEAD) return;
	int found_node_index = -1;
	int j, k, z; /*Indexes used to iterate through the frontier*/
	int already_explored_flag; /*When a valid node has been found, this flag is used to exclude nodes that have already been explored*/
	int x,y; /*Extracting coordinates from each node to ease manipulation*/


	frontier_queue.nodes[0].coordinates.x = ghost.x;
	frontier_queue.nodes[0].coordinates.y = ghost.y;
	frontier_queue.nodes[0].father = -1;
	frontier_queue.nodes[0].depth = 0;	

	frontier_queue.head = 1;	
	frontier_queue.tail = 0;

	if(player.status == NORMAL){
		while(frontier_queue.tail < frontier_queue.head && found_node_index == -1){
			for(j=-1; j<=1 && found_node_index == -1; j++){
				for(k=-1; k<=1 && found_node_index == -1; k++){
					if(j != 0 && k != 0) continue; /*So diagonal squares get skipped*/
					x = frontier_queue.nodes[frontier_queue.tail].coordinates.x + j; /*x to explore*/
					y = frontier_queue.nodes[frontier_queue.tail].coordinates.y + k; /*y to explore*/
				
					/*To warp to both ends of the labyrinth*/
					if(x == -1) x = LABYRINTH_X_SIZE - 1;
					if(x == LABYRINTH_X_SIZE) x = 0;
					if(board[x][y] != WALL){
						/*Check if the node has already been explored*/
						already_explored_flag=0;
						for(z=0; z<frontier_queue.head; z++){
							if(frontier_queue.nodes[z].coordinates.x == x && frontier_queue.nodes[z].coordinates.y == y){
								already_explored_flag = 1;
								break;
							}
						}

						/*If the node hasn't already been explored*/
						if(!already_explored_flag){
							/*Insert new node in frontier*/
							frontier_queue.nodes[frontier_queue.head].coordinates.x = x;
							frontier_queue.nodes[frontier_queue.head].coordinates.y = y;
							frontier_queue.nodes[frontier_queue.head].father = frontier_queue.tail;
							frontier_queue.nodes[frontier_queue.head].depth = frontier_queue.nodes[frontier_queue.tail].depth + 1 ; /*This line sets the depth to that of the father +1*/

							if(board[x][y] == PACMAN){
								found_node_index = frontier_queue.head;
							}

							frontier_queue.head++;
						}
					}
				}
			}	
			frontier_queue.tail++;
		}
		
		while(frontier_queue.nodes[found_node_index].father != 0){
			found_node_index = frontier_queue.nodes[found_node_index].father;
		}
		//Here we have the new coordinates
		x = frontier_queue.nodes[found_node_index].coordinates.x;
		y = frontier_queue.nodes[found_node_index].coordinates.y;
	}
	else if(player.status == SUPER){
		int max_distance = 0, tmp_distance=0, new_x, new_y;
		for(j=-1; j<=1 && found_node_index == -1; j++){
				for(k=-1; k<=1 && found_node_index == -1; k++){
					if(j != 0 && k != 0) continue; /*So diagonal squares get skipped*/
					new_x = ghost.x + j; /*x to explore*/
					new_y = ghost.y + k; /*y to explore*/
				
					/*To warp to both ends of the labyrinth*/
					if(new_x == -1) new_x = LABYRINTH_X_SIZE - 1;
					if(new_x == LABYRINTH_X_SIZE) new_x = 0;
					if(board[new_x][new_y] != WALL && board[new_x][new_y] != SPAWN){
						/*Calculates the distance of the current chosen tile from pacman*/
						tmp_distance = (new_x-player.x)*(new_x-player.x) + (new_y-player.y)*(new_y-player.y);
						if(tmp_distance > max_distance){
							max_distance = tmp_distance;
							x = new_x;
							y = new_y;
						}
					}
			}
		}
	}
	//For every node Until the queue isn't empty


	if(x!=ghost.x || y!=ghost.y){ /*Checks if a new position has been found. This should always be the case*/

		if(player.status==NORMAL) draw_tile(x,y, ghost_chasing_pattern);
		else if(player.status == SUPER) draw_tile(x,y, ghost_scared_pattern);
		
		switch (ghost.hovering_tile_type)
		{
		case SPAWN:
		case EMPTY:
			draw_tile(ghost.x,ghost.y, empty_pattern);
			break;
		case STANDARD_PILL:
			draw_tile(ghost.x,ghost.y, standard_pill_pattern);
			break;
		case POWER_PILL:
			draw_tile(ghost.x,ghost.y, power_pill_pattern);
			break;
		default:
			break;
		}
		
		board[ghost.x][ghost.y] = ghost.hovering_tile_type;
		ghost.hovering_tile_type = board[x][y];
		board[x][y] = GHOST;		

		ghost.x = x; 
		ghost.y = y;
	
		if(ghost.x == player.x && ghost.y == player.y){
			if(player.status == SUPER){
				init_ghost();
				ghost.status = DEAD;
				ghost_death_tick = ticks;
				increase_score(GHOST_VALUE);
			}
			else if(player.status == NORMAL){
				player.lives--;
				toggle_lives = 1;
				if(player.lives == 0){
					game_state = GAME_OVER;
				}
				else{
					draw_tile(ghost.x, ghost.y, empty_pattern);
					init_ghost();
					init_player();
					draw_tile(player.x, player.y, pacman_pattern);
					return;
				}
			}
		}
	
	}
}

void increase_score(int value){
	int floored_new_score, floored_old_score;
	toggle_score = 1;
	
	floored_new_score = (int)((player.score+value)/1000);
	if(floored_new_score > (player.score+value)/1000) floored_new_score -= 1;

	floored_old_score = (int)(player.score / 1000);
	if(floored_old_score > (player.score / 1000)) floored_old_score -= 1;

	if(floored_new_score > floored_old_score){ /*If the player has surpassed the 1000 points threshold*/
		player.lives++;
		toggle_lives = 1;
	}
	player.score += value;
}