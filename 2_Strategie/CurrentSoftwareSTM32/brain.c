/*
 *	Club Robot ESEO 2008 - 2012
 *	Archi'Tech, CHOMP, CheckNorris, Shark & Fish
 *
 *	Fichier : brain.c
 *	Package : Carte Principale
 *	Description : Fonctions de g�n�ration des ordres
 *	Auteur : Jacen, modifi� par Gonsevi
 *	Version 2012/01/14
 */

#include "brain.h"

#include "clock.h"
#include "button.h"	//pour SWITCH_change_color
#include "environment.h"
#include "Supervision/Selftest.h"
#include "Supervision/RTC.h"
#include "Supervision/SD/SD.h"
#include "propulsion/pathfind.h"
#include "propulsion/astar.h"
#include "QS/QS_CANmsgList.h"
#include "QS/QS_can_over_xbee.h"
#include "QS/QS_who_am_i.h"
#include "QS/QS_outputlog.h"
#include "QS/QS_IHM.h"
#include "QS/QS_lowLayer/QS_sys.h"
#include "Supervision/Supervision.h"
#include "Supervision/Buzzer.h"
#include "utils/actionChecker.h"
#include "zones.h"
#include "elements.h"
#include "avoidance.h"
#include "Supervision/Selftest.h"
#include "actuator/act_functions.h"

#include "strats_2019/score.h"
#include "strats_2019/high_level_strat.h"
#include "strats_2019/actions_both_generic.h"
#include "strats_2019/actions_prop.h"
#include "strats_2019/inutile/strat_inutile.h"


//Strat�gie par d�faut... (modifiable par les codeurs qui auraient la flemme ou l'impossibilit� de configurer leur strat sur le LCD � chaque reset...)
//							Valeur souhaitable pour le commit SVN : high_level_strat


#define DEFAULT_STRAT_BIG		high_level_strat

#define DEFAULT_STRAT_SMALL		high_level_strat


static ia_fun_t strategy;
static char *strategy_name;
static time32_t match_duration = MATCH_DURATION;
static time32_t time_end_of_match;
static Sint8 index_strategy = -1;
static Uint8 number_of_strategy = 0;
static Uint8 number_of_displayed_strategy = 0;
static bool_e strat_updated = TRUE;

static void BRAIN_update_match_duration(void);
static void BRAIN_action_in_match(void);
static void BRAIN_action_at_end_of_match(void);
static void BRAIN_action_after_end_of_match(void);

typedef enum{
	BIG,
	SMALL,
	BOTH
}robot_type_e;

typedef struct{
	char *name;
	ia_fun_t function;
	time32_t match_duration;
	bool_e display_on;
	robot_type_e robot_type;
}strategy_list_s;

static const strategy_list_s list_strategy[] = {

	// Pour les deux robots
	//display name				name function								// match duration	// afficher sur le LCD	// strat�gie pour quel robot BIG/SMALL/BOTH(les deux)
	{"high_level_strat",		high_level_strat,							MATCH_DURATION,		TRUE,					BOTH},
	{"strat_odo_rot",			strat_reglage_odo_rotation,					0,					TRUE,					BOTH},
	{"strat_odo_tra",			strat_reglage_odo_translation,				0,					TRUE,					BOTH},
	{"strat_odo_sym",			strat_reglage_odo_symetrie,					0,					TRUE,					BOTH},
	{"strat_prop",				strat_reglage_prop,							0,					TRUE,					BOTH},

	// Pour Big robot
	{"strat_alexy",				alexy_strat_inutile_big,					MATCH_DURATION,		TRUE,					BIG},
	{"strat_arnaud",			arnaud_strat_inutile_big,					MATCH_DURATION,		TRUE,					BIG},
	{"strat_cailyn",			cailyn_strat_inutile_big,					MATCH_DURATION,		TRUE,					BIG},
	{"strat_corentin",			corentin_strat_inutile_big,					MATCH_DURATION,		TRUE,					BIG},
	{"strat_cyrille",			cyrille_strat_inutile_big,					MATCH_DURATION,		TRUE,					BIG},
	{"strat_damien",			damien_strat_inutile_big,					MATCH_DURATION,		TRUE,					BIG},
	{"strat_guillaumBe",		guillaumeBe_strat_inutile_big,				MATCH_DURATION,		TRUE,					BIG},
	{"strat_guillaumDu",		guillaumeDu_strat_inutile_big,				MATCH_DURATION,		TRUE,					BIG},
	{"strat_guillaumMa",		guillaumeMa_strat_inutile_big,				MATCH_DURATION,		TRUE,					BIG},
	{"strat_julien",			julien_strat_inutile_big,					MATCH_DURATION,		TRUE,					BIG},
	{"strat_lucien",			lucien_strat_inutile_big,					MATCH_DURATION,		TRUE,					BIG},
	{"strat_marion",			marion_strat_inutile_big,					MATCH_DURATION,		TRUE,					BIG},
	{"strat_mathilde",			mathilde_strat_inutile_big,					MATCH_DURATION,		TRUE,					BIG},
	{"strat_romain",			romain_strat_inutile_big,					MATCH_DURATION,		TRUE,					BIG},
	{"strat_romane",			romane_strat_inutile_big,					MATCH_DURATION,		TRUE,					BIG},
	{"strat_paul",				paul_strat_inutile_big,						MATCH_DURATION,		TRUE,					BIG},
	{"strat_pierre_louis",		pierre_louis_strat_inutile_big,				MATCH_DURATION,		TRUE,					BIG},
	{"strat_pierre",			pierre_strat_inutile_big,					MATCH_DURATION,		TRUE,					BIG},
	{"strat_samuel",			samuel_strat_inutile_big,					MATCH_DURATION,		TRUE,					BIG},
	{"strat_thomas",			thomas_strat_inutile_big,					MATCH_DURATION,		TRUE,					BIG},
	{"strat_valentin",			valentin_strat_inutile_big,					MATCH_DURATION,		TRUE,					BIG},

	// Pour Small robot
	{"strat_alexy",				alexy_strat_inutile_small,					MATCH_DURATION,		TRUE,					SMALL},
	{"strat_arnaud",			arnaud_strat_inutile_small,					MATCH_DURATION,		TRUE,					SMALL},
	{"strat_cailyn",			cailyn_strat_inutile_small,					MATCH_DURATION,		TRUE,					SMALL},
	{"strat_corentin",			corentin_strat_inutile_small,				MATCH_DURATION,		TRUE,					SMALL},
	{"strat_cyrille",			cyrille_strat_inutile_small,				MATCH_DURATION,		TRUE,					SMALL},
	{"strat_damien",			damien_strat_inutile_small,					MATCH_DURATION,		TRUE,					SMALL},
	{"strat_guillaumBe",		guillaumeBe_strat_inutile_small,			MATCH_DURATION,		TRUE,					SMALL},
	{"strat_guillaumDu",		guillaumeDu_strat_inutile_small,			MATCH_DURATION,		TRUE,					SMALL},
	{"strat_guillaumMa",		guillaumeMa_strat_inutile_small,			MATCH_DURATION,		TRUE,					SMALL},
	{"strat_julien",			julien_strat_inutile_small,					MATCH_DURATION,		TRUE,					SMALL},
	{"strat_lucien",			lucien_strat_inutile_small,					MATCH_DURATION,		TRUE,					SMALL},
	{"strat_marion",			marion_strat_inutile_small,					MATCH_DURATION,		TRUE,					SMALL},
	{"strat_mathilde",			mathilde_strat_inutile_small,				MATCH_DURATION,		TRUE,					SMALL},
	{"strat_romain",			romain_strat_inutile_small,					MATCH_DURATION,		TRUE,					SMALL},
	{"strat_romane",			romane_strat_inutile_small,					MATCH_DURATION,		TRUE,					SMALL},
	{"strat_paul",				paul_strat_inutile_small,					MATCH_DURATION,		TRUE,					SMALL},
	{"strat_pierre_louis",		pierre_louis_strat_inutile_small,			MATCH_DURATION,		TRUE,					SMALL},
	{"strat_pierre",			pierre_strat_inutile_small,					MATCH_DURATION,		TRUE,					SMALL},
	{"strat_samuel",			samuel_strat_inutile_small,					MATCH_DURATION,		TRUE,					SMALL},
	{"strat_thomas",			thomas_strat_inutile_small,					MATCH_DURATION,		TRUE,					SMALL},
	{"strat_valentin",			valentin_strat_inutile_small,				MATCH_DURATION,		TRUE,					SMALL},

};
#define MAX_NB_STRAT_BY_ROBOT	50
static uint8_t list_displayed_strategy[MAX_NB_STRAT_BY_ROBOT];

