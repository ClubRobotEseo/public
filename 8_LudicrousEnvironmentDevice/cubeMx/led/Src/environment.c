/*
 *	Club Robot ESEO 2018
 *
 *	Fichier : environment.c
 *	Package : Carte LED
 *	Description : Fonctions de traitement des données de l'environnement
 *  Auteur : Valentin
 *  Version 20180407
 */

#include "environment.h"
#include "QS/QS_CANmsgList.h"

// Variables privées pour la combinaison de couleur
static CUBE_color_e color_combination[3] = {0};
static bool_e color_combination_available = FALSE;

// Variables privées pour l'estimation du score
static Uint16 estimation[2];
static ScoreEventCounter event_counter[2];


void ENV_process_main() {
	global.flags.score_updated = FALSE;
	global.flags.color_combination_updated = FALSE;
}


void ENV_process_color_combination(CAN_msg_t * msg) {
	if(msg->sid == XBEE_ELEMENTS_COLOR_COMBINATION) {
		color_combination[0] = msg->data.xbee_elements_color_combination.first_color;
		color_combination[1] = msg->data.xbee_elements_color_combination.second_color;
		color_combination[2] = msg->data.xbee_elements_color_combination.third_color;
		color_combination_available = TRUE;
	}
}

void ENV_process_score_estimation(CAN_msg_t * msg) {
	if(msg->sid == XBEE_SCORE_ESTIMATION) {
		robot_id_e id = msg->data.xbee_score_estimation.robot_id;
		estimation[id] = msg->data.xbee_score_estimation.estimation;
		event_counter[id].foe_ball_placed = msg->data.xbee_score_estimation.foe_ball_placed;
		event_counter[id].our_ball_placed = msg->data.xbee_score_estimation.our_ball_placed;
		event_counter[id].water_getted = msg->data.xbee_score_estimation.water_getted;
		event_counter[id].first_stage = msg->data.xbee_score_estimation.first_stage;
		event_counter[id].second_stage = msg->data.xbee_score_estimation.second_stage;
		event_counter[id].third_stage = msg->data.xbee_score_estimation.third_stage;
		event_counter[id].fourth_stage = msg->data.xbee_score_estimation.fourth_stage;
		event_counter[id].fifth_stage = msg->data.xbee_score_estimation.fifth_stage;
		event_counter[id].well_done_building = msg->data.xbee_score_estimation.well_done_building;
		event_counter[id].bee_placed = msg->data.xbee_score_estimation.bee_placed;
		event_counter[id].bursted_balloon = msg->data.xbee_score_estimation.bursted_balloon;
		event_counter[id].domotic_placed = msg->data.xbee_score_estimation.domotic_placed;
		event_counter[id].switch_activated = msg->data.xbee_score_estimation.switch_activated;
	}
}

bool_e ENV_get_color_combination(CUBE_color_e ** combination_ptr) {
	if(combination_ptr != NULL) {
		*combination_ptr = color_combination;
	}
	return color_combination_available;
}

Uint16 ENV_get_score_estimation(robot_id_e robot_id) {
	assert(robot_id < 2);
	return estimation[robot_id];
}

Uint16 ENV_get_total_score_estimation() {
	return estimation[BIG_ROBOT] + estimation[SMALL_ROBOT];
}

ScoreEventCounter ENV_get_event_counter(robot_id_e robot_id) {
	assert(robot_id < 2);
	return event_counter[robot_id];
}
