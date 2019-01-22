/*  Club Robot ESEO 2012 - 2013
 *
 *	Fichier : ActManager.c
 *	Package : Carte actionneur
 *	Description : Gestion des actionneurs
 *  Auteur : Alexis, remake Arnaud, reremake Valentin
 *  Version 20130227
 */
#include <string.h>
#include "ActManager.h"

#include "QS/QS_all.h"
#include "QS/QS_CANmsgList.h"
#include "QS/QS_actuator/QS_ax12.h"
#include "QS/QS_actuator/QS_rx24.h"
#include "QS/QS_outputlog.h"
#include "QS/QS_lowLayer/QS_can.h"
#include "QS/QS_stateMachineHelper.h"
#include "QS/QS_who_am_i.h"

#include "actuators/actuator.h"
#include "act_queue_utils.h"
#include "actuators/actuators_big.h"
#include "actuators/actuators_small.h"

#include "actuators/impl/impl_ax12/impl_ax12_pos.h"
#include "actuators/impl/impl_ax12/impl_ax12_pos_double.h"
#include "actuators/impl/impl_ax12/impl_ax12_speed.h"
#include "actuators/impl/impl_rx24/impl_rx24_pos.h"
#include "actuators/impl/impl_rx24/impl_rx24_pos_double.h"
#include "actuators/impl/impl_rx24/impl_rx24_speed.h"
#include "actuators/impl/impl_motor/impl_motor_pos.h"
#include "actuators/impl/impl_motor/impl_motor_speed.h"
#include "actuators/impl/impl_pwm/impl_pwm.h"
#include "actuators/impl/impl_pwm/impl_ppm.h"
#include "actuators/impl/impl_tor/impl_mosfet.h"
#include "Can_msg_processing.h"

// Macro permettant de linker vers des fonctions actionneurs
#define ACT_DECLARE_FUNCTIONS(kind, prefix)               		act_functions[kind] = (ACT_functions_t){&prefix##_init, &prefix##_init_pos, &prefix##_stop, NULL,                   &prefix##_CAN_process_msg, NULL, 				 NULL,  NULL,                 NULL,                 NULL}
#define ACT_DECLARE_FUNCTIONS_WITH_CONFIG(kind, prefix)   		act_functions[kind] = (ACT_functions_t){&prefix##_init, &prefix##_init_pos, &prefix##_stop, &prefix##_reset_config, &prefix##_CAN_process_msg, NULL, 				 NULL, &prefix##_get_config, &prefix##_set_config, &prefix##_set_warner}
#define ACT_DECLARE_FUNCTIONS_WITH_CONFIG_AND_IT(kind, prefix)  act_functions[kind] = (ACT_functions_t){&prefix##_init, &prefix##_init_pos, &prefix##_stop, &prefix##_reset_config, &prefix##_CAN_process_msg, &prefix##_process_it, NULL, &prefix##_get_config, &prefix##_set_config, &prefix##_set_warner}

static ACT_functions_t act_functions[NB_ACTUATOR_KINDS];  // Tableau des fonctions des actionneurs

// Macro permettant de linker vers l'instance d'un actionneur
#define ACT_DECLARE_ACTUATOR(prefix) 	actuators[i++] = (ACTMGR_actuator_t) {prefix.queue_id, &prefix, &(act_functions[prefix.kind])}

static ACTMGR_actuator_t actuators[NB_ACTUATORS];  // Tableau des instances des actionneurs

// Temps d'attente avant l'initialisation en position des actionneurs (temps d'établisssment de l'alim)
#define TIME_BEFORE_INIT_POS	(200)

// Délai d'attente entre l'initialisation de deux actionneurs
#define DELAY_INIT_POS			(50)

// Fonctions privees
static void ACTMGR_declare_actuators();
static void ACTMGR_declare_functions();
static void ACTMGR_run_reset_act(queue_id_t queueId, void* useless_param_here, bool_e init);
static error_e ACTMGR_mae_reset_act(bool_e reset);

// Variables privées
static bool_e first_init_pos_done = FALSE; // Indique si une première séquence d'init en position a été effectuée

/**
 * @brief Fonction permettant de déclarer les actionneurs.
 * Cette fonction permet d'initialiser le tableau actuators[].
 * Ceci ne peut pas être fait statiquement à la compilation car les instances des actionneurs ne sont pas constantes.
 */
