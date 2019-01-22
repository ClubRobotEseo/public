/**
 *	Club Robot ESEO 2017 - 2018
 *
 * @file impl_rx24_pos_double.c
 * @brief Implémentation des fonctions pour un actionneur de type KIND_RX24_POS_DOUBLE.
 * @author Valentin
 */

// Les différents includes necessaires
#include "impl_rx24_pos_double.h"

#include "../QS/QS_CANmsgList.h"
#include "../QS/QS_actuator/QS_rx24.h"
#include "../QS/QS_lowLayer/QS_can.h"
#include "../act_queue_utils.h"
#include "../ActManager.h"
#include "../queue.h"

// Les différents define pour le verbose sur uart
#define LOG_PREFIX "[RX24_POS_D]  "
#define LOG_COMPONENT OUTPUT_LOG_COMPONENT_IMPL_RX24_POS_DOUBLE
#include "../QS/QS_outputlog.h"

// Les fonctions internes au fonctionnement de l'actionneur
static void IMPL_RX24_POS_DOUBLE_init_servo(Actuator_servo_double_t* actuator);
static void IMPL_RX24_POS_DOUBLE_get_position_config(Actuator_servo_double_t* actuator, ACT_order_e *order_ptr, Uint16 *pos_ptr);
static void IMPL_RX24_POS_DOUBLE_run_command_position(queue_id_t queueId, void* actuator_gen_ptr, bool_e init);
static void IMPL_RX24_POS_DOUBLE_command_position_init(Actuator_servo_double_t* actuator);
static void IMPL_RX24_POS_DOUBLE_command_position_run(Actuator_servo_double_t* actuator);
static void IMPL_RX24_POS_DOUBLE_add_torque_order(Actuator_servo_double_t *actuator, Uint8 cmd_index);
static void IMPL_RX24_POS_DOUBLE_run_command_torque(queue_id_t queueId, void* actuator_gen_ptr, bool_e init);
static void IMPL_RX24_POS_DOUBLE_check_warner(Actuator_servo_double_t* actuator, Uint16 pos);


/**************************************************************************/
/**	Fonctions usuelles de gestion de l'actionneur                        **/
/**************************************************************************/

/**
 * @brief Fonction permettant d'initialiser le servo.
 * Elle initialise le bus de communication ainsi que les différents registres du servo.
 * Cette fonction est appelée au lancement de la carte via ActManager.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
 */
void IMPL_RX24_POS_DOUBLE_init(void* actuator_gen_ptr) {
	Actuator_servo_double_t* actuator = (Actuator_servo_double_t*) actuator_gen_ptr;

	if(actuator->is_initialized)
		return;

	RX24_init();
	//IMPL_RX24_POS_DOUBLE_init_servo(actuator); // Inutile
}

/**
 * @brief Fonction qui réinitialise la config du servo.
 * Fonction appellée si la carte IHM a détecté une grosse chute de la tension d'alimentation des actionneurs.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
 */
void IMPL_RX24_POS_DOUBLE_reset_config(void* actuator_gen_ptr) {
	Actuator_servo_double_t* actuator = (Actuator_servo_double_t*) actuator_gen_ptr;
	actuator->is_initialized = FALSE;
	IMPL_RX24_POS_DOUBLE_init_servo(actuator);
}

/**
 * @brief Fonction qui initialise les registres du servo avec la config souhaitée.
 * Initialise le servo s'il n'était pas alimenté lors d'initialisations précédentes, si déjà initialisé, ne fait rien.
 * @param actuator l'instance de l'actionneur.
 */
