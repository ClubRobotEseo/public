/*
 *	Club Robot ESEO 2017 - 2018
 *
 *	Fichier : led_manager.h
 *	Package : Carte Stratégie
 *	Description : Traitement des informations pour le panneau domotique LED (Ludicrous Environment Device)
 *	Auteur : Valentin
 *	Version 20180406
 */

#include "led_manager.h"
#include "QS/QS_stateMachineHelper.h"
#include "QS/QS_can_over_xbee.h"
#include "QS/QS_outputlog.h"
#include "strats_2019/score.h"
#include "elements.h"


#define LED_WAIT_TIME		(5000)   // Temps en ms d'attente entre l'envoi de 2 messages XBEE
#define LED_TIMEOUT			(1000)	  // Temps au dela duquel on considere que le panneau est eteint

static volatile bool_e led_is_active = FALSE;
static volatile uint32_t last_msg_time_from_led = 0;

void LED_process_main() {
	CREATE_MAE(
			INIT,
			WAIT_LED_ACTIVATION,
			SEND_MESSAGE,
			WAIT,
			MATCH_ENDED
		);

	static time32_t local_time;
	static bool_e combinaison_sent = FALSE;

	switch(state){
		case INIT:
			//led_is_active = FALSE;
			combinaison_sent = FALSE;
			state = WAIT_LED_ACTIVATION;
			break;
		case WAIT_LED_ACTIVATION:
			if(led_is_active)
				state = SEND_MESSAGE;
			break;
		case SEND_MESSAGE:
			local_time = global.absolute_time;
			LED_send_xbee_messages(XBEE_SCORE_ESTIMATION);
			state = (global.flags.match_over)?MATCH_ENDED:WAIT;
			break;

		case WAIT:
			//On envoit un message si :
			// le match vient de se terminer
			// ou si une combinaison de couleur vient d'être reçue sur BROTHER
			// ou si le dernier message date de plus de LED_WAIT_TIME ms
//			if(global.flags.match_over || (I_AM_SMALL() && is_combination_saved() && !combinaison_sent) || (global.absolute_time > (local_time + LED_WAIT_TIME))) {
//				state = SEND_MESSAGE;
//				local_time = global.absolute_time;
//			}
			UNUSED_VAR(local_time);
			UNUSED_VAR(combinaison_sent);

			//si le panneau LED n'est plus actif :
			if(global.absolute_time-last_msg_time_from_led > 6000)
				state = INIT;
			break;
		case MATCH_ENDED:
			//état puits.
			break;
		default:
			if(entrance){
				error_printf("default case in LED_process_main\n");
			}
			break;
	}

}

//Fonction appelée à chaque message reçu du panneau LED.
void LED_is_alive(void)
{
	last_msg_time_from_led = global.absolute_time;
	led_is_active = TRUE;
}

void LED_send_xbee_messages(Uint11 sid) {

	static CAN_msg_t msg;
	//ScoreEventCounter event_counter;

	switch(sid){

		case XBEE_SCORE_ESTIMATION:
			//event_counter = SCORE_get_event_counter();
			msg.sid = XBEE_SCORE_ESTIMATION;
			msg.size = SIZE_XBEE_SCORE_ESTIMATION;
			msg.data.xbee_score_estimation.estimation = SCORE_get_score();
			msg.data.xbee_score_estimation.robot_id = (I_AM_BIG() ? BIG_ROBOT:SMALL_ROBOT);
			CANMsgToXBeeDestination(&msg, LED_MODULE);
			break;

		default:

			break;
	}
}

/*
 * Cette machine permet de forcer l'envoi d'un ping au panneau LED pour savoir si là, maintenant, tout de suite, il est bien dispo.
 */
error_e LED_is_active() {
	CREATE_MAE_WITH_VERBOSE(SM_ID_LED_IS_ACTIVE,
			SEND_REQUEST,
			WAIT_RESPONSE,
			DONE,
			ERROR
		);

	static time32_t local_time;

	switch(state){
		case SEND_REQUEST:
			XBee_set_module_reachable(LED_MODULE, FALSE);
			XBee_Ping(LED_MODULE);
			state = WAIT_RESPONSE;
			break;

		case WAIT_RESPONSE:
			if(entrance) {
				local_time = global.absolute_time;
			}
			if(XBee_is_module_reachable(LED_MODULE)) {
				state = DONE;
			} else if(global.absolute_time > local_time + LED_TIMEOUT) {
				state = ERROR;
			}
			break;

		case DONE:
			RESET_MAE();
			return END_OK;
			break;

		case ERROR:
			RESET_MAE();
			return END_WITH_TIMEOUT;
			break;

		default:
			if(entrance)
				debug_printf("default case in LED_is_active\n");
			break;
	}
	return IN_PROGRESS;
}