static void ACTMGR_declare_actuators() {
	Uint8 i = 0, j = 0;

	// Declaration des actionneurs
	if(I_AM_BIG()) {

		// Déclaration des actionneurs de BIG_ROBOT
		//ACT_DECLARE_ACTUATOR(exemple);
		ACT_DECLARE_ACTUATOR(big_elevator_front_right);
		ACT_DECLARE_ACTUATOR(big_elevator_front_middle);
		ACT_DECLARE_ACTUATOR(big_elevator_front_left);
		ACT_DECLARE_ACTUATOR(big_locker_front_right);
		ACT_DECLARE_ACTUATOR(big_locker_front_middle);
		ACT_DECLARE_ACTUATOR(big_locker_front_left);
		ACT_DECLARE_ACTUATOR(big_locker_back);
		ACT_DECLARE_ACTUATOR(big_slope_taker);
		ACT_DECLARE_ACTUATOR(big_sorting_back_very_right);
		ACT_DECLARE_ACTUATOR(big_sorting_back_right);
		ACT_DECLARE_ACTUATOR(big_sorting_back_left);
		ACT_DECLARE_ACTUATOR(big_sorting_back_very_left);

	} else {

		// Déclaration des actionneurs de SMALL_ROBOT
		//ACT_DECLARE_ACTUATOR(exemple);
		ACT_DECLARE_ACTUATOR(small_locker_back);
		ACT_DECLARE_ACTUATOR(small_locker_front_right);
		ACT_DECLARE_ACTUATOR(small_locker_front_left);
		ACT_DECLARE_ACTUATOR(small_sorting_back_left);
		ACT_DECLARE_ACTUATOR(small_sorting_back_right);
		ACT_DECLARE_ACTUATOR(small_sorting_back_middle);
		ACT_DECLARE_ACTUATOR(small_elevator_front_left);
		ACT_DECLARE_ACTUATOR(small_elevator_front_right);

	}

	// On remplit les dernières cases par des actionneurs vides.
	for(j = i; j < NB_ACTUATORS; j++) {
		actuators[j] = (ACTMGR_actuator_t) {0xFF, NULL, NULL};
	}

	// Vérification
	assert(i <= NB_ACTUATORS); // Nombre d'actionneurs déclarés supérieur à NB_ACTUATORS

	const char* robot_name = QS_WHO_AM_I_get_name();
	debug_printf("%s has %d actuators available.\n", robot_name, i);
}

/**
 * @brief Fonction permettant de déclarer les fonctions actionneurs.
 * Cette fonction permet d'initialiser le tableau act_functions[].
 * Cette affectation est faite dynamiquement pour être indépendant de l'ordre de déclaration des kinds dans l'enum Actuator_kind_e.
 */
static void ACTMGR_declare_functions() {
	ACT_DECLARE_FUNCTIONS_WITH_CONFIG_AND_IT(KIND_AX12_POS, IMPL_AX12_POS);
	ACT_DECLARE_FUNCTIONS_WITH_CONFIG(KIND_AX12_SPEED, IMPL_AX12_SPEED);
	ACT_DECLARE_FUNCTIONS_WITH_CONFIG_AND_IT(KIND_AX12_POS_DOUBLE, IMPL_AX12_POS_DOUBLE);
	ACT_DECLARE_FUNCTIONS_WITH_CONFIG_AND_IT(KIND_RX24_POS, IMPL_RX24_POS);
	ACT_DECLARE_FUNCTIONS_WITH_CONFIG(KIND_RX24_SPEED, IMPL_RX24_SPEED);
	ACT_DECLARE_FUNCTIONS_WITH_CONFIG_AND_IT(KIND_RX24_POS_DOUBLE, IMPL_RX24_POS_DOUBLE);
	ACT_DECLARE_FUNCTIONS(KIND_PWM, IMPL_PWM);
	ACT_DECLARE_FUNCTIONS(KIND_PPM, IMPL_PPM);
	ACT_DECLARE_FUNCTIONS(KIND_MOSFET, IMPL_MOSFET);
#ifdef USE_MOSTFET_BOARD
	ACT_DECLARE_FUNCTIONS(KIND_MOSFET_ON_MOSFET_BOARD, MOSFET_BOARD_ACT);
#endif
	act_functions[KIND_MOTOR_POS] = (ACT_functions_t){.init = &IMPL_MOTOR_POS_init,
													  .init_pos = NULL,
													  .stop = &IMPL_MOTOR_POS_stop,
													  .reset_config = NULL,
													  .process_msg = &IMPL_MOTOR_POS_CAN_process_msg,
													  .process_it = &IMPL_MOTOR_POS_process_it,
													  .process_main = &IMPL_MOTOR_POS_process_main,
													  .get_config = &IMPL_MOTOR_POS_get_config,
													  .set_config = &IMPL_MOTOR_POS_set_config,
													  .set_warner = NULL};
	ACT_DECLARE_FUNCTIONS(KIND_MOTOR_SPEED, IMPL_MOTOR_SPEED);
}

