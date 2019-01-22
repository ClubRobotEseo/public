#include <string.h>

#include "elements.h"
#include "environment.h"

#define LOG_PREFIX "element: "
#define LOG_COMPONENT OUTPUT_LOG_COMPONENT_ELEMENTS
#include "QS/QS_outputlog.h"
#include "QS/QS_IHM.h"
#include "QS/QS_can_over_xbee.h"
#include "QS/QS_watchdog.h"
#include "utils/actionChecker.h"
#include "QS/QS_stateMachineHelper.h"
#include "utils/generic_functions.h"
#include "propulsion/astar.h"
#include "strats_2019/score.h"

typedef struct{
	bool_e sending;
	bool_e received;
	bool_e toUpdate;
	time32_t lastUpdate;
	//bool_e flag;
}hardflag_s;

#define HARDFLAGS_NB  (F_ELEMENTS_HARDFLAGS_END - F_ELEMENTS_HARDFLAGS_START - 1)
#define HARDFLAGS_PERIOD  	(1000)
#define HARDFLAGS_TIMEOUT  	(HARDFLAGS_PERIOD + 200)

static volatile bool_e elements_flags[F_ELEMENTS_FLAGS_NB];
#if USE_HARDFLAGS
	static volatile hardflag_s elements_hardflags[HARDFLAGS_NB];
#endif

#if USE_HARDFLAGS
	static void ELEMENTS_process_hardflags();
	static void ELEMENTS_send_hardflags();
#endif



void ELEMENTS_init(){
	Uint8 i;

	for(i=0;i<F_ELEMENTS_FLAGS_NB;i++)
	{
		elements_flags[i] = FALSE;
	}

	#if USE_HARDFLAGS
		for(i = 0; i < HARDFLAGS_NB; i++)
		{
			elements_hardflags[i].sending = FALSE;
			elements_hardflags[i].received = FALSE;
			elements_hardflags[i].lastUpdate = global.absolute_time;
		}

		// Init watchdog to send messages each HARDFLAGS_PERIOD ms
		watchdog_id_t watchdog_id = WATCHDOG_create(HARDFLAGS_PERIOD, &ELEMENTS_send_hardflags, TRUE);
		if(watchdog_id == WATCHDOG_ERROR) {
			error_printf("ERROR hardflag: creation of watchdog failed\n");
		}
	#endif
}



void ELEMENTS_process_main(){
	#if USE_HARDFLAGS
		ELEMENTS_process_hardflags();
	#endif
}

bool_e ELEMENTS_get_flag(elements_flags_e flag_id)
{
	assert(flag_id < F_ELEMENTS_FLAGS_NB);
	return elements_flags[flag_id];
}

void ELEMENTS_set_flag(elements_flags_e flag_id, bool_e new_state)
{
	assert(flag_id < F_ELEMENTS_FLAGS_NB);
	elements_flags[flag_id] = new_state;

#if USE_SYNC_ELEMENTS
	if(flag_id < F_ELEMENTS_FLAGS_END_SYNCH)
	{
		CAN_msg_t msg;
		msg.sid = XBEE_SYNC_ELEMENTS_FLAGS;
		msg.size = SIZE_XBEE_SYNC_ELEMENTS_FLAGS;
		msg.data.xbee_sync_elements_flags.flagId = flag_id;
		msg.data.xbee_sync_elements_flags.flag = new_state;
		CANMsgToXbee(&msg,FALSE);
	}
#endif

#if USE_HARDFLAGS
	if(flag_id > F_ELEMENTS_HARDFLAGS_START && flag_id < F_ELEMENTS_HARDFLAGS_END)
	{
		elements_hardflags[flag_id - F_ELEMENTS_HARDFLAGS_START - 1].sending = new_state;
		// Dans tous les cas (même en cas de remise à 0 du flag), on doit envoyer la nouvelle valeur à l'autre robot au moins une fois.
		elements_hardflags[flag_id - F_ELEMENTS_HARDFLAGS_START - 1].toUpdate = TRUE;
	}
#endif
}

void ELEMENTS_set_flag_without_synchro(elements_flags_e flag_id, bool_e new_state)
{
	assert(flag_id < F_ELEMENTS_FLAGS_NB);
	elements_flags[flag_id] = new_state;
}

