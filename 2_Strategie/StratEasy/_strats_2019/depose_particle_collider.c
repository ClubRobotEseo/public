#include "strats_2019.h"
#include "../strategy.h"
#include "../QS/QS_stateMachineHelper.h"
#include "../actuators.h"

error_e sub_depose_particle_collider(void){
	CREATE_MAE_WITH_VERBOSE(SM_ID_TEST_ACTUATORS,
				INIT,
				TURN,
				FORWARD,
				TURN2,
				FORWARDTOACCELERATOR,
				CALAGE,
				REVERSE1,
				MOVEARMS,
				ORIENTATION,
				FORWARD2,
				ARMSDOWN,
				PUMPOFF,
				WAIT,
				VIDEPUMP,
				LIFTARMSTOLEAVE,
				REVERSE2,
				BACKARMSTOLEAVE,
				GOHOME,
				DONE,
				FAIL
				);
	error_e ret = IN_PROGRESS;
	static Uint8 nb_moves = 0;
	static time32_t local_time;
	static enum state_e state1, state2,state3,state4;
	switch(state)
	{
		case INIT:
			state=TURN;
			break;
		case TURN:
			state =try_go_angle(PI4096*0, state,MOVEARMS ,FAIL);
				break;
		case MOVEARMS:
							//CHANGER L'ANGLE D'ASSERVISSEMENT
							state =try_actuator(ACTUATOR_ARM_RIGHT,ACT_STRATEASY_ARM_ACCELERATOR_UP, 0,state, ORIENTATION, FAIL);
							break;
		case ORIENTATION :
			DOUBLE_ACTIONS(
									try_actuator(ACTUATOR_ARM_RIGHT,ACT_STRATEASY_ARM_ORIENTATION, 0,  IN_PROGRESS, END_OK, NOT_HANDLED),
									try_actuator(ACTUATOR_ARM_LEFT,ACT_STRATEASY_ARM_ACCELERATOR_UP, 0, IN_PROGRESS, END_OK, NOT_HANDLED),
									FORWARD,
									FAIL);
			break;
		case FORWARD:
			state = try_go(500, COLOR_Y(2000), state,TURN2, FAIL, AVOIDANCE_DISABLED,BACKWARD);
			break;
			break;
		case TURN2:
			state =try_go_angle(-PI4096, state,FORWARDTOACCELERATOR ,FAIL);
			break;

		case FORWARDTOACCELERATOR:
			state = try_go(180, COLOR_Y(2000), state,CALAGE, FAIL, AVOIDANCE_DISABLED,BACKWARD);
					break;
		case CALAGE:
			if(entrance)
						PROP_set_max_speed(MAX_SPEED/64);
					state = try_go(135, COLOR_Y(2000), state,REVERSE1, FAIL, AVOIDANCE_DISABLED, ANY_WAY);
					if (ON_LEAVE()){
						PROP_set_max_speed(0);	//Retour à la vitesse par défaut !
					ODOMETRY_set_position(140, global.pos.y,-PI4096);
					}//state = try_go(1992, COLOR_Y(220), state,REVERSE2, FAIL, AVOIDANCE_DISABLED, ANY_WAY);
					break;
		case REVERSE1:
					state=try_go(200, COLOR_Y(2000), state,FORWARD2, FAIL, AVOIDANCE_DISABLED, ANY_WAY);;
					break;

		case FORWARD2:
					state=try_go(150, COLOR_Y(2000), state,ARMSDOWN, FAIL, AVOIDANCE_DISABLED, ANY_WAY);;
					break;
		/*case PUMPOFF:
			if(entrance)
						{
							state1=IN_PROGRESS;
							state2=IN_PROGRESS;
						}
						if(state1==IN_PROGRESS)
							state1 = try_actuator(ACTUATOR_PUMP_LEFT, 1, 0, IN_PROGRESS, END_OK, NOT_HANDLED);
						if(state2==IN_PROGRESS)
							state2 = try_actuator(ACTUATOR_PUMP_RIGHT, 1, 0, IN_PROGRESS, END_OK, NOT_HANDLED);
						if(state1!=IN_PROGRESS && state2!=IN_PROGRESS)
							state = (state1!=END_OK || state2!=END_OK)?REVERSE2:FAIL;
						break;
*/
		case ARMSDOWN:
			//CHANGER L'ANGLE D'ASSERVISSEMENT
			DOUBLE_ACTIONS(
			try_actuator(ACTUATOR_ARM_RIGHT,ACT_STRATEASY_ARM_ACCELERATOR_DOWN, 0, IN_PROGRESS, END_OK, NOT_HANDLED),
			try_actuator(ACTUATOR_ARM_LEFT,ACT_STRATEASY_ARM_ACCELERATOR_DOWN, 0, IN_PROGRESS, END_OK, NOT_HANDLED),
			PUMPOFF,
			FAIL
			);
					break;
		case PUMPOFF:
			DOUBLE_ACTIONS(
			try_actuator(ACTUATOR_PUMP_LEFT, 0, 0,  IN_PROGRESS, END_OK, NOT_HANDLED),
			try_actuator(ACTUATOR_PUMP_RIGHT, 0, 0,  IN_PROGRESS, END_OK, NOT_HANDLED),
			VIDEPUMP,
			DONE
			);
			break;


		case VIDEPUMP:
			DOUBLE_ACTIONS(
			try_actuator(ACTUATOR_SOLENOID_VALVE_LEFT, 1, 0,  IN_PROGRESS, END_OK, NOT_HANDLED),
			try_actuator(ACTUATOR_SOLENOID_VALVE_RIGHT, 1, 0,  IN_PROGRESS, END_OK, NOT_HANDLED),
			WAIT,
			DONE
			);
			break;

		case WAIT:
			state = try_wait(1000, state,LIFTARMSTOLEAVE);
			break;

		case LIFTARMSTOLEAVE:
			DOUBLE_ACTIONS(
						try_actuator(ACTUATOR_ARM_RIGHT,ACT_STRATEASY_ARM_ACCELERATOR_UP, 0, IN_PROGRESS, END_OK, NOT_HANDLED),
						try_actuator(ACTUATOR_ARM_LEFT,ACT_STRATEASY_ARM_ACCELERATOR_UP, 0, IN_PROGRESS, END_OK, NOT_HANDLED),
						REVERSE2,
						DONE
						);
						break;
		case REVERSE2:
					state=try_go(200, COLOR_Y(2000), state,GOHOME, FAIL, AVOIDANCE_DISABLED, ANY_WAY);
					break;
		case BACKARMSTOLEAVE:
					DOUBLE_ACTIONS(
								try_actuator(ACTUATOR_ARM_RIGHT,ACT_STRATEASY_ARM_INIT, 0, IN_PROGRESS, END_OK, NOT_HANDLED),
								try_actuator(ACTUATOR_ARM_LEFT,ACT_STRATEASY_ARM_INIT, 0, IN_PROGRESS, END_OK, NOT_HANDLED),
								GOHOME,
								DONE
								);
								break;
		case GOHOME:
					state=try_go(750, COLOR_Y(225), state,DONE, FAIL, AVOIDANCE_DISABLED, ANY_WAY);
					break;

	/*	case TURNTOFACEDISPENSER:
				state = try_go_angle(PI4096*1, state,FORWARDTODISPENSER ,FAIL);
				//state=FORWARDTODISPENSER;
					break;

		*/
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