static void IMPL_RX24_POS_DOUBLE_init_servo(Actuator_servo_double_t* actuator) {
	Uint8 i;
	bool_e is_ready[2];
	for(i = 0; i < 2; i++) {
		is_ready[i] = RX24_is_ready(actuator->servo[i].id);
	}

	if(actuator->is_initialized == FALSE && is_ready[0] && is_ready[1]) {
		actuator->is_initialized = TRUE;

		for(i = 0; i < 2; i++) {
			RX24_config_set_lowest_voltage(actuator->servo[i].id, RX24_MIN_VOLTAGE);
			RX24_config_set_highest_voltage(actuator->servo[i].id, RX24_MAX_VOLTAGE);
			RX24_set_torque_limit(actuator->servo[i].id, actuator->servo[i].max_torque);
			RX24_config_set_temperature_limit(actuator->servo[i].id, actuator->servo[i].max_temperature);

			RX24_config_set_maximal_angle(actuator->servo[i].id, actuator->servo[i].max_angle);
			RX24_config_set_minimal_angle(actuator->servo[i].id, actuator->servo[i].min_angle);
			RX24_set_move_to_position_speed(actuator->servo[i].id, actuator->servo[i].speed);

			RX24_config_set_error_before_led(actuator->servo[i].id, RX24_BEFORE_LED);
			RX24_config_set_error_before_shutdown(actuator->servo[i].id, RX24_BEFORE_SHUTDOWN);

			debug_printf("RX24 n°%d init config DONE\n", actuator->servo[i].id);
		}
	}else if(actuator->is_initialized == FALSE){
		for(i = 0; i < 2; i++) {
			if(is_ready[i]) {
				debug_printf("RX24 n°%d init config FAIL : ready\n", actuator->servo[i].id);
			} else {
				debug_printf("RX24 n°%d init config FAIL : not ready\n", actuator->servo[i].id);
			}
		}
	}
}

/**
 * Fonction appellée pour initialiser en position le servo dès l'arrivée de l'alimentation via ActManager.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
 * @return le status d'initialisation.
 */
error_e IMPL_RX24_POS_DOUBLE_init_pos(void *actuator_gen_ptr) {
	Actuator_servo_double_t* actuator = (Actuator_servo_double_t*) actuator_gen_ptr;
	Uint8 i;
	IMPL_RX24_POS_DOUBLE_init_servo(actuator);

	// Pour la simulation
	for(i = 0; i < 2; i++) {
		actuator->servo[i].simu.current_pos = actuator->servo[i].init_pos;
		actuator->servo[i].simu.current_speed = actuator->servo[i].speed;
	}

	if(actuator->is_initialized == FALSE)
		return END_OK;

	for(i = 0; i < 2; i++) {
		if(!RX24_set_position(actuator->servo[i].id, actuator->servo[i].init_pos))
			debug_printf("RX24 n°%d is not here\n", actuator->servo[i].id);
		else
			debug_printf("RX24 n°%d has been initialized in position\n", actuator->servo[i].id);
	}

	return END_OK;
}

/**
 * @brief Fonction pour stopper l'asservissement du servo.
 * Fonction appellée à la fin du match via ActManager.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
 */
void IMPL_RX24_POS_DOUBLE_stop(void *actuator_gen_ptr) {
	Actuator_servo_double_t* actuator = (Actuator_servo_double_t*) actuator_gen_ptr;
	Uint8 i;
	for(i = 0; i < 2; i++) {
		RX24_set_torque_enabled(actuator->servo[i].id, FALSE);
	}
}

/**
 * @brief Fonction de dispatch des messages CAN pour les ordres actionneurs.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
 * @param msg le message CAN reçu.
 * @return TRUE si le message CAN a été traité et FALSE sinon.
 */
bool_e IMPL_RX24_POS_DOUBLE_CAN_process_msg(void *actuator_gen_ptr, CAN_msg_t* msg) {
	Actuator_servo_double_t* actuator = (Actuator_servo_double_t*) actuator_gen_ptr;
	Uint8 i;
	bool_e order_is_valid = TRUE;
	Sint16 index[2] = {0};
	Command_kind_e kind[2] = {0};
	Sint16 param = 0;

	if(msg->sid == actuator->sid){
		//IMPL_RX24_POS_DOUBLE_init_servo(actuator);	// Inutile

		for(i = 0; i < 2; i++) {
			order_is_valid &= ACT_get_command_index(&index[i], msg->data.act_order.order, actuator->servo[i].commands, actuator->servo[i].nb_commands);
			kind[i] = actuator->servo[i].commands[index[i]].kind;
		}

		if(order_is_valid) {
			// Stoppe l'asservissement en couple avant la commande suivante
			if(actuator->servo[0].torque_asser.activated && actuator->servo[1].torque_asser.activated) {
				actuator->servo[0].torque_asser.ask_to_finish = TRUE;
				actuator->servo[1].torque_asser.ask_to_finish = TRUE;
			}

			// Puisqu'on ne peut pas stocker les valeurs des positions actionneurs dans le parametre
			// on stocke les index des commandes pour quand même avoir un acces assez direct.
			param = (index[1] << 8) | index[0];
			ACTQ_push_operation_from_msg(msg, actuator->queue_id, &IMPL_RX24_POS_DOUBLE_run_command_position, param, TRUE);

			// Ajoute l'ordre d'asservissement en couple si necessaire
			if(kind[0] == CMD_ASSER_TORQUE && kind[1] == CMD_ASSER_TORQUE) {
				IMPL_RX24_POS_DOUBLE_add_torque_order(actuator, param);
			} else if(kind[0] == CMD_ASSER_TORQUE || kind[1] == CMD_ASSER_TORQUE) {
				error_printf("Asser torque asked for only one servo for order=%u and sid=0x%x\n",  msg->data.act_order.order, actuator->sid);
			}
		} else {
			component_printf(LOG_LEVEL_Warning, "invalid CAN msg order=%u for sid=0x%x!\n",  msg->data.act_order.order, actuator->sid);
		}
		return TRUE;
	}
	return FALSE;
}


/**************************************************************************/
/**	Fonctions pour la gestion des configs                                **/
/**************************************************************************/

/**
 * @brief Fonction permettant de modifier la config du servo comme la vitesse ou le couple.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
 * @param msg le message CAN reçu.
 */
void IMPL_RX24_POS_DOUBLE_set_config(void *actuator_gen_ptr, CAN_msg_t* msg) {
	Actuator_servo_double_t* actuator = (Actuator_servo_double_t*) actuator_gen_ptr;
	Uint8 i;
	for(i = 0; i < 2; i++) {
		ACTMGR_config_RX24(&(actuator->servo[i]), msg);
	}
}

/**
 * @brief Fonction permettant d'obtenir des infos sur la config du servo comme la vitesse ou le couple.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
 * @param incoming_msg le message CAN reçu.
 */
void IMPL_RX24_POS_DOUBLE_get_config(void *actuator_gen_ptr, CAN_msg_t *incoming_msg) {
	Actuator_servo_double_t* actuator = (Actuator_servo_double_t*) actuator_gen_ptr;
	bool_e error = FALSE;
	Uint16 pos = 0;
	ACT_order_e order = 0;
	CAN_msg_t msg;
	msg.sid = ACT_GET_CONFIG_ANSWER;
	msg.size = SIZE_ACT_GET_CONFIG_ANSWER;
	msg.data.act_get_config_answer.act_sid = 0xFF & (actuator->sid);
	msg.data.act_get_config_answer.config = incoming_msg->data.act_set_config.config;

	switch(incoming_msg->data.act_set_config.config){
		case POSITION_CONFIG:
			IMPL_RX24_POS_DOUBLE_get_position_config(actuator, &order, &pos);
			msg.data.act_get_config_answer.act_get_config_data.act_get_config_pos_answer.order = order;
			msg.data.act_get_config_answer.act_get_config_data.act_get_config_pos_answer.pos = pos;
			break;
		case TORQUE_CONFIG:
			if(global.flags.virtual_mode) {
				msg.data.act_get_config_answer.act_get_config_data.torque = 0;
			} else {
				msg.data.act_get_config_answer.act_get_config_data.torque = RX24_get_speed_percentage(actuator->servo[0].id);
			}
			break;
		case TEMPERATURE_CONFIG:
			if(global.flags.virtual_mode) {
				msg.data.act_get_config_answer.act_get_config_data.temperature = 0;
			} else {
				msg.data.act_get_config_answer.act_get_config_data.temperature = RX24_get_temperature(actuator->servo[0].id);
			}
			break;
		case LOAD_CONFIG:
			if(global.flags.virtual_mode) {
				msg.data.act_get_config_answer.act_get_config_data.load = 0;
			} else {
				msg.data.act_get_config_answer.act_get_config_data.load = RX24_get_load_percentage(actuator->servo[0].id);
			}
			break;
		default:
			error = TRUE;
			error_printf("The config %u is not available\n", incoming_msg->data.act_set_config.config);
	}

	if(!error){
		CAN_send(&msg); // Envoi du message CAN
	}
}

/**
 * @brief Fonction permettant d'obtenir la position de l'actionneur en tant que "ordre actionneur".
 * Cette fonction convertit la position du rx24 entre 0 et 1023 en une position en tant que "ordre actionneur".
 * @param actuator l'instance de l'actionneur
 * @param order_ptr un pointeur dans lequel on doit stocker l'ordre de retour
 * @param pos_ptr un pointeur dans lequel on doit stocker la position courante de l'actionneur.
 */
static void IMPL_RX24_POS_DOUBLE_get_position_config(Actuator_servo_double_t* actuator, ACT_order_e *order_ptr, Uint16 *pos_ptr) {
	ACT_order_e order = ACT_DEFAULT_STOP;
	Sint16 pos = 0;
	Sint16 epsilon = (Sint16) actuator->servo[0].large_epsilon;

	if(global.flags.virtual_mode) {
		pos = actuator->servo[0].simu.current_pos;
	} else {
		pos = (Sint16) RX24_get_position(actuator->servo[0].id);
	}

	Uint8 i = 0;
	bool_e found = FALSE;
	while(i < actuator->servo[0].nb_commands && !found) {
		Sint16 value = actuator->servo[0].commands[i].value;
		if((pos > value - epsilon) && (pos < value + epsilon)){
			found = TRUE;
			order = actuator->servo[0].commands[i].order;
		}
		i++;
	}

	if(order_ptr != NULL)
		*order_ptr = order;

	if(pos_ptr != NULL)
		*pos_ptr = pos;
}


/**************************************************************************/
/**	Fonctions pour la gestion de l'asservissement en position            **/
/**************************************************************************/

/**
 * @brief Fonction appellée par la queue pendant tout le temps d'exécution de la commande en cours.
 * Fonction utilisée pour les commandes de position.
 * Le booleen init est à TRUE au premier lancement de la commande.
 * @param queueId l'id de la queue.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
 * @param init TRUE au premier appel pour initiliser la commande et FALSE ensuite.
 */
static void IMPL_RX24_POS_DOUBLE_run_command_position(queue_id_t queueId, void* actuator_gen_ptr, bool_e init) {
	Actuator_servo_double_t* actuator = (Actuator_servo_double_t*) actuator_gen_ptr;
	if(QUEUE_has_error(queueId)) {
		QUEUE_behead(queueId);
		return;
	}

	// Gestion des mouvements de l'actionneur
	if(queueId == actuator->queue_id) {
		if(init){
			IMPL_RX24_POS_DOUBLE_command_position_init(actuator);
		} else {
			IMPL_RX24_POS_DOUBLE_command_position_run(actuator);
		}
	}
}

/**
 * @brief Fonction permettant d'initialiser une commande.
 * Fonction utilisée pour les commandes de position.
 * @param actuator l'instance de l'actionneur.
 */