//################################## SYNCHRONISATION #################################
#if USE_SYNC_ELEMENTS
void ELEMENTS_receive_flags(CAN_msg_t* msg)
{
	if(msg->data.xbee_sync_elements_flags.flagId < F_ELEMENTS_FLAGS_END_SYNCH){
		//debug_printf("\n\n\n\nReception message Xbee\n\n\n\n");
		elements_flags[msg->data.xbee_sync_elements_flags.flagId] = msg->data.xbee_sync_elements_flags.flag;
		ELEMENTS_update_info(msg->data.xbee_sync_elements_flags.flagId);
	}
}

void ELEMENTS_update_info(elements_flags_e flagId) {

}
#endif

#define	TIMEOUT_ANSWER	200

error_e ELEMENTS_check_communication(CAN_msg_t * msg)
{
#if USE_SYNC_ELEMENTS
	CREATE_MAE(INIT,
			SEND_REQUEST,
			WAIT_FOR_ANSWER,
			WAIT_TIMEOUT,
			TIMEOUT,
			ANSWER_RECEIVED,
			END);
	static bool_e watchdog_flag = FALSE;
	static watchdog_id_t watchdog_id = 0;
	static time32_t local_time = 0;

	switch(state)
	{
		case INIT:
			if(msg == NULL)	//On s'autorise à continuer seulement si ce n'est pas la réception d'un message qui nous active...
				state = SEND_REQUEST;
			break;
		case SEND_REQUEST:{
			CAN_msg_t request;
			request.sid = XBEE_COMMUNICATION_AVAILABLE;
			request.size = 0;
			CANMsgToXbee(&request,FALSE);
			state = WAIT_FOR_ANSWER;
			break;}
		case WAIT_FOR_ANSWER:
			if(entrance)
			{
				watchdog_id = WATCHDOG_create_flag(TIMEOUT_ANSWER,&watchdog_flag);
			}
			if(watchdog_flag)
			{
				state = WAIT_TIMEOUT;
			}
			else if(msg)
			{
				if(msg->sid == XBEE_COMMUNICATION_RESPONSE)
				{
					ELEMENTS_set_flag(F_COMMUNICATION_AVAILABLE, TRUE);
					state = ANSWER_RECEIVED;
					WATCHDOG_stop(watchdog_id);
				}
			}
			break;
		case ANSWER_RECEIVED:
			state = END;
			break;

		case WAIT_TIMEOUT:
			if(entrance){
				local_time = global.absolute_time;
			}
			if(global.absolute_time > local_time + 1000){
				state = TIMEOUT;
			}
			break;
		case TIMEOUT:
			RESET_MAE();
			ELEMENTS_set_flag(F_COMMUNICATION_AVAILABLE, FALSE);
			return END_WITH_TIMEOUT;
			break;
		case END:
			RESET_MAE();
			return END_OK;
			break;
		default:
			RESET_MAE();
			break;
	}
	#endif
	return IN_PROGRESS;

}

