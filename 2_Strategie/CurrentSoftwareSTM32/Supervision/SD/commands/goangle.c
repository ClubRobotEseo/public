/*
 * goangle.c
 *
 *  Created on: 12 oct. 2013
 *      Author: Arnaud
 */

#include "goangle.h"
#include <errno.h>
#include "../term_commands_utils.h"
#include "../../../propulsion/prop_functions.h"
#include "../../../QS/QS_all.h"

const char term_cmd_goangle_brief[] = "Tourner le robot � l'angle voulu";
const char term_cmd_goangle_help[] =
		"Utilisation : goangle teta\n"
		"\n"
		"teta       angle en degres\n";


int term_cmd_goangle(int argc, const char *argv[]) {
	 Sint32 teta;

	if(argc < 1)
		return EINVAL;
	else if(argc > 1)
		return EINVAL;

	if(!argtolong(argv[0], 0, &teta))
		return EINVAL;

	if(global.pos.x < 200 || global.pos.x > 1800)
		return EINVAL;

	if(global.pos.y < 200 || global.pos.y > 2800)
			return EINVAL;

	if(teta > 180){
		teta %= 180;
		teta = teta - 180;
	}else if(teta < -180){
		teta %= 180;
		teta = teta + 180;
	}

	teta *= PI4096;
	teta /= 180;

	PROP_goAngle(teta, PROP_ABSOLUTE, FAST, ANY_WAY, FALSE, END_AT_LAST_POINT, 0);

	return 0;
}



