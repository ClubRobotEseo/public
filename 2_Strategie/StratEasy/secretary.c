/*
 * secretary.c
 *
 *  Created on: 28 oct. 2018
 *      Author: Admin
 */

#include "secretary.h"
#include "actuators.h"
#include "remote.h"
#include "strategy.h"
#include "QS/QS_simulator/QS_simulator.h"
#include "QS/QS_lowLayer/QS_uart.h"
#include "QS/QS_can_over_uart.h"

static void SECRETARY_process_received_msg(CAN_msg_t * msg);
static bool_e initialized = FALSE;

void SECRETARY_init(void)
{
	if(CAN_init())
		initialized = TRUE;
}

void SECRETARY_process_main(void)
{
	CAN_msg_t msg;
	if(CAN_data_ready())
	{
		msg = CAN_get_next_msg();
		SECRETARY_process_received_msg(&msg);
	}

	static CAN_msg_t msg_u1rx;
	static bool_e parseInProgress = FALSE;
	if(UART1_data_ready())
	{
		Uint8 c;
		c = UART1_get_next_msg();
		//printf("c=%02x\n", c);
		if(u1rxToCANmsg(&msg_u1rx, c, &parseInProgress))
		{
			//printf("CANMSG>%4x-%d\n",msg_u1rx.sid, msg_u1rx.size);
			SECRETARY_process_received_msg(&msg_u1rx);
			SIMULATOR_processMain(&msg_u1rx, TRUE);
		}
		else
		{
			if(!parseInProgress)
				REMOTE_read_uart(c);
		}
	}

}


void SECRETARY_send(CAN_msg_t * msg)
{
	if(initialized)
		CAN_send(msg);
}


static void SECRETARY_process_received_msg(CAN_msg_t * msg)
{
	switch(msg->sid)
	{
		case ACT_EXPANDER_RESULT_SET_PUMP:
			ACTUATOR_process_msg(msg);
			break;
		case BROADCAST_START:
			//no break
		case IHM_BIROUTE_IS_REMOVED:
			STRATEGY_set_ask_for_broadcast_start();
			break;
		default:
			break;
	}
}

