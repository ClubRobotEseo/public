/**
 *	Club Robot ESEO 2017 - 2018
 *
 * @file impl_ax12_pos.c
 * @brief Implémentation des fonctions pour un actionneur de type KIND_AX12_POS.
 * @author Valentin
 */

// Les différents includes necessaires
#include "impl_ax12_pos.h"

#include "../QS/QS_CANmsgList.h"
#include "../QS/QS_actuator/QS_ax12.h"
#include "../QS/QS_lowLayer/QS_can.h"
#include "../act_queue_utils.h"
#include "../ActManager.h"
#include "../queue.h"

// Les différents define pour le verbose sur uart
#define LOG_PREFIX "[AX12_POS_S]  "
#define LOG_COMPONENT OUTPUT_LOG_COMPONENT_IMPL_AX12_POS
#include "../QS/QS_outputlog.h"

// Les fonctions internes au fonctionnement de l'actionneur
static void IMPL_AX12_POS_init_servo(Actuator_servo_t* actuator);
static void IMPL_AX12_POS_get_position_config(Actuator_servo_t* actuator, ACT_order_e *order_ptr, Uint16 *pos_ptr);
static void IMPL_AX12_POS_run_command_position(queue_id_t queueId, void* actuator_gen_ptr, bool_e init);
static void IMPL_AX12_POS_command_position_init(Actuator_servo_t* actuator);
static void IMPL_AX12_POS_command_position_run(Actuator_servo_t* actuator);
static void IMPL_AX12_POS_run_command_torque(queue_id_t queueId, void* actuator_gen_ptr, bool_e init);
static void IMPL_AX12_POS_add_torque_order(Actuator_servo_t *actuator, Uint8 cmd_index);


/**************************************************************************/
/**	Fonctions usuelles de gestion de l'actionneur                        **/
/**************************************************************************/

/**
 * @brief Fonction permettant d'initialiser le servo.
 * Elle initialise le bus de communication ainsi que les différents registres du servo.
 * Cette fonction est appelée au lancement de la carte via ActManager.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
 */
void IMPL_AX12_POS_init(void* actuator_gen_ptr) {
	Actuator_servo_t* actuator = (Actuator_servo_t*) actuator_gen_ptr;

	if(actuator->is_initialized)
		return;

	AX12_init();
	//IMPL_AX12_POS_init_servo(actuator); // Inutile

}

/**
 * @brief Fonction qui réinitialise la config du servo.
 * Fonction appellée si la carte IHM a détecté une grosse chute de la tension d'alimentation des actionneurs.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
 */
void IMPL_AX12_POS_reset_config(void* actuator_gen_ptr) {
	Actuator_servo_t* actuator = (Actuator_servo_t*) actuator_gen_ptr;
	actuator->is_initialized = FALSE;
	IMPL_AX12_POS_init_servo(actuator);
}

/**
 * @brief Fonction qui initialise les registres du servo avec la config souhaitée.
 * Initialise le servo s'il n'était pas alimenté lors d'initialisations précédentes, si déjà initialisé, ne fait rien.
 * @param actuator l'instance de l'actionneur.
 */
static void IMPL_AX12_POS_init_servo(Actuator_servo_t* actuator) {
	if(actuator->is_initialized == FALSE && AX12_is_ready(actuator->servo.id) == TRUE) {
		actuator->is_initialized = TRUE;
		AX12_config_set_lowest_voltage(actuator->servo.id, AX12_MIN_VOLTAGE);
		AX12_config_set_highest_voltage(actuator->servo.id, AX12_MAX_VOLTAGE);
		AX12_set_torque_limit(actuator->servo.id, actuator->servo.max_torque);
		AX12_config_set_temperature_limit(actuator->servo.id, actuator->servo.max_temperature);

		AX12_config_set_maximal_angle(actuator->servo.id, actuator->servo.max_angle);
		AX12_config_set_minimal_angle(actuator->servo.id, actuator->servo.min_angle);
		AX12_set_move_to_position_speed(actuator->servo.id, actuator->servo.speed);

		AX12_config_set_error_before_led(actuator->servo.id, AX12_BEFORE_LED);
		AX12_config_set_error_before_shutdown(actuator->servo.id, AX12_BEFORE_SHUTDOWN);
		debug_printf("AX12 n°%u init config DONE\n", actuator->servo.id);
	}else if(actuator->is_initialized == FALSE)
		debug_printf("AX12 n°%u init config FAIL\n", actuator->servo.id);
}

/**
 * Fonction appellée pour initialiser en position le servo dès l'arrivée de l'alimentation via ActManager.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
 * @return le status d'initialisation.
 */
error_e IMPL_AX12_POS_init_pos(void *actuator_gen_ptr) {
	Actuator_servo_t* actuator = (Actuator_servo_t*) actuator_gen_ptr;
	IMPL_AX12_POS_init_servo(actuator);

	// Pour la simulation
	actuator->servo.simu.current_pos = actuator->servo.init_pos;
	actuator->servo.simu.current_speed = actuator->servo.speed;

	if(actuator->is_initialized == FALSE)
		return END_OK;

	if(!AX12_set_position(actuator->servo.id, actuator->servo.init_pos))
		debug_printf("AX12 n°%u is not here\n", actuator->servo.id);
	else
		debug_printf("AX12 n°%u has been initialized in position\n", actuator->servo.id);

	return END_OK;
}

/**
 * @brief Fonction pour stopper l'asservissement du servo.
 * Fonction appellée à la fin du match via ActManager.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
 */
void IMPL_AX12_POS_stop(void *actuator_gen_ptr) {
	Actuator_servo_t* actuator = (Actuator_servo_t*) actuator_gen_ptr;
	AX12_set_torque_enabled(actuator->servo.id, FALSE);
}

/**
 * @brief Fonction de dispatch des messages CAN pour les ordres actionneurs.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
 * @param msg le message CAN reçu.
 * @return TRUE si le message CAN a été traité et FALSE sinon.
 */
bool_e IMPL_AX12_POS_CAN_process_msg(void *actuator_gen_ptr, CAN_msg_t* msg) {
	Actuator_servo_t* actuator = (Actuator_servo_t*) actuator_gen_ptr;
	if(msg->sid == actuator->sid){
		//IMPL_AX12_POS_init_servo(actuator); // Inutile

		Sint16 param = 0;
		bool_e order_is_valid = ACT_get_command_index(&param, msg->data.act_order.order, actuator->servo.commands, actuator->servo.nb_commands);
		Command_kind_e kind = actuator->servo.commands[param].kind;

		if(order_is_valid) {
			// Stoppe l'asservissement en couple avant la commande suivante
			if(actuator->servo.torque_asser.activated) {
				actuator->servo.torque_asser.ask_to_finish = TRUE;
			}

			// Ajoute l'ordre d'asservissement demande
			ACTQ_push_operation_from_msg(msg, actuator->queue_id, &IMPL_AX12_POS_run_command_position, param, TRUE);

			// Ajoute l'ordre d'asservissement en couple si necessaire
			if(kind == CMD_ASSER_TORQUE) {
				IMPL_AX12_POS_add_torque_order(actuator, param);
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
void IMPL_AX12_POS_set_config(void *actuator_gen_ptr, CAN_msg_t* msg) {
	Actuator_servo_t* actuator = (Actuator_servo_t*) actuator_gen_ptr;
	ACTMGR_config_AX12(&(actuator->servo), msg);
}

/**
 * @brief Fonction permettant d'obtenir des infos sur la config du servo comme la vitesse ou le couple.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
 * @param incoming_msg le message CAN reçu.
 */
void IMPL_AX12_POS_get_config(void *actuator_gen_ptr, CAN_msg_t *incoming_msg) {
	Actuator_servo_t* actuator = (Actuator_servo_t*) actuator_gen_ptr;
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
			IMPL_AX12_POS_get_position_config(actuator, &order, &pos);
			msg.data.act_get_config_answer.act_get_config_data.act_get_config_pos_answer.order = order;
			msg.data.act_get_config_answer.act_get_config_data.act_get_config_pos_answer.pos = pos;
			break;
		case TORQUE_CONFIG:
			if(global.flags.virtual_mode) {
				msg.data.act_get_config_answer.act_get_config_data.torque = 0;
			} else {
				msg.data.act_get_config_answer.act_get_config_data.torque = AX12_get_speed_percentage(actuator->servo.id);
			}
			break;
		case TEMPERATURE_CONFIG:
			if(global.flags.virtual_mode) {
				msg.data.act_get_config_answer.act_get_config_data.temperature = 0;
			} else {
				msg.data.act_get_config_answer.act_get_config_data.temperature = AX12_get_temperature(actuator->servo.id);
			}
			break;
		case LOAD_CONFIG:
			if(global.flags.virtual_mode) {
				msg.data.act_get_config_answer.act_get_config_data.load = 0;
			} else {
				msg.data.act_get_config_answer.act_get_config_data.load = AX12_get_load_percentage(actuator->servo.id);
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
 * Cette fonction convertit la position du ax12 entre 0 et 1023 en une position en tant que "ordre actionneur".
 * @param actuator l'instance de l'actionneur
 * @param order_ptr un pointeur dans lequel on doit stocker l'ordre de retour
 * @param pos_ptr un pointeur dans lequel on doit stocker la position courante de l'actionneur.
 */
static void IMPL_AX12_POS_get_position_config(Actuator_servo_t* actuator, ACT_order_e *order_ptr, Uint16 *pos_ptr) {
	ACT_order_e order = ACT_DEFAULT_STOP;
	Sint16 pos = 0;
	Sint16 epsilon = (Sint16) actuator->servo.large_epsilon;

	if(global.flags.virtual_mode) {
		pos = actuator->servo.simu.current_pos;
	} else {
		pos = (Sint16) AX12_get_position(actuator->servo.id);
	}

	Uint8 i = 0;
	bool_e found = FALSE;
	while(i < actuator->servo.nb_commands && !found) {
		Sint16 value = actuator->servo.commands[i].value;
		if((pos > value - epsilon) && (pos < value + epsilon)){
			found = TRUE;
			order = actuator->servo.commands[i].order;
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
static void IMPL_AX12_POS_run_command_position(queue_id_t queueId, void* actuator_gen_ptr, bool_e init) {
	Actuator_servo_t* actuator = (Actuator_servo_t*) actuator_gen_ptr;

	if(QUEUE_has_error(queueId)) {
		QUEUE_behead(queueId);
		return;
	}

	// Gestion des mouvements de l'actionneur
	if(queueId == actuator->queue_id) {
		if(init) {
			IMPL_AX12_POS_command_position_init(actuator);
		} else {
			IMPL_AX12_POS_command_position_run(actuator);
		}
	}
}

/**
 * @brief Fonction permettant d'initialiser une commande.
 * Fonction utilisée pour les commandes de position.
 * @param actuator l'instance de l'actionneur.
 */
static void IMPL_AX12_POS_command_position_init(Actuator_servo_t* actuator) {
	ACT_order_e order = QUEUE_get_arg(actuator->queue_id)->command;
	Uint8 cmd_index = 0;
	Sint16 goal_position = 0;

	if(!global.flags.virtual_mode) {
		IMPL_AX12_POS_init_servo(actuator);
	}

	if(order == ACT_DEFAULT_STOP) {
		if(!global.flags.virtual_mode) {
			AX12_set_torque_enabled(actuator->servo.id, FALSE); //Stopper l'asservissement de l'AX12
		}
		QUEUE_next(actuator->queue_id, ACT_RESULT_DONE, ACT_RESULT_ERROR_OK, __LINE__);
		return;
	} else {
		// On obtient une valeur correcte car on a vérifié précédemment que l'ordre existait.
		cmd_index = QUEUE_get_arg(actuator->queue_id)->param;
		if(cmd_index < actuator->servo.nb_commands) {
			goal_position = actuator->servo.commands[cmd_index].value;
		} else {
			goal_position = ERROR_ACT_VALUE;
		}
	}

	if(!global.flags.virtual_mode && actuator->is_initialized == FALSE){
		error_printf("Impossible to set the actuator sid=0x%x in position, it is not initialized\n", actuator->sid);
		QUEUE_next(actuator->queue_id, ACT_RESULT_NOT_HANDLED, ACT_RESULT_ERROR_NOT_INITIALIZED, __LINE__);
		return;
	}

	if(goal_position == ERROR_ACT_VALUE) {
		error_printf("Invalid AX12 position for order: %u, code is broken !\n", order);
		QUEUE_next(actuator->queue_id, ACT_RESULT_NOT_HANDLED, ACT_RESULT_ERROR_LOGIC, __LINE__);
		return;
	}

	if(global.flags.virtual_mode) {
		// Calcul du temps d'execution de la commande (en ms)
		time32_t exec_time = ACT_SERVO_compute_cmd_exec_time(actuator->kind, actuator->servo.simu.current_speed, goal_position, actuator->servo.simu.current_pos);
		actuator->servo.simu.exec_finished_time = global.absolute_time + exec_time;
		debug_printf("Simulation: AX12 n°%u take %ld ms to move.\n", actuator->servo.id, exec_time);
	} else {
		AX12_reset_last_error(actuator->servo.id); //Sécurité anti terroriste. Nous les parano on aime pas voir des erreurs là ou il n'y en a pas.
		AX12_set_torque_enabled(actuator->servo.id, TRUE); // Activation de l'asservissement de l'AX12
		if(!AX12_set_position(actuator->servo.id, goal_position)) {	//Si la commande n'a pas été envoyée correctement et/ou que l'AX12 ne répond pas a cet envoi, on l'indique à la carte stratégie
			error_printf("AX12 n°%u don't answered when setting position: error=0x%x\n",  actuator->servo.id, AX12_get_last_error(actuator->servo.id).error);
			QUEUE_next(actuator->queue_id, ACT_RESULT_FAILED, ACT_RESULT_ERROR_NOT_HERE, __LINE__);
			return;
		}
	}
	//La commande a été envoyée et l'AX12 l'a bien reçu
	debug_printf("Move AX12 n°%u to position %d\n", actuator->servo.id, goal_position);
}

/**
 * @brief Fonction qui vérifie l'exécution de la commande en cours.
 * Fonction utilisée pour les commandes de position.
 * @param actuator l'instance de l'actionneur.
 */
static void IMPL_AX12_POS_command_position_run(Actuator_servo_t* actuator) {
	Uint8 result = 0, error_code = 0;
	Uint16 line = 0;

	Sint16 cmd_index = QUEUE_get_arg(actuator->queue_id)->param;
	Command_kind_e cmd_kind = actuator->servo.commands[cmd_index].kind;
	Sint16 wanted_goal = actuator->servo.commands[cmd_index].value;
	Sint16 current_goal = 0;

	if(global.flags.virtual_mode) {
		if(global.absolute_time > actuator->servo.simu.exec_finished_time) {
			actuator->servo.simu.current_pos = wanted_goal;
			QUEUE_next(actuator->queue_id, ACT_RESULT_DONE, ACT_RESULT_ERROR_OK, __LINE__);
		}
	} else {
		current_goal = AX12_get_position(actuator->servo.id);
		if(ACTQ_check_status_ax12(actuator->queue_id, actuator->servo.id, wanted_goal, current_goal, actuator->servo.epsilon, actuator->servo.timeout, actuator->servo.large_epsilon, &result, &error_code, &line)) {
			if(cmd_kind == CMD_ASSER_POSITION){
				QUEUE_next(actuator->queue_id, result, error_code, line);
			} else if(cmd_kind == CMD_ASSER_TORQUE){
				if(result == ACT_RESULT_FAILED && error_code == ACT_RESULT_ERROR_TIMEOUT) {
					// L'action a échoué, c'est donc qu'on a rencontré un obstacle comme on le souhaitait.
					// On passe à l'ordre d'asservissement en couple.
					QUEUE_next(actuator->queue_id, ACT_RESULT_DONE, ACT_RESULT_ERROR_OK, line);
				} else if(result == ACT_RESULT_DONE) {
					// Le servo a atteint sa position finale. C'est un échec puisqu'on devait faire de l'asservissement en couple.
					QUEUE_next(actuator->queue_id, ACT_RESULT_FAILED, ACT_RESULT_ERROR_OTHER, line);
				} else {
					// Une erreur s'est produite, on remonte cette erreur à la stratégie.
					QUEUE_next(actuator->queue_id, result, error_code, line);
				}
			} else {
				error_printf("Error when running command for actuator sid=0x%u\n", actuator->sid);
				QUEUE_next(actuator->queue_id, ACT_RESULT_FAILED, ACT_RESULT_ERROR_CANCELED, line);
			}
		}
	}

    // On ne surveille le warner que si il est activé
	if(actuator->warner.activated)
		IMPL_AX12_POS_check_warner(actuator, current_goal);
}


/**************************************************************************/
/**	Fonctions pour la gestion de l'asservissement en couple              **/
/**************************************************************************/

/**
 * @brief Fonction permettant d'ajouter un ordre d'asservissement en couple dans la queue.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
 * @param cmd_index l'index de la commande à exécuter.
 */
static void IMPL_AX12_POS_add_torque_order(Actuator_servo_t *actuator, Uint8 cmd_index) {
	Uint8 threshold = actuator->servo.commands[cmd_index].param.torque.threshold;

	if(threshold <= 100) {
		QUEUE_add(actuator->queue_id, &IMPL_AX12_POS_run_command_torque, (QUEUE_arg_t){ACT_SERVO_ASSER_TORQUE, cmd_index, &ACTQ_finish_SendNothing, TRUE});
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
static void IMPL_AX12_POS_run_command_torque(queue_id_t queueId, void* actuator_gen_ptr, bool_e init) {
	Actuator_servo_t* actuator = (Actuator_servo_t*) actuator_gen_ptr;

	if(QUEUE_has_error(queueId)) {
		QUEUE_behead(queueId);
		return;
	}

	// Gestion des mouvements de l'actionneur
	if(queueId == actuator->queue_id) {
		if(init) {
			actuator->servo.torque_asser.activated = TRUE;
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
void IMPL_AX12_POS_set_warner(void* actuator_gen_ptr, CAN_msg_t *msg) {
	Actuator_servo_t* actuator = (Actuator_servo_t*) actuator_gen_ptr;
	Sint16 warner_pos = 0;
	bool_e warner_is_valid = ACT_get_command_value(&warner_pos, msg->data.act_warner.warner_param, actuator->servo.commands, actuator->servo.nb_commands);

	if(warner_is_valid){
		actuator->warner.warner_value = warner_pos;
		actuator->warner.activated = TRUE;
		actuator->warner.last_value = AX12_get_position(actuator->servo.id);
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
void IMPL_AX12_POS_check_warner(Actuator_servo_t* actuator, Uint16 pos) {
	CAN_msg_t msg;
	Sint16 warner_pos = actuator->warner.warner_value;
	Sint16 last_pos = actuator->warner.last_value;

	if( (last_pos < pos && last_pos < warner_pos && pos > warner_pos)
     || (last_pos > pos && last_pos > warner_pos && pos < warner_pos)
	 || (last_pos > AX12_MAX_WARNER && pos < AX12_MIN_WARNER && (last_pos < warner_pos || pos > warner_pos))
	 || (last_pos < AX12_MIN_WARNER && pos > AX12_MAX_WARNER && (last_pos > warner_pos || pos < warner_pos))
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
void IMPL_AX12_POS_process_it(void* actuator_gen_ptr) {
	Actuator_servo_t* actuator = (Actuator_servo_t*) actuator_gen_ptr;

	// Asservissement en couple
	if(actuator->servo.torque_asser.activated) {
		if(actuator->servo.torque_asser.ask_to_finish) {
			// On stoppe l'asservissement en couple suite à l'arrivée d'un nouvel ordre
			actuator->servo.torque_asser.activated = FALSE;
			actuator->servo.torque_asser.ask_to_finish = FALSE;
			QUEUE_next(actuator->queue_id, ACT_RESULT_DONE, ACT_RESULT_ERROR_OK, __LINE__);
		} else if(!global.flags.virtual_mode) {
			// On exécute l'asservissement en couple
			Uint8 cmd_index = QUEUE_get_arg(actuator->queue_id)->param;
			Way_asser_torque_e way = actuator->servo.commands[cmd_index].param.torque.way;
			Sint8 wanted_load = 0;
			Sint8 current_load = - AX12_get_load_percentage(actuator->servo.id);
			Uint16 current_pos = AX12_get_position(actuator->servo.id);
			Uint16 limit_pos = actuator->servo.commands[cmd_index].value;

			if(way == WAY_ASSER_TORQUE_INC) {
				wanted_load = actuator->servo.commands[cmd_index].param.torque.threshold;
			} else {
				wanted_load = - actuator->servo.commands[cmd_index].param.torque.threshold;
			}

			// Calcul de la nouvelle position.
			Sint32 torque_error = wanted_load - current_load;
			Sint32 new_pos = current_pos + ( (actuator->servo.torque_asser.Kp * torque_error) >> 6 );
			new_pos = ACT_compute_modulo(new_pos, DEFAULT_SERVO_MAX_VALUE);

			// Désactivation de l'asservissement si on dépasse la position limite
			if((way == WAY_ASSER_TORQUE_INC && current_pos >= limit_pos)
			|| (way == WAY_ASSER_TORQUE_DEC && current_pos <= limit_pos)){
				new_pos = limit_pos;
				actuator->servo.torque_asser.activated = FALSE;
			}

			if(!AX12_set_position(actuator->servo.id, new_pos)) {
				//Si la commande n'a pas été envoyée correctement et/ou que le AX12 ne répond pas a cet envoi, on stoppe l'asservissement
				actuator->servo.torque_asser.activated = FALSE;
			}

			if(!actuator->servo.torque_asser.activated) {
				// Si l'asservissement en couple doit être arrêté suite à une erreur, on passe à l'action suivante
				QUEUE_next(actuator->queue_id, ACT_RESULT_FAILED, ACT_RESULT_ERROR_NOT_HERE, __LINE__);
			}
		}
	}
}

