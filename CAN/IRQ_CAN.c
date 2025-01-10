/*----------------------------------------------------------------------------
 * Name:    Can.c
 * Purpose: CAN interface for for LPC17xx with MCB1700
 * Note(s): see also http://www.port.de/engl/canprod/sv_req_form.html
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2009 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#include <LPC17xx.h>                  /* LPC17xx definitions */
#include "CAN.h"                      /* LPC17xx CAN adaption layer */
#include "../GLCD/GLCD.h"
#include <stdio.h>

extern uint8_t icr ; 										//icr and result must be global in order to work with both real and simulated landtiger.
extern uint32_t result;
extern CAN_msg       CAN_TxMsg;    /* CAN message for sending */
extern CAN_msg       CAN_RxMsg;    /* CAN message for receiving */                                

/*----------------------------------------------------------------------------
  CAN interrupt handler
 *----------------------------------------------------------------------------*/
	custom_data current_data;

void CAN_IRQHandler (void)  {
	static custom_data old_data = {0};
	char string[40];
	/* check CAN controller 2 */
	icr = 0;
	icr = (LPC_CAN2->ICR | icr) & 0xFF;             /* clear interrupts */

	if (icr & (1 << 0)) {                          	/* CAN Controller #2 meassage is received */
		CAN_rdMsg (2, &CAN_RxMsg);	               	/* Read the message */
        LPC_CAN2->CMR = (1 << 2);                   /* Release receive buffer */

		current_data = *(custom_data*)&CAN_RxMsg.data;
		
		if(current_data.lives != old_data.lives){
			sprintf(string, "%d", current_data.lives);
			GUI_Text(54, 300, (uint8_t *) string, White, Black);
		}

		if(current_data.time != old_data.time){
			sprintf(string, "%02d", current_data.time);
			GUI_Text(112, 300, (uint8_t *) string, White, Black);
		}

		if(current_data.score != old_data.score){
			sprintf(string, "%04d", current_data.score);
			GUI_Text(200, 300, (uint8_t *) string, White, Black);
		}

		old_data = current_data;
	
	}
}
