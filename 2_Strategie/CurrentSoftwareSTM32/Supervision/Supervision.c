/*
 *	Club Robot ESEO 2008 - 2011
 *	Archi-Tech', PACMAN, ChekNorris
 *
 *	Fichier : main.h
 *	Package : Supervision
 *	Description : sequenceur de la supervision
 *	Auteur : Jacen
 *	Version 20110413
 */

#include "Supervision.h"
#include "../QS/QS_who_am_i.h"
#include "../QS/QS_can_over_xbee.h"
#include "../QS/QS_outputlog.h"
#include "../QS/QS_can_over_xbee.h"
#include "../QS/QS_IHM.h"
#include "SD/SD.h"
#include "Selftest.h"
#include "RTC.h"
#include "Buffer.h"
#include "LCD_interface.h"
#include "Buzzer.h"
#include "../environment.h"
#include "../QS/QS_stateMachineHelper.h"

//@pre : QS_WHO_I_AM doit être found.
//@pre : le CAN doit être initialisé...
void Supervision_init(void)
{
	SELFTEST_init();
	RTC_init();
	BUFFER_init();

	#if USE_LCD
		LCD_init();
	#endif

	#if USE_XBEE_OLD
		if(QS_WHO_AM_I_get() == SMALL_ROBOT)
			CAN_over_XBee_init(SMALL_ROBOT_MODULE, BIG_ROBOT_MODULE);
		else
			CAN_over_XBee_init(BIG_ROBOT_MODULE, SMALL_ROBOT_MODULE);
	#endif
}
volatile static Uint16 t = 0;
void Supervision_process_1ms(void)
{
	if(t)
		t--;
	CAN_over_XBee_process_ms();
}

/*
 * @brief Demande à la prop l'envoie périodique tout les param ms
 */
void Supervision_ask_prop_to_send_periodically_pos(Uint16 dist, Sint16 angle){
	CAN_msg_t msg;
	msg.sid = PROP_SEND_PERIODICALLY_POSITION;
	msg.size = SIZE_PROP_SEND_PERIODICALLY_POSITION;
	msg.data.prop_send_periodically_position.translation = dist;
	msg.data.prop_send_periodically_position.rotation = angle;
	msg.data.prop_send_periodically_position.period = 0;
	CAN_send(&msg);
}


void SUPERVISION_send_pos_over_xbee(void)
{
	CAN_msg_t msg;
	msg.sid = XBEE_MY_POSITION_IS;
	msg.size = SIZE_XBEE_MY_POSITION_IS;
	msg.data.xbee_my_position_is.x = global.pos.x;
	msg.data.xbee_my_position_is.y = global.pos.y;
	msg.data.xbee_my_position_is.robot_id = QS_WHO_AM_I_get();
	CANMsgToXbee(&msg, TRUE);	//Envoi en BROADCAST...aux modules PINGés
}

//TODO un jour, il faudra recoder complètement cette fonction... avec une MAE... sa vieillesse la rend ridée et toute moche.
void Supervision_process_main(void)
{
	CREATE_MAE(INIT,
			WAIT_FIRST_SECOND,
			ENABLE_SD_RTC_PROPPERIODICALLY,
			WAIT_XBEE_REACHABLE,
			IDLE);
	static bool_e current_destination_reachable = FALSE;

	switch(state)
	{
		case INIT:
			state = WAIT_FIRST_SECOND;
			break;
		case WAIT_FIRST_SECOND:
			if(entrance)
				t = 1000;
			if(!t)
				state = ENABLE_SD_RTC_PROPPERIODICALLY;
			break;
		case ENABLE_SD_RTC_PROPPERIODICALLY:
			debug_printf("\n|-----------------------initialize Supervision---------------------------|\n");
			debug_printf("| Robot       : %s\n", QS_WHO_AM_I_get_name());
			debug_printf("| Date        : ");
			RTC_print_time();
			debug_printf("| Mesure 24V  : %dmV\n",SELFTEST_measure24_mV());
			SD_init();
			//A partir de maintenant, on peut loguer sur la carte SD...
			debug_printf("|------------------------------------------------------------------------|\n\n");

			Supervision_ask_prop_to_send_periodically_pos(1, PI4096/180); // Tous les milimetres et degrés: ca flood mais on est pas en match donc pas déplacment
			state = WAIT_XBEE_REACHABLE;
			break;
		case WAIT_XBEE_REACHABLE:
			//Si on a pas de lien XBEE avec l'autre Robot : les leds clignotent.
			//ATTENTION, si l'on désactive après allumage le XBEE sur l'un des robot... l'autre robot qui a eu le temps de dialoguer en XBEE ne clignotera pas !
			if(entrance)
			{
				IHM_leds_send_msg(1,(led_ihm_t){LED_COLOR_IHM, SPEED_BLINK_4HZ});
			}
			if(current_destination_reachable)
			{
				IHM_leds_send_msg(1,(led_ihm_t){LED_COLOR_IHM, ON});
				state = IDLE;
			}
			break;
		case IDLE:
			if(!IHM_switchs_get(SWITCH_XBEE))//On change l'état du switch XBee...après l'avoir mis à ON !
			{
				current_destination_reachable = FALSE;
				state = WAIT_XBEE_REACHABLE;
			}
			break;
		default:
			break;
	}

	/* Gestion du selftest*/
	SELFTEST_process_main();

	#if USE_XBEE_OLD
		if(IHM_switchs_get(SWITCH_XBEE))
		{
			CAN_over_XBee_process_main();
			current_destination_reachable = XBee_is_destination_reachable();
		}
	#endif

	/* Mise à jour des informations affichées à l'écran*/
	#if USE_LCD
		LCD_processMain();
	#endif

	SD_process_main();

	BUZZER_process_main();
}











