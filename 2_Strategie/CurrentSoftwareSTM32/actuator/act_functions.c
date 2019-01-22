/*
 *	Club Robot ESEO 2009 - 2010
 *	CHOMP
 *
 *	Fichier : act_functions.c
 *	Package : Carte Principale
 *	Description : 	Fonctions de gestion de la pile
 *					de l'actionneur
 *	Auteur : Julien et Ronan
 *	Version 20110313
 */

#include "act_functions.h"

#define LOG_PREFIX "act_f: "
#define LOG_COMPONENT OUTPUT_LOG_COMPONENT_ACTFUNCTION
#include "../QS/QS_outputlog.h"
#include "../QS/QS_IHM.h"
#include "../QS/QS_can_verbose.h"
#include "../QS/QS_who_am_i.h"
#include "../QS/QS_buffer_fifo.h"
#include "../QS/QS_watchdog.h"
#include "../QS/QS_stateMachineHelper.h"
#include "../utils/actionChecker.h"
#include "../utils/generic_functions.h"
#include "act_avoidance.h"

#define ACT_CONFIG_ANSWER_TIMEOUT					500
#define ACT_SENSOR_ANSWER_TIMEOUT					500
#define ULU_TIME                        			300

#define ACT_EXPANDER_TIME_MAX_WAIT_DISTANCE_SENSOR	500
#define ACT_EXPANDER_TIME_MAX_WAIT_COLOR_SENSOR		500

#define ACT_EXPANDER_VACUOSTAT_TIME_VALID			2000
#define ACT_EXPANDER_VACUOSTAT_TIME_REASKING		500
#define ACT_EXPANDER_VACUOSTAT_TIMEOUT				500

typedef enum{
	ACT_SENSOR_WAIT = 0,
	ACT_SENSOR_ABSENT,
	ACT_SENSOR_PRESENT
}act_sensor_e;

typedef struct{
	bool_e thresholdState;
	Sint16 distance;	//distance remontant un code d'erreur si négative.
	bool_e received;
	time32_t timeAsk;
}actExpander_distanceSensor_s;

typedef struct{
	ACT_EXPANDER_pumpStatus_e state;
	time32_t lastRefresh;
	time32_t timeBegin;
	actExpanderId_e actExpanderId;
	Uint8 idPump;
}actExpander_vacuostat_s;

typedef struct{
	ACT_EXPANDER_colorSensor_e color;
	actExpanderId_e actExpanderId;
	bool_e received;
	time32_t timeAsk;
}actExpander_colorSensor_s;

const act_link_SID_Queue_s act_link_SID_Queue[] = {
	{0,							   			NB_QUEUE,									""},

	// BIG_ROBOT
	//{ACT_EXEMPLE,							ACT_QUEUE_Exemple,								"Exemple"},
	{ACT_BIG_ELEVATOR_FRONT_RIGHT,				ACT_QUEUE_Big_elevator_front_right,			"Elevator_front_right"},
	{ACT_BIG_ELEVATOR_FRONT_MIDDLE,				ACT_QUEUE_Big_elevator_front_middle,		"Elevator_front_middle"},
	{ACT_BIG_ELEVATOR_FRONT_LEFT,				ACT_QUEUE_Big_elevator_front_left,			"Elevator_front_left"},
	{ACT_BIG_LOCKER_FRONT_RIGHT,				ACT_QUEUE_Big_locker_front_right,			"Locker_front_right"},
	{ACT_BIG_LOCKER_FRONT_MIDDLE,				ACT_QUEUE_Big_locker_front_middle,			"Locker_front_middle"},
	{ACT_BIG_LOCKER_FRONT_LEFT,					ACT_QUEUE_Big_locker_front_left,			"Locker_front_left"},
	{ACT_BIG_LOCKER_BACK,						ACT_QUEUE_Big_lock_back,					"Lock_back"},
	{ACT_BIG_SLOPE_TAKER_BACK,					ACT_QUEUE_Big_slope_taker_back,				"Slope_taker_back"},
	{ACT_BIG_SORTING_BACK_VERY_RIGHT,			ACT_QUEUE_Big_sorting_back_very_right,		"Sorting_back_very_right"},
	{ACT_BIG_SORTING_BACK_RIGHT,				ACT_QUEUE_Big_sorting_back_right,			"Sorting_back_right"},
	{ACT_BIG_SORTING_BACK_LEFT,					ACT_QUEUE_Big_sorting_back_left,			"Sorting_back_left"},
	{ACT_BIG_SORTING_BACK_VERY_LEFT,			ACT_QUEUE_Big_sorting_back_very_left,		"Sorting_back_very_left"},


	// SMALL_ROBOT
	//{ACT_EXEMPLE,							ACT_QUEUE_Exemple,							"Exemple"},
	{ACT_SMALL_ELEVATOR_FRONT_LEFT,			ACT_QUEUE_small_elevator_front_left,		"Elevator_front_left"},
	{ACT_SMALL_ELEVATOR_FRONT_RIGHT,		ACT_QUEUE_small_elevator_front_right,		"Elevator_front_right"},
	{ACT_SMALL_SORTING_BACK_LEFT,			ACT_QUEUE_small_sorting_back_left,			"Sorting_back_left"},
	{ACT_SMALL_SORTING_BACK_RIGHT,			ACT_QUEUE_small_sorting_back_right,			"Sorting_fron_right"},
	{ACT_SMALL_LOCKER_BACK,					ACT_QUEUE_small_locker_back,				"Locker_back"},
	{ACT_SMALL_LOCKER_FRONT_RIGHT,			ACT_QUEUE_small_locker_front_right,			"Locker_front_right"},
	{ACT_SMALL_LOCKER_FRONT_LEFT,			ACT_QUEUE_small_locker_front_left,			"Locker_front_left"},


	// Mosfets actionneurs
	//{ACT_TURBINE,					ACT_QUEUE_Turbine,						"Mosfet_act_turbine"},
	{ACT_MOSFET_8,					ACT_QUEUE_Mosfet_act_8,				    "Mosfet_act_8"},
	{ACT_MOSFET_MULTI,      		ACT_QUEUE_Mosfet_act_multi,		        "Mosfet_act_multi"},

	//Mosfets stratégie
	{STRAT_MOSFET_1,				ACT_QUEUE_Mosfet_strat_1,				"Mosfet_strat_0"},
	{STRAT_MOSFET_2,				ACT_QUEUE_Mosfet_strat_2,				"Mosfet_strat_1"},
	{STRAT_MOSFET_3,				ACT_QUEUE_Mosfet_strat_3,				"Mosfet_strat_2"},
	{STRAT_MOSFET_4,				ACT_QUEUE_Mosfet_strat_4,				"Mosfet_strat_3"},
	{STRAT_MOSFET_5,	           	ACT_QUEUE_Mosfet_strat_5,				"Mosfet_strat_5"},
	{STRAT_MOSFET_6,                ACT_QUEUE_Mosfet_strat_6,               "Mosfet_strat_6"},
	{STRAT_MOSFET_7,               	ACT_QUEUE_Mosfet_strat_7,               "Mosfet_strat_7"},
	{STRAT_MOSFET_8,                ACT_QUEUE_Mosfet_strat_8,		        "Mosfet_strat_8"},
	{STRAT_MOSFET_MULTI,	        ACT_QUEUE_Mosfet_strat_multi,			"Mosfet_strat_multi"},

};

