/*
 * actions.h
 *
 *  Created on: 14 juin 2018
 *      Author: Nirgal
 */

#ifndef ACTIONS_H_
#define ACTIONS_H_

#include "QS/QS_all.h"

	typedef enum
	{
		AVOIDANCE_DISABLED = 0,
		AVOIDANCE_ENABLED,
		AVOIDANCE_FRONT,
		AVOIDANCE_FRONT_LEFT,
		AVOIDANCE_FRONT_RIGHT,
		AVOIDANCE_BACK,
		AVOIDANCE_BACK_LEFT,
		AVOIDANCE_BACK_RIGHT,
		AVOIDANCE_ENABLED_WITHOUT_WAIT	//mode d'évitement où on n'attend pas lors d'une rencontre
	} avoidance_strateasy_e;


 Uint8 try_go(Uint32 x, Uint32 y, Uint8 in_progress, Uint8 success, Uint8 fail, avoidance_strateasy_e avoidance, way_e way);

 Uint8 try_go_angle(Sint32 teta, Uint8 in_progress, Uint8 success, Uint8 fail);

 Uint8 try_actuator(actuator_e id, Sint32 position, Sint32 param, Uint8 in_progress, Uint8 success, Uint8 fail);

 Uint8 check_actuator(actuator_e id, Uint8 in_progress, Uint8 success, Uint8 fail);

 Uint8 try_subaction(error_e result, Uint8 in_progress_state, Uint8 success_state, Uint8 error_state);

 Uint8 try_wait(uint32_t duration_ms, Uint8 in_progress_state, Uint8 success_state);

#endif /* ACTIONS_H_ */
