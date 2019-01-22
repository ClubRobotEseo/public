/*
 *	Club Robot ESEO 2010 - 2011
 *	Check Norris
 *
 *	Fichier : Can_msg_processing.c
 *	Package : Carte actionneur
 *	Description : Fonctions de traitement des messages CAN
 *  Auteur : Aurélien
 *  Version 20110225
 */


#include "Can_msg_processing.h"
#include "QS/QS_actuator/QS_DCMotor2.h"
#include "QS/QS_lowLayer/QS_can.h"
#include "QS/QS_can_verbose.h"
#include "QS/QS_watchdog.h"
#include "QS/QS_IHM.h"
#include "QS/QS_simulator/QS_simulator.h"
#include "queue.h"
#include "ActManager.h"

#define LOG_PREFIX "CANProcess: "
#define LOG_COMPONENT OUTPUT_LOG_COMPONENT_CANPROCESSMSG
#include "QS/QS_outputlog.h"
#include "QS/QS_who_am_i.h"

static void CAN_send_callback(CAN_msg_t* msg);

void CAN_process_init(){
	CAN_init();
	CAN_set_send_callback(CAN_send_callback);
}

void CAN_process_msg(CAN_msg_t* msg) {
	CAN_msg_t answer;
	// Les messages de broadcast ne doivent pas passer dans le if qui suit
	if((msg->sid & 0xF00) == ACT_FILTER && ACTMGR_process_msg(msg)) {
		component_printf(LOG_LEVEL_Debug, "Act Msg SID: 0x%03x, cmd: 0x%x(%u), size: %d\n", msg->sid, msg->data.act_order.order, msg->data.act_order.order, msg->size);
		return;  //Le message a déja été géré
	}


	// Traitement des autres messages reçus
	switch (msg->sid)
	{
		case ACT_BOOST_ASSER:
			if(I_AM_BIG()) {
				#ifdef BOOST_ASSER_PORT
							if(msg->data.act_boost_asser.enable)
								GPIO_SetBits(BOOST_ASSER_PORT);
							else
								GPIO_ResetBits(BOOST_ASSER_PORT);
				#endif
			}
			break;

		case BROADCAST_RESET:
			NVIC_SystemReset();
			break;

		//Fin de la partie
		case BROADCAST_STOP_ALL :{
			global.flags.match_started = FALSE;
			global.flags.match_over = TRUE;
			#ifdef USE_DCMOTOR2
				DCM_stop_all();
			#endif
			QUEUE_flush_all(); // Vidage des queues
			ACTMGR_stop(); // Stopper l'asservissement des actionneurs
			}break;

		//Reprise de la partie
		case BROADCAST_START :
			global.flags.match_started = TRUE;
			break;

		case BROADCAST_POSITION_ROBOT:
				global.pos.x = msg->data.broadcast_position_robot.x;
				global.pos.y = msg->data.broadcast_position_robot.y;
				global.pos.angle = msg->data.broadcast_position_robot.angle;
				global.pos.updated = TRUE;
			break;

		case BROADCAST_BEACON_ADVERSARY_POSITION_IR:
			break;

		case BROADCAST_ADVERSARIES_POSITION:
			break;

		case BROADCAST_ALIM:
			if(!global.flags.match_started || global.flags.match_over) {
					if(msg->data.broadcast_alim.state & (BATTERY_DISABLE | ARU_ENABLE)){
						global.flags.power = FALSE;
					}else if(msg->data.broadcast_alim.state & ARU_DISABLE){
						global.flags.power = TRUE;

						// Lister ici les choses à faire quand la puissance est de retour
						#ifdef USE_DCMOTOR2
							DCM_reset_integrator();
						#endif
						if (ACTMRG_is_first_init_pos_done()) {
							ACTMGR_reset_config();
						}
					}

				if(global.flags.power){

				}

				global.alim_value = msg->data.broadcast_alim.battery_value;
			}
			break;

		case ACT_PING:
			answer.sid = STRAT_ACT_PONG;
			answer.size = SIZE_STRAT_ACT_PONG;
			if(I_AM_BIG()) {
				answer.data.strat_act_pong.robot_id = BIG_ROBOT;
			} else {
				answer.data.strat_act_pong.robot_id = SMALL_ROBOT;
			}
			CAN_send(&answer);
			break;

		case ACT_GET_CONFIG:
			ACTMGR_get_config(msg);
			break;

		case ACT_SET_CONFIG:
			ACTMGR_set_config(msg);
			break;

		case ACT_WARNER:
			ACTMGR_set_warner(msg);
			break;

		case ACT_ASK_SENSOR:{
			bool_e found = TRUE;
			answer.sid = STRAT_INFORM_CAPTEUR;
			answer.size = SIZE_STRAT_INFORM_CAPTEUR;
			answer.data.strat_inform_capteur.sensor_id = msg->data.act_ask_sensor.act_sensor_id;
			switch(msg->data.act_ask_sensor.act_sensor_id){

				default:
					found = FALSE;
					break;
			}
			if(found)
				CAN_send(&answer);
		}break;

		case IHM_SWITCH_ALL:
		case IHM_BUTTON:
		case IHM_SWITCH:
			IHM_process_main(msg);
			break;

		default:
			component_printf(LOG_LEVEL_Trace, "Msg SID: 0x%03x(%u)\n", msg->sid, msg->sid);
			break;
	}//End switch

	if(msg->sid & DEBUG_FILTER)
		SIMULATOR_processMain(msg, FALSE);
}

static void CAN_send_callback(CAN_msg_t* msg){
	UNUSED_VAR(msg);
	#ifdef CAN_VERBOSE_MODE
		QS_CAN_VERBOSE_can_msg_print(msg, VERB_OUTPUT_MSG);
	#endif
}
