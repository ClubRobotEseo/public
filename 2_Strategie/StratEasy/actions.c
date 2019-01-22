/*
 * actions.c
 *
 *  Created on: 14 juin 2018
 *      Author: Nirgal
 */

#include "actions.h"
#include "actuators.h"
#include "prop.h"

#include "QS/QS_stateMachineHelper.h"
#include "QS/QS_simulator/QS_simulator.h"

#define WAIT_DURATION	3000

 Uint8 try_go(Uint32 x, Uint32 y, Uint8 in_progress, Uint8 success, Uint8 fail, avoidance_strateasy_e avoidance, way_e way)
 {
	 CREATE_MAE(
			 INIT,
			 RUNNING,
			 WAIT_AVOIDING,
			 DONE,
			 FAIL);
	 Uint8 ret;
	 ret = in_progress;
	 static Uint32 avoiding_duration;
	 switch(state)
	 {
	 	 case INIT:
	 		 avoiding_duration = 0;
	 		 if(SIMULATOR_is_pause_asked_by_toolkit())
	 		 {
	 			 //match en pause (demande de la toolkit !)
	 			 //NB: on ne fait les pauses qu'en tout début de trajectoire en prop.
	 			global.flags.match_suspended = TRUE;
	 		 }
	 		 else
	 		 {
	 			state = RUNNING;
	 			global.flags.match_suspended = FALSE;
	 		 }
	 		 break;
	 	 case RUNNING:
	 		 //TODO
	 		 if(PROP_go(x, y, way))
	 		 {
	 			 state = DONE;
	 		 }
	 		 else if(avoidance != AVOIDANCE_DISABLED)
	 		 {
	 			if(0)//SENSOR_is_adversary(way))
	 			{
	 				PROP_stop();
	 				state = (avoidance!=AVOIDANCE_ENABLED_WITHOUT_WAIT)?WAIT_AVOIDING:ERROR;
	 			}
	 		 }
	 		 break;
	 	 case WAIT_AVOIDING:{
	 		 Uint32 local_time;
	 		 if(entrance)
	 		 {
	 			 local_time = global.match_time;
	 		 }
	 		 if(0)//!SENSOR_is_adversary(way))
	 		 {
	 			avoiding_duration += global.match_time-local_time;
	 			 state = RUNNING;
	 		 }
	 		 if(global.match_time-local_time + avoiding_duration > WAIT_DURATION)
	 		 {
	 			 state = FAIL;
	 		 }

	 		 break;}
	 	 case DONE:
	 		 state = INIT;
	 		 ret = success;
	 		 break;
	 	 case FAIL:
	 		 state = INIT;
	 		 ret = fail;
	 		 break;
	 	 default:
	 		 break;
	 }
	 return ret;
 }


 Uint8 try_go_angle(Sint32 teta, Uint8 in_progress, Uint8 success, Uint8 fail)
 {
	 CREATE_MAE(
			 INIT,
			 RUNNING,
			 DONE,
			 FAIL);
	 Uint8 ret;
	 ret = in_progress;
	 switch(state)
	 {
	 	 case INIT:
	 		 if(SIMULATOR_is_pause_asked_by_toolkit())
			 {
				 //match en pause (demande de la toolkit !)
				 //NB: on ne fait les pauses qu'en tout début de trajectoire en prop.
				global.flags.match_suspended = TRUE;
			 }
			 else
			 {
				state = RUNNING;
				global.flags.match_suspended = FALSE;
			 }
	 		 break;
	 	case RUNNING:
	 		 //TODO
	 		 if(PROP_go_angle(teta))
	 		 {
	 			 state = DONE;
	 		 }

	 		 break;

	 	 case DONE:
	 		 state = INIT;
	 		 ret = success;
	 		 break;
	 	 case FAIL:
	 		 state = INIT;
	 		 ret = fail;
	 		 break;
	 	 default:
	 		 break;
	 }
	 return ret;
 }

 Uint8 try_actuator(actuator_e id, Sint32 position, Sint32 param, Uint8 in_progress, Uint8 success, Uint8 fail)
 {
	 CREATE_MAE(
			 INIT,
			 SEND_COMMAND,
			 RUNNING,
			 DONE,
			 FAIL);
	 Uint8 ret;
	 ret = in_progress;
	 switch(state)
	 {
	 	 case INIT:
	 		 state = SEND_COMMAND;
	 		 break;
	 	 case SEND_COMMAND:
	 		 ACTUATOR_go(id, position, param);
	 		 state = RUNNING;
	 		 break;
	 	 case RUNNING:
	 		 if(ACTUATOR_is_arrived(id))
	 		 {
	 			 state = DONE;
	 		 }
	 		 else if(ACTUATOR_is_in_timeout(id))
	 		 {
	 			 state = FAIL;
	 		 }
	 		 break;

	 	 case DONE:
	 		 state = INIT;
	 		 ret = success;
	 		 break;
	 	 case FAIL:
	 		 state = INIT;
	 		 ret = fail;
	 		 break;
	 	 default:
	 		 break;
	 }
	 return ret;
 }


 Uint8 check_actuator(actuator_e id, Uint8 in_progress, Uint8 success, Uint8 fail)
 {
	 if(ACTUATOR_is_arrived(id))
	 {
		 return success;
	 }
	 else if(ACTUATOR_is_in_timeout(id))
	 {
		 return fail;
	 }
	 return in_progress;
 }


 Uint8 try_subaction(error_e result, Uint8 in_progress_state, Uint8 success_state, Uint8 error_state)
 {
	 switch(result)
	 {
	 	 case IN_PROGRESS:
	 		 return in_progress_state;
	 		 break;
	 	 case END_OK:
	 		 return success_state;
	 		 break;
	 	 case NOT_HANDLED:
	 	 case END_WITH_TIMEOUT:
	 	 case FOE_IN_PATH:
	 	 default:
	 		 return error_state;
	 		 break;
	 }
	 return in_progress_state;
 }

 Uint8 try_wait(uint32_t duration_ms, Uint8 in_progress_state, Uint8 success_state)
 {
	 CREATE_MAE(
				 INIT,
				 WAIT);
	 Uint8 ret;
	 ret = in_progress_state;
	 static uint32_t local_time = 0;

	 switch(state)
	 {
	 	 case INIT:
	 		 local_time = global.absolute_time;
	 		 state = WAIT;
	 		 break;
	 	 case WAIT:
	 		 if(global.absolute_time >= local_time + duration_ms)
	 		 {
	 			 state = INIT;
	 			 ret = success_state;
	 		 }
	 		 break;
	 	 default:
	 		 break;
	 }
	 return ret;
 }


