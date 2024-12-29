/****************************************Copyright (c)****************************************************
**                                      
**                                 http://www.powermcu.com
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               main.c
** Descriptions:            The GLCD application function
**
**--------------------------------------------------------------------------------------------------------
** Created by:              AVRman
** Created date:            2010-11-7
** Version:                 v1.0
** Descriptions:            The original version
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             Paolo Bernardi
** Modified date:           03/01/2020
** Version:                 v2.0
** Descriptions:            basic program for LCD and Touch Panel teaching
**
*********************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "LPC17xx.h"
#include "GLCD/GLCD.h" 
#include "timer/timer.h"
#include "RIT/RIT.h"
#include "Pacman/Pacman.h"
#include <stdlib.h>
#include "led/led.h"
#include "joystick/joystick.h"
#include "button_EXINT/button.h"


#ifdef SIMULATOR
extern uint8_t ScaleFlag; // <- ScaleFlag needs to visible in order for the emulator to find the symbol (can be placed also inside system_LPC17xx.h but since it is RO, it needs more work)
#endif
int main(void)
{
	SystemInit();  												/* System Initialization (i.e., PLL)  */
		
	LCD_Initialization();
	LED_init();                           /* LED Initialization                 */
	BUTTON_init();												/* BUTTON Initialization              */
	joystick_init();
	init_timer(0, 0x2625a0); 						/* 100ms * 25MHz = 2.5*10^6 = 0x2625a0 */
	init_RIT(0x004C4B40);									/* RIT Initialization 50 msec       	*/
	enable_RIT();													/* RIT enabled												*/

	init_board();
	init_player();
	init_ghost();	
	draw_board();

	
	
	//LED_On(0);

	
	
	//LCD_DrawLine(0, 0, 200, 200, White);
	//init_timer(0, 0x6108 ); 						  /* 1ms * 25MHz = 25*10^3 = 0x6108 */
	//init_timer(0, 0x4E2 ); 						    /* 500us * 25MHz = 1.25*10^3 = 0x4E2 */
	//init_timer(0, 0xC8 ); 						    /* 8us * 25MHz = 200 ~= 0xC8 */
	
	enable_timer(0);
	
	LPC_SC->PCON |= 0x1;									/* power-down	mode										*/
	LPC_SC->PCON &= ~(0x2);						
	
  while (1)	
  {
		__ASM("wfi");
  }
}

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