void BRAIN_init(void){
	Uint8 i;
	bool_e found = FALSE;
	if(QS_WHO_AM_I_get() == BIG_ROBOT)
		strategy = DEFAULT_STRAT_BIG;
	else
		strategy = DEFAULT_STRAT_SMALL;

	number_of_strategy = sizeof(list_strategy)/sizeof(strategy_list_s);
	index_strategy = -1;
	number_of_displayed_strategy = 0;

	for(i=0;i<number_of_strategy;i++){
		if(list_strategy[i].display_on == TRUE && (
					(QS_WHO_AM_I_get() == BIG_ROBOT && list_strategy[i].robot_type == BIG)
					|| (QS_WHO_AM_I_get() == SMALL_ROBOT && list_strategy[i].robot_type == SMALL)
					|| list_strategy[i].robot_type == BOTH)){
			list_displayed_strategy[number_of_displayed_strategy++] = i;
			if(list_strategy[i].function == strategy)
				index_strategy = number_of_displayed_strategy-1;
		}

		if(list_strategy[i].function == strategy){
			strategy_name = list_strategy[i].name;
			match_duration = list_strategy[i].match_duration;
			found = TRUE;
		}
	}
	if(!found)
		BRAIN_update_match_duration();
}

/* 	execute un match de match_duration secondes � partir de la
	liberation de la biroute. Arrete le robot � la fin du match.
	Appelle une autre routine pour l'IA pendant le match.
	Une dur�e de 0 indique un match infini
*/
void any_match(void)
{
	static error_e ret;

	if (!global.flags.match_started)
	{
		// Initialisation � FALSE des machines � �tat principale pour que les autres strat�gie n'ai pas d'influence
		// Variable mise � TRUE au lancement d'une strat�gie principale
		main_strategie_used = FALSE;
		stop_request = FALSE;

		/* we are before the match */
		/* regarder si le match doit commencer */
		if (global.flags.ask_start && strategy != NULL)
		{
			global.flags.match_started = TRUE;
			CLOCK_run_match();
			XBEE_send_sid(XBEE_START_MATCH, TRUE);				//URGENT : on lance le match sur notre robot AMI...
			date_t date;
			Uint16 matchId = SD_get_match_id();
			RTC_get_local_time(&date);
			RTC_print_time();
			CAN_msg_t msg;
			msg.sid = BROADCAST_START;
			msg.size = SIZE_BROADCAST_START;
			msg.data.broadcast_start.matchId = matchId;
			msg.data.broadcast_start.seconde = date.seconds;
			msg.data.broadcast_start.minute = date.minutes;
			msg.data.broadcast_start.heure = date.hours;
			msg.data.broadcast_start.jour = date.date;
			msg.data.broadcast_start.mois = date.month;
			msg.data.broadcast_start.annee = date.year;
			msg.data.broadcast_start.journee = date.day;
			CAN_send(&msg);
			Supervision_ask_prop_to_send_periodically_pos(20,PI4096/45);	//Demande d'envoi de la position : tout les 20 mm et tout les 4�
			XBEE_ping_pong_enable(FALSE);						//D�sactive le ping/pong... c'est trop tard pour �a...
			//PATHFIND_MAJ_COLOR();								// Configuration du pathfind sp�cifique � la couleur
			ASTAR_init();										// Configuration du pathfind A*
			//ELEMENTS_init_with_color();							// Initialisation des �l�ments ayant besoin de la couleur
			ZONES_init();										// Configuration des zones suivant la couleur
			SELFTEST_resetHokuyoDisconnectionTime();
			return;
		}


		if(strategy == high_level_strat)	//Liste ici les strat�gie qui doivent �tre appel�es m�me avant le d�but du match
			high_level_strat();
	} else {

		if(global.flags.ask_suspend_match){
			global.flags.match_suspended = TRUE;
			QUEUE_reset_all();
			BUZZER_play(150, NOTE_RE, 2);
		}

		if(global.flags.go_to_home){
			debug_printf("Appel de la fonction go to home\n");
			ret = func_go_to_home();
			if(ret != IN_PROGRESS){
				global.flags.go_to_home = FALSE;
			}
		}else if(global.flags.match_suspended){

			strat_stop_robot();

		}else if (!global.flags.match_over && !global.flags.match_suspended) {

			BRAIN_action_in_match();

			if(match_duration != 0 && (global.match_time >= (match_duration))) {
				//MATCH QUI SE TERMINE
				Selftest_print_sd_hokuyo_lost();
				CAN_send_sid(BROADCAST_STOP_ALL);
				global.flags.match_over = TRUE;
				QUEUE_reset_all();
				BUZZER_play(500,NOTE_SOL,2);
				time_end_of_match = global.absolute_time;
				Supervision_ask_prop_to_send_periodically_pos(1, PI4096/180); // Tous les milimetres et degr�s: ca flood mais on est pas en match donc pas d�placment
				SYS_check_stack_level();
				BRAIN_action_at_end_of_match();
			}
			else
			{
				(*strategy)();	//Strat�gie du match
			}
		}
		else
		{
			BRAIN_action_after_end_of_match();
			/* match is over */
		}
	}
}

