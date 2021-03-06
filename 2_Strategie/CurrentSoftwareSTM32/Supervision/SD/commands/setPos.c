/*
 * setPos.c
 *
 *  Created on: 2 avril 2014
 *      Author: Anthony
 */
#include "setPos.h"
#include <errno.h>
#include "../term_commands_utils.h"
#include "../../../QS/QS_all.h"
#include "../../../QS/QS_CANmsgList.h"
#include "../../../QS/QS_lowLayer/QS_can.h"

const char term_cmd_setPos_brief[] = "Redefinis la position du robot via un message CAN � la carte propulsion";
const char term_cmd_setPos_help[] =
		"Utilisation : setPosition\n"
		"\n"
		"x doit �tre compris entre 200 et 1800\n"
		"y doit �tre compris entre 200 et 2800\n"
		"teta doit �tre compris entre 0 et 2*PI4096\n";


int term_cmd_setPos(int argc, const char *argv[]) {
	long x,y,teta;

	if(argc != 3)
		return EINVAL;

	if(!argtolong(argv[0], 0, &x))
		return EINVAL;

	if(!argtolong(argv[1], 0, &y))
		return EINVAL;

	if(!argtolong(argv[2], 0, &teta))
		return EINVAL;

	if(x >= 200 && x <= 1800 && y >= 200 && y <= 2800)
	{
		CAN_msg_t msg;
		msg.sid = PROP_SET_POSITION;
		msg.size = SIZE_PROP_SET_POSITION;
		msg.data.prop_set_position.x = x;
		msg.data.prop_set_position.y = x;
		msg.data.prop_set_position.teta = teta;
		CAN_send(&msg);
	}

	return 0;
}




