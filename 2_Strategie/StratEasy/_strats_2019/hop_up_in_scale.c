#include "strats_2019.h"
#include "../strategy.h"
#include "../QS/QS_stateMachineHelper.h"
#include "../actuators.h"


error_e sub_hop_up_in_scale(void){
	CREATE_MAE_WITH_VERBOSE(SM_ID_TEST_ACTUATORS,
				INIT,
				MOVE_TO_CHAOS,
				PICK_UP_CHAOS,
					FORWARD_TROUGH_CHAOS,
				TURN_REV,
				FORWARD_TO_SCALE,
				REVERSE_FROM_SCALE,
				FORWARD_TO_SCALE2nd,
				LIFT_SERVO,
				PUMPOFF,
				VIDEPUMP,
				REVERSE_FROM_SCALE2nd,
				GOCHAOS,
				GOHOME,
				DONE,
				FAIL
				);
	error_e ret = IN_PROGRESS;
	static Uint8 nb_moves = 0;
	static time32_t local_time;
	static enum state_e state1, state2,state3,state4;
	int palet1,palet2,palet3,palet4;
	palet1=0;
	palet2=0;
	palet3=0;
	palet4=0;
	switch(state)
	{
		case INIT:
			state=MOVE_TO_CHAOS;
			break;
		case MOVE_TO_CHAOS:
			if (palet1==0 | palet2==0 | palet3==0 | palet4==0){
				state =PICK_UP_CHAOS;
				break;
			}
			else
				state = FORWARD_TO_SCALE;
				break;
			break;

		case PICK_UP_CHAOS:
			state = try_go(1050, COLOR_Y(900), state,FORWARD_TROUGH_CHAOS, FAIL, AVOIDANCE_DISABLED, ANY_WAY);
			break;
		case FORWARD_TROUGH_CHAOS:
					state = try_go(1050, COLOR_Y(1300), state,TURN_REV, FAIL, AVOIDANCE_DISABLED, ANY_WAY);
					break;
		case TURN_REV:
				state = try_go_angle(-PI4096/2, state,FORWARD_TO_SCALE ,FAIL);
				break;
		case FORWARD_TO_SCALE:
					state = try_go(1530, COLOR_Y(1300), state,REVERSE_FROM_SCALE, FAIL, AVOIDANCE_DISABLED, BACKWARD);
					break;
		case REVERSE_FROM_SCALE:
					state = try_go(1480, COLOR_Y(1300), state,LIFT_SERVO, FAIL, AVOIDANCE_DISABLED,ANY_WAY);
					break;

		case LIFT_SERVO:
			//Augmenter l'angle délivré pour faire passer le palet au-dessus de la balance
			state =FORWARD_TO_SCALE2nd ;
			break;
		case FORWARD_TO_SCALE2nd:
			state = try_go(1550, COLOR_Y(1300), state, PUMPOFF, FAIL, AVOIDANCE_DISABLED, BACKWARD);
			break;

		case PUMPOFF:
				DOUBLE_ACTIONS(
				state=try_actuator(ACTUATOR_PUMP_LEFT, 0, 1, IN_PROGRESS, END_OK, NOT_HANDLED),
				state=try_actuator(ACTUATOR_PUMP_RIGHT, 0, 1, IN_PROGRESS, END_OK, NOT_HANDLED),
				VIDEPUMP,
				DONE)
				break;

		case VIDEPUMP:
				DOUBLE_ACTIONS(
					try_actuator(ACTUATOR_SOLENOID_VALVE_LEFT, 1, 0, IN_PROGRESS, END_OK, NOT_HANDLED),
					try_actuator(ACTUATOR_SOLENOID_VALVE_RIGHT, 1, 0, IN_PROGRESS, END_OK, NOT_HANDLED),
					 REVERSE_FROM_SCALE2nd,
					DONE)
					 break;
		case REVERSE_FROM_SCALE2nd:
		state = try_go(1480, COLOR_Y(1300), state,GOCHAOS, FAIL, AVOIDANCE_DISABLED, ANY_WAY);
		break;

		case GOCHAOS:
					state = try_go(1030, COLOR_Y(950), state,GOHOME, FAIL, AVOIDANCE_DISABLED, ANY_WAY);
					break;
		case GOHOME:
			state = try_go(300, COLOR_Y(150), state,DONE, FAIL, AVOIDANCE_DISABLED, ANY_WAY);
			break;
		case DONE:
			ret = END_OK;

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