static const Uint8 act_link_SID_Queue_size = sizeof(act_link_SID_Queue)/sizeof(act_link_SID_Queue_s);

// Config actionneurs
static act_config_s act_config[sizeof(act_link_SID_Queue)/sizeof(act_link_SID_Queue_s)] = {0};

// Warner actionneurs
static bool_e act_warner[sizeof(act_link_SID_Queue)/sizeof(act_link_SID_Queue_s)] = {0};

// Capteurs
static volatile act_sensor_e sensor_answer[NB_ACT_SENSOR] = {0};

static volatile actExpander_vacuostat_s actExpander_vacuostat[ACT_EXPANDER_NB_PUMP] = {0};

static volatile actExpander_distanceSensor_s actExpander_distanceSensor[ACT_EXPANDER_NB_SENSOR] = {0};

static volatile actExpander_colorSensor_s actExpander_colorSensor[ACT_EXPANDER_NB_SENSOR] = {0};

/* Pile contenant les arguments d'une demande d'opération
 * Contient les messages CAN à envoyer à la carte actionneur pour exécuter l'action.
 * fallbackMsg contient le message CAN lorsque l'opération demandée par le message CAN msg ne peut pas être complétée (bras bloqué, robot adverse qui bloque l'actionneur par exemple)
 * On utilise une structure différente de CAN_msg_t pour économiser la RAM (voir matrice act_args)
 *
 * Pour chaque fonction appelée par le code stratégie pour empiler une action, on remplit les messages CAN à envoyer et on empile l'action.
 * Un timeout est disponible si jamais la carte actionneur ne renvoie pas de message CAN ACT_RESULT (elle le devrait, mais on sait jamais)
 *
 * Le code de ACT_check_result() gère le renvoi de message lorsque la carte actionneur indique qu'il n'y avait pas assez de ressource ou l'envoi du message fallback si la position demandé par le message CAN msg n'est pas atteignable.
 * Si le message fallback se solve par un echec aussi, on indique au code de stratégie que cet actionneur n'est pas dispo maintenant et de réessayer plus tard (il faut faire une autre strat pendant ce temps, si c'est un robot qui bloque le bras, il faut que l'environnement du jeu bouge)
 * Si le renvoi du message à cause du manque de ressource cause la même erreur, on marque l'actionneur comme inutilisable (ce cas est grave, il y a probablement un problème dans le code actionneur ou un flood de demande d'opération de la carte stratégie)
 */


//Info sur la gestion d'erreur des actionneurs:
//La carte actionneur génère des resultats et détaille les erreurs suivant ce qu'elle sait et les envois par message CAN avec ACT_RESULT
//La fonction ACT_process_result (act_function.c) convertit les messages ACT_RESULT en ces valeurs dans act_state_info_t::operationResult et act_state_info_t::recommendedBehavior (environnement.h)
//La fonction ACT_check_result (act_function.c) convertit et gère les messages act_state_info_t::operationResult et act_state_info_t::recommendedBehavior en information ACT_function_result_e (dans act_function.h) pour être ensuite utilisé par le reste du code stratégie.


//FONCTIONS D'ACTIONNEURS

////////////////////////////////////////
////////// ORDRES ACTIONNEURS //////////
////////////////////////////////////////

//Recherche l'indice de la case du tableau act_link_SID_Queue ayant pour sid le sid passé en paramètre
//Attention pour avoir l'indice de la Queue il faut faire -1 sur la résultat.
//ex: ACT_FIRST_ACT=301  ACT_QUEUE_First_act=0
Uint8 ACT_search_link_SID_Queue(ACT_sid_e sid){
	Uint8 i;
	for(i=0;i<act_link_SID_Queue_size;i++){
		if(act_link_SID_Queue[i].sid == sid){
			return i;
		}
	}
	return act_link_SID_Queue_size;
}