//################################## HARDFLAGS #################################
#if USE_HARDFLAGS
	static void ELEMENTS_send_hardflags(){
		Uint8 nb = 0;
		bool_e value;
		CAN_msg_t request;
		request.sid = XBEE_ELEMENTS_HARDFLAGS;
		request.size = SIZE_XBEE_ELEMENTS_HARDFLAGS;

		Uint8 hardflag_id = 0;
		while(hardflag_id < HARDFLAGS_NB){
			nb = 0;
			while(hardflag_id < HARDFLAGS_NB && nb < NB_HARDFLAG_PER_MSG){
				value = ELEMENTS_get_flag( F_ELEMENTS_HARDFLAGS_START + 1 + hardflag_id);
				if(value || elements_hardflags[hardflag_id].toUpdate) {
					request.data.xbee_elements_hardflags.flagId[nb] = hardflag_id;
					if(value) {
						BIT_SET(request.data.xbee_elements_hardflags.flags, nb);
					} else {
						BIT_CLR(request.data.xbee_elements_hardflags.flags, nb);
					}
					elements_hardflags[hardflag_id].toUpdate = FALSE; // Mise à jour prise en compte
					nb++;
				}
				hardflag_id++;
			}
			if(nb > 0){
				request.data.xbee_elements_hardflags.nb = nb;
				CANMsgToXbee(&request,FALSE);
			}
		}
	}


	void ELEMENTS_receive_hardflags(CAN_msg_t * msg)
	{
		Uint8 i;
		Uint8 hardflag_id = 0;
		elements_flags_e flag_id = 0;
		bool_e value = FALSE;
		if(msg->sid == XBEE_ELEMENTS_HARDFLAGS) {
			for(i = 0; i < msg->data.xbee_elements_hardflags.nb && i < NB_HARDFLAG_PER_MSG; i++){
				hardflag_id = msg->data.xbee_elements_hardflags.flagId[i];
				flag_id = F_ELEMENTS_HARDFLAGS_START + 1 + hardflag_id;
				value = BIT_TEST(msg->data.xbee_elements_hardflags.flags, i);
				elements_hardflags[hardflag_id].lastUpdate = global.absolute_time;
				ELEMENTS_set_flag_without_synchro(flag_id, value);
				//debug_printf("XBEE set flag[%d]=%d\n", flag_id, value);
				if(elements_hardflags[hardflag_id].sending) {
					error_printf("ERROR Hardflag : This hardflag %d could not be used by both robots\n", hardflag_id);
				}
			}
		}
	}

	static void ELEMENTS_process_hardflags()
	{
		Uint8 i;
		elements_flags_e flag_id = 0;
		for(i = 0; i < HARDFLAGS_NB; i++){
			flag_id = F_ELEMENTS_HARDFLAGS_START + 1 + i;
			if(!elements_hardflags[i].sending && (global.absolute_time - elements_hardflags[i].lastUpdate) > HARDFLAGS_TIMEOUT) {
				if(ELEMENTS_get_flag(flag_id)) {
					ELEMENTS_set_flag_without_synchro(flag_id, FALSE);
				}
			}
		}
	}
#endif


//################################## ELEMENTS OF THE YEAR #################################

// Id of atoms in dispensors begin at 0 from the middle of the playing area
static ATOM_e our_small_dispensor[NB_ATOMS_IN_SMALL_DISPENSOR] = {
		(ATOM_e) {REDIUM, 1},
		(ATOM_e) {GREENIUM, 1},
		(ATOM_e) {BLUEIUM, 1}
};
static ATOM_e our_large_dispensor[NB_ATOMS_IN_LARGE_DISPENSOR] = {
		(ATOM_e) {GREENIUM, 1},
		(ATOM_e) {REDIUM, 1},
		(ATOM_e) {BLUEIUM, 1},
		(ATOM_e) {REDIUM, 1},
		(ATOM_e) {GREENIUM, 1},
		(ATOM_e) {REDIUM, 1}
};
static ATOM_e adv_large_dispensor[NB_ATOMS_IN_LARGE_DISPENSOR] = {
		(ATOM_e) {GREENIUM, 1},
		(ATOM_e) {REDIUM, 1},
		(ATOM_e) {BLUEIUM, 1},
		(ATOM_e) {REDIUM, 1},
		(ATOM_e) {GREENIUM, 1},
		(ATOM_e) {REDIUM, 1}
};

static SCALE_e scale = {.nb = 0};
static ACCELERATOR_e accelerator = {.nb = 0};
static ATOM_kind_e big_robot[ATOM_NB_POS] = {0};

// Private functions
static void ATOM_take_atom_in_dispensor(ATOM_e * atom, ATOM_kind_e * atom_in_robot, bool_e status);

/**
 * Take atoms into a dispensor.
 * @param dispensor_id the id of the dispensor.
 * @param property the property of the dispensor (OUR_ELEMENT or ADV_ELEMENT).
 * @param config the configuration used to tkae this dispensor.
 * @param status an array of pumps status used to take this dispensor (maybe some atoms have not been taken).
 * 	This array can be filled using ids defined by PUMP_STATUS_id_e.
 * 		PUMP_STATUS_LEFT: the pump at the left of the robot (e.g., ACT_EXPANDER_PUMP_BIG_BACKWARD_VERY_LEFT, ACT_EXPANDER_PUMP_BIG_FORWARD_LEFT)
 * 		PUMP_STATUS_CENTER: the pump at the middle of the robot.
 * 		PUMP_STATUS_RIGHT: the pump at the right of the robot (e.g., ACT_EXPANDER_PUMP_BIG_BACKWARD_VERY_RIGHT, ACT_EXPANDER_PUMP_BIG_FORWARD_RIGHT)
 */
