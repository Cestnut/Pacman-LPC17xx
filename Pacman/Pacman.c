#include "Pacman/Pacman.h"
#include "GLCD/GLCD.h"
#include <stdlib.h>     

player_struct player;
ghost_struct ghost;
game_state_enum game_state;
int eaten_pills = 0;
extern int toggle_lives;
extern int toggle_score;	

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

char ghost_pattern[TILE_X_SIZE][TILE_Y_SIZE] = {
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

entity board[LABYRINTH_X_SIZE][LABYRINTH_Y_SIZE] = {{EMPTY}};

typedef struct coordinates{
	int x, y;
} coordinates;

typedef struct tiles_list{
	coordinates tiles[400];
	int16_t index;
} tiles_list;
tiles_list empty_tiles, standard_pill_tiles;

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
					case GHOST: draw_tile(x,y, ghost_pattern); break;
					case WALL: draw_tile(x,y, wall_pattern); break;
					case STANDARD_PILL: draw_tile(x,y, standard_pill_pattern); break;
					case POWER_PILL: draw_tile(x,y, power_pill_pattern); break;
					case SPAWN: break;
				}
		}		
	}
}

void init_player(){
	player.x = 1;
	player.y = 1;
	board[player.x][player.y] = PACMAN;
	player.lives = 1;
	player.score = 0;
	player.status = NORMAL;
	player.direction = RIGHT;
}

void init_ghost(){
	ghost.x = 12;
	ghost.y = 13;
	board[ghost.x][ghost.y] = GHOST;

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
				new_points = 10;
				moved_flag = 1;
				eaten_pills++;
				break;
		case POWER_PILL:
				new_points = 50;
				moved_flag = 1;
				eaten_pills++;
				player.status = SUPER;
				break;
		case EMPTY: 
				moved_flag = 1;
				break;
		case GHOST: break;//gameover break;
	}
	
	if(new_points!=0){
		player.score += new_points;
		toggle_score = 1;
		if(!(player.score%1000)){
					player.lives++;
					toggle_lives = 1;
		}
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