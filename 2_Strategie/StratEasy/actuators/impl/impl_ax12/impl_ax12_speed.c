/**
 *	Club Robot ESEO 2017 - 2018
 *
 * @file impl_ax12_speed.c
 * @brief Implémentation des fonctions pour un actionneur de type KIND_AX12_SPEED.
 * @author Valentin
 */

// Les différents includes nécessaires...
#include "impl_ax12_speed.h"

#ifdef USE_AX12_SERVO

#include "../QS/QS_CANmsgList.h"
#include "../QS/QS_actuator/QS_ax12.h"
#include "../QS/QS_lowLayer/QS_can.h"
#include "../actuators/act_utils.h"

// Les différents define pour le verbose sur uart
#define LOG_PREFIX "[AX12_SPEED]  "
#define LOG_COMPONENT OUTPUT_LOG_COMPONENT_IMPL_AX12_SPEED
#include "../QS/QS_outputlog.h"

// Les fonctions internes au fonctionnement de l'actionneur
static void IMPL_AX12_SPEED_init_servo(Actuator_servo_t* actuator);

/**
 * @brief Fonction permettant d'initialiser le servo.
 * Elle initialise le bus de communication ainsi que les différents registres du servo.
 * Cette fonction est appelée au lancement de la carte via ActManager.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
 */
void IMPL_AX12_SPEED_init(void* actuator_gen_ptr) {
	Actuator_servo_t* actuator = (Actuator_servo_t*) actuator_gen_ptr;

	if(actuator->is_initialized)
		return;

	AX12_init();
	//IMPL_AX12_SPEED_init_servo(actuator); // Inutile
}

/**
 * @brief Fonction qui réinitialise la config du servo.
 * Fonction appellée si la carte IHM a détecté une grosse chutte de la tension d'alimentation des actionneurs.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
 */
void IMPL_AX12_SPEED_reset_config(void* actuator_gen_ptr) {
	Actuator_servo_t* actuator = (Actuator_servo_t*) actuator_gen_ptr;
	actuator->is_initialized = FALSE;
	IMPL_AX12_SPEED_init_servo(actuator);
}

/**
 * @brief Fonction qui initialise les registres du servo avec la config souhaitee.
 * Initialise le servo s'il n'était pas alimente lors d'initialisations precedentes, si deja initialise, ne fait rien.
 * @param actuator_ l'instance de l'actionneur.
 */
static void IMPL_AX12_SPEED_init_servo(Actuator_servo_t* actuator) {
	if(actuator->is_initialized == FALSE && AX12_is_ready(actuator->servo.id) == TRUE) {
		actuator->is_initialized = TRUE;

		AX12_set_wheel_mode_enabled(actuator->servo.id, TRUE); // Set the servo in wheel mode

		AX12_config_set_lowest_voltage(actuator->servo.id, AX12_MIN_VOLTAGE);
		AX12_config_set_highest_voltage(actuator->servo.id, AX12_MAX_VOLTAGE);
		AX12_set_torque_limit(actuator->servo.id, actuator->servo.max_torque);
		AX12_config_set_temperature_limit(actuator->servo.id, actuator->servo.max_temperature);

		AX12_config_set_error_before_led(actuator->servo.id, AX12_BEFORE_LED);
		AX12_config_set_error_before_shutdown(actuator->servo.id, AX12_BEFORE_SHUTDOWN);
		debug_printf("AX12 n°%u init config DONE\n", actuator->servo.id);
	}else if(actuator->is_initialized == FALSE) {
		debug_printf("AX12 n°%u init config FAIL\n", actuator->servo.id);
	}

	actuator->cmd_launched = FALSE;
}

/**
 * Fonction appellée pour initialiser en position le servo dès l'arrivée de l'alimentation via ActManager.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
 * @return le status d'initialisation.
 */
error_e IMPL_AX12_SPEED_init_pos(void *actuator_gen_ptr) {
	Actuator_servo_t* actuator = (Actuator_servo_t*) actuator_gen_ptr;
	IMPL_AX12_SPEED_init_servo(actuator);

	// Pour la simulation
	actuator->servo.simu.current_pos = actuator->servo.init_pos;
	actuator->servo.simu.current_speed = actuator->servo.speed;

	if(actuator->is_initialized == FALSE)
		return END_OK;

	if(!AX12_set_speed_percentage(actuator->servo.id, actuator->servo.init_pos))
		debug_printf("AX12 n°%u is not here\n", actuator->servo.id);
	else
		debug_printf("AX12 n°%u has been initialized in position\n", actuator->servo.id);

	return END_OK;
}