void ATOM_take_dispensor(DISPENSOR_id_e dispensor_id, ELEMENTS_property_e property, ATOM_CONFIG_kind_e config, bool_e status[]) {
	ATOM_e * dispensor = NULL;

	// Select dispensor
	if(property == OUR_ELEMENT) {
		switch(dispensor_id) {
			case DISPENSOR_SMALL:						dispensor = our_small_dispensor;   		break;
			case DISPENSOR_LARGE_NEXT_TO_SCALE:			dispensor = our_large_dispensor;   		break;
			case DISPENSOR_LARGE_NEXT_TO_START_ZONE:	dispensor = &(our_large_dispensor[3]);  break;
			default:
				error_printf("ERROR in ATOM_take_dispensor: dispensor_id=%d is not correct\n", dispensor_id);
				break;
		}
	} else if(property == ADV_ELEMENT) {
		switch(dispensor_id) {
			case DISPENSOR_LARGE_NEXT_TO_SCALE:			dispensor = adv_large_dispensor;   		break;
			case DISPENSOR_LARGE_NEXT_TO_START_ZONE:	dispensor = &(adv_large_dispensor[3]);  break;
			default:
				error_printf("ERROR in ATOM_take_dispensor: dispensor_id=%d is not correct\n", dispensor_id);
				break;
		}
	} else {
		error_printf("ERROR in ATOM_take_dispensor: property is not correct");
	}

	// We cannot continue if the dispensor is invalid
	if(dispensor == NULL) {
		return;
	}


	if(I_AM_BIG()) {
		// Take atoms in dispensor
		switch(config) {
			case ATOM_CONFIG_ELEVATOR_FRONT:
				if((global.color == YELLOW && property == OUR_ELEMENT) || (global.color == PURPLE && property == ADV_ELEMENT)) {
					ATOM_take_atom_in_dispensor(&dispensor[0], &big_robot[ATOM_POS_ELEVATOR_FRONT_LEFT], status[PUMP_STATUS_LEFT]);
					ATOM_take_atom_in_dispensor(&dispensor[1], &big_robot[ATOM_POS_ELEVATOR_FRONT_MIDDLE], status[PUMP_STATUS_MIDDLE]);
					ATOM_take_atom_in_dispensor(&dispensor[2], &big_robot[ATOM_POS_ELEVATOR_FRONT_RIGHT], status[PUMP_STATUS_RIGHT]);
				} else {
					ATOM_take_atom_in_dispensor(&dispensor[2], &big_robot[ATOM_POS_ELEVATOR_FRONT_LEFT], status[PUMP_STATUS_LEFT]);
					ATOM_take_atom_in_dispensor(&dispensor[1], &big_robot[ATOM_POS_ELEVATOR_FRONT_MIDDLE], status[PUMP_STATUS_MIDDLE]);
					ATOM_take_atom_in_dispensor(&dispensor[0], &big_robot[ATOM_POS_ELEVATOR_FRONT_RIGHT], status[PUMP_STATUS_RIGHT]);
				}
				break;
			case ATOM_CONFIG_SORTING_BACK_RIGHT:
				if((global.color == YELLOW && property == OUR_ELEMENT) || (global.color == PURPLE && property == ADV_ELEMENT)) {
					ATOM_take_atom_in_dispensor(&dispensor[0], &big_robot[ATOM_POS_SORTING_BACK_VERY_RIGHT], status[PUMP_STATUS_RIGHT]);
					ATOM_take_atom_in_dispensor(&dispensor[1], &big_robot[ATOM_POS_SORTING_BACK_RIGHT], status[PUMP_STATUS_MIDDLE]);
					ATOM_take_atom_in_dispensor(&dispensor[2], &big_robot[ATOM_POS_SORTING_BACK_VERY_LEFT], status[PUMP_STATUS_LEFT]);
				} else {
					ATOM_take_atom_in_dispensor(&dispensor[2], &big_robot[ATOM_POS_SORTING_BACK_VERY_RIGHT], status[PUMP_STATUS_RIGHT]);
					ATOM_take_atom_in_dispensor(&dispensor[1], &big_robot[ATOM_POS_SORTING_BACK_RIGHT], status[PUMP_STATUS_MIDDLE]);
					ATOM_take_atom_in_dispensor(&dispensor[0], &big_robot[ATOM_POS_SORTING_BACK_VERY_LEFT], status[PUMP_STATUS_LEFT]);
				}
				break;
			case ATOM_CONFIG_SORTING_BACK_LEFT:
				if((global.color == YELLOW && property == OUR_ELEMENT) || (global.color == PURPLE && property == ADV_ELEMENT)) {
					ATOM_take_atom_in_dispensor(&dispensor[0], &big_robot[ATOM_POS_SORTING_BACK_VERY_RIGHT], status[PUMP_STATUS_RIGHT]);
					ATOM_take_atom_in_dispensor(&dispensor[1], &big_robot[ATOM_POS_SORTING_BACK_LEFT], status[PUMP_STATUS_MIDDLE]);
					ATOM_take_atom_in_dispensor(&dispensor[2], &big_robot[ATOM_POS_SORTING_BACK_VERY_LEFT], status[PUMP_STATUS_LEFT]);
				} else {
					ATOM_take_atom_in_dispensor(&dispensor[2], &big_robot[ATOM_POS_SORTING_BACK_VERY_RIGHT], status[PUMP_STATUS_RIGHT]);
					ATOM_take_atom_in_dispensor(&dispensor[1], &big_robot[ATOM_POS_SORTING_BACK_LEFT], status[PUMP_STATUS_MIDDLE]);
					ATOM_take_atom_in_dispensor(&dispensor[0], &big_robot[ATOM_POS_SORTING_BACK_VERY_LEFT], status[PUMP_STATUS_LEFT]);
				}
				break;
			default:
				error_printf("Default case in ATOM_take_small_dispensor\n");
		}
	}
}

