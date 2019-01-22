/**
 *	Club Robot ESEO 2017 - 2018
 *
 * @file impl_rx24_pos.c
 * @brief Implémentation des fonctions pour un actionneur de type KIND_RX24_POS.
 * @author Valentin
 */

// Les différents includes necessaires
#include "impl_rx24_pos.h"

#ifdef USE_RX24_SERVO

#include "../QS/QS_CANmsgList.h"
#include "../QS/QS_actuator/QS_rx24.h"
#include "../QS/QS_lowLayer/QS_can.h"
#include "../actuators/act_utils.h"

// Les différents define pour le verbose sur uart
#define LOG_PREFIX "[RX24_POS_S]  "
#define LOG_COMPONENT OUTPUT_LOG_COMPONENT_IMPL_RX24_POS
#include "../QS/QS_outputlog.h"

// Les fonctions internes au fonctionnement de l'actionneur
static void IMPL_RX24_POS_init_servo(Actuator_servo_t* actuator);
static void IMPL_RX24_POS_get_position_config(Actuator_servo_t* actuator, ACT_order_e *order_ptr, Uint16 *pos_ptr);


/**************************************************************************/
/**	Fonctions usuelles de gestion de l'actionneur                        **/
/**************************************************************************/

/**
 * @brief Fonction permettant d'initialiser le servo.
 * Elle initialise le bus de communication ainsi que les différents registres du servo.
 * Cette fonction est appelée au lancement de la carte via ActManager.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
 */
void IMPL_RX24_POS_init(void* actuator_gen_ptr) {
	Actuator_servo_t* actuator = (Actuator_servo_t*) actuator_gen_ptr;

	if(actuator->is_initialized)
		return;

	RX24_init();
	//IMPL_RX24_POS_init_servo(actuator); // Inutile
}

/**
 * @brief Fonction qui réinitialise la config du servo.
 * Fonction appellée si la carte IHM a détecté une grosse chute de la tension d'alimentation des actionneurs.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
 */
void IMPL_RX24_POS_reset_config(void* actuator_gen_ptr) {
	Actuator_servo_t* actuator = (Actuator_servo_t*) actuator_gen_ptr;
	actuator->is_initialized = FALSE;
	IMPL_RX24_POS_init_servo(actuator);
}

/**
 * @brief Fonction qui initialise les registres du servo avec la config souhaitée.
 * Initialise le servo s'il n'était pas alimenté lors d'initialisations précédentes, si déjà initialisé, ne fait rien.
 * @param actuator l'instance de l'actionneur.
 */
static void IMPL_RX24_POS_init_servo(Actuator_servo_t* actuator) {
	if(actuator->is_initialized == FALSE && RX24_is_ready(actuator->servo.id) == TRUE) {
		actuator->is_initialized = TRUE;
		RX24_config_set_lowest_voltage(actuator->servo.id, RX24_MIN_VOLTAGE);
		RX24_config_set_highest_voltage(actuator->servo.id, RX24_MAX_VOLTAGE);
		RX24_set_torque_limit(actuator->servo.id, actuator->servo.max_torque);
		RX24_config_set_temperature_limit(actuator->servo.id, actuator->servo.max_temperature);

		RX24_config_set_maximal_angle(actuator->servo.id, actuator->servo.max_angle);
		RX24_config_set_minimal_angle(actuator->servo.id, actuator->servo.min_angle);
		RX24_set_move_to_position_speed(actuator->servo.id, actuator->servo.speed);

		RX24_config_set_error_before_led(actuator->servo.id, RX24_BEFORE_LED);
		RX24_config_set_error_before_shutdown(actuator->servo.id, RX24_BEFORE_SHUTDOWN);

		debug_printf("RX24 n°%u init config DONE\n", actuator->servo.id);
	}else if(actuator->is_initialized == FALSE) {
		debug_printf("RX24 n°%u init config FAIL\n", actuator->servo.id);
	}

	actuator->cmd_launched = FALSE;
}

/**
 * Fonction appellée pour initialiser en position le servo dès l'arrivée de l'alimentation via ActManager.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
 * @return le status d'initialisation.
 */
error_e IMPL_RX24_POS_init_pos(void *actuator_gen_ptr) {
	Actuator_servo_t* actuator = (Actuator_servo_t*) actuator_gen_ptr;
	IMPL_RX24_POS_init_servo(actuator);

	// Pour la simulation
	actuator->servo.simu.current_pos = actuator->servo.init_pos;
	actuator->servo.simu.current_speed = actuator->servo.speed;

	if(actuator->is_initialized == FALSE)
		return END_OK;

	if(!RX24_set_position(actuator->servo.id, actuator->servo.init_pos))
		debug_printf("RX24 n°%u is not here\n", actuator->servo.id);
	else
		debug_printf("RX24 n°%u has been initialized in position\n", actuator->servo.id);

	return END_OK;
}

/**
 * @brief Fonction pour stopper l'asservissement du servo.
 * Fonction appellée à la fin du match via ActManager.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
 */