bool_e ACT_push_order_with_all_params(ACT_sid_e sid,  ACT_order_e order, Uint16 param, Uint16 timeout, bool_e run_now){
	QUEUE_arg_t args;
	Uint8 i;

	i = ACT_search_link_SID_Queue(sid);

	if(i >= act_link_SID_Queue_size){
		error_printf("Link SID non trouvé dans ACT_push_order !\n");
		return FALSE;
	}

	ACT_arg_init_with_param(&args, sid, order, param, run_now);
	if(timeout != 0){
		ACT_arg_set_timeout(&args, timeout);
	}
	ACT_arg_set_fallbackmsg(&args, sid,  ACT_DEFAULT_STOP);

	debug_printf("Pushing %s Run cmd (sid : %d   order : %d)\n", act_link_SID_Queue[i].name, sid, order);

	ACT_AVOIDANCE_new_classic_cmd(act_link_SID_Queue[i].queue_id, order);

	QUEUE_reset(act_link_SID_Queue[i].queue_id);

	return ACT_push_operation(act_link_SID_Queue[i].queue_id, &args);
}

inline bool_e ACT_push_order(ACT_sid_e sid,  ACT_order_e order){
	return ACT_push_order_with_all_params(sid, order, 0, 0, TRUE);
}

inline bool_e ACT_push_order_with_timeout(ACT_sid_e sid,  ACT_order_e order, Uint16 timeout){
	return ACT_push_order_with_all_params(sid, order, 0, timeout, TRUE);
}

inline bool_e ACT_push_order_with_param(ACT_sid_e sid,  ACT_order_e order, Uint16 param){
	return ACT_push_order_with_all_params(sid, order, param, 0, TRUE);
}

inline bool_e ACT_push_order_now(ACT_sid_e sid,  ACT_order_e order){
	return ACT_push_order_with_all_params(sid, order, 0, 0, TRUE);
}

inline bool_e ACT_push_order_queued(ACT_sid_e sid,  ACT_order_e order){
	return ACT_push_order_with_all_params(sid, order, 0, 0, FALSE);
}

#define ACT_CHECK_STATUS_NB_LAST_SID_CHECK	4

Uint8 ACT_check_status(ACT_sid_e sid, Uint8 in_progress_state, Uint8 success_state, Uint8 failed_state){
	static Uint8 lastSidUsed[ACT_CHECK_STATUS_NB_LAST_SID_CHECK] = {0};
	Uint8 index, i;
	bool_e found = FALSE;

	for(i=0; i<ACT_CHECK_STATUS_NB_LAST_SID_CHECK; i++){	// Recherche optimisée du SID parmis les ACT_CHECK_STATUS_NB_LAST_SID_CHECK dernier utilisé
		if(lastSidUsed[i] < act_link_SID_Queue_size && sid == act_link_SID_Queue[lastSidUsed[i]].sid){
			index = lastSidUsed[i];
			found = TRUE;
		}
	}

	if(found == FALSE){
		index = ACT_search_link_SID_Queue(sid);

		if(index < act_link_SID_Queue_size){							// On ne mémorise pas une recherche non fructueuse
			for(i=0; i<ACT_CHECK_STATUS_NB_LAST_SID_CHECK - 1; i++){
				lastSidUsed[i+1] = lastSidUsed[i];
			}
			lastSidUsed[0] = index;
		}
	}

	if(index >= act_link_SID_Queue_size){
		error_printf("Link SID non trouvé dans ACT_check_status !\n");
		return failed_state;
	}

	ACT_function_result_e result = ACT_get_last_action_result(act_link_SID_Queue[index].queue_id);
	switch(result) {
		case ACT_FUNCTION_InProgress:
			return in_progress_state;

		case ACT_FUNCTION_Done:
			return success_state;

		case ACT_FUNCTION_ActDisabled:
		case ACT_FUNCTION_RetryLater:
			return failed_state;

		default:
			debug_printf("/!\\ ACT_check_status:%d: in default case, result = %d\n", __LINE__, result);
			return failed_state;
	}
}

////////////////////////////////////////
//////////////// CONFIG ////////////////
////////////////////////////////////////

bool_e ACT_set_config(Uint16 sid, act_config_e cmd, Uint16 value){
//	QUEUE_arg_t args;
	CAN_msg_t msg;

	msg.sid = ACT_SET_CONFIG;
	msg.size = SIZE_ACT_SET_CONFIG;
	msg.data.act_set_config.act_sid = 0xFF & sid;
	msg.data.act_set_config.config = cmd;
	msg.data.act_set_config.data_config.raw_data = value;

//	ACT_arg_init_with_msg(&args, msg);
//	ACT_arg_set_timeout(&args, 500);

	debug_printf("Config : %s\n", act_link_SID_Queue[ACT_search_link_SID_Queue(sid)].name);
	debug_printf("    cmd : %d\n", cmd);
	debug_printf("    value : %d\n", value);

//	return ACT_push_operation(act_link_SID_Queue[ACT_search_link_SID_Queue(sid)].queue_id, &args);

	CAN_send(&msg);  // Le message est envoyé immédiatement.

	return TRUE;
}

bool_e ACT_get_config_request(Uint16 sid, act_config_e config){
	QUEUE_arg_t args;
	CAN_msg_t msg;

	msg.sid = ACT_GET_CONFIG;
	msg.size = SIZE_ACT_GET_CONFIG;
	msg.data.act_get_config.act_sid = 0xFF & sid;
	msg.data.act_get_config.config = config;

	ACT_arg_init_with_msg(&args, msg);
	ACT_arg_set_timeout(&args, 500);

	return ACT_push_operation(act_link_SID_Queue[ACT_search_link_SID_Queue(sid)].queue_id, &args);
}

void ACT_get_config_answer(CAN_msg_t* msg){
	assert(msg->sid == ACT_GET_CONFIG_ANSWER);
	Uint8 i = ACT_search_link_SID_Queue((Uint16)(ACT_FILTER | msg->data.act_get_config_answer.act_sid));

	if(i >= act_link_SID_Queue_size){
		error_printf("Link SID non trouvé dans ACT_get_config_answer !\n");
		return;
	}

	switch(msg->data.act_get_config_answer.config){
		case POSITION_CONFIG:
			act_config[i].pos = msg->data.act_get_config_answer.act_get_config_data.act_get_config_pos_answer.order;
			break;
		case TORQUE_CONFIG:
			act_config[i].torque = msg->data.act_get_config_answer.act_get_config_data.torque;
			debug_printf("Torque : %d\n", act_config[i].torque);
			break;
		case TEMPERATURE_CONFIG:
			act_config[i].temperature = msg->data.act_get_config_answer.act_get_config_data.temperature;
			debug_printf("Temperature : %d\n", act_config[i].temperature);
			break;
		case LOAD_CONFIG:
			act_config[i].load = msg->data.act_get_config_answer.act_get_config_data.load;
			debug_printf("Load : %d\n", act_config[i].load);
			break;
		default:
			error_printf("Error config recieved but not requested !! file %s line %d\n", __FILE__, __LINE__);
	}

	act_config[i].info_received = TRUE; // On vient de recevoir une réponse
	// On acquitte le message (du moins c'est une tentative)
	//QUEUE_next(i);
	ACT_set_result(act_link_SID_Queue[i].queue_id, ACT_RESULT_Ok);
}

error_e ACT_check_position_config(Uint16 sid, ACT_order_e order){
	CREATE_MAE(SEND,
				WAIT);

	static time32_t begin_time;
	static Uint8 i = 0;
	error_e ret = IN_PROGRESS;

	switch(state){
		case SEND:
			i = ACT_search_link_SID_Queue(sid);

			if(i >= act_link_SID_Queue_size){
				error_printf("Link SID non trouvé dans ACT_check_position_config !\n");
				return NOT_HANDLED;
			}

			ACT_get_config_request(sid, POSITION_CONFIG);
			act_config[i].info_received = FALSE;
			begin_time = global.absolute_time;
			state = WAIT;
			break;

		case WAIT:
			if(global.absolute_time - begin_time > ACT_CONFIG_ANSWER_TIMEOUT){
				RESET_MAE();
				ret = END_WITH_TIMEOUT;
			}else if(act_config[i].info_received && act_config[i].pos == order){ // On a recu une réponse
				RESET_MAE();
				ret = END_OK;
			}else if(act_config[i].info_received){
				RESET_MAE();
				ret = NOT_HANDLED;
			}
			break;
	}
	return ret;
}



////////////////////////////////////////
//////////////// WARNER ////////////////
////////////////////////////////////////

bool_e ACT_set_warner(Uint16 sid, ACT_order_e pos){
	QUEUE_arg_t args;
	CAN_msg_t msg;
	Uint8 i = ACT_search_link_SID_Queue(sid);

	if(i >= act_link_SID_Queue_size){
		error_printf("Link SID non trouvé dans ACT_set_warner !\n");
		return FALSE;
	}

	msg.sid = ACT_WARNER;
	msg.size = SIZE_ACT_WARNER;
	msg.data.act_warner.act_sid = 0xFF & sid;
	msg.data.act_warner.warner_param = pos;

	ACT_arg_init_with_msg(&args, msg);
	ACT_arg_set_timeout(&args, 0);

	act_warner[i] = FALSE; // On baisse le flag

	return ACT_push_operation(act_link_SID_Queue[i].queue_id, &args);
}

void ACT_warner_answer(CAN_msg_t* msg){
	assert(msg->sid == ACT_WARNER_ANSWER);
	Uint8 i = ACT_search_link_SID_Queue((Uint16)(ACT_FILTER | msg->data.act_warner_answer.act_sid));

	if(i >= act_link_SID_Queue_size){
		error_printf("Link SID non trouvé dans ACT_warner_answer !\n");
		return;
	}

	act_warner[i] = TRUE; // On lève le flag
}

bool_e ACT_get_warner(Uint16 sid){

	Uint8 i = ACT_search_link_SID_Queue(sid);

	if(i >= act_link_SID_Queue_size){
		error_printf("Link SID non trouvé dans ACT_warner_answer !\n");
		return FALSE;
	}

	return act_warner[ACT_search_link_SID_Queue(sid)];
}

void ACT_reset_all_warner(){
	Uint8 i;
	for(i = 0; i < sizeof(act_link_SID_Queue)/sizeof(act_link_SID_Queue_s); i++){
		act_warner[i] = FALSE;
	}
}

////////////////////////////////////////
//////////////// SENSOR ////////////////
////////////////////////////////////////

