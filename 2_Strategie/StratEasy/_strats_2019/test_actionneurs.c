#include "strats_2019.h"
#include "../strategy.h"
#include "../QS/QS_stateMachineHelper.h"
#include "../actuators.h"

error_e testActuators2019(void)
{
	CREATE_MAE_WITH_VERBOSE(SM_ID_TEST_ACTUATORS,
				INIT,
				MOVE_RIGHT_UP,
				MOVE_RIGHT_DOWN,
				MOVE_LEFT_UP,
				MOVE_LEFT_DOWN,
				MOVE_BOTH,
				DECREASE_SPEED,
				DONE,
				FAIL
				);
	error_e ret = IN_PROGRESS;
	static Uint8 nb_moves = 0;
	static time32_t local_time;

	switch(state)
	{
		case INIT:
			if(nb_moves >= 2) {
				state = DONE;
			} else {
				state = MOVE_RIGHT_UP;
			}
			nb_moves++;
			break;
		case MOVE_RIGHT_UP:
			state = try_actuator(ACTUATOR_ARM_RIGHT, ACT_STRATEASY_ARM_UP, 0, state, MOVE_RIGHT_DOWN, FAIL);
			break;
		case MOVE_RIGHT_DOWN:
			state = try_actuator(ACTUATOR_ARM_RIGHT, ACT_STRATEASY_ARM_DOWN, 0, state, MOVE_LEFT_UP, FAIL);
			break;
		case MOVE_LEFT_UP:
			state = try_actuator(ACTUATOR_ARM_LEFT, ACT_STRATEASY_ARM_UP, 0, state, MOVE_LEFT_DOWN, FAIL);
			break;
		case MOVE_LEFT_DOWN:
			state = try_actuator(ACTUATOR_ARM_LEFT, ACT_STRATEASY_ARM_DOWN, 0, state, MOVE_BOTH, FAIL);
			break;
		case MOVE_BOTH:
			if(entrance) {
				local_time = global.absolute_time;
				ACTUATOR_go(ACTUATOR_ARM_RIGHT, ACT_STRATEASY_ARM_MID, 0);
				ACTUATOR_go(ACTUATOR_ARM_LEFT, ACT_STRATEASY_ARM_MID, 0);
			}
			if(global.absolute_time > local_time + 1000) {
				state = DECREASE_SPEED;
			}
			break;
		case DECREASE_SPEED:
			ACTUATOR_set_config(ACTUATOR_ARM_RIGHT, SPEED_CONFIG, 30);
			ACTUATOR_set_config(ACTUATOR_ARM_LEFT, SPEED_CONFIG, 30);
			state = INIT;
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
