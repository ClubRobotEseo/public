/*
 * atoms_floor.c
 *
 *  Created on: 1 nov. 2018
 *      Author: Admin
 */

#include "strats_2019.h"
#include "../QS/QS_stateMachineHelper.h"

error_e sub_mark_ground_atoms_on_floor(void)
{
	CREATE_MAE_WITH_VERBOSE(SM_ID_MARK_GROUND_ATOMS_ON_FLOOR,
				INIT,
				FAR_POINT,
				PUSH_RED_ATOM_IN_ZONE,
				EXTRACT_FROM_FIRST_DISPOSE,
				FAR_POINT_TO_TAKE_SECOND_ATOM,
				PUSH_SECOND_ATOM,
				EXTRACT_FROM_SECOND_DISPOSE,
				FAR_POINT_TO_TAKE_THIRD_ATOM,
				PUSH_THIRD_ATOM,
				EXTRACT_FROM_THIRD_DISPOSE,
				STEP6,
				STEP7,
				STEP8,
				DONE,
				IDLE,
				FAIL
				);

	error_e ret = IN_PROGRESS;
	switch(state)
	{
		case INIT:
			state = FAR_POINT;
			break;
		case FAR_POINT:
			state = try_go(390, COLOR_Y(700), state, PUSH_RED_ATOM_IN_ZONE, PUSH_RED_ATOM_IN_ZONE, AVOIDANCE_DISABLED, ANY_WAY);
			break;
		case PUSH_RED_ATOM_IN_ZONE:
			state = try_go(545, COLOR_Y(140), state, EXTRACT_FROM_FIRST_DISPOSE, EXTRACT_FROM_FIRST_DISPOSE, AVOIDANCE_DISABLED, BACKWARD);
			break;


		case EXTRACT_FROM_FIRST_DISPOSE:
			state = try_go(448, COLOR_Y(521), state, FAR_POINT_TO_TAKE_SECOND_ATOM, FAR_POINT_TO_TAKE_SECOND_ATOM, AVOIDANCE_DISABLED, ANY_WAY);
			break;
		case FAR_POINT_TO_TAKE_SECOND_ATOM:
			state = try_go(930, COLOR_Y(3000-2196), state, PUSH_SECOND_ATOM, PUSH_SECOND_ATOM, AVOIDANCE_DISABLED, ANY_WAY);
			break;
		case PUSH_SECOND_ATOM:
			state = try_go(594, COLOR_Y(3000-2749), state, EXTRACT_FROM_SECOND_DISPOSE, EXTRACT_FROM_SECOND_DISPOSE, AVOIDANCE_DISABLED, ANY_WAY);
			break;
		case EXTRACT_FROM_SECOND_DISPOSE:
			state = try_go(918, COLOR_Y(3000-2306), state, FAR_POINT_TO_TAKE_THIRD_ATOM, FAR_POINT_TO_TAKE_THIRD_ATOM, AVOIDANCE_DISABLED, ANY_WAY);
			break;
		case FAR_POINT_TO_TAKE_THIRD_ATOM:
			state = try_go(1518, COLOR_Y(3000-2328), state, PUSH_THIRD_ATOM, PUSH_THIRD_ATOM, AVOIDANCE_DISABLED, ANY_WAY);
			break;
		case PUSH_THIRD_ATOM:
			state = try_go(620, COLOR_Y(3000-2637), state, EXTRACT_FROM_THIRD_DISPOSE, EXTRACT_FROM_THIRD_DISPOSE, AVOIDANCE_DISABLED, ANY_WAY);
			break;
		case EXTRACT_FROM_THIRD_DISPOSE:
			state = try_go(715, COLOR_Y(3000-2604), state, IDLE, IDLE, AVOIDANCE_DISABLED, ANY_WAY);
			break;


	//	case STEP5:
	//		state = try_go(200, 200, state, STEP6, STEP6, AVOIDANCE_DISABLED, ANY_WAY);
	//		break;
		case STEP6:
			state = try_go_angle(-PI4096, state, STEP7, STEP7);
			break;
		case STEP7:
			state = try_go_angle(PI4096, state, STEP8, STEP8);
			break;
		case STEP8:
			//state = try_go_angle(0, state, STEP1, STEP1);
			break;
		case IDLE:

			ret = END_OK;
			break;
		default:
			break;
	}
	return ret;
}

