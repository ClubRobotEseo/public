/*
 *	Club Robot ESEO 2008 - 2010
 *	Archi'Tech, CHOMP
 *
 *	Fichier : environment.c
 *	Package : Carte Principale
 *	Description : 	Fonctions de supervision de l'environnement
 *	Auteur : Jacen, Nirgal
 *	Version 20100420, modifs 201304
 */

#include "environment.h"

#include "Supervision/Buffer.h"
#include "Supervision/RTC.h"
#include "Supervision/Selftest.h"
#include "Supervision/Supervision.h"
#include "Supervision/Buzzer.h"
#include "Supervision/SD/term_io.h"
#include "Supervision/SD/SD.h"
#include "Supervision/LCD_interface.h"

#include "QS/QS_outputlog.h"
#include "QS/QS_can_over_uart.h"
#include "QS/QS_can_over_xbee.h"
#include "QS/QS_lowLayer/QS_can.h"
#include "QS/QS_lowLayer/QS_adc.h"
#include "QS/QS_lowLayer/QS_uart.h"
#include "QS/QS_maths.h"
#include "QS/QS_can_verbose.h"
#include "QS/QS_IHM.h"
#include "QS/QS_lowLayer/QS_ports.h"
#include "QS/QS_simulator/QS_simulator.h"

#include "actuator/act_functions.h"
#include "fix_beacon.h"
#include "button.h"
#include "elements.h"
#include "zones.h"
#include "propulsion/prop_functions.h"
#include "led_manager.h"
#include "scan.h"
#include "strats_2019/actions_both_generic.h"
#include "strats_2019/actions_prop.h"


/* met � jour l'environnement en fonction du message CAN re�u */
void CAN_update (CAN_msg_t* incoming_msg);

/* met a jour la position a partir d'un message asser la d�livrant */
void ENV_pos_update (CAN_msg_t* msg);

void ENV_process_can_msg_sent(CAN_msg_t * sent_msg);

void ENV_clean (void);

/* Regarde si les switchs � risque sont activ�s et mets des avertissements (LED,LCD) */
void ENV_warning_switch();

#define ADC_THRESHOLD 10 //Valeur de l'ADC sans dispositif de connect�

/* initialise les variables d'environnement */
void ENV_init(void)
{
	Uint8 i;
	CAN_init();
	CAN_set_send_callback(ENV_process_can_msg_sent);
	BUTTON_init();
	DETECTION_init();

	ENV_clean();
	global.color = COLOR_INIT_VALUE; //update -> color = wanted + dispatch
	global.flags.match_started = FALSE;
	global.flags.match_over = FALSE;
	global.flags.match_suspended = FALSE;
	global.flags.match_paused = FALSE;
	global.flags.ask_suspend_match = FALSE;
	global.flags.ask_start = FALSE;
	global.flags.alim = FALSE;
	global.flags.aru = TRUE;
	global.flags.foes_updated_for_lcd = FALSE;
	global.flags.initial_position_received = FALSE;
	global.flags.go_to_home = FALSE;
	global.flags.virtual_mode = FALSE;
	global.friend_position_lifetime = 0;
	global.friend_pos.x = 0;
	global.friend_pos.y = 0;

	for(i=0;i<MAX_NB_FOES;i++)
	{
		global.foe[i].fiability_error = 0;
		global.foe[i].enable = FALSE;
		global.foe[i].update_time = 0;
	}
	global.match_time = 0;
	global.pos.dist = 0;
	global.prop.calibrated = FALSE;
	global.prop.pos_updated = FALSE;
	global.prop.current_way = ANY_WAY;
	global.prop.is_in_translation = FALSE;
	global.prop.is_in_rotation = FALSE;
	global.prop.current_status = NO_ERROR;
	global.prop.prop_memory_startup_check_state = PROP_MEMORY_STARTUP_CHECK__UNKNOW;
	global.alim_value = 0;
	global.destination = (GEOMETRY_point_t){0,0};
	for(i=0;i<PROPULSION_NUMBER_COEFS;i++)
		global.debug.propulsion_coefs[i] = 0;
	global.com.reach_point_get_out_init = FALSE;

	FIX_BEACON_init();
}



