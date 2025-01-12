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
#include "CAN/CAN.h"     
#include <stdlib.h>     

/* LPC17xx CAN adaption layer */
/******************************************************************************
** Function name:		Timer0_IRQHandler
**
** Descriptions:		Timer/Counter 0 interrupt handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/

extern ghost_struct ghost;
extern player_struct player;
extern game_state_enum game_state;
extern int eaten_pills;
int toggle_pause_flag = 1;
int toggle_score = 1;
int toggle_lives = 1;
int toggle_end = 1;
int toggle_beginning = 1;
int ticks=1;
int seconds=MAX_TIME;
int power_pill_tick; /*The tick in which pacman has eaten the power pill*/
int ghost_death_tick; /*The tick in which the ghost has died*/
int new_power_pill_generation_tick;

extern int board_drawn_sound;
custom_data CAN_data;
extern CAN_msg CAN_TxMsg;

void TIMER0_IRQHandler (void)
{
	static int generated_power_pills=0;	
	char string[40];

	if(game_state==RUNNING && board_drawn_sound == 0){
		if(toggle_beginning){
			sprintf(string, "Lives:        %d   Score:", MAX_TIME);
			generate_standard_pills();
			GUI_Text(0, 300, (uint8_t *)string, White, Black);	
			toggle_beginning = 0;
			new_power_pill_generation_tick = (rand()%(POWER_PILL_GENERATION_MAX_TICKS-POWER_PILL_GENERATION_MIN_TICKS))+POWER_PILL_GENERATION_MIN_TICKS;
		}
		if(toggle_pause_flag){
			GUI_Text(100, 130, (uint8_t *) "     ", Yellow, Black);
			toggle_pause_flag = 0;
		}
		
		if(!(ticks%PACMAN_TICKS)) move_player();		
		if(!(ticks%GHOST_TICKS)) move_ghost();
		
		if(!(ticks%10) && seconds > 0){
			seconds--;
			#ifdef SIMULATOR //Will send message to CAN message if the simulator is not being used
			sprintf(string, "%02d", seconds);
			GUI_Text(112, 300, (uint8_t *) string, White, Black);
			#else
			CAN_data.time = seconds;
			#endif
		}
		
		if((ticks == new_power_pill_generation_tick) && generated_power_pills < POWER_PILLS_NUMBER){
			generate_power_pill();
			generated_power_pills++;
			new_power_pill_generation_tick = ticks + (rand()%(POWER_PILL_GENERATION_MAX_TICKS-POWER_PILL_GENERATION_MIN_TICKS))+POWER_PILL_GENERATION_MIN_TICKS;
		}
		
		if(toggle_lives){
			#ifdef SIMULATOR
			sprintf(string, "%d", player.lives);
			GUI_Text(54, 300, (uint8_t *) string, White, Black);
			#else
			CAN_data.lives = player.lives;
			#endif
			toggle_lives = 0;
		}
		
		if(toggle_score){
			#ifdef SIMULATOR
			sprintf(string, "%04d", player.score);
			GUI_Text(200, 300, (uint8_t *) string, White, Black);
			#else
			CAN_data.score = player.score;
			#endif
			toggle_score = 0;
		}
		
		if(player.status == SUPER){
			if(ticks - power_pill_tick == SUPER_STATUS_DURATION_TICKS){
				player.status = NORMAL;
			}
		}

		if(ghost.status == DEAD){
			/*See if N ticks have elapsed*/
			if((ticks - ghost_death_tick) == GHOST_RESPAWN_TICKS){
				ghost.status = ALIVE;
			}
		}

		ticks++;
		#ifndef SIMULATOR
		CAN_TxMsg.data = *(unsigned int*)&CAN_data;
		CAN_TxMsg.len = 4;
		CAN_TxMsg.id = 2;
		CAN_TxMsg.format = STANDARD_FORMAT;
		CAN_TxMsg.type = DATA_FRAME;
		CAN_wrMsg(1, &CAN_TxMsg);
		#endif

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
uint16_t SinTable[45] =                                       /* ���ұ�                       */
{
    410, 467, 523, 576, 627, 673, 714, 749, 778,
    799, 813, 819, 817, 807, 789, 764, 732, 694, 
    650, 602, 550, 495, 438, 381, 324, 270, 217,
    169, 125, 87 , 55 , 30 , 12 , 2  , 0  , 6  ,   
    20 , 41 , 70 , 105, 146, 193, 243, 297, 353
};

void TIMER1_IRQHandler (void)
{
	static int sineticks=0;
	/* DAC management */	
	static int currentValue; 
	currentValue = SinTable[sineticks];
	currentValue -= 410;
	currentValue /= 1;
	currentValue += 410;
	LPC_DAC->DACR = currentValue <<6;
	sineticks++;
	if(sineticks==45) sineticks=0;
	
  LPC_TIM1->IR = 1;			/* clear interrupt flag */
  return;
}

void TIMER2_IRQHandler (void)
{	
	disable_timer(1);
  LPC_TIM2->IR = 1;			/* clear interrupt flag */
  return;
}

/******************************************************************************
**                            End Of File
******************************************************************************/