void IMPL_RX24_POS_stop(void *actuator_gen_ptr) {
	Actuator_servo_t* actuator = (Actuator_servo_t*) actuator_gen_ptr;
	RX24_set_torque_enabled(actuator->servo.id, FALSE);
}



/**************************************************************************/
/**	Fonctions pour la gestion des configs                                **/
/**************************************************************************/

/**
 * @brief Fonction permettant de modifier la config du servo comme la vitesse ou le couple.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
 * @param config la configuration à changer.
 * @param value la nouvelle valeur de la configuration
 */
void IMPL_RX24_POS_set_config(void *actuator_gen_ptr, act_config_e config, Uint16 value) {
	Actuator_servo_t* actuator = (Actuator_servo_t*) actuator_gen_ptr;
	ACT_SERVO_config_RX24(&(actuator->servo), config, value);
}

/**
 * @brief Fonction permettant d'obtenir des infos sur la config du servo comme la vitesse ou le couple.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
 * @param id l'id de l'actionneur concerné.
 * @return la valeur de la config demandée.
 */
Uint16 IMPL_RX24_POS_get_config(void *actuator_gen_ptr, act_config_e config) {
	Actuator_servo_t* actuator = (Actuator_servo_t*) actuator_gen_ptr;
	Uint16 value = 0;

	switch(config){
		case POSITION_CONFIG:
			IMPL_RX24_POS_get_position_config(actuator, NULL, &value);
			break;
		case TORQUE_CONFIG:
			if(global.flags.virtual_mode) {
				value = 0;
			} else {
				value = RX24_get_speed_percentage(actuator->servo.id);
			}
			break;
		case TEMPERATURE_CONFIG:
			if(global.flags.virtual_mode) {
				value = 0;
			} else {
				value = RX24_get_temperature(actuator->servo.id);
			}
			break;
		case LOAD_CONFIG:
			if(global.flags.virtual_mode) {
				value = 0;
			} else {
				value = RX24_get_load_percentage(actuator->servo.id);
			}
			break;
		default:
			error_printf("The config %u is not available\n", config);
	}

	return value;
}

/**
 * @brief Fonction permettant d'obtenir la position de l'actionneur en tant que "ordre actionneur".
 * Cette fonction convertit la position du rx24 entre 0 et 1023 en une position en tant que "ordre actionneur".
 * @param actuator l'instance de l'actionneur
 * @param order_ptr un pointeur dans lequel on doit stocker l'ordre de retour
 * @param pos_ptr un pointeur dans lequel on doit stocker la position courante de l'actionneur.
 */