void ENV_check_filter(CAN_msg_t * msg, bool_e * bUART_filter, bool_e * bCAN_filter, bool_e * bSAVE_filter)
 {
	//static time32_t filter_beacon_ir = 0;
	static time32_t filter_broadcast_position = 0;

	*bUART_filter = TRUE;	//On suppose que le message est autoris�.
	*bSAVE_filter = TRUE;

	switch(msg->sid)
	{
		//FILTRAGE POUR NE PAS ETRE SPAMMES PAR LE MESSAGE DE POSITION_ROBOT....
		case BROADCAST_POSITION_ROBOT:
			//si le message est porteur d'un warning, on ne le filtre pas.
			if((msg->data.broadcast_position_robot.reason & (WARNING_TRANSLATION | WARNING_ROTATION | WARNING_TIMER)))	//Si le message ne porte pas de warning : on filtre.
			{
				//On ne propage pas les messages de BROADCAST_POSITION_ROBOT (dans le cas o� les raisons ne sont pas des WARN).
				*bSAVE_filter = FALSE;
				//Traitement sp�cial pour les messages d'asser position : maxi 1 par seconde !
				if(global.absolute_time-1000>filter_broadcast_position)
					filter_broadcast_position=global.absolute_time;
				else
					*bUART_filter = FALSE;	//Ca passe pas...
			}

			break;
		case BROADCAST_ADVERSARIES_POSITION:
			*bUART_filter = FALSE;	//Ca passe pas... 				(mieux vaut carr�ment afficher 	ponctuellement les infos qui d�coulent de ce message)
			*bSAVE_filter = FALSE;	//Pas d'enregistrement non plus	(mieux vaut carr�ment sauver 	ponctuellement les infos qui d�coulent de ce message)
			break;
		case BROADCAST_BEACON_ADVERSARY_POSITION_IR:
			//if(global.absolute_time-1000>filter_beacon_ir)
			//	filter_beacon_ir=global.absolute_time;
			//else
			*bUART_filter = FALSE;	//Ca passe pas...
			*bSAVE_filter = FALSE;
			break;
		case DEBUG_AVOIDANCE_POLY:
			*bUART_filter = FALSE;	//Ca passe pas...
			*bSAVE_filter = FALSE;
			break;

		case XBEE_COMMUNICATION_AVAILABLE:
			*bUART_filter = FALSE;	//Ca passe pas...
			*bSAVE_filter = FALSE;
			break;

		case XBEE_COMMUNICATION_RESPONSE:
			*bUART_filter = FALSE;	//Ca passe pas...
			*bSAVE_filter = FALSE;
			break;

		default:
			//Message autoris�.
			break;
	}

	if	( ((msg->sid & 0xF00) == STRAT_FILTER) ||  ((msg->sid & 0xF00) == XBEE_FILTER) || ((msg->sid & 0xF00) == BROADCAST_FILTER))
		*bCAN_filter = FALSE;	//On n'envoie pas sur le bus CAN des messages qui nous sont destin�s uniquement.
	else
		*bCAN_filter = TRUE;	//Seuls les messages DEBUG, ou destin�s aux cartes PROPULSION, ACTIONNEUR, BALISES, IHM sont transmis sur le bus can.

	if(SIMU_FILTER(msg->sid)) {
		*bCAN_filter = FALSE;
	}
}


void ENV_process_can_msg(CAN_msg_t * incoming_msg, bool_e bCAN, bool_e bU1, bool_e bU2, bool_e bXBee)
{
	bool_e bUART_filter = FALSE;	//bUART_filter indique si ce message est filtr� ou s'il doit �tre propag� sur les uarts...
	bool_e bCAN_filter = FALSE;		//bCAN_filter indique si ce message doit �tre propag� sur le CAN...o� s'il nous �tait directement destin�...
	bool_e bSAVE_filter = FALSE;	//bEEPROM_filter indique si ce message doit �tre enregistr� en EEPROM

	CAN_update(incoming_msg);

	// Le ToolKit utilise l'uart 1 pour communiquer (le boolean est en �tat inverse)
	SIMULATOR_processMain(incoming_msg, !bU1);

	ENV_check_filter(incoming_msg, &bUART_filter, &bCAN_filter, &bSAVE_filter);

	//Enregistrement du message CAN.
	BUFFER_add(incoming_msg);						//BUFFERISATION

	if(bSAVE_filter)
	{
		#if SD_ENABLE
			source_e source;
			if(bCAN == FALSE)
				source = FROM_BUS_CAN;
			else if(bU1 == FALSE)
				source = FROM_UART1;
			else if(!IHM_switchs_get(SWITCH_XBEE))
				source = FROM_UART2;
			else
				source = FROM_XBEE;
			SD_new_event(source, incoming_msg, NULL, TRUE);
		#endif
	}

	//Propagation du message CAN.
	#if (!DISABLE_CAN)
		if(bCAN && bCAN_filter)
			CAN_send(incoming_msg);
	#endif

	if(bUART_filter)
	{
		#if USE_XBEE_OLD
			if(bXBee && IHM_switchs_get(SWITCH_XBEE))
			{
				if((incoming_msg->sid & 0xF00) == XBEE_FILTER)
					CANMsgToXbee(incoming_msg,TRUE);	//Envoi en BROADCAST... aux modules joignables
			}
		#endif

		QS_CAN_VERBOSE_can_msg_print(incoming_msg, VERB_INPUT_MSG);


		if(!IHM_switchs_get(SWITCH_XBEE) && bU2)
		{
			//D�sactiv� parce qu'on en a pas besoin...
			//CANmsgToU2tx(incoming_msg);
		}
	}
}

//Traite les messages que nous venons juste d'envoyer sur le bus can (via un CAN_send)
void ENV_process_can_msg_sent(CAN_msg_t * sent_msg)
{

	BUFFER_add(sent_msg);	//BUFFERISATION

	#if SD_ENABLE
		SD_new_event(TO_BUSCAN, sent_msg, NULL, TRUE);
	#endif

	//UART1
	QS_CAN_VERBOSE_can_msg_print(sent_msg, VERB_OUTPUT_MSG);

	//UART2 - d�sactiv� volontairement -> parce qu'on en a pas besoin...
	//if(IHM_switchs_get(SWITCH_RAW_DATA)  && !IHM_switchs_get(SWITCH_XBEE))
	//	CANmsgToU2tx(sent_msg);

	//Messages de BROADCAST transmis aussi � la balise m�re.
	switch(sent_msg->sid)
	{
		case BROADCAST_START:
			//no break;
		case BROADCAST_STOP_ALL:
			//no break;
		case BROADCAST_COULEUR:
			CANMsgToXBeeDestination(sent_msg, BALISE_MERE);
			break;
		default:
			break;
	}
}



void ENV_update(void){

	CAN_msg_t incoming_msg_from_bus_can;
	static CAN_msg_t can_msg_from_uart1;
	static CAN_msg_t can_msg_from_uart2;
	static time32_t last_time_tell_position = 0;
	char c;

	// RAZ des drapeaux temporaires pour la prochaine it�ration
	ENV_clean();

	if(global.flags.initial_position_received == FALSE && last_time_tell_position != 0 && global.absolute_time - last_time_tell_position > 500){
		CAN_send_sid(PROP_TELL_POSITION);
		last_time_tell_position = global.absolute_time;
	}

	// R�cuperation de l'�volution de l'environnement renseignee par les messages CAN
	while (CAN_data_ready())
	{
		toggle_led(LED_CAN);
		incoming_msg_from_bus_can = CAN_get_next_msg();
		ENV_process_can_msg(&incoming_msg_from_bus_can,FALSE, TRUE, TRUE, FALSE);	//Everywhere except CAN
	}


	while(UART1_data_ready())
	{
		static bool_e parseInProgress;

		c = UART1_get_next_msg();

		if(u1rxToCANmsg(&can_msg_from_uart1, c, &parseInProgress))
		{
			ENV_process_can_msg(&can_msg_from_uart1,TRUE, FALSE, TRUE, TRUE);	//Everywhere except U1.
		}
		else
		{
			if(parseInProgress == FALSE)	//si pas de message valide maintenant ET QUE pas de parse en cours, c'est qu'on re�oit du texte !
				char_from_user(c);
		}


	}


#define INTERVAL_ASK_TIME 1000  //Temps entre deux demandes d'�tat de la communication

#if USE_XBEE_OLD
	if(IHM_switchs_get(SWITCH_XBEE))
	{
		if(XBeeToCANmsg(&can_msg_from_uart2))
			ENV_process_can_msg(&can_msg_from_uart2,TRUE, TRUE, FALSE, FALSE);	//Everywhere except U2 and XBee.

#if 0	//non seulement c'est pas beau, mais en plus on s'en sert pas...

		//Mise � jour du flag sur l'�tat de la communication XBEE
		static error_e result = NOT_HANDLED;
		static time32_t time = 0;
		if(result == IN_PROGRESS)
		{
			result = ELEMENTS_check_communication(NULL);
		}
		else if(result == END_WITH_TIMEOUT && ((time + INTERVAL_ASK_TIME) < global.absolute_time) )
		{
			result = ELEMENTS_check_communication(NULL);
			time = global.absolute_time;
		}
		else if(result == END_OK && ((time + INTERVAL_ASK_TIME) < global.absolute_time) )
		{
			result = ELEMENTS_check_communication(NULL);
			time = global.absolute_time;
		}
		else if(result == NOT_HANDLED)
		{
			time = global.absolute_time;
			result = ELEMENTS_check_communication(NULL);
		}
#endif
	}

	//Mise � jour des �l�ments
	ELEMENTS_process_main();
#endif


	/* R�cup�ration des donn�es des boutons */
	BUTTON_update();

	if (!global.flags.match_started)
	{
		color_e current_color = (color_e)(IHM_switchs_get(SWITCH_COLOR));
		if(current_color != global.color)	//Check the Color switch
		{
			ENV_set_color(current_color);
		}
	}
}