/**
 * Helper to take an atom in a dispensor.
 * (This is a private function)
 * @param atom the atom to take.
 * @param atom_in_robot the position of the atom in the robot.
 * @param status the pûmp status.
 */
static void ATOM_take_atom_in_dispensor(ATOM_e * atom, ATOM_kind_e * atom_in_robot, bool_e status) {
	if (status) {
		*atom_in_robot = atom->kind;
		atom->available = FALSE;
	}
}

/**
 * Take an atom in the robot.
 * @param pos_id the position of the atom.
 * @param atom the atom taken.
 */
void ATOM_take(ATOM_POS_id_e pos_id, ATOM_kind_e atom) {
	if(I_AM_BIG()) {
		assert(pos_id < ATOM_NB_POS);
		if(big_robot[pos_id] != NO_ATOM) {
			error_printf("ERROR in ATOM_take: atom position is not available [pos=%d atom=%d]\n", pos_id, atom);
		}
		big_robot[pos_id] = atom;
	}
}

/**
 * Release an atom out of the robot.
 * @param pos_id the position of the atom.
 * @param atom the atom taken.
 */
void ATOM_release(ATOM_POS_id_e pos_id, ATOM_RELEASE_AREA_kind_e release_area) {
	bool_e ground_well_placed = FALSE;
	if(I_AM_BIG()) {
		assert(pos_id < ATOM_NB_POS);
		if(big_robot[pos_id] == NO_ATOM) {
			error_printf("ERROR in ATOM_release: atom position is not occupied [pos=%d]\n", pos_id);
		}

		switch(release_area) {
			case ATOM_RELEASE_AREA_GROUND_UNKNOWN:
				if(big_robot[pos_id] == REDIUM || big_robot[pos_id] == GREENIUM) {
					// Un coup sur deux, on en compte un de bien placé et un de mal placé.
					if(ground_well_placed) {
						SCORE_something_happen(EVENT_ATOM_WELL_PLACED_IN_PERIODIC_TABLE, 1);
					} else {
						SCORE_something_happen(EVENT_ATOM_IN_PERIODIC_TABLE, 1);
					}
					ground_well_placed = !ground_well_placed;
				} else {
					// On depose soit dans la zone GREENIUM ou REDIUM donc les BLUEIUM et le GOLDENIUM sont forcément mal placés
					SCORE_something_happen(EVENT_ATOM_IN_PERIODIC_TABLE, 1);
				}
				break;
			case ATOM_RELEASE_AREA_GROUND_WELL_PLACED:
				if(big_robot[pos_id] == GOLDENIUM) {
					SCORE_something_happen(EVENT_ATOM_WELL_PLACED_IN_PERIODIC_TABLE, 1);
				} else {
					SCORE_something_happen(EVENT_GOLDENIUM_WELL_PLACED_IN_PERIODIC_TABLE, 1);
				}
				break;
			case ATOM_RELEASE_AREA_GROUND_NOT_WELL_PLACED:
				SCORE_something_happen(EVENT_ATOM_IN_PERIODIC_TABLE, 1);
				break;
			case ATOM_RELEASE_AREA_SCALE:
				if(scale.nb < NB_ATOMS_IN_SCALE) {
					scale.atoms[scale.nb] = big_robot[pos_id];
					scale.nb++;
					switch(big_robot[pos_id]) {
						case REDIUM:		SCORE_something_happen(EVENT_REDIUM_IN_SCALE, 1);		break;
						case GREENIUM:		SCORE_something_happen(EVENT_GREENIUM_IN_SCALE, 1);		break;
						case BLUEIUM:		SCORE_something_happen(EVENT_BLUEIUM_IN_SCALE, 1);		break;
						case GOLDENIUM:		SCORE_something_happen(EVENT_GOLDENIUM_IN_SCALE, 1);	break;
						default: error_printf("ERROR in ATOM_release_in_scale: Unknown atom in scale\n");  break;
					}

				} else {
					error_printf("ERROR in ATOM_release_in_scale: Too much atoms in scale\n");
				}
				break;
			case ATOM_RELEASE_AREA_ACCELERATOR:
				if(accelerator.nb < NB_ATOMS_IN_SCALE) {
					accelerator.atoms[accelerator.nb] = big_robot[pos_id];
					accelerator.nb++;
					SCORE_something_happen(EVENT_ATOM_IN_ACCELERATOR, 1);
				} else {
					error_printf("ERROR in ATOM_release_in_accelerator: Too much atoms in accelerator\n");
				}
				break;
			default:
				error_printf("ERROR in ATOM_release: No release area has been specified\n");
		}
		// In all cases, put NO_ATOM on this position.
		big_robot[pos_id] = NO_ATOM;
	}
}