error_e ACT_get_sensor(act_sensor_id_e act_sensor_id){
	CREATE_MAE(SEND,
				WAIT);

	static time32_t begin_time;

	switch(state){
		case SEND:{
			sensor_answer[act_sensor_id] = ACT_SENSOR_WAIT;
			begin_time = global.absolute_time;
			CAN_msg_t msg;
			msg.sid = ACT_ASK_SENSOR;
			msg.size = SIZE_ACT_ASK_SENSOR;
			msg.data.act_ask_sensor.act_sensor_id = act_sensor_id;
			CAN_send(&msg);
			state = WAIT;
			}break;

		case WAIT:
			if(global.absolute_time - begin_time > ACT_SENSOR_ANSWER_TIMEOUT){
				RESET_MAE();
				return END_WITH_TIMEOUT;
			}else if(sensor_answer[act_sensor_id] == ACT_SENSOR_PRESENT){
				RESET_MAE();
				return END_OK;
			}else if(sensor_answer[act_sensor_id] == ACT_SENSOR_ABSENT){
				RESET_MAE();
				return NOT_HANDLED;
			}
			break;
	}
	return IN_PROGRESS;
}

void ACT_sensor_answer(CAN_msg_t* msg){
	assert(msg->data.strat_inform_capteur.sensor_id < NB_ACT_SENSOR);
	if(msg->data.strat_inform_capteur.present)
		sensor_answer[msg->data.strat_inform_capteur.sensor_id] = ACT_SENSOR_PRESENT;
	else
		sensor_answer[msg->data.strat_inform_capteur.sensor_id] = ACT_SENSOR_ABSENT;
}


//------------------------------------------------------------------------------------------
// ACT Expander
//------------------------------------------------------------------------------------------

static actExpanderId_e ACT_EXPANDER_computeActExpanderIdFromPumpId(actExpanderPumpId_e id, Uint8 * idPump){
	actExpanderId_e actExpanderId = ACT_EXPANDER_ID_ERROR;

	if(id > ACT_EXPANDER_PUMP_BIG_BACKWARD_BEGIN && id < ACT_EXPANDER_PUMP_BIG_BACKWARD_END){
		actExpanderId = ACT_EXPANDER_BIG_BACKWARD;
		*idPump = id - ACT_EXPANDER_PUMP_BIG_BACKWARD_BEGIN - 1;
	}else if(id > ACT_EXPANDER_PUMP_BIG_FORWARD_BEGIN && id < ACT_EXPANDER_PUMP_BIG_FORWARD_END){
		actExpanderId = ACT_EXPANDER_BIG_FORWARD;
		*idPump = id - ACT_EXPANDER_PUMP_BIG_FORWARD_BEGIN - 1;
	} else {
		error_printf("ACT_EXPANDER_computeActExpanderId : Erreur id non conforme\n");
		actExpanderId = ACT_EXPANDER_ID_ERROR;
	}

	return actExpanderId;
}

static actExpanderPumpId_e ACT_EXPANDER_computePumpId(actExpanderId_e id, Uint8 idPump){
	actExpanderPumpId_e pumpId = ACT_EXPANDER_PUMP_ERROR;

	if(QS_WHO_AM_I_get() == BIG_ROBOT){
		if(id == ACT_EXPANDER_BIG_BACKWARD){
			pumpId = ACT_EXPANDER_PUMP_BIG_BACKWARD_BEGIN + 1 + idPump;
		} else if(id == ACT_EXPANDER_BIG_FORWARD){
			pumpId = ACT_EXPANDER_PUMP_BIG_FORWARD_BEGIN + 1 + idPump;
		}
	}else{
//		if(id == ACT_EXPANDER_SMALL){
//			pumpId = ACT_EXPANDER_PUMP_SMALL_BEGIN + 1 + idPump;
//		}
	}

	return pumpId;
}

static actExpanderId_e ACT_EXPANDER_computeActExpanderIdFromSensorId(actExpanderSensorId_e id, Uint8 * idSensor){
	actExpanderId_e actExpanderId = ACT_EXPANDER_ID_ERROR;

//	if(id > ACT_EXPANDER_SENSOR_BIG_BACKWARD_BEGIN && id < ACT_EXPANDER_SENSOR_BIG_BACKWARD_END){
//		actExpanderId = ACT_EXPANDER_BIG_BACKWARD;
//		*idSensor = id - ACT_EXPANDER_SENSOR_BIG_BACKWARD_BEGIN - 1;
//	}else{
//		error_printf("ACT_EXPANDER_computeActExpanderId : Erreur id non conforme\n");
//		actExpanderId = ACT_EXPANDER_ID_ERROR;
//	}

	return actExpanderId;
}

static actExpanderSensorId_e ACT_EXPANDER_computeSensorId(actExpanderId_e id, Uint8 idSensor){
	actExpanderSensorId_e pumpId = ACT_EXPANDER_SENSOR_ERROR;

	if(QS_WHO_AM_I_get() == BIG_ROBOT){
//		if(id == ACT_EXPANDER_BIG_BACKWARD){
//			pumpId = ACT_EXPANDER_SENSOR_BIG_BACKWARD_BEGIN + 1 + idSensor;
//		}
	}else{
//		if(id == ACT_EXPANDER_SMALL){
//			pumpId = ACT_EXPANDER_SENSOR_SMALL_BEGIN + 1 + idSensor;
//		}
	}

	return pumpId;
}

void ACT_EXPANDER_setPump(actExpanderPumpId_e id, bool_e state){
	actExpanderId_e actExpanderId;
	Uint8 idPump;

	actExpanderId = ACT_EXPANDER_computeActExpanderIdFromPumpId(id, &idPump);

	if(actExpanderId == ACT_EXPANDER_ID_ERROR){
		error_printf("ACT_EXPANDER_setPump : Erreur id non conforme\n");
		return;
	}

	CAN_msg_t msg;
	msg.sid = ACT_EXPANDER_SET_PUMP;
	msg.size = SIZE_ACT_EXPANDER_SET_PUMP;
	msg.data.actExpander_setPump.id = actExpanderId;
	msg.data.actExpander_setPump.idPump = idPump;
	msg.data.actExpander_setPump.state = state;
	CAN_send(&msg);
}