/* met � jour l'environnement en fonction du message CAN re�u */
void CAN_update (CAN_msg_t* incoming_msg)
{
	date_t date;
//****************************** Messages carte supervision *************************/
	switch (incoming_msg->sid)
	{
		case BROADCAST_POSITION_ROBOT:	   //Les raisons seront ensuite traitees dans la tache de fond
		case STRAT_ROBOT_FREINE:
		case PROP_ROBOT_CALIBRE:
		case STRAT_PROP_ERREUR:
		case STRAT_TRAJ_FINIE:
		case STRAT_POS_UPDATED:
			ENV_pos_update(incoming_msg);	//Tout ces messages contiennent une position... et d'autres infos communes
		break;
		default:
		break;
	}

	switch (incoming_msg->sid)
	{
//****************************** Messages venant des geeks du club robot  *************************/
		case DEBUG_RTC_SET:
			date.seconds 	= incoming_msg->data.debug_rtc_set.seconde;
			date.minutes 	= incoming_msg->data.debug_rtc_set.minute;
			date.hours 		= incoming_msg->data.debug_rtc_set.heure;
			date.day 		= incoming_msg->data.debug_rtc_set.journee;
			date.date 		= incoming_msg->data.debug_rtc_set.jour;
			date.month 		= incoming_msg->data.debug_rtc_set.mois;
			date.year 		= incoming_msg->data.debug_rtc_set.annee;
			RTC_set_time(&date);
			RTC_print_time();
			RTC_can_send();	//Retour ... pour v�rifier que ca a fonctionn�..
			break;
		case DEBUG_RTC_GET:
			RTC_print_time();
			RTC_can_send();
			break;
		case DEBUG_RTC_TIME:
			RTC_print_time();
			break;
		case DEBUG_SELFTEST_LAUNCH:
			SELFTEST_ask_launch();
			break;
		case STRAT_BUZZER_PLAY:
			BUZZER_play(incoming_msg->data.strat_buffer_play.duration,
						BUZZER_convert_enum_QS(incoming_msg->data.strat_buffer_play.note),
						incoming_msg->data.strat_buffer_play.nb_bip);
			break;
//****************************** Messages carte propulsion/asser *************************/
		case STRAT_PROP_BOARD_REBOOTED:
			if(SIMULATOR_get_virtual_mode_for_prop())
				SIMULATOR_refresh_simu_mode();
			break;
		case STRAT_TRAJ_FINIE:
			global.prop.ended = TRUE;
			break;
		case STRAT_PROP_FOE_DETECTED:
			PROP_set_detected_foe(incoming_msg);
			break;
		case STRAT_PROP_ERREUR:
			global.prop.error = TRUE;
			break;
		case STRAT_PROP_MEMORY_STARTUP_CHECK:
			global.prop.prop_memory_startup_check_state = incoming_msg->data.strat_prop_memory_startup.state;
			break;
		case PROP_ROBOT_CALIBRE:
			global.prop.calibrated = TRUE;
			break;
		case STRAT_POS_UPDATED:
			global.prop.pos_updated = TRUE;
			break;
		case DEBUG_PROPULSION_COEF_IS:
			if(incoming_msg->data.debug_propulsion_coef_is.id < PROPULSION_NUMBER_COEFS)
			{
				global.debug.propulsion_coefs_updated |=  (Uint32)(1) << incoming_msg->data.debug_propulsion_coef_is.id;
				global.debug.propulsion_coefs[incoming_msg->data.debug_propulsion_coef_is.id] = incoming_msg->data.debug_propulsion_coef_is.value;
			}
			break;
		case DEBUG_PROPULSION_SET_COEF_ANSWER:
			stratAction_memoryWriteCoefState(incoming_msg->data.debug_propulsion_set_coef_answer.state);
			break;
		case BROADCAST_POSITION_ROBOT:
			//ATTENTION : Pas de switch car les raisons peuvent �tre cumul�es !!!
			//Les raisons WARNING_TRANSLATION, WARNING_ROTATION, WARNING_NO et WARNING_TIMER ne font rien d'autres que d�clencher un ENV_pos_update();

			if((incoming_msg->data.broadcast_position_robot.reason & (WARNING_REACH_X | WARNING_REACH_Y)) == (WARNING_REACH_X | WARNING_REACH_Y)){ //Nous venons d'atteindre une position en END_AT_DISTANCE (ie la position � 100mm pr�s) pour laquelle on a demand� une surveillance � la propulsion.
				global.prop.reach_distance = TRUE;
				debug_printf("Rd\n");
			}else{
				if(incoming_msg->data.broadcast_position_robot.reason & WARNING_REACH_X)	//Nous venons d'atteindre une position en X pour laquelle on a demand� une surveillance � la propulsion.
				{
					global.prop.reach_x = TRUE;
					debug_printf("Rx\n");
				}

				if(incoming_msg->data.broadcast_position_robot.reason & WARNING_REACH_Y)	//Nous venons d'atteindre une position en Y pour laquelle on a demand� une surveillance � la propulsion.
				{
					global.prop.reach_y = TRUE;
					debug_printf("Ry\n");
				}
			}

			if(incoming_msg->data.broadcast_position_robot.reason & WARNING_REACH_TETA)	//Nous venons d'atteindre une position en Teta pour laquelle on a demand� une surveillance � la propulsion.
			{
				global.prop.reach_teta = TRUE;
				debug_printf("Rt\n");
			}

			break;
		case STRAT_ROBOT_FREINE:
			global.prop.brake = TRUE;
			break;
		case DEBUG_TRAJECTORY_FOR_TEST_COEFS_DONE:
			global.debug.duration_trajectory_for_test_coefs = incoming_msg->data.debug_trajectory_for_test_coefs_done.duration;
			break;
		case STRAT_SEND_REPORT:
			//LCD_printf(1, FALSE, FALSE, "Dist:%d", incoming_msg->data.strat_send_report.actual_trans << 1);
			//LCD_printf(2, FALSE, FALSE, "Rot :%4d MRot:%4d", (incoming_msg->data.strat_send_report.actual_rot << 3)*180/PI4096, (incoming_msg->data.strat_send_report.max_rot << 3)*180/PI4096);
			break;

		case STRAT_SCAN_DISTRI_RESULT:
			SCAN_receivedCanMsg(incoming_msg);
			break;
		case STRAT_SCAN_DISTRI_HOKUYO_RESULT:
			SCAN_HOKUYO_DISTRI(incoming_msg);
			break;
//****************************** Messages de la carte actionneur *************************/
		case STRAT_ACT_BOARD_REBOOTED:
			if(SIMULATOR_get_virtual_mode_for_act())
				SIMULATOR_refresh_simu_mode();
			break;

		case ACT_RESULT:
			ACT_process_result(incoming_msg);
			break;

		case ACT_ACKNOWLEDGE:
			ACT_process_acknowledge(incoming_msg);
			break;

		case STRAT_INFORM_CAPTEUR:
			ACT_sensor_answer(incoming_msg);
			break;

		case ACT_GET_CONFIG_ANSWER:
			ACT_get_config_answer(incoming_msg);
			break;

		case ACT_WARNER_ANSWER:
			ACT_warner_answer(incoming_msg);
			break;

		case ACT_EXPANDER_TELL_PUMP_STATUS:
			ACT_EXPANDER_receiveVacuostat_msg(incoming_msg);
			break;

		case ACT_EXPANDER_TELL_DISTANCE_SENSOR:
			ACT_EXPANDER_receiveDistanceSensorMsg(incoming_msg);
			break;

		case ACT_EXPANDER_TELL_COLOR_SENSOR:
			ACT_EXPANDER_receiveColorSensorMsg(incoming_msg);
			break;

/************************************ R�cup�ration des donn�es de la balise *******************************/
		case BROADCAST_BEACON_ADVERSARY_POSITION_IR:
			//En absence d'hokuyo et du fonctionnement correct de la carte propulsion, les msg balises IR sont tr�s important pour l'�vitement.
			DETECTION_pos_foe_update(incoming_msg);
			//SELFTEST_update_led_beacon(incoming_msg);
			break;
		case BROADCAST_ADVERSARIES_POSITION:
			DETECTION_pos_foe_update(incoming_msg);
			break;
/************************************* R�cup�ration des envois de l'autre robot ***************************/
		case XBEE_START_MATCH:
			if(global.prop.calibrated)	//V�rification pour �viter de lancer un match si on est pas "sur le terrain"...et pr�s � partir.
				global.flags.ask_start = TRUE;	//Inconv�nient : il FAUT �tre calibr� pour lancer un match � partir de l'autre robot.
			break;
		case XBEE_PING:
			//if(incoming_msg->data.xbee_ping.module_id == MODULE_3)
				//LED_is_alive();
			//On recoit un ping, on r�pond par un PONG.
			//Le lien est �tabli
			//Le module QS_CanoverXBee se d�brouille pour PONGer
			break;
		case XBEE_PONG:
			//if(incoming_msg->data.xbee_ping.module_id == MODULE_3)
				//LED_is_alive();
			//On re�oit un pong, tant mieux, le lien est �tabli
			break;
		case XBEE_LED_IS_ALIVE:
			//LED_is_alive();
			event_new_xbee_msg_from_pannel_received(incoming_msg);
			break;
		case DEBUG_DETECT_FOE:
			global.debug.force_foe = TRUE;
			break;

		case XBEE_ZONE_COMMAND:
			ZONE_CAN_process_msg(incoming_msg);
			break;

		case XBEE_REACH_POINT_GET_OUT_INIT:
			global.com.reach_point_get_out_init = TRUE;
			break;

		case XBEE_MY_POSITION_IS:
			if(QS_WHO_AM_I_get() == SMALL_ROBOT)
			{
				if(incoming_msg->data.xbee_my_position_is.robot_id == BIG_ROBOT)
				{
					global.friend_position_lifetime = 4000;		//Dur�e de vie pour cette donn�e. (attention � la latence du XBEE)
					global.friend_pos.x = incoming_msg->data.xbee_my_position_is.x;
					global.friend_pos.y = incoming_msg->data.xbee_my_position_is.y;
					//PEARL_set_state_black_for_com(incoming_msg);
				}
			}
			break;

		case XBEE_SYNC_ELEMENTS_FLAGS:
#if USE_SYNC_ELEMENTS
			ELEMENTS_receive_flags(incoming_msg);
#endif
			break;

		case XBEE_COMMUNICATION_AVAILABLE:{
			CAN_msg_t answer;
			answer.sid = XBEE_COMMUNICATION_RESPONSE;
			answer.size = 0;
			CANMsgToXbee(&answer,FALSE);
			break;}

		case XBEE_COMMUNICATION_RESPONSE:
			ELEMENTS_check_communication(incoming_msg);
			break;

		case XBEE_ELEMENTS_HARDFLAGS:
			#if USE_HARDFLAGS
				ELEMENTS_receive_hardflags(incoming_msg);
			#endif
			break;




/************************************* R�cup�ration des messages li�s au selftest ***************************/
		case STRAT_BEACON_SELFTEST_DONE :
		case STRAT_ACT_SELFTEST_DONE :
		case STRAT_PROP_SELFTEST_DONE :
		case STRAT_IHM_SELFTEST_DONE:
		case STRAT_ACT_PONG:
		case STRAT_PROP_PONG:
		case STRAT_BEACON_PONG:
		case STRAT_IHM_PONG:
			SELFTEST_update(incoming_msg);
			break;
/************************************* R�cup�ration des messages de la balise fixe ***************************/
		case STRAT_ZONE_INFOS:
			FIX_BEACON_process_msg(incoming_msg);
			break;

/************************************* R�cup�ration des messages de la carte IHM ***************************/
		case IHM_SWITCH_ALL:
		case IHM_BUTTON:
		case IHM_SWITCH:
			IHM_process_main(incoming_msg);
			ENV_warning_switch();
			break;
		case IHM_BIROUTE_IS_REMOVED:
			global.flags.ask_start = TRUE;
			break;
		case BROADCAST_ALIM:
			global.alim_value = incoming_msg->data.broadcast_alim.battery_value;

			if(!global.flags.match_started || global.flags.match_over) {
				if(incoming_msg->data.broadcast_alim.state & (BATTERY_ENABLE | BATTERY_LOW))
					global.flags.alim = TRUE;
				else if(incoming_msg->data.broadcast_alim.state & BATTERY_DISABLE)
					global.flags.alim = FALSE;

				if(incoming_msg->data.broadcast_alim.state & ARU_ENABLE)
					global.flags.aru = TRUE;
				else if(incoming_msg->data.broadcast_alim.state & ARU_DISABLE)
					global.flags.aru = FALSE;
			}
			break;

/************************************* R�cup�ration des messages de la simulation ***************************/
		case IHM_GET_SWITCH:
			CAN_send_sid(IHM_GET_SWITCH);
			break;

		default:
			break;
	}
	return;
}