static void IMPL_RX24_POS_DOUBLE_command_position_init(Actuator_servo_double_t* actuator) {
	Uint8 i;
	ACT_order_e order = QUEUE_get_arg(actuator->queue_id)->command;
	Sint16 goal_position[2] = {0, 0}, param = 0, index = 0;
	bool_e error = FALSE;

	if(!global.flags.virtual_mode) {
		IMPL_RX24_POS_DOUBLE_init_servo(actuator);
	}

	if(order == ACT_DEFAULT_STOP) {
		if(!global.flags.virtual_mode) {
			for(i = 0; i < 2; i++) {
				RX24_set_torque_enabled(actuator->servo[i].id, FALSE); //Stopper l'asservissement de l'RX24
			}
		}
		QUEUE_next(actuator->queue_id, ACT_RESULT_DONE, ACT_RESULT_ERROR_OK, __LINE__);
		return;
	} else {
		// On obtient une valeur correcte car on a vérifié précédemment que l'ordre existait.
		param = QUEUE_get_arg(actuator->queue_id)->param;
		for(i = 0; i < 2; i++) {
			index = (param >> (8 * i)) & 0xFF;
			goal_position[i] = actuator->servo[i].commands[index].value;
		}
	}

	if(!global.flags.virtual_mode && actuator->is_initialized == FALSE){
		error_printf("Impossible to set the actuator sid=0x%x in position, it is not initialized\n", actuator->sid);
		QUEUE_next(actuator->queue_id, ACT_RESULT_NOT_HANDLED, ACT_RESULT_ERROR_NOT_INITIALIZED, __LINE__);
		return;
	}


//	if(!order_is_valid) {
//		error_printf("Invalid RX24 position for order: %u, code is broken !\n", order);
//		QUEUE_next(actuator->queue_id, actuator->sid, ACT_RESULT_NOT_HANDLED, ACT_RESULT_ERROR_LOGIC, __LINE__);
//		return;
//	}

	for(i = 0; i < 2; i++) {
		if(global.flags.virtual_mode) {
			// Calcul du temps d'execution de la commande (en ms)
			time32_t exec_time = ACT_SERVO_compute_cmd_exec_time(actuator->kind, actuator->servo[i].simu.current_speed, goal_position[i], actuator->servo[i].simu.current_pos);
			actuator->servo[i].simu.exec_finished_time = global.absolute_time + exec_time;
			debug_printf("Move RX24 n°%u to position %d\n", actuator->servo[i].id, goal_position[i]);
			debug_printf("Simulation: RX24 n°%u take %ld ms to move.\n", actuator->servo[i].id, exec_time);
		} else {
			RX24_reset_last_error(actuator->servo[i].id); //Sécurité anti terroriste. Nous les parano on aime pas voir des erreurs là ou il n'y en a pas.
			RX24_set_torque_enabled(actuator->servo[i].id, TRUE); // Activation de l'asservissement de l'RX24
			if(!RX24_set_position(actuator->servo[i].id, goal_position[i])) {
				//Si la commande n'a pas été envoyée correctement et/ou que l'RX24 ne répond pas a cet envoi, on l'indique à la carte stratégie
				error_printf("RX24 n°%u don't answered when setting position: error=0x%x\n",  actuator->servo[i].id, RX24_get_last_error(actuator->servo[i].id).error);
				error = TRUE;
			} else {
				//La commande a été envoyée et l'RX24 l'a bien reçu
				debug_printf("Move RX24 n°%u to position %d\n", actuator->servo[i].id, goal_position[i]);
			}
		}
	}

	// en cas d'erreur de l'un des servos
	if(error) {
		debug_printf("Actuator sid=0x%x failed: not here ?\n", actuator->sid);
		QUEUE_next(actuator->queue_id, ACT_RESULT_FAILED, ACT_RESULT_ERROR_NOT_HERE, __LINE__);
	}
}

/**
 * @brief Fonction qui vérifie l'exécution de la commande en cours.
 * Fonction utilisée pour les commandes de position.
 * @param actuator l'instance de l'actionneur.
 */
static void IMPL_RX24_POS_DOUBLE_command_position_run(Actuator_servo_double_t* actuator) {
	Uint8 result[2] = {0}, error_code[2] = {0};
	Uint16 line[2] = {0};
	Sint16 pos[2] = {0}, wanted_goal[2] = {0};
	bool_e check_finish[2] = {0};
	Sint16 index = 0;
	Uint8 i = 0;

	Sint16 param = QUEUE_get_arg(actuator->queue_id)->param;

	for(i = 0; i < 2; i++) {
		if(!check_finish[i]) {
			index = (param >> (8 * i)) & 0xFF;
			wanted_goal[i] = actuator->servo[i].commands[index].value;
			if(global.flags.virtual_mode) {
				if(global.absolute_time > actuator->servo[i].simu.exec_finished_time) {
					check_finish[i] = TRUE;
					result[i] = ACT_RESULT_DONE;
					error_code[i] = ACT_RESULT_ERROR_OK;
					line[i] = __LINE__;
					actuator->servo[i].simu.current_pos = wanted_goal[i];
				}
			} else {
				pos[i] = RX24_get_position(actuator->servo[i].id);
				check_finish[i] = ACTQ_check_status_rx24(actuator->queue_id, actuator->servo[i].id, wanted_goal[i], pos[i], actuator->servo[i].epsilon, actuator->servo[i].timeout, actuator->servo[i].large_epsilon, &result[i], &error_code[i], &line[i]);
			}
		}
	}

	if(check_finish[0] && check_finish[1]) {
		if(result[0] == ACT_RESULT_DONE && result[1] == ACT_RESULT_DONE) {
			QUEUE_next(actuator->queue_id, ACT_RESULT_DONE, ACT_RESULT_ERROR_OK, line[0]);
		} else if(result[0] != ACT_RESULT_DONE) {
			QUEUE_next(actuator->queue_id, result[0], error_code[0], line[0]);
		} else {
			QUEUE_next(actuator->queue_id, result[1], error_code[1], line[1]);
		}
	}

    // On ne surveille le warner que si il est activé
	if(actuator->warner.activated)
		IMPL_RX24_POS_DOUBLE_check_warner(actuator, pos[0]);
}


