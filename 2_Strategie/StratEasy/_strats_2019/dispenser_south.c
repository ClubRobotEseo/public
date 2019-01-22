#include "strats_2019.h"
#include "../strategy.h"
#include "../QS/QS_stateMachineHelper.h"
#include "../actuators.h"

//

error_e sub_dispenser_south(void)
{
	CREATE_MAE_WITH_VERBOSE(SM_ID_TEST_ACTUATORS,
				INIT,
				FORWARD, //FORWARD
				TURNTOFACEDISPENSER, //BACKWARD
				FORWARDTODISPENSER,
				REVERSE1,
				UPUPARM,
				LIFTARMS,
				ARMUP,
				PUMPON,
				FORWARDTOPALETCALAGE,
				FORWARD2,
				REVERSE2,
				PUSHBACKFROMCOAST,
				DONE,
				FAIL
				);
	error_e ret = IN_PROGRESS;
	static Uint8 nb_moves = 0;
	static time32_t local_time;

	switch(state)
	{
		case INIT:
			state=FORWARD;
			//A vérifier si on se situe près d'une bordure ou non pour la rotation à effectuer

			break;
		case FORWARD:
			state = try_go(450, COLOR_Y(220), state,TURNTOFACEDISPENSER , FAIL, AVOIDANCE_DISABLED, ANY_WAY);
			break;
		case TURNTOFACEDISPENSER:
			state = try_go_angle(PI4096, state,UPUPARM ,FAIL);
			break;

		case UPUPARM:
			DOUBLE_ACTIONS(
								try_actuator(ACTUATOR_ARM_RIGHT,ACT_STRATEASY_ARM_ACCELERATOR_UP, 0, IN_PROGRESS, END_OK, NOT_HANDLED),
								try_actuator(ACTUATOR_ARM_LEFT,ACT_STRATEASY_ARM_ACCELERATOR_UP, 0, IN_PROGRESS, END_OK, NOT_HANDLED),
								//Angle délivrés par les servomoteurs à assigner
								FORWARDTODISPENSER,
								FAIL
								);
			break;
		case FORWARDTODISPENSER:
			state = try_go(1870, COLOR_Y(220), state,FORWARDTOPALETCALAGE, FAIL, AVOIDANCE_DISABLED, ANY_WAY);
			break;
		case REVERSE1:
			state = try_go(1750, COLOR_Y(220), state,LIFTARMS, FAIL, AVOIDANCE_DISABLED, ANY_WAY);
			break;
		case LIFTARMS:
			DOUBLE_ACTIONS(
					try_actuator(ACTUATOR_ARM_RIGHT,ACT_STRATEASY_ARM_DISTRI_SOUTH, 0, IN_PROGRESS, END_OK, NOT_HANDLED),
					try_actuator(ACTUATOR_ARM_LEFT,ACT_STRATEASY_ARM_DISTRI_SOUTH, 0, IN_PROGRESS, END_OK, NOT_HANDLED),
					//Angle délivrés par les servomoteurs à assigner
					PUMPON,
					FAIL
					);
			break;

		case FORWARD2:
			//set speed slow
			state =  try_go(1905, COLOR_Y(220), state,REVERSE2, FAIL, AVOIDANCE_DISABLED, ANY_WAY);
			break;

		/*case ARMUP:
			/*{

		static error_e state1, state2;
			if(entrance)
			{
				state1 = IN_PROGRESS;
				state2 = IN_PROGRESS;
			}
			if(state1 == IN_PROGRESS)
				state1 = try_actuator(ACTUATOR_ARM_RIGHT, ACT_STRATEASY_ARM_UP, 0, state, PUMPON, FAIL);
			if(state2 == IN_PROGRESS)
				state2 = try_actuator(ACTUATOR_ARM_LEFT, ACT_STRATEASY_ARM_UP, 0, state, PUMPON, FAIL);

			if(state1 != IN_PROGRESS && state2 != IN_PROGRESS)
			{
				if(state1 == END_OK && state2 == END_OK)
					state = PUMPON;
				else
					state = ERROR;
			}

			break;}

			DOUBLE_ACTIONS(
					try_actuator(ACTUATOR_ARM_RIGHT, ACT_STRATEASY_ARM_UP, 0, IN_PROGRESS, END_OK, NOT_HANDLED),
					try_actuator(ACTUATOR_ARM_LEFT, ACT_STRATEASY_ARM_UP, 0, IN_PROGRESS, END_OK, NOT_HANDLED),
					//Angle délivrés par les servomoteurs à assigner
					PUMPON,
					FAIL
					);
			break;*/

		case PUMPON:
			DOUBLE_ACTIONS(
				try_actuator(ACTUATOR_PUMP_RIGHT, 1, 0, IN_PROGRESS, END_OK, NOT_HANDLED),
				try_actuator(ACTUATOR_PUMP_LEFT, 1, 0, IN_PROGRESS, END_OK, NOT_HANDLED),
				FORWARD2,
				FAIL
				);

			break;
		case FORWARDTOPALETCALAGE:
			//set speed slow
			if(entrance)
				PROP_set_max_speed(MAX_SPEED/64);
			state = try_go(1910, COLOR_Y(220), state,REVERSE1, FAIL, AVOIDANCE_DISABLED, ANY_WAY);
			if (ON_LEAVE()){
				PROP_set_max_speed(0);	//Retour à la vitesse par défaut !
			ODOMETRY_set_position(1899, global.pos.y, PI4096);
			}//state = try_go(1992, COLOR_Y(220), state,REVERSE2, FAIL, AVOIDANCE_DISABLED, ANY_WAY);
			break;
		case REVERSE2:
			state = try_go(1300, COLOR_Y(220), state,PUSHBACKFROMCOAST, FAIL, AVOIDANCE_DISABLED, ANY_WAY);
			break;
		case PUSHBACKFROMCOAST:
			state = try_go_angle(-1*PI4096, state,DONE,FAIL);
			break;
		case DONE:
			ret = END_OK;
			state = INIT;
			break;
		case FAIL:
			ret = NOT_HANDLED;
			state = INIT;
			break;
		default:
			break;
	}
	return ret;
}