/**
 * @brief Fonction d'initialisation de ActManager. Initialise les actionneurs en position.
 */
void ACTMGR_init() {
	Uint8 i;

	first_init_pos_done = FALSE;

	// Declaration des fonctions et des instances des actionneurs
	ACTMGR_declare_functions();
	ACTMGR_declare_actuators();

	//Initialisation des actionneurs
	debug_printf("---------------- Initialisation des actionneurs -----------------\n");
	for(i = 0; i < NB_ACTUATORS; i++) {
		if(actuators[i].functions != NULL && actuators[i].functions->init != NULL) {
			QUEUE_set_actuator(actuators[i].queue_id, actuators[i].actuator);
			actuators[i].functions->init(actuators[i].actuator);
		}
	}
	debug_printf("-----------------------------------------------------------------\n");

	ACTMGR_reset_act();
}

/**
 * @brief Fonction qui indique à la queue de réinitialiser tous les actionneurs en position.
 */
void ACTMGR_reset_act() {
	QUEUE_add(QUEUE_RESET_ACT, &ACTMGR_run_reset_act, (QUEUE_arg_t){0, 0, NULL});
}

/**
 * @brief Fonction qui réinitialise tous les actionneurs en position.
 * @param queueId l'id de la queue
 * @param useless_param_here le paramètre correspondant à l'instance de l'actionneur (inutile ici)
 * @param init TRUE lors du premier appel pour faire l'init de l'action.
 */
static void ACTMGR_run_reset_act(queue_id_t queueId, void* useless_param_here, bool_e init) {
	static error_e ret_mae = IN_PROGRESS;
	static bool_e reset_act_launch = FALSE;

	if(init) {
		ACTMGR_mae_reset_act(TRUE);
		reset_act_launch = FALSE;
	} else {
		if(global.flags.power) { // Si il y a le +12/24V
			ret_mae = ACTMGR_mae_reset_act(FALSE);
			reset_act_launch = TRUE;
			if(ret_mae != IN_PROGRESS) {
				QUEUE_behead(queueId);
				first_init_pos_done = TRUE;
			}
		} else if(global.flags.match_started == TRUE || reset_act_launch) {
			//Le match a démarré, on arrete d'essayer de bouger les actionneurs
			QUEUE_behead(queueId);
			first_init_pos_done = TRUE;
		}
	}
}

/**
 * @brief Machine à étét qui initialise tous les actionneurs en position.
 * On attend TIME_BEFORE_INIT_POS avant de lancé le premier init.
 * On attent DELAY_INIT_POS entre chaque servo pour ne pas trop en demander à l'alim.
 * @param reset permet de réinitialiser la mae.
 * @return IN_PROGRESS si l'action est en cours et END_OK lorsque c'est fini.
 */
static error_e ACTMGR_mae_reset_act(bool_e reset) {

	CREATE_MAE(INIT,
			SELECT_ACTUATOR,
			INIT_POS_ACTUATOR,
			WAIT,
			SPECIAL_INIT,
			DONE);

	static time32_t local_time;
	static Uint8 act_id;
	static error_e status;
	char * name;

	if(reset){
		RESET_MAE();
		return IN_PROGRESS;
	}

	switch(state) {
		case INIT:
			if(entrance) {
				act_id = 0;
				local_time = global.absolute_time + TIME_BEFORE_INIT_POS;
			}

			if(global.absolute_time > local_time) {
				state = SELECT_ACTUATOR;
				debug_printf("------------------ Initialisation en position -------------------\n");
			}
			break;

		case SELECT_ACTUATOR:
			while(act_id < NB_ACTUATORS &&
					(actuators[act_id].functions == NULL ||
					actuators[act_id].functions->init_pos == NULL ||
					((Actuator_t*) actuators[act_id].actuator)->standard_init == FALSE)
			) {
				act_id++;
			}

			if(act_id >= NB_ACTUATORS)
				state = SPECIAL_INIT;
			else
				state = INIT_POS_ACTUATOR;
			break;

		case INIT_POS_ACTUATOR:
			if(entrance) {
				name = ((Actuator_t *) (actuators[act_id].actuator))->name;
				if(name == NULL || strncmp(name, "", 10) == 0) {
					name = "unknown";
				}
				debug_printf("Actuator \"%s\" init position\n", name);
			}
			status = actuators[act_id].functions->init_pos(actuators[act_id].actuator);

			if(status != IN_PROGRESS) {
				act_id++; // Incremente le act_id, celui-ci a été traité
				state = WAIT;
			}
			break;

		case WAIT:
			// Il est préférable d'attendre un peu entre l'init de deux actionneurs pour que l'alim se rétablisse correctement.
			if(entrance) {
				local_time = global.absolute_time + DELAY_INIT_POS;
			}
			if(global.absolute_time > local_time) {
				state = SELECT_ACTUATOR;
			}
			break;

		case SPECIAL_INIT:
			status = ACTMGR_special_init();
			if(status == END_OK) {
				state = DONE;
			}
			break;

		case DONE:
			debug_printf("-----------------------------------------------------------------\n");
			RESET_MAE();
			return END_OK;
			break;

		default:
			debug_printf("Default case of ACTMGR_mae_reset_act()\n");
	}
	return IN_PROGRESS;
}