void BRAIN_start(void){
	global.flags.ask_start = TRUE;
}

//Attention, ne renvoit qu'une fois VRAI lorsque la strategy a �t� mise � jour.... (destination LCD...)
bool_e BRAIN_get_strat_updated(void)
{
	if(strat_updated)
	{
		strat_updated = FALSE;
		return TRUE;
	}
	return FALSE;
}


void BRAIN_set_strategy_index(Uint8 i){
	if(global.flags.match_started == FALSE){
		assert(i >= 0 && i < number_of_displayed_strategy);
		strategy = list_strategy[list_displayed_strategy[i]].function;
		strategy_name = list_strategy[list_displayed_strategy[i]].name;
		index_strategy = i;
		BRAIN_update_match_duration();
		debug_printf("Using strat : %s\n", BRAIN_get_current_strat_name());
		strat_updated = TRUE;
	}else
		debug_printf("Essai de modification de la strat�gie alors que le match est lanc� !\n");
}

//Retourne une chaine de 20 caract�res max
char * BRAIN_get_current_strat_name(void)
{
	if(strategy_name != NULL)
		return strategy_name;
	else
		return "strat not declared !";
}

ia_fun_t BRAIN_get_displayed_strat_function(Uint8 i){
	assert(i<number_of_displayed_strategy);
	return list_strategy[list_displayed_strategy[i]].function;
}

char * BRAIN_get_displayed_strat_name(Uint8 i){
	assert(i<number_of_displayed_strategy);
	return list_strategy[list_displayed_strategy[i]].name;
}

Uint8 BRAIN_get_number_of_displayed_strategy(){
	return number_of_displayed_strategy;
}

ia_fun_t BRAIN_get_current_strat_function(void){
	return strategy;
}

static void BRAIN_update_match_duration(void)
{
	if(index_strategy != -1)
		match_duration = list_strategy[index_strategy].match_duration;
	else
		match_duration = MATCH_DURATION;
}

time32_t BRAIN_getMatchDuration(void){
	return match_duration;
}

//Execut� pendant tout le match
static void BRAIN_action_in_match(void){

}

//Execut� juste une fois � la fin du match
static void BRAIN_action_at_end_of_match(void){
}

//Execut� d�s la fin du match
static void BRAIN_action_after_end_of_match(void){
	static bool_e is_displayed = FALSE;
	if(!is_displayed) {
		SCORE_print_score();

		if(I_AM_BIG()) {

		} else {

		}

		is_displayed = TRUE;
	}
}
