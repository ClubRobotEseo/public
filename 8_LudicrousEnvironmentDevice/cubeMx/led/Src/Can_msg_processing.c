/*
 *	Club Robot ESEO 2018
 *
 *	Fichier : Can_msg_processing.c
 *	Package : Carte LED
 *	Description : Fonctions de traitement des messages CAN
 *  Auteur : Valentin
 *  Version 20180407
 */


#include "Can_msg_processing.h"
#include "QS/QS_CANmsgList.h"
#include "QS/QS_uart.h"
#include "environment.h"


void CAN_process_msg(CAN_msg_t* msg) {
	// Enable the communication
	global.flags.communication_available = TRUE;

	// Process messages
	switch (msg->sid){
		case XBEE_ELEMENTS_COLOR_COMBINATION:
			global.match_time = msg->data.xbee_elements_color_combination.match_time;
			global.flags.color_combination_updated = TRUE;
			ENV_process_color_combination(msg);
			break;

		case XBEE_SCORE_ESTIMATION:
			global.flags.score_updated = TRUE;
			ENV_process_score_estimation(msg);
			break;

		default:
			break;
	}
}

void QS_CAN_VERBOSE_can_msg_print(CAN_msg_t* msg){
	switch (msg->sid){
		case XBEE_ELEMENTS_COLOR_COMBINATION:			debug_printf("%.3x XBEE_ELEMENTS_COLOR_COMBINATION\n", 		XBEE_ELEMENTS_COLOR_COMBINATION	);	break;
		case XBEE_SCORE_ESTIMATION:						debug_printf("%.3x XBEE_SCORE_ESTIMATION\n", 				XBEE_SCORE_ESTIMATION			);	break;
		default:										debug_printf("%.3x UNKNOWN SID", 							msg->sid						);	 break;
	}
}