/**
 * Check if a position is available.
 * @param pos_id the position to check.
 * @return TRUE if this position is available and FALSE otherwise.
 */
bool_e ATOM_is_position_available(ATOM_POS_id_e pos_id) {
	assert(pos_id < ATOM_NB_POS);
	return (big_robot[pos_id] == NO_ATOM);
}

/**
 * Getter of an atom at a position in the robot.
 * @param pos_id the position to check.
 * @return the atome at the given position or NO_ATOM is empty.
 */
ATOM_kind_e ATOM_get_atom_at_position(ATOM_POS_id_e pos_id) {
	assert(pos_id < ATOM_NB_POS);
	return big_robot[pos_id];
}


///**
// * Getter of the number of atoms in the robot.
// * @return the number of atoms in the robot.
// */
//Uint8 ATOM_get_nb_atoms_in_robot() {
//	Uint8 pos;
//	Uint8 nb = 0;
//
//	if(I_AM_BIG()) {
//		for(pos = 0; pos < ATOM_NB_POS; pos++) {
//			if(big_robot[pos] != NO_ATOM) {
//				nb++;
//			}
//		}
//	}
//
//	return nb;
//}

/**
 * Check if all positions of a config are available.
 * @param config the config to check.
 * @return TRUE if this config is available and FALSE otherwise.
 */