static void IMPL_RX24_POS_get_position_config(Actuator_servo_t* actuator, ACT_order_e *order_ptr, Uint16 *pos_ptr) {
	ACT_order_e order = ACT_DEFAULT_STOP;
	Sint16 pos = 0;
	Sint16 epsilon = (Sint16) actuator->servo.large_epsilon;

	if(global.flags.virtual_mode) {
		pos = actuator->servo.simu.current_pos;
	} else {
		pos = (Sint16) RX24_get_position(actuator->servo.id);
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
 * @brief Fonction permettant d'exécuter une commande.
 * Fonction utilisée pour les commandes de position.
 * @param actuator_gen_ptr l'instance de l'actionneur.
 * @param order l'ordre à exécuter.
 * @param param un paramètre (inutilisé ici)
 * @return l'état de l'initialisation de la commande.
 */
act_result_state_e IMPL_RX24_POS_run_order(void* actuator_gen_ptr, ACT_order_e order, Sint32 param) {
	Actuator_servo_t* actuator = (Actuator_servo_t*) actuator_gen_ptr;
	Sint16 goal_position = 0;
	Sint16 cmd_index = 0;
	bool_e order_is_valid = ACT_get_command_index(&cmd_index, order, actuator->servo.commands, actuator->servo.nb_commands);

	if(order_is_valid == FALSE && order != ACT_STRATEASY_CUSTOM_POS_IN_PARAM) {
		ACT_printResult((void*) actuator, order, ACT_RESULT_FAILED, ACT_RESULT_ERROR_INVALID_ARG, __LINE__);
		return ACT_RESULT_NOT_HANDLED;
	}

	// On enregistre l'ordre courante et l'index de la commande correspondant
	actuator->current_order = order;
	actuator->servo.current_cmd_index = cmd_index;

	if(!global.flags.virtual_mode) {
		IMPL_RX24_POS_init_servo(actuator);
	}

	if(order == ACT_DEFAULT_STOP) {
		if(!global.flags.virtual_mode) {
			RX24_set_torque_enabled(actuator->servo.id, FALSE); //Stopper l'asservissement de l'RX24
		}
		ACT_printResult((void*) actuator, order, ACT_RESULT_DONE, ACT_RESULT_ERROR_OK, __LINE__);
		return ACT_RESULT_DONE;
	} else {
		if(order == ACT_STRATEASY_CUSTOM_POS_IN_PARAM)
		{
			goal_position = param;
		}
		else
		{
			// On obtient une valeur correcte car on a vérifié précédemment que l'ordre existait.
			if(cmd_index >= 0 && cmd_index < actuator->servo.nb_commands) {
				goal_position = actuator->servo.commands[cmd_index].value;
			} else {
				goal_position = ERROR_ACT_VALUE;
			}
		}

	}

	if(!global.flags.virtual_mode && actuator->is_initialized == FALSE){
		error_printf("Impossible to set the actuator n°0x%u in position, it is not initialized\n", actuator->servo.id);
		ACT_printResult((void*) actuator, order, ACT_RESULT_FAILED, ACT_RESULT_ERROR_NOT_HERE, __LINE__);
		return ACT_RESULT_NOT_HANDLED;
	}

	if(goal_position == ERROR_ACT_VALUE) {
		error_printf("Invalid RX24 position for order: %u, code is broken !\n", order);
		ACT_printResult((void*) actuator, order, ACT_RESULT_NOT_HANDLED, ACT_RESULT_ERROR_LOGIC, __LINE__);
		return ACT_RESULT_NOT_HANDLED;
	}

	if(global.flags.virtual_mode) {
		// Calcul du temps d'execution de la commande (en ms)
		time32_t exec_time = ACT_SERVO_compute_cmd_exec_time(actuator->kind, actuator->servo.simu.current_speed, goal_position, actuator->servo.simu.current_pos);
		actuator->servo.simu.exec_finished_time = global.absolute_time + exec_time;
		actuator->cmd_launched = TRUE;
		actuator->servo.exec_start_time = global.absolute_time;
		debug_printf("Simulation: RX24 n°%u take %ld ms to move.\n", actuator->servo.id, exec_time);
	} else {
		RX24_reset_last_error(actuator->servo.id); //Sécurité anti terroriste. Nous les parano on aime pas voir des erreurs là ou il n'y en a pas.
		RX24_set_torque_enabled(actuator->servo.id, TRUE); // Activation de l'asservissement de l'RX24
		if(!RX24_set_position(actuator->servo.id, goal_position)) {	//Si la commande n'a pas été envoyée correctement et/ou que l'RX24 ne répond pas a cet envoi, on l'indique à la carte stratégie
			error_printf("RX24 n°%u don't answered when setting position: error=0x%x\n",  actuator->servo.id, RX24_get_last_error(actuator->servo.id).error);
			return ACT_RESULT_FAILED;
		} else {
			actuator->cmd_launched = TRUE;
			actuator->servo.exec_start_time = global.absolute_time; // On enregistre le temps auquel la commande est envoyée.
		}
	}
	//La commande a été envoyée et le RX24 l'a bien reçu
	debug_printf("Move RX24 n°%u to position %d\n", actuator->servo.id, goal_position);
	return ACT_RESULT_DONE;
}

/**
 * @brief Fonction qui vérifie l'exécution de la commande en cours.
 * Fonction utilisée pour les commandes de position.
 * @param actuator_gen_ptr l'instance de l'actionneur.
 * @return l'état d'exécution de la commande.
 */
act_result_state_e IMPL_RX24_POS_check_order(void* actuator_gen_ptr) {
	Actuator_servo_t* actuator = (Actuator_servo_t*) actuator_gen_ptr;
	Uint8 result = 0, error_code = 0;
	Uint16 line = 0;
	Sint16 cmd_index = actuator->servo.current_cmd_index;

	if(actuator->cmd_launched == FALSE) {
		//ACT_printResult((void*) actuator, actuator->current_order, ACT_RESULT_FAILED, ACT_RESULT_ERROR_INVALID_ARG, __LINE__);
		return ACT_RESULT_NOT_HANDLED;
	}

	Sint16 wanted_goal = actuator->servo.commands[cmd_index].value;
	Sint16 current_goal = 0;

	if(global.flags.virtual_mode) {
		if(global.absolute_time > actuator->servo.simu.exec_finished_time) {
			actuator->servo.simu.current_pos = wanted_goal;
			ACT_printResult((void*) actuator, actuator->current_order, ACT_RESULT_DONE, ACT_RESULT_ERROR_OK, __LINE__);
			actuator->cmd_launched = FALSE;
			return ACT_RESULT_DONE;
		}
	} else {
		current_goal = RX24_get_position(actuator->servo.id);
		if(ACTQ_check_status_rx24(actuator->servo.id, wanted_goal, current_goal, actuator->servo.epsilon, actuator->servo.exec_start_time, actuator->servo.timeout, actuator->servo.large_epsilon, &result, &error_code, &line)) {
			ACT_printResult((void*) actuator, actuator->current_order, result, error_code, line);
			actuator->cmd_launched = FALSE;
			return result;
		}
	}

	return ACT_RESULT_IN_PROGRESS;
}

#endif /* USE_RX24_SERVO */