void ACT_EXPANDER_setSolenoidValve(actExpanderPumpId_e id, bool_e state){
	actExpanderId_e actExpanderId;
	Uint8 idPump;

	actExpanderId = ACT_EXPANDER_computeActExpanderIdFromPumpId(id, &idPump);

	if(actExpanderId == ACT_EXPANDER_ID_ERROR){
		error_printf("ACT_EXPANDER_setSolenoidValve : Erreur id non conforme\n");
		return;
	}

	CAN_msg_t msg;
	msg.sid = ACT_EXPANDER_SET_SOLENOID_VALVE;
	msg.size = SIZE_ACT_EXPANDER_SET_SOLENOID_VALVE;
	msg.data.actExpander_setSolenoideValve.id = actExpanderId;
	msg.data.actExpander_setSolenoideValve.idSolenoidValve = idPump;
	msg.data.actExpander_setSolenoideValve.state = state;
	msg.data.actExpander_setSolenoideValve.duration = 3000;
	CAN_send(&msg);
}

Uint8 ACT_EXPANDER_waitStateVacuostat(actExpanderPumpId_e id, ACT_EXPANDER_pumpStatus_e stateToWait, Uint8 inProgress, Uint8 sucess, Uint8 fail){
	CREATE_MAE(INIT,
					GET_STATE,
					WAIT_STATE);

		switch(state){
			case INIT:
				actExpander_vacuostat[id].actExpanderId = ACT_EXPANDER_computeActExpanderIdFromPumpId(id, (Uint8*) (&(actExpander_vacuostat[id].idPump)));

				if(actExpander_vacuostat[id].actExpanderId == ACT_EXPANDER_ID_ERROR){
					error_printf("ACT_EXPANDER_waitStateVacuostat : Erreur id non conforme\n");
					RESET_MAE();
					return fail;
				}

				actExpander_vacuostat[id].timeBegin = global.absolute_time;
				state = GET_STATE;
				break;

			case GET_STATE:{
				CAN_msg_t msg;
				msg.sid = ACT_EXPANDER_GET_PUMP_STATUS;
				msg.size = SIZE_ACT_EXPANDER_GET_PUMP_STATUS;
				msg.data.actExpander_getPumpStatus.id = actExpander_vacuostat[id].actExpanderId;
				msg.data.actExpander_getPumpStatus.idPump = actExpander_vacuostat[id].idPump;
				CAN_send(&msg);

				state = WAIT_STATE;
				}break;

			case WAIT_STATE:
				if(global.absolute_time - actExpander_vacuostat[id].lastRefresh < ACT_EXPANDER_VACUOSTAT_TIME_REASKING && actExpander_vacuostat[id].state == stateToWait){
					RESET_MAE();
					return sucess;
				}else if(global.absolute_time - actExpander_vacuostat[id].timeBegin > ACT_EXPANDER_VACUOSTAT_TIMEOUT){
					info_printf("vacuostat %d timeout\n", id);
					RESET_MAE();
					return fail;
				}

				break;

			default:
				error_printf("waitVacuostat case default\n");
				RESET_MAE();
				return fail;
		}

		return inProgress;
}

ACT_EXPANDER_pumpStatus_e ACT_EXPANDER_getStateVacuostat(actExpanderPumpId_e idVacuostat){
	if(idVacuostat >= ACT_EXPANDER_NB_PUMP){
		error_printf("ACT_EXPANDER_getStateVacuostat : Erreur id non conforme\n");
		return ACT_EXPANDER_PUMP_STATUS_NO_PUMPING;
	}
	if(global.flags.virtual_mode)
		return ACT_EXPANDER_PUMP_STATUS_PUMPING_OBJECT;
	else
		return actExpander_vacuostat[idVacuostat].state;

}

void ACT_EXPANDER_receiveVacuostat_msg(CAN_msg_t *msg){
	if(msg->sid != ACT_EXPANDER_TELL_PUMP_STATUS)
		return;

	actExpanderPumpId_e id = ACT_EXPANDER_computePumpId(msg->data.actExpander_tellPumpStatus.id, msg->data.actExpander_tellPumpStatus.idPump);

	if(id >= ACT_EXPANDER_NB_PUMP)
		return;

	actExpander_vacuostat[id].state = msg->data.actExpander_tellPumpStatus.status;
	actExpander_vacuostat[id].lastRefresh = global.absolute_time;

}

void ACT_EXPANDER_vacuostaProcessMain(){
	Uint8 i = 0;
	for(i = 0; i < ACT_EXPANDER_NB_PUMP; i++){
		if(global.absolute_time - actExpander_vacuostat[i].lastRefresh > ACT_EXPANDER_VACUOSTAT_TIME_VALID){
			actExpander_vacuostat[i].state = ACT_EXPANDER_PUMP_STATUS_NO_PUMPING;
		}
	}
}

void ACT_EXPANDER_setPumpAll(actExpanderId_e id, bool_e state) {
	switch(id) {
		case ACT_EXPANDER_BIG_BACKWARD:
			for(int i = ACT_EXPANDER_PUMP_BIG_BACKWARD_BEGIN + 1; i < ACT_EXPANDER_PUMP_BIG_BACKWARD_END; i++) {
				 ACT_EXPANDER_setPump(i, state);
			}
			break;
		case ACT_EXPANDER_BIG_FORWARD:
			for(int i = ACT_EXPANDER_PUMP_BIG_FORWARD_BEGIN + 1; i < ACT_EXPANDER_PUMP_BIG_FORWARD_END; i++) {
				 ACT_EXPANDER_setPump(i, state);
			}
			break;
		default:
			error_printf("ACT_EXPANDER_setPumpAll: id = %d not used\n", id);
	}
}

void ACT_EXPANDER_setSolenoideValveAll(actExpanderId_e id, bool_e state) {
	switch(id) {
		case ACT_EXPANDER_BIG_BACKWARD:
			for(int i = ACT_EXPANDER_PUMP_BIG_BACKWARD_BEGIN + 1; i < ACT_EXPANDER_PUMP_BIG_BACKWARD_END; i++) {
				ACT_EXPANDER_setSolenoidValve(i, state);
			}
			break;
		case ACT_EXPANDER_BIG_FORWARD:
			for(int i = ACT_EXPANDER_PUMP_BIG_FORWARD_BEGIN + 1; i < ACT_EXPANDER_PUMP_BIG_FORWARD_END; i++) {
				ACT_EXPANDER_setSolenoidValve(i, state);
			}
			break;
		default:
			error_printf("ACT_EXPANDER_setSolenoidValve: id = %d not used\n", id);
	}
}

Uint8 ACT_EXPANDER_waitGetColorSensor(actExpanderSensorId_e id, ACT_EXPANDER_colorSensor_e * color, time32_t timeout, Uint8 inProgress, Uint8 success, Uint8 fail){
	CREATE_MAE(INIT,
				WAIT_STATE);

	static actExpanderId_e actExpanderId;
	static Uint8 idSensor;

	switch(state){
		case INIT:
			actExpanderId = ACT_EXPANDER_computeActExpanderIdFromSensorId(id, &idSensor);

			if(actExpanderId == ACT_EXPANDER_ID_ERROR){
				error_printf("ACT_EXPANDER_waitGetColorSensor : Erreur id non conforme\n");
				RESET_MAE();
				return fail;
			}

			actExpander_colorSensor[id].received = FALSE;
			actExpander_colorSensor[id].timeAsk = global.absolute_time;

			CAN_msg_t msg;
			msg.sid = ACT_EXPANDER_GET_COLOR_SENSOR;
			msg.size = SIZE_ACT_EXPANDER_GET_COLOR_SENSOR;
			msg.data.actExpander_getColorSensor.id = actExpanderId;
			msg.data.actExpander_getColorSensor.idSensor = idSensor;
			CAN_send(&msg);

			state = WAIT_STATE;
			break;

		case WAIT_STATE:
			if(actExpander_colorSensor[id].received){

				if(color != NULL){
					*color = actExpander_colorSensor[id].color;
					RESET_MAE();
					return success;
				}

			}else if(global.absolute_time - actExpander_colorSensor[id].timeAsk > timeout){
				info_printf("ACT_EXPANDER_waitGetColorSensor : Capteur couleur %d timeout\n", id);
				RESET_MAE();
				return fail;
			}
			break;

		default:
			error_printf("ACT_EXPANDER_waitGetColorSensor case default\n");
			RESET_MAE();
			return fail;
	}

	return inProgress;
}

Uint8 ACT_EXPANDER_waitStateColorSensor(actExpanderSensorId_e id, ACT_EXPANDER_colorSensor_e color, time32_t timeout, Uint8 inProgress, Uint8 success, Uint8 fail){
	CREATE_MAE(INIT,
				WAIT_STATE);

	static actExpanderId_e actExpanderId;
	static Uint8 idSensor;

	switch(state){
		case INIT:
			actExpanderId = ACT_EXPANDER_computeActExpanderIdFromSensorId(id, &idSensor);

			if(actExpanderId == ACT_EXPANDER_ID_ERROR){
				error_printf("ACT_EXPANDER_waitStateColorSensor : Erreur id non conforme\n");
				RESET_MAE();
				return fail;
			}

			actExpander_colorSensor[id].received = FALSE;
			actExpander_colorSensor[id].timeAsk = global.absolute_time;

			CAN_msg_t msg;
			msg.sid = ACT_EXPANDER_GET_COLOR_SENSOR;
			msg.size = SIZE_ACT_EXPANDER_GET_COLOR_SENSOR;
			msg.data.actExpander_getColorSensor.id = actExpanderId;
			msg.data.actExpander_getColorSensor.idSensor = idSensor;
			CAN_send(&msg);

			state = WAIT_STATE;
			break;

		case WAIT_STATE:
			if(actExpander_distanceSensor[id].received){

				if(color == actExpander_colorSensor[id].color){
					RESET_MAE();
					return success;
				}else{
					error_printf("ACT_EXPANDER_waitStateColorSensor : Couleur recu (%d) différente de celle attendue(%d)\n", actExpander_colorSensor[id].color, color);
					RESET_MAE();
					return fail;
				}

			}else if(global.absolute_time - actExpander_colorSensor[id].timeAsk > ACT_EXPANDER_TIME_MAX_WAIT_COLOR_SENSOR){
				info_printf("ACT_EXPANDER_waitStateColorSensor : Capteur couleur %d timeout\n", id);
				RESET_MAE();
				return fail;
			}
			break;

		default:
			error_printf("ACT_EXPANDER_waitStateColorSensor case default\n");
			RESET_MAE();
			return fail;
	}

	return inProgress;
}