bool_e ATOM_is_config_available(ATOM_CONFIG_kind_e config) {
	assert(config < ATOM_NB_CONFIG);
	bool_e answer = FALSE;

	switch(config) {
		case ATOM_CONFIG_ELEVATOR_FRONT:
			answer = (big_robot[ATOM_POS_ELEVATOR_FRONT_LEFT] == NO_ATOM && big_robot[ATOM_POS_ELEVATOR_FRONT_MIDDLE] == NO_ATOM && big_robot[ATOM_POS_ELEVATOR_FRONT_RIGHT] == NO_ATOM);
			break;
		case ATOM_CONFIG_SORTING_BACK_RIGHT:
			answer = (big_robot[ATOM_POS_SORTING_BACK_VERY_RIGHT] == NO_ATOM && big_robot[ATOM_POS_SORTING_BACK_RIGHT] == NO_ATOM && big_robot[ATOM_POS_SORTING_BACK_VERY_LEFT] == NO_ATOM);
			break;
		case ATOM_CONFIG_SORTING_BACK_LEFT:
			answer = (big_robot[ATOM_POS_SORTING_BACK_VERY_RIGHT] == NO_ATOM && big_robot[ATOM_POS_SORTING_BACK_LEFT] == NO_ATOM && big_robot[ATOM_POS_SORTING_BACK_VERY_LEFT] == NO_ATOM);
			break;
		default:
			error_printf("Default case in ATOM_take_small_dispensor\n");
	}

	return answer;
}

/**
 * Check if a dispensor is empty.
 * @param dispensor the dispensor to check.
 * @param property the property of the dispensor.
 * @return TRUE if this dispensor is empty and FALSE otherwise.
 */
bool_e ATOM_is_dispensor_empty(DISPENSOR_id_e dispensor_id, ELEMENTS_property_e property) {
	ATOM_e * dispensor = NULL;
	bool_e answer = FALSE;

	// Select dispensor
	if(property == OUR_ELEMENT) {
		switch(dispensor_id) {
			case DISPENSOR_SMALL:						dispensor = our_small_dispensor;   		break;
			case DISPENSOR_LARGE_NEXT_TO_SCALE:			dispensor = our_large_dispensor;   		break;
			case DISPENSOR_LARGE_NEXT_TO_START_ZONE:	dispensor = &(our_large_dispensor[3]);  break;
			default:
				error_printf("ERROR in ATOM_is_dispensor_empty: dispensor_id=%d is not correct\n", dispensor_id);
				break;
		}
	} else if(property == ADV_ELEMENT) {
		switch(dispensor_id) {
			case DISPENSOR_LARGE_NEXT_TO_SCALE:			dispensor = adv_large_dispensor;   		break;
			case DISPENSOR_LARGE_NEXT_TO_START_ZONE:	dispensor = &(adv_large_dispensor[3]);  break;
			default:
				error_printf("ERROR in ATOM_is_dispensor_empty: dispensor_id=%d is not correct\n", dispensor_id);
				break;
		}
	} else {
		error_printf("ERROR in ATOM_is_dispensor_empty: property is not correct");
	}

	if(dispensor != NULL) {
		answer = !(dispensor[0].available || dispensor[1].available || dispensor[2].available);
	}

	return answer;
}


/**
 * Getter of number of atoms in accelerator.
 * @return the number of atoms in accelerator.
 */
Uint8 ATOM_get_nb_in_accelerator() {
	return accelerator.nb;
}


/**
 * Print an atom.
 * @param atom the atom to print.
 */
char * ATOM_print(ATOM_kind_e atom) {
	char * str = NULL;

	switch(atom) {
		case NO_ATOM: 		str = "NO_ATOM"; 		break;
		case REDIUM: 		str = "REDIUM"; 		break;
		case GREENIUM: 		str = "GREENIUM"; 		break;
		case BLUEIUM:		str = "BLUEIUM"; 		break;
		case GOLDENIUM: 	str = "GOLDENIUM"; 		break;
		case UNKNOWN_ATOM: 	str = "UNKNOWN_ATOM"; 	break;
		default: 			str = "?? ATOM ??"; 	break;
	}

	return str;
}

#define ATOM_BUFFER_PRINT_SIZE	(256)

/**
 * Print atoms currently in the robot.
 */