/**
 * @brief Fonction pour initialiser les actionneurs particulier
 * A compléter en fonction des années
 */
error_e ACTMGR_special_init() {

	CREATE_MAE(INIT,
				INIT_LOCKER_LEFT,
				INIT_LOCKER_RIGHT,
				DONE);

	static time32_t local_time = 0;

	switch(state) {
			case INIT:
				if(entrance) {
					local_time = global.absolute_time;
				}
				if(global.absolute_time > local_time + 500) {
					state = INIT_LOCKER_LEFT;
				}
				break;

			case INIT_LOCKER_LEFT:
				if(entrance) {
					local_time = global.absolute_time;
				}
				if(global.absolute_time > local_time + DELAY_INIT_POS) {
					act_functions[KIND_RX24_POS].init_pos(&big_locker_front_left);
					debug_printf("Actuator \"%s\" init position\n", (big_locker_front_left.name != NULL) ? "unknown":big_locker_front_left.name);
					state = INIT_LOCKER_RIGHT;
				}
				break;

			case INIT_LOCKER_RIGHT:
				if(entrance) {
					local_time = global.absolute_time;
				}
				if(global.absolute_time > local_time + DELAY_INIT_POS) {
					act_functions[KIND_RX24_POS].init_pos(&big_locker_front_right);
					debug_printf("Actuator \"%s\" init position\n", (big_locker_front_right.name != NULL) ? "unknown":big_locker_front_right.name);
					state = DONE;
				}
				break;

			case DONE:
				RESET_MAE();
				return END_OK;
				break;

			default:
				debug_printf("Default case of ACTMGR_special_init()\n");
		}
		return IN_PROGRESS;
}

/**
 * @brief Fonction qui stoppe tous les actionneurs (asservissement et puissance).
 */
void ACTMGR_stop()
{
	Uint8 i;
	for(i = 0; i < NB_ACTUATORS; i++) {
		if(actuators[i].functions != NULL && actuators[i].functions->stop != NULL)
			actuators[i].functions->stop(actuators[i].actuator);
	}
}

/**
 * @brief Fonction permettant de réinitialiser la config de tous les actionneurs.
 */
void ACTMGR_reset_config() {
	Uint8 i;
	debug_printf("--------------- Re-initialisation des actionneurs ---------------\n");
	for(i = 0; i < NB_ACTUATORS; i++) {
		if(actuators[i].functions != NULL && actuators[i].functions->reset_config != NULL)
			actuators[i].functions->reset_config(actuators[i].actuator);
	}
	debug_printf("-----------------------------------------------------------------\n");
}

/**
 * @brief Fonction qui dispatch les messages CAN recus pour les ordres actionneurs.
 * @param msg le message CAN a traiter.
 * @return TRUE si le message a ete gere et FALSE sinon.
 */
bool_e ACTMGR_process_msg(CAN_msg_t* msg) {
	bool_e return_code = FALSE;

	// Recherche de l'index de l'actionneur concerné
	// (si non trouvé, index = -1 et le message est traité dans Can_msg_processing.c)
	Sint16 index = ACTMGR_get_actuator_index_by_sid(ACT_FILTER | msg->sid);

	// Dispatch du message
	if(index != -1 && actuators[index].functions != NULL && actuators[index].functions->process_msg != NULL) {
		return_code = actuators[index].functions->process_msg(actuators[index].actuator, msg);
	}

	return return_code; // TRUE si le message a été traité et FALSE sinon
}

/**
 * @brief Fonction qui execute le code d'IT de tous les actionneurs.
 * Cette fonction est appelé dans clock.c
 */
void ACTMGR_process_it() {
	Uint8 i;
	for(i = 0; i < NB_ACTUATORS; i++) {
		if(actuators[i].functions != NULL && actuators[i].functions->process_it != NULL)
			actuators[i].functions->process_it(actuators[i].actuator);
	}
}

/**
 * @brief Fonction qui execute le code de tâche de fond de tous les actionneurs.
 * Cette fonction est appelé dans main.c
 */
void ACTMGR_process_main() {
	Uint8 i;
	for(i = 0; i < NB_ACTUATORS; i++) {
		if(actuators[i].functions != NULL && actuators[i].functions->process_main != NULL)
			actuators[i].functions->process_main(actuators[i].actuator);
	}
}

/**
 * @brief Fonction qui dispatch les messages CAN recus pour le get config.
 */
void ACTMGR_get_config(CAN_msg_t* msg) {
	if(msg->sid == ACT_GET_CONFIG){
		// Recherche de l'index de l'actionneur concerné
		Sint16 index = ACTMGR_get_actuator_index_by_sid(ACT_FILTER | msg->data.act_get_config.act_sid);

		// Dispatch du message
		if(index != -1 && actuators[index].functions != NULL && actuators[index].functions->get_config != NULL) {
			actuators[index].functions->get_config(actuators[index].actuator, msg);
		} else {
			error_printf("Msg ACT_GET_CONFIG not processed for actuator sid=0x%x\n", msg->data.act_get_config.act_sid);
		}
	}
}

/**
 * @brief Fonction qui dispatch les messages CAN recus pour le set config.
 */
void ACTMGR_set_config(CAN_msg_t* msg) {
	if(msg->sid == ACT_SET_CONFIG){
		// Recherche de l'index de l'actionneur concerné
		Sint16 index = ACTMGR_get_actuator_index_by_sid(ACT_FILTER | msg->data.act_set_config.act_sid);

		// Dispatch du message
		if(index != -1 && actuators[index].functions != NULL && actuators[index].functions->set_config != NULL) {
			actuators[index].functions->set_config(actuators[index].actuator, msg);
		} else {
			error_printf("Msg ACT_SET_CONFIG not processed for actuator sid=0x%x\n", msg->data.act_set_config.act_sid);
		}
	}
}

/**
 * @brief Fonction qui dispatch les messages CAN recus pour les warners.
 */
void ACTMGR_set_warner(CAN_msg_t* msg) {
	if(msg->sid == ACT_WARNER){
		// Recherche de l'index de l'actionneur concerné
		Sint16 index = ACTMGR_get_actuator_index_by_sid(ACT_FILTER | msg->data.act_warner.act_sid);

		// Dispatch du message
		if(index != -1 && actuators[index].functions != NULL && actuators[index].functions->set_warner != NULL) {
			actuators[index].functions->set_warner(actuators[index].actuator, msg);
		} else {
			error_printf("Msg ACT_WARNER not processed for actuator sid=0x%x\n", msg->data.act_warner.act_sid);
		}
	}
}

/**
 * @brief Fonction permettant de chercher l'index d'un actionneur dans le tableau actuators[] a partir de son sid.
 * @param sid le sid de l'actionneur a chercher.
 * @return l'index de l'actionneur dans le tableau actuators[] ou -1 s'il n'a pas ete trouve.
 */
Sint16 ACTMGR_get_actuator_index_by_sid(Uint11 sid) {
	Uint8 i = 0;
	bool_e found = FALSE;
	Sint16 index = -1;

	while(i < NB_ACTUATORS && !found) {
		if( (actuators[i].actuator != NULL) && (((Actuator_t*) (actuators[i].actuator))->sid == sid) ) {
			found = TRUE;
			index = i;
		} else {
			i++;
		}
	}

//	if(!found) {
//		error_printf("Actuator not found in the actuators[] array : check this array and your sid = 0x%x\n", sid);
//	}

	return index;
}


/**
 * @brief Fonction de configuration des AX12.
 * @param servo l'instance du servo concerne (ici juste Actuator_servo_data_t suffit).
 * @param msg le message CAN contenant la nouvelle configuration.
 */
