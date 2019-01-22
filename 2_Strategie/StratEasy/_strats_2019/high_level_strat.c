/*
 * high_level_strat.c
 *
 *  Created on: 1 nov. 2018
 *      Author: Admin
 */

#include "strats_2019.h"
#include "../strategy.h"
#include "../QS/QS_stateMachineHelper.h"


error_e strat2019(void)
{
	CREATE_MAE_WITH_VERBOSE(SM_ID_HIGH_LEVEL_STRAT,
				INIT,
				//MARK_ATOMS_ON_FLOOR
				RUSH,
				DISPENSER_SOUTH,
				DEPOSE_PARTICLE_COLLIDER,
				HOP_UP_IN_SCALE,
				RECUPDISTRIBPENTE,
				DONE,
				FAIL
				);
	error_e ret = IN_PROGRESS;
	switch(state)
	{
		case INIT:
			state = DISPENSER_SOUTH;
			break;
	//	case MARK_ATOMS_ON_FLOOR:
	//		state = try_subaction(sub_mark_ground_atoms_on_floor(), state, DONE, FAIL);
		//	break;
		case DISPENSER_SOUTH:
			state = try_subaction(sub_dispenser_south(), state,DEPOSE_PARTICLE_COLLIDER, FAIL);
			break;
		case DEPOSE_PARTICLE_COLLIDER:
			state = try_subaction(sub_depose_particle_collider(), state,HOP_UP_IN_SCALE, FAIL);
			break;
		case HOP_UP_IN_SCALE:
			state = try_subaction(sub_hop_up_in_scale(), state,RECUPDISTRIBPENTE, FAIL);
			break;
		case RECUPDISTRIBPENTE :
		state = try_subaction(sub_recup_distrbuteur_pente(),state,DONE,FAIL);
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