/* mise � jour de la position re�ue dans l'un des messages de la propulsion.*/
void ENV_pos_update (CAN_msg_t* msg)
{
	Sint16 cosinus, sinus;
	global.pos.x = msg->data.broadcast_position_robot.x;
	global.pos.y = msg->data.broadcast_position_robot.y;
	global.pos.angle = msg->data.broadcast_position_robot.angle;
	COS_SIN_4096_get(global.pos.angle, &cosinus, &sinus);
	global.pos.cosAngle = cosinus;
	global.pos.sinAngle = sinus;
	global.prop.last_time_pos_updated = global.match_time;
	global.prop.current_way = msg->data.broadcast_position_robot.way;
	global.prop.current_status = msg->data.broadcast_position_robot.error;
	global.prop.is_in_translation = msg->data.broadcast_position_robot.in_translation;
	global.prop.is_in_rotation = msg->data.broadcast_position_robot.in_rotation;

	if(msg->sid == STRAT_ROBOT_FREINE || msg->sid == STRAT_PROP_ERREUR || msg->sid == STRAT_TRAJ_FINIE)
		global.prop.idTrajActual = msg->data.broadcast_position_robot.idTraj;

	global.flags.initial_position_received = TRUE;
	global.pos.updated = TRUE;
	//debug_printf("Position update to x=%d y=%d teta=%ld\n", global.pos.x, global.pos.y, global.pos.angle);
}