/**************************************************************************/
/**	Fonctions pour la gestion de l'asservissement en couple              **/
/**************************************************************************/

/**
 * @brief Fonction permettant d'ajouter un ordre d'asservissement en couple dans la queue.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
 * @param cmd_index les deux index de la commande à exécuter.
 */
static void IMPL_RX24_POS_DOUBLE_add_torque_order(Actuator_servo_double_t *actuator, Uint8 cmd_index) {
	Uint8 i;
	Uint8 threshold[2] = {0};
	for(i = 0; i < 2; i++) {
		Uint8 index = (cmd_index >> (8 * i)) & 0xFF;
		threshold[i] = actuator->servo[i].commands[index].param.torque.threshold;
	}

	if(threshold[0] <= 100 && threshold[1] <= 100) {
		QUEUE_add(actuator->queue_id, &IMPL_RX24_POS_DOUBLE_run_command_torque, (QUEUE_arg_t){ACT_SERVO_ASSER_TORQUE, cmd_index, &ACTQ_finish_SendNothing, TRUE});
	} else {
		error_printf("Error in settings of torque asser for actuator sid=0x%u\n", actuator->sid);
	}
}

/**
 * @brief Fonction appellée par la queue pendant tout le temps d'exécution de la commande en cours.
 * Fonction utilisée pour les commandes de couple.
 * Le booleen init est a TRUE au premier lancement de la commande.
 * @param queueId l'id de la queue.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
 * @param init TRUE au premier appel pour initiliser la commande et FALSE ensuite.
 */
void IMPL_RX24_POS_DOUBLE_run_command_torque(queue_id_t queueId, void* actuator_gen_ptr, bool_e init) {
	Actuator_servo_double_t* actuator = (Actuator_servo_double_t*) actuator_gen_ptr;

	if(QUEUE_has_error(queueId)) {
		QUEUE_behead(queueId);
		return;
	}

	// Gestion des mouvements de l'actionneur
	if(queueId == actuator->queue_id) {
		if(init) {
			actuator->servo[0].torque_asser.activated = TRUE;
			actuator->servo[1].torque_asser.activated = TRUE;
		} // sinon rien à faire, tout est fait en interruption.
	}
}


/**************************************************************************/
/**	Fonctions pour la gestion des warners                                **/
/**************************************************************************/

/**
 * @brief Fonction permettant d'activer un warner sur une position du servo.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
 * @param msg le message CAN reçu.
 */
void IMPL_RX24_POS_DOUBLE_set_warner(void* actuator_gen_ptr, CAN_msg_t *msg) {
	Actuator_servo_double_t* actuator = (Actuator_servo_double_t*) actuator_gen_ptr;
	Sint16 warner_pos = 0;
	bool_e warner_is_valid = ACT_get_command_value(&warner_pos, msg->data.act_warner.warner_param, actuator->servo[0].commands, actuator->servo[0].nb_commands);

	if(warner_is_valid){
		actuator->warner.warner_value = warner_pos;
		actuator->warner.activated = TRUE;
		actuator->warner.last_value = RX24_get_position(actuator->servo[0].id);
	} else {
		actuator->warner.activated = FALSE;
		warn_printf("Position not available for setting a warner: sid=0x%x order=%u\n", actuator->sid, msg->data.act_warner.warner_param);
	}
}

/**
 * @brief Fonction permettant de vérifier si le warner est franchi lors du mouvement du servo.
 * @param actuator l'instance de l'actionneur.
 * @param pos la position courante du servo.
 */
static void IMPL_RX24_POS_DOUBLE_check_warner(Actuator_servo_double_t* actuator, Uint16 pos) {
	CAN_msg_t msg;
	Sint16 warner_pos = actuator->warner.warner_value;
	Sint16 last_pos = actuator->warner.last_value;

	if( (last_pos < pos && last_pos < warner_pos && pos > warner_pos)
     || (last_pos > pos && last_pos > warner_pos && pos < warner_pos)
	 || (last_pos > RX24_MAX_WARNER && pos < RX24_MIN_WARNER && (last_pos < warner_pos || pos > warner_pos))
	 || (last_pos < RX24_MIN_WARNER && pos > RX24_MAX_WARNER && (last_pos > warner_pos || pos < warner_pos))
	){
		// Envoi du message CAN pour prévenir la strat
		msg.sid = ACT_WARNER_ANSWER;
		msg.size = SIZE_ACT_WARNER_ANSWER;
		msg.data.act_warner_answer.act_sid = 0xFF & (actuator->sid);
		CAN_send(&msg);

		// Désactivation du warner
		actuator->warner.activated = FALSE;
	}

	actuator->warner.last_value = pos;
}