void ACT_EXPANDER_receiveColorSensorMsg(CAN_msg_t * msg){
	if(msg->sid != ACT_EXPANDER_TELL_COLOR_SENSOR)
		return;

	actExpanderSensorId_e id = ACT_EXPANDER_computeSensorId(msg->data.actExpander_tellColorSensor.id, msg->data.actExpander_tellColorSensor.idSensor);

	if(id >= ACT_EXPANDER_NB_SENSOR){
		error_printf("ACT_EXPANDER_receiveColorSensorMsg : Erreur id non conforme\n");
		return;
	}

	actExpander_colorSensor[id].color = msg->data.actExpander_tellColorSensor.sensorColor;
	actExpander_colorSensor[id].received = TRUE;
}

bool_e ACT_EXPANDER_setDistanceSensorThreshold(actExpanderSensorId_e id, Uint16 thresholdDistance, ACT_EXPANDER_DistanceThresholdWay_e way){
	actExpanderId_e actExpanderId;
	Uint8 idSensor;

	actExpanderId = ACT_EXPANDER_computeActExpanderIdFromSensorId(id, &idSensor);

	if(actExpanderId == ACT_EXPANDER_ID_ERROR){
		error_printf("ACT_EXPANDER_setDistanceSensorThreshold : Erreur id non conforme\n");
		return FALSE;
	}

	CAN_msg_t msg;
	msg.sid = ACT_EXPANDER_SET_DISTANCE_SENSOR_THRESHOLD;
	msg.size = SIZE_ACT_EXPANDER_SET_DISTANCE_SENSOR_THRESHOLD;
	msg.data.actExpander_setDistanceSensorThreshold.id = actExpanderId;
	msg.data.actExpander_setDistanceSensorThreshold.idSensor = idSensor;
	msg.data.actExpander_setDistanceSensorThreshold.threshold = thresholdDistance;
	msg.data.actExpander_setDistanceSensorThreshold.way = way;
	CAN_send(&msg);

	return TRUE;
}

bool_e ACT_EXPANDER_getThreshold(actExpanderSensorId_e id){
	if(id >= ACT_EXPANDER_NB_SENSOR){
		error_printf("ACT_EXPANDER_getThreshold : Erreur id non conforme\n");
		return FALSE;
	}

	return actExpander_distanceSensor[id].thresholdState;
}

Uint8 ACT_EXPANDER_getDistanceSensorMeasure(actExpanderSensorId_e id, bool_e * resultPresence, Sint16 * resultDistance, Uint8 inProgress, Uint8 success, Uint8 fail){

	CREATE_MAE(INIT,
				WAIT_STATE);

	static actExpanderId_e actExpanderId;
	static Uint8 idSensor;

	switch(state){
		case INIT:
			actExpanderId = ACT_EXPANDER_computeActExpanderIdFromSensorId(id, &idSensor);

			if(actExpanderId == ACT_EXPANDER_ID_ERROR){
				error_printf("ACT_EXPANDER_waitDistanceSensorMeasure : Erreur id non conforme\n");
				RESET_MAE();
				return fail;
			}

			actExpander_distanceSensor[id].received = FALSE;
			actExpander_distanceSensor[id].timeAsk = global.absolute_time;

			CAN_msg_t msg;
			msg.sid = ACT_EXPANDER_GET_DISTANCE_SENSOR;
			msg.size = SIZE_ACT_EXPANDER_GET_DISTANCE_SENSOR;
			msg.data.actExpander_getDistanceSensor.id = actExpanderId;
			msg.data.actExpander_getDistanceSensor.idSensor = idSensor;
			CAN_send(&msg);

			state = WAIT_STATE;
			break;

		case WAIT_STATE:
			if(actExpander_distanceSensor[id].received){
				if(resultPresence != NULL)
					*resultPresence = actExpander_distanceSensor[id].thresholdState;
				if(resultDistance != NULL)
					*resultDistance = actExpander_distanceSensor[id].distance;
				RESET_MAE();
				return success;

			}else if(global.absolute_time - actExpander_distanceSensor[id].timeAsk > ACT_EXPANDER_TIME_MAX_WAIT_DISTANCE_SENSOR){
				info_printf("Capteur distance %d timeout\n", id);
				RESET_MAE();
				return fail;
			}
			break;

		default:
			error_printf("ACT_EXPANDER_getDistanceSensorMeasure case default\n");
			RESET_MAE();
			return fail;
	}

	return inProgress;
}

void ACT_EXPANDER_receiveDistanceSensorMsg(CAN_msg_t * msg){
	if(msg->sid != ACT_EXPANDER_TELL_DISTANCE_SENSOR)
		return;

	actExpanderSensorId_e id = ACT_EXPANDER_computeSensorId(msg->data.actExpander_tellDistanceSensor.id, msg->data.actExpander_tellDistanceSensor.idSensor);

	if(id >= ACT_EXPANDER_NB_SENSOR){
		error_printf("ACT_EXPANDER_receiveDistanceSensorMsg : Erreur id non conforme\n");
		return;
	}

	actExpander_distanceSensor[id].distance = msg->data.actExpander_tellDistanceSensor.distance;
	actExpander_distanceSensor[id].thresholdState = msg->data.actExpander_tellDistanceSensor.threshold;
	actExpander_distanceSensor[id].received = TRUE;
}

void ACT_EXPANDER_receiveErrorShortCircuitMsg(CAN_msg_t * msg){
	if(msg->sid != ACT_EXPENDER_RESULT_SHORT_CIRCUIT)
			return;



}