void ATOM_print_robot() {
	Uint8 pos;
	char str[ATOM_BUFFER_PRINT_SIZE] = {0};

	if(I_AM_BIG()) {
		debug_printf("ATOMS in BIG_ROBOT:\n");
		for(pos = 0; pos < ATOM_NB_POS; pos++) {
			strncpy(str, "\t\t", ATOM_BUFFER_PRINT_SIZE);
			switch(pos) {
				case ATOM_POS_ELEVATOR_FRONT_LEFT: 			strncat(str, "ATOM_POS_ELEVATOR_FRONT_LEFT", ATOM_BUFFER_PRINT_SIZE); 			break;
				case ATOM_POS_ELEVATOR_FRONT_MIDDLE: 		strncat(str, "ATOM_POS_ELEVATOR_FRONT_MIDDLE", ATOM_BUFFER_PRINT_SIZE); 		break;
				case ATOM_POS_ELEVATOR_FRONT_RIGHT: 		strncat(str, "ATOM_POS_ELEVATOR_FRONT_RIGHT", ATOM_BUFFER_PRINT_SIZE); 			break;
				case ATOM_POS_SORTING_BACK_VERY_LEFT: 		strncat(str, "ATOM_POS_SORTING_BACK_VERY_LEFT", ATOM_BUFFER_PRINT_SIZE); 		break;
				case ATOM_POS_SORTING_BACK_LEFT:			strncat(str, "ATOM_POS_SORTING_BACK_LEFT", ATOM_BUFFER_PRINT_SIZE); 			break;
				case ATOM_POS_SORTING_BACK_RIGHT:			strncat(str, "ATOM_POS_SORTING_BACK_RIGHT", ATOM_BUFFER_PRINT_SIZE); 			break;
				case ATOM_POS_SORTING_BACK_VERY_RIGHT:		strncat(str, "ATOM_POS_SORTING_BACK_VERY_RIGHT", ATOM_BUFFER_PRINT_SIZE);		break;
				case ATOM_POS_CHAOS_LEFT:					strncat(str, "ATOM_POS_CHAOS_LEFT", ATOM_BUFFER_PRINT_SIZE); 					break;
				case ATOM_POS_CHAOS_MIDDLE:					strncat(str, "ATOM_POS_CHAOS_MIDDLE", ATOM_BUFFER_PRINT_SIZE); 					break;
				case ATOM_POS_CHAOS_RIGHT:					strncat(str, "ATOM_POS_CHAOS_RIGHT", ATOM_BUFFER_PRINT_SIZE);					break;
				case ATOM_POS_CHAOS_BACK:					strncat(str, "ATOM_POS_CHAOS_BACK", ATOM_BUFFER_PRINT_SIZE); 					break;
				case ATOM_POS_SLOPE_TAKER:					strncat(str, "ATOM_POS_SLOPE_TAKER", ATOM_BUFFER_PRINT_SIZE); 					break;
				case ATOM_POS_LOCKER_BACK:					strncat(str, "ATOM_POS_LOCKER_BACK", ATOM_BUFFER_PRINT_SIZE); 					break;
				default: 									strncat(str, "UNKNOWM_POS", ATOM_BUFFER_PRINT_SIZE);							break;
			}
			debug_printf("%s: %s\n", str, ATOM_print(big_robot[pos]));
		}
	}
}

/**
 * Print atoms currently in a dispensor.
 * @param property the property of dispensors to print.
 */
void ATOM_print_dispensor(ELEMENTS_property_e property) {
	Uint8 pos = 0;

	if((global.color == YELLOW && property == OUR_ELEMENT) || (global.color == PURPLE && property != OUR_ELEMENT)) {
		debug_printf("ATOMS in YELLOW small dispensor:\n");
		for(pos = 0; pos < NB_ATOMS_IN_SMALL_DISPENSOR; pos++) {
			debug_printf("\tdispensor[%d]: %s - %d\n", pos, ATOM_print(our_small_dispensor[pos].kind), our_small_dispensor[pos].available);
		}

		debug_printf("ATOMS in YELLOW large dispensor:\n");
		for(pos = 0; pos < NB_ATOMS_IN_LARGE_DISPENSOR; pos++) {
			debug_printf("\tdispensor[%d]: %s - %d\n", pos, ATOM_print(our_large_dispensor[pos].kind), our_large_dispensor[pos].available);
		}
	} else {
		debug_printf("ATOMS in PURPLE large dispensor:\n");
		for(pos = 0; pos < NB_ATOMS_IN_LARGE_DISPENSOR; pos++) {
			debug_printf("\tdispensor[%d]: %s - %d\n", pos, ATOM_print(adv_large_dispensor[pos].kind), adv_large_dispensor[pos].available);
		}
	}
}
