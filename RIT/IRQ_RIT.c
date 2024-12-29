/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_RIT.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    RIT.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "LPC17xx.h"
#include "RIT.h"
#include "../led/led.h"
#include "../Pacman/Pacman.h"
#include <stdlib.h>

/******************************************************************************
** Function name:		RIT_IRQHandler
**
** Descriptions:		REPETITIVE INTERRUPT TIMER handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
#define check_joystick_direction(mask,pressed,new_direction){ \
		if((LPC_GPIO1->FIOPIN & mask) == 0){								\
				/* Joytick UP pressed */												\
				pressed++;																			\
				switch(pressed){																\
					case 2:	/*100ms = 100ms/50ms = 2*/							\
						player.direction = new_direction;														\
						break;																			\
					default:																			\
						break;																			\
				}																								\
			}																									\
			else{																							\
				pressed=0;																			\
			}																									\
		}

extern player_struct player;
extern game_state_enum game_state;
volatile int button_down=0;
extern int toggle_pause_flag;	
static int up=0, down=0, left=0, right=0;
static int position = 0;
int start_pressed = 0;
		
void RIT_IRQHandler (void)
{					

		check_joystick_direction(1<<29, up, UP);
		check_joystick_direction(1<<28, right, RIGHT);
		check_joystick_direction(1<<27, left, LEFT);
		check_joystick_direction(1<<26, down, DOWN);

	/* button management */
	if(button_down>=1){ 
		if((LPC_GPIO2->FIOPIN & (1<<10)) == 0){	/* INT0 pressed */
			switch(button_down){				
				case 2:				/* pay attention here: please see slides 19_ to understand value 2 */
					if(!start_pressed){
						srand(LPC_TIM0->TC);
						start_pressed = 1;
					}
					if(game_state != GAME_OVER){ /*To avoid Glitches*/
						if(game_state == PAUSED){
							toggle_pause_flag = 1;
							game_state = RUNNING;
						}
						else if(game_state == RUNNING){
							toggle_pause_flag = 1;
							game_state = PAUSED;
						}
					}
					break;
				default:
					break;
			}
			button_down++;
		}
		else {	/* button released */
			button_down=0;			
			NVIC_EnableIRQ(EINT0_IRQn);							 /* enable Button interrupts			*/
			LPC_PINCON->PINSEL4    |= (1 << 20);     /* External interrupt 0 pin selection */
		}
	}
	
  LPC_RIT->RICTRL |= 0x1;	/* clear interrupt flag */
	
  return;
}

/******************************************************************************
**                            End Of File
******************************************************************************/