void ACTMGR_config_AX12(Actuator_servo_data_t * servo, CAN_msg_t* msg) {
	Uint8 percentage;
	switch(msg->data.act_set_config.config){
		case SPEED_CONFIG : // Configuration de la vitesse
			if(AX12_is_wheel_mode_enabled(servo->id)){
				if(msg->data.act_set_config.data_config.speed_wheel == 0){
					percentage = servo->speed;
				}else{
					percentage = msg->data.act_set_config.data_config.speed_wheel;
				}
				servo->simu.current_speed = percentage;
				if(!global.flags.virtual_mode) {
					AX12_set_speed_percentage(servo->id, percentage);
				}
				debug_printf("Configuration de la vitesse (wheel mode) de l'AX12 %d avec une valeur de %d\n", servo->id, msg->data.act_set_config.data_config.speed_wheel);
			}else{
				if(msg->data.act_set_config.data_config.speed_position == 0){
					percentage = servo->speed;
				}else{
					percentage = msg->data.act_set_config.data_config.speed_position;
				}
				servo->simu.current_speed = percentage;
				if(!global.flags.virtual_mode) {
					AX12_set_move_to_position_speed(servo->id, percentage);
				}
				debug_printf("Configuration de la vitesse (position mode) de l'AX12 %d avec une valeur de %d\n", servo->id, msg->data.act_set_config.data_config.speed_position);
			}
			break;

		case TORQUE_CONFIG : // Configuration du couple
			if(msg->data.act_set_config.data_config.torque == 0){
				percentage = servo->max_torque;
			}else{
				percentage = msg->data.act_set_config.data_config.torque;
			}
			if(!global.flags.virtual_mode) {
				AX12_set_torque_limit(servo->id, percentage);
			}
			debug_printf("Configuration du couple de l'AX12 %d avec une valeur de %d\n", servo->id, msg->data.act_set_config.data_config.torque);
			break;

		default :
			warn_printf("invalid CAN msg data[2]=%u (configuration impossible)!\n", msg->data.act_set_config.config);
	}
}

/**
 * @brief Fonction de configuration des RX24.
 * @param servo l'instance du servo concerne (ici juste Actuator_servo_data_t suffit).
 * @param msg le message CAN contenant la nouvelle configuration.
 */
void ACTMGR_config_RX24(Actuator_servo_data_t * servo, CAN_msg_t* msg) {
	Uint8 percentage;
	switch(msg->data.act_set_config.config){
		case SPEED_CONFIG : // Configuration de la vitesse
			if(RX24_is_wheel_mode_enabled(servo->id)){
				if(msg->data.act_set_config.data_config.speed_wheel == 0){
					percentage = servo->speed;
				}else{
					percentage = msg->data.act_set_config.data_config.speed_wheel;
				}
				servo->simu.current_speed = percentage;
				if(!global.flags.virtual_mode) {
					RX24_set_speed_percentage(servo->id, percentage);
				}
				debug_printf("Configuration de la vitesse (wheel mode) du RX24 %d avec une valeur de %d\n", servo->id, msg->data.act_set_config.data_config.speed_wheel);
			}else{
				if(msg->data.act_set_config.data_config.speed_position == 0){
					percentage = servo->speed;
				}else{
					percentage = msg->data.act_set_config.data_config.speed_position;
				}
				servo->simu.current_speed = percentage;
				if(!global.flags.virtual_mode) {
					RX24_set_move_to_position_speed(servo->id, percentage);
				}
				debug_printf("Configuration de la vitesse (position mode) du RX24 %d avec une valeur de %d\n", servo->id, msg->data.act_set_config.data_config.speed_position);
			}
			break;

		case TORQUE_CONFIG : // Configuration du couple
			if(msg->data.act_set_config.data_config.torque == 0){
				percentage = servo->max_torque;
			}else{
				percentage = msg->data.act_set_config.data_config.torque;
			}
			if(!global.flags.virtual_mode) {
				RX24_set_torque_limit(servo->id, percentage);
			}
			debug_printf("Configuration du couple du RX24 %d avec une valeur de %d\n", servo->id, msg->data.act_set_config.data_config.torque);
			break;

		default :
			warn_printf("invalid CAN msg data[2]=%u (configuration impossible)!\n", msg->data.act_set_config.config);
	}
}

/**
 * @brief Permet de savoir si la première séquence d'initialisation des actionneurs a été effectuée.
 * @return TRUE si la première séquence d'initialisation est terminée ou FALSE sinon.
 */
bool_e ACTMRG_is_first_init_pos_done() {
	return first_init_pos_done;
}