/* Appel�e en d�but de tache de fond : baisse les drapeaux d'environnement pour pr�parer la prochaine MaJ */
void ENV_clean (void)
{
	DETECTION_clean();
	global.debug.propulsion_coefs_updated = 0x00000000;
	global.flags.ask_start = FALSE;
	global.flags.ask_suspend_match = FALSE;
	global.prop.ended = FALSE;
	global.prop.error = FALSE;
	global.prop.brake = FALSE;
	global.prop.reach_x = FALSE;
	global.prop.reach_y = FALSE;
	global.prop.reach_teta = FALSE;
	global.prop.reach_distance = FALSE;
	global.pos.updated = FALSE;
	global.debug.force_foe = FALSE;
	global.debug.duration_trajectory_for_test_coefs = 0;
	FIX_BEACON_clean();	//Doit �tre apr�s le any_match !

	ACT_reset_all_warner(); // Reset de tous les warner actionneurs
}

/* envoie un message CAN BROADCAST_COULEUR � jour */
void ENV_set_color(color_e color)
{
	/* changer la couleur */
	global.color = color;
	global.prop.calibrated = FALSE;	//si c'�tait le cas, le robot n'est plus calibr�, on SET sa position !
	/* indiquer au monde la nouvelle couleur */
	CAN_msg_t msg;
	msg.sid = BROADCAST_COULEUR;
	msg.size = SIZE_BROADCAST_COULEUR;
	msg.data.broadcast_couleur.color = color;
	CAN_send(&msg);

	IHM_set_led_color((color == BOT_COLOR)?LED_COLOR_YELLOW:LED_COLOR_MAGENTA);
}

void ENV_warning_switch(){
	static bool_e previous_asser_switch = FALSE;
	bool_e current_asser_switch;

	current_asser_switch = IHM_switchs_get(SWITCH18_DISABLE_ASSER);
	if(current_asser_switch != previous_asser_switch){
		CAN_msg_t msg;
		msg.sid = IHM_SET_ERROR;
		msg.size = SIZE_IHM_SET_ERROR;
		msg.data.ihm_set_error.error = IHM_ERROR_ASSER;
		msg.data.ihm_set_error.state = current_asser_switch;
		CAN_send(&msg);
	}
	previous_asser_switch = current_asser_switch;
}

