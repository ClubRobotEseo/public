/*
 * strategy.c
 *
 *  Created on: 14 juin 2018
 *      Author: Nirgal
 */
#include "strategy.h"
#include "prop.h"
#include "QS/QS_stateMachineHelper.h"
#include "QS/QS_lowLayer/QS_can.h"
#include "QS/QS_can_over_uart.h"
#include "actions.h"
#include "odometry.h"
#include "stepper_motors.h"
#include "actuators.h"
#include "_strats_2019/strats_2019.h"

volatile static bool_e ask_for_broadcast_start = FALSE;
void color_update(void);

void sample_bank(void)
{
	return;	//cette fonction ne doit jamais être appelée, on y pioche juste des lignes en copier-coller
	Uint8 state, SUCCESS_STATE, FAIL_STATE;

	//Allumer une pompe :
	state = try_actuator(ACTUATOR_PUMP_RIGHT, 1, 0, state, SUCCESS_STATE, FAIL_STATE);

	//Eteindre une pompe (attention, il faut deux états successifs)
	state = try_actuator(ACTUATOR_SOLENOID_VALVE_RIGHT, 1, 0, state, SUCCESS_STATE, FAIL_STATE);
	state = try_actuator(ACTUATOR_PUMP_RIGHT, 0, 0, state, SUCCESS_STATE, FAIL_STATE);

	//Bouger un servomoteur (envoi de l'ordre et attente de la réponse)
	state = try_actuator(ACTUATOR_ARM_RIGHT, ACT_STRATEASY_ARM_INIT, 0, state, SUCCESS_STATE, FAIL_STATE);
	//Bouger un servomoteur (envoi de l'ordre sans attente de la réponse)
	ACTUATOR_go(ACTUATOR_ARM_RIGHT, ACT_STRATEASY_ARM_INIT, 0);

	//Changer à la volée la configuration (vitesse ou couple) d'un servomoteur
	ACTUATOR_set_config(ACTUATOR_ARM_RIGHT, SPEED_CONFIG, 50);
	ACTUATOR_set_config(ACTUATOR_ARM_RIGHT, TORQUE_CONFIG, 50);

	//Récupérer la configuration d'un servomoteur
	Uint16 pos = ACTUATOR_get_config(ACTUATOR_ARM_RIGHT, POSITION_CONFIG);
	Uint16 speed = ACTUATOR_get_config(ACTUATOR_ARM_RIGHT, SPEED_CONFIG);
	Uint16 torque = ACTUATOR_get_config(ACTUATOR_ARM_RIGHT, TORQUE_CONFIG);
	Uint16 temperature = ACTUATOR_get_config(ACTUATOR_ARM_RIGHT, TEMPERATURE_CONFIG);

	//Déplacement en translation vers x;y
	state = try_go(600, COLOR_Y(650), state, SUCCESS_STATE, FAIL_STATE, AVOIDANCE_DISABLED, ANY_WAY);

	//Déplacement en rotation vers teta
	state = try_go_angle(-PI4096, state, SUCCESS_STATE, FAIL_STATE);

	//Délai d'attente
	state = try_wait(1000, state, SUCCESS_STATE);

	//Appel d'une subaction
	state = try_subaction(sub_mark_ground_atoms_on_floor(), state, SUCCESS_STATE, FAIL_STATE);

	//Forçage de la position (en cas de calage...)
	ODOMETRY_set_position(600, COLOR_Y(BACKWARD_DISTANCE), COLOR_ANGLE(PI4096/2));

	// Liste des variables non utilisés (pour éviter les warnings)
	UNUSED_VAR(pos);
	UNUSED_VAR(speed);
	UNUSED_VAR(torque);
	UNUSED_VAR(temperature);
}


