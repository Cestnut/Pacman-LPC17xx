/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_timer.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    timer.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include <string.h>
#include "LPC17xx.h"
#include "timer.h"
#include "../GLCD/GLCD.h" 
#include "../led/led.h"
#include "Pacman/Pacman.h"
#include "config/config.h"
#include <stdio.h> /*for sprintf*/

/******************************************************************************
** Function name:		Timer0_IRQHandler
**
** Descriptions:		Timer/Counter 0 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/

extern player_struct player;
extern game_state_enum game_state;
extern int eaten_pills;
int toggle_pause_flag = 1;
int toggle_score = 1;
int toggle_lives = 1;
int toggle_end = 1;
int toggle_beginning = 1;

void TIMER0_IRQHandler (void)
{
	static int ticks=1, seconds=MAX_TIME;
	static int generated_power_pills=0;
	
	char string[40];
	
	if(game_state==RUNNING){
		if(toggle_beginning){
			sprintf(string, "Lives:        %d   Score:", MAX_TIME);
			generate_standard_pills();
			GUI_Text(0, 300, (uint8_t *)string, White, Black);	
			toggle_beginning = 0;
		}
		if(toggle_pause_flag){
			GUI_Text(100, 130, (uint8_t *) "     ", Yellow, Black);
			toggle_pause_flag = 0;
		}
		
		if(!(ticks%PACMAN_TICKS)) move_player();
		if(!(ticks%GHOST_TICKS)) move_ghost();

		if(!(ticks%10) && seconds > 0){
			seconds--;
			sprintf(string, "%02d", seconds);
			GUI_Text(112, 300, (uint8_t *) string, White, Black);
		}
		
		if(!(ticks%50) && generated_power_pills < POWER_PILLS_NUMBER){
			generate_power_pill();
			generated_power_pills++;
		}
		
		if(toggle_lives){
			sprintf(string, "%d", player.lives);
			GUI_Text(54, 300, (uint8_t *) string, White, Black);
			toggle_lives = 0;
		}
		
		if(toggle_score){
			sprintf(string, "%04d", player.score);
			GUI_Text(200, 300, (uint8_t *) string, White, Black);
			toggle_score = 0;
		}
		
		ticks++;
			
		if(seconds == 0) game_state = GAME_OVER;
		if(eaten_pills == STANDARD_PILLS_NUMBER) game_state = VICTORY;
	}
	else if(game_state == PAUSED){
		if(toggle_pause_flag){
			GUI_Text(100, 130, (uint8_t *) "PAUSE", Yellow, Black);
			toggle_pause_flag = 0;
		}
	}
	else if(game_state == VICTORY){
		if(toggle_end){
			GUI_Text(90, 130, (uint8_t *) "VICTORY", Yellow, Black);
			toggle_end = 0;
		}
	}
	else if(game_state == GAME_OVER){
		if(toggle_end){
			GUI_Text(85, 130, (uint8_t *) "GAME OVER", Red, Black);
			toggle_end = 0;
		}
	}

  LPC_TIM0->IR = 1;			/* clear interrupt flag */
  return;
}


/******************************************************************************
** Function name:		Timer1_IRQHandler
**
** Descriptions:		Timer/Counter 1 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
void TIMER1_IRQHandler (void)
{
  LPC_TIM1->IR = 1;			/* clear interrupt flag */
  return;
}

/******************************************************************************
**                            End Of File
******************************************************************************/
