/*
 * Toolkit.c
 *
 *  Created on: 29 oct. 2018
 *      Author: Admin
 */

#include "toolkit.h"
#include "QS/QS_can_over_uart.h"

static void TOOLKIT_process_send(void);

#define UPDATE_GRID_XY		50
#define UPDATE_GRID_TETA	PI4096/6

static position_t local_pos;
static bool_e flag_update_position = FALSE;

void TOOLKIT_process_main(void)
{
	if(	(absolute(local_pos.x - global.pos.x) + absolute((local_pos.y - global.pos.y)) > UPDATE_GRID_XY
		||	absolute(local_pos.teta - global.pos.teta) > UPDATE_GRID_TETA)
		||	flag_update_position
	  )
		{
			flag_update_position = FALSE;
			local_pos = global.pos;
			TOOLKIT_process_send();
		}
}


void TOOLKIT_set_flag_update_position(void)
{
	flag_update_position = TRUE;
}


static void TOOLKIT_process_send(void)
{
	CAN_msg_t msg;
	msg.sid = BROADCAST_POSITION_ROBOT;
	msg.size = SIZE_BROADCAST_POSITION_ROBOT;
	msg.data.broadcast_position_robot.x = global.pos.x;
	msg.data.broadcast_position_robot.y = global.pos.y;
	msg.data.broadcast_position_robot.angle = global.pos.teta;
	msg.data.broadcast_position_robot.reason = WARNING_NO;
	msg.data.broadcast_position_robot.error = NO_ERROR;
	msg.data.broadcast_position_robot.way = ANY_WAY;
	msg.data.broadcast_position_robot.in_rotation = 0;
	msg.data.broadcast_position_robot.in_translation = 0;
	msg.data.broadcast_position_robot.idTraj = 0;
	CANmsgToU1tx(&msg);
}