void strategy(void)
{
	CREATE_MAE_WITH_VERBOSE(SM_ID_STRATEGY,
			INIT,
			WAIT_BIROUTE_INSERTED,
			WAIT_BIROUTE_REMOVED,
			CALIBRATE,
			LAUNCH_MATCH,
			GET_OUT_FROM_START_ZONE,
			RUN_MATCH,
			END_OF_MATCH,
			MATCH_ENDED,
			IDLE
			);
	switch(state)
	{
		case INIT:
			state = WAIT_BIROUTE_INSERTED;
			ACTUATOR_go(ACTUATOR_PUMP_RIGHT, 0, 0);
			ACTUATOR_go(ACTUATOR_PUMP_LEFT, 0, 0);
			break;
		case WAIT_BIROUTE_INSERTED:
			//if(GPIO_ReadInputDataBit(MY_BIROUTE))
				state = WAIT_BIROUTE_REMOVED;

			color_update();
			break;
		case WAIT_BIROUTE_REMOVED:
			color_update();

			if(BUTTON0_PORT || ask_for_broadcast_start)		// ajouter :    || !GPIO_ReadInputDataBit(MY_BIROUTE)
			{
				global.flags.match_started = TRUE;
				printf("let's go !!!\n");
				state = LAUNCH_MATCH;
			}
			else
			{
				if(!BUTTON_PUSH_3_PORT && !global.flags.calibrated)
					state = CALIBRATE;
			}
			break;
		case CALIBRATE:
			state = try_go(global.pos.x, COLOR_Y(300), state, WAIT_BIROUTE_REMOVED, WAIT_BIROUTE_REMOVED, AVOIDANCE_DISABLED, ANY_WAY);
			if(ON_LEAVE())
				global.flags.calibrated = TRUE;
			break;
		case LAUNCH_MATCH:

			//state = PUMP_ON;
			state = GET_OUT_FROM_START_ZONE;
			break;
		case GET_OUT_FROM_START_ZONE:
			state = try_go(600, COLOR_Y(650), state, RUN_MATCH, RUN_MATCH, AVOIDANCE_DISABLED, ANY_WAY);
			//state = RUN_MATCH;
			break;
		case RUN_MATCH:
			if(global.flags.match_over)
				state = END_OF_MATCH;
			else
				state = try_subaction(strat2019(), state, IDLE, IDLE);
			break;
		case IDLE:
			if(global.flags.match_over)
				state = END_OF_MATCH;
			break;
		case END_OF_MATCH:
			printf("Fin du match - désactivation des actionneurs\n");
			ACTUATOR_stop();
			CAN_send_sid(BROADCAST_STOP_ALL);
			state = MATCH_ENDED;
			break;
		case MATCH_ENDED:
			//fini...
			break;

		default:
			break;
	}

}


void color_update(void)
{
	static bool_e initialized = FALSE;
	color_e color;
	color = (GPIO_ReadInputDataBit(MY_SWITCH_COLOR))?TOP_COLOR:BOT_COLOR;

	if(!initialized || global.color != color)
	{
		global.color = color;

		initialized = TRUE;
		if(color == BOT_COLOR)
		{
			//Jaune
			GPIO_WriteBit(LED_COLOR_BLUE, 1);	//blue off
			GPIO_WriteBit(LED_COLOR_RED, 0);	//red on
			GPIO_WriteBit(LED_COLOR_GREEN, 0);	//green on
		}
		else
		{
			//Violet
			GPIO_WriteBit(LED_COLOR_BLUE, 0);	//blue on
			GPIO_WriteBit(LED_COLOR_RED, 0);	//red on
			GPIO_WriteBit(LED_COLOR_GREEN, 1);	//green off
		}
		CAN_msg_t msg;
		msg.sid=BROADCAST_COULEUR;
		msg.size = 1;
		msg.data.broadcast_couleur.color = color;
		CANmsgToU1tx(&msg);
		ODOMETRY_set_position(600, COLOR_Y(BACKWARD_DISTANCE), COLOR_ANGLE(PI4096/2));
	}
}

void STRATEGY_set_ask_for_broadcast_start(void)
{
	ask_for_broadcast_start = TRUE;
}