/**
 * @brief Fonction pour stopper l'asservissement du servo.
 * Fonction appellée à la fin du match via ActManager.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
 */
void IMPL_AX12_SPEED_stop(void *actuator_gen_ptr) {
	Actuator_servo_t* actuator = (Actuator_servo_t*) actuator_gen_ptr;
	AX12_set_torque_enabled(actuator->servo.id, FALSE);
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
void IMPL_AX12_SPEED_set_config(void *actuator_gen_ptr, act_config_e config, Uint16 value) {
	Actuator_servo_t* actuator = (Actuator_servo_t*) actuator_gen_ptr;
	ACT_SERVO_config_AX12(&(actuator->servo), config, value);
}

/**
 * @brief Fonction permettant d'obtenir des infos sur la config du servo comme la vitesse ou le couple.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
 * @param id l'id de l'actionneur concerné.
 * @return la valeur de la config demandée.
 */
Uint16 IMPL_AX12_SPEED_get_config(void *actuator_gen_ptr, act_config_e config) {
	Actuator_servo_t* actuator = (Actuator_servo_t*) actuator_gen_ptr;
	Uint16 value = 0;

	switch(config){
		case POSITION_CONFIG:
			if(global.flags.virtual_mode) {
				value = 0;
			} else {
				value = AX12_get_position(actuator->servo.id);
			}
			break;
		case TORQUE_CONFIG:
			if(global.flags.virtual_mode) {
				value = 0;
			} else {
				value = AX12_get_speed_percentage(actuator->servo.id);
			}
			break;
		case TEMPERATURE_CONFIG:
			if(global.flags.virtual_mode) {
				value = 0;
			} else {
				value = AX12_get_temperature(actuator->servo.id);
			}
			break;
		case LOAD_CONFIG:
			if(global.flags.virtual_mode) {
				value = 0;
			} else {
				value = AX12_get_load_percentage(actuator->servo.id);
			}
			break;
		default:
			error_printf("The config %u is not available\n", config);
	}

	return value;
}

/**************************************************************************/
/**	Fonctions pour la gestion de l'asservissement en vitesse            **/
/**************************************************************************/

/**
 * @brief Fonction permettant d'exécuter une commande.
 * Fonction utilisée pour les commandes de vitesse.
 * @param actuator_gen_ptr l'instance de l'actionneur.
 * @param order l'ordre à exécuter.
 * @param param un paramètre (inutilisé ici)
 * @return l'état de l'initialisation de la commande.
 */
act_result_state_e IMPL_AX12_SPEED_run_order(void* actuator_gen_ptr, ACT_order_e order, Sint32 param) {
	Actuator_servo_t* actuator = (Actuator_servo_t*) actuator_gen_ptr;
	UNUSED_VAR(param);
	Sint16 cmd_index = 0;
	Sint16 goal_speed = 0;

	bool_e order_is_valid = ACT_get_command_index(&cmd_index, order, actuator->servo.commands, actuator->servo.nb_commands);

	if(order_is_valid == FALSE) {
		//ACT_printResult((void*) actuator, order, ACT_RESULT_FAILED, ACT_RESULT_ERROR_INVALID_ARG, __LINE__);
		return ACT_RESULT_NOT_HANDLED;
	}

	// On enregistre l'ordre courante et l'index de la commande correspondant
	actuator->current_order = order;
	actuator->servo.current_cmd_index = cmd_index;

	if(order == ACT_DEFAULT_STOP) {
		if(!global.flags.virtual_mode) {
			AX12_set_torque_enabled(actuator->servo.id, FALSE); //Stopper l'asservissement de l'AX12
		}
		ACT_printResult((void*) actuator, order, ACT_RESULT_DONE, ACT_RESULT_ERROR_OK, __LINE__);
		return ACT_RESULT_DONE;
	} else {
		if(cmd_index >= 0 && cmd_index < actuator->servo.nb_commands) {
			goal_speed = actuator->servo.commands[cmd_index].value;
		} else {
			goal_speed = ERROR_ACT_VALUE;
		}
	}

	if(!global.flags.virtual_mode && actuator->is_initialized == FALSE){
		error_printf("Impossible to set the actuator id=0x%x at desired speed, it is not initialized\n", actuator->id);
		ACT_printResult((void*) actuator, order,  ACT_RESULT_NOT_HANDLED, ACT_RESULT_ERROR_NOT_HERE, __LINE__);
		return ACT_RESULT_NOT_HANDLED;
	}

	if(goal_speed == ERROR_ACT_VALUE) {
		error_printf("Invalid AX12 speed for order: %u, code is broken !\n", order);
		ACT_printResult((void*) actuator, order,ACT_RESULT_NOT_HANDLED, ACT_RESULT_ERROR_LOGIC, __LINE__);
		return ACT_RESULT_NOT_HANDLED;
	}

	if(global.flags.virtual_mode) {
		// Calcul du temps d'execution de la commande (en ms)
		actuator->servo.simu.exec_finished_time = global.absolute_time; // Temps théorique de réponse quasiment nul
		actuator->cmd_launched = TRUE;
		debug_printf("Simulation: AX12 n°%u take %d ms to move.\n", actuator->servo.id, 0);
	} else {
		AX12_reset_last_error(actuator->servo.id); //Sécurité anti terroriste. Nous les parano on aime pas voir des erreurs là ou il n'y en a pas.
		AX12_set_torque_enabled(actuator->servo.id, TRUE); // Activation de l'asservissement de l'AX12
		if(!AX12_set_speed_percentage(actuator->servo.id, goal_speed)) {	//Si la commande n'a pas été envoyée correctement et/ou que l'AX12 ne répond pas a cet envoi, on l'indique à la carte stratégie
			error_printf("AX12 n°%u don't answered when setting speed: error=0x%x\n",  actuator->servo.id, AX12_get_last_error(actuator->servo.id).error);
			ACT_printResult((void*) actuator, order, ACT_RESULT_FAILED, ACT_RESULT_ERROR_NOT_HERE, __LINE__);
			return ACT_RESULT_FAILED;
		} else {
			actuator->cmd_launched = TRUE;
			actuator->servo.exec_start_time = global.absolute_time; // On enregistre le temps auquel la commande est envoyée.
		}
	}
	//La commande a été envoyée et l'AX12 l'a bien reçu
	debug_printf("Move AX12 n°%u at speed %d\n", actuator->servo.id, goal_speed);
	return ACT_RESULT_DONE;
}

/**
 * @brief Fonction qui vérifie l'exécution de la commande en cours.
 * Fonction utilisée pour les commandes de vitesse.
 * @param actuator_gen_ptr l'instance de l'actionneur.
 * @return l'état d'exécution de la commande.
 */
act_result_state_e IMPL_AX12_SPEED_check_order(void* actuator_gen_ptr) {
	Actuator_servo_t* actuator = (Actuator_servo_t*) actuator_gen_ptr;
	Uint8 result = 0, error_code = 0;
	Uint16 line = 0;
	Sint16 cmd_index = actuator->servo.current_cmd_index;

	if(actuator->cmd_launched == FALSE) {
		ACT_printResult((void*) actuator, actuator->current_order, ACT_RESULT_FAILED, ACT_RESULT_ERROR_INVALID_ARG, __LINE__);
		return ACT_RESULT_NOT_HANDLED;
	}

	Uint16 wanted_speed = actuator->servo.commands[cmd_index].value;

	if(global.flags.virtual_mode) {
		if(global.absolute_time > actuator->servo.simu.exec_finished_time) {
			actuator->servo.simu.current_speed = wanted_speed;
			ACT_printResult((void*) actuator, actuator->current_order, ACT_RESULT_DONE, ACT_RESULT_ERROR_OK, __LINE__);
			actuator->cmd_launched = FALSE;
			return ACT_RESULT_DONE;
		}
	} else {
		// L'asservissement en vitesse n'est pas top, du coup dès que le servo se met à bouger on considère que c'est bon.
		Uint16 speed = AX12_get_speed_percentage(actuator->servo.id);
		if(wanted_speed != 0 && speed != 0)
			speed = wanted_speed;
		else
			speed = 0;

		if(ACTQ_check_status_ax12(actuator->servo.id, wanted_speed, speed, actuator->servo.epsilon, actuator->servo.exec_start_time, actuator->servo.timeout, actuator->servo.large_epsilon, &result, &error_code, &line)) {
			ACT_printResult((void*) actuator, actuator->current_order, result, error_code, line);
			actuator->cmd_launched = FALSE;
			return result;
		}
	}

	return ACT_RESULT_IN_PROGRESS;
}

#endif /* #ifdef USE_AX12_SERVO */