/**************************************************************************/
/**	Fonction d'interruption                                              **/
/**************************************************************************/

/**
 * @brief Fonction d'interruption d'un servo.
 * Cette fonction permet d'effectuer l'asservissement en couple d'un servo.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
 */
void IMPL_RX24_POS_DOUBLE_process_it(void* actuator_gen_ptr) {
	Actuator_servo_double_t* actuator = (Actuator_servo_double_t*) actuator_gen_ptr;
	Uint8 i = 0;

	// Asservissement en couple
	if(actuator->servo[0].torque_asser.activated && actuator->servo[1].torque_asser.activated) {
		if(actuator->servo[0].torque_asser.ask_to_finish || actuator->servo[1].torque_asser.ask_to_finish ) {
			// On stoppe l'asservissement en couple suite à l'arrivée d'un nouvel ordre
			for(i = 0; i < 2; i++) {
				actuator->servo[i].torque_asser.activated = FALSE;
				actuator->servo[i].torque_asser.ask_to_finish = FALSE;
			}
			QUEUE_next(actuator->queue_id, ACT_RESULT_DONE, ACT_RESULT_ERROR_OK, __LINE__);
		} else if(!global.flags.virtual_mode) {
			// On exécute l'asservissement en couple
			Uint8 index = 0;
			Uint8 cmd_index = QUEUE_get_arg(actuator->queue_id)->param;
			Way_asser_torque_e way[2] = {0};
			Sint8 wanted_load[2] = {0}, current_load[2] = {0};
			Uint16 current_pos[2] = {0}, limit_pos[2] = {0};
			Sint32 torque_error[2] = {0};
			Sint32 new_pos[2] = {0};

			for(i = 0; i < 2; i++) {
				index = (cmd_index >> (8 * i)) & 0xFF;
				way[i] = actuator->servo[0].commands[index].param.torque.way;
				current_load[i] = - RX24_get_load_percentage(actuator->servo[i].id);
				current_pos[i] = RX24_get_position(actuator->servo[i].id);
				limit_pos[i] = actuator->servo[i].commands[index].value;

				if(way[i] == WAY_ASSER_TORQUE_INC) {
					wanted_load[i] = actuator->servo[i].commands[index].param.torque.threshold;
				} else {
					wanted_load[i] = - actuator->servo[i].commands[index].param.torque.threshold;
				}
			}

			// Calcul des nouvelles positions.
			for(i = 0; i < 2; i++) {
				torque_error[i] = wanted_load[i] - current_load[i];
				new_pos[i] = current_pos[i] + ( (actuator->servo[i].torque_asser.Kp * torque_error[i]) >> 6 );
				new_pos[i] = ACT_compute_modulo(new_pos[i], DEFAULT_SERVO_MAX_VALUE);

				// Désactivation de l'asservissement si on dépasse la position limite
				if((way[i] == WAY_ASSER_TORQUE_INC && current_pos[i] >= limit_pos[i])
				|| (way[i] == WAY_ASSER_TORQUE_DEC && current_pos[i] <= limit_pos[i])){
					new_pos[i] = limit_pos[i];
					actuator->servo[i].torque_asser.activated = FALSE;
				}

				if(!RX24_set_position(actuator->servo[i].id, new_pos[i])) {
					//Si la commande n'a pas été envoyée correctement et/ou que le RX24 ne répond pas a cet envoi, on stoppe l'asservissement
					actuator->servo[i].torque_asser.activated = FALSE;
				}
			}

			if(!actuator->servo[0].torque_asser.activated || !actuator->servo[1].torque_asser.activated) {
				// Si l'asservissement en couple doit être arrêté suite à une erreur, on passe à l'action suivante
				actuator->servo[0].torque_asser.activated = FALSE;
				actuator->servo[1].torque_asser.activated = FALSE;
				QUEUE_next(actuator->queue_id, ACT_RESULT_FAILED, ACT_RESULT_ERROR_NOT_HERE, __LINE__);
			}
		}
	}
}

