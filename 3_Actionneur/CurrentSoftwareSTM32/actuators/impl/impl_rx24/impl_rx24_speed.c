/**
 *	Club Robot ESEO 2017 - 2018
 *
 * @file impl_rx24_speed.c
 * @brief Implémentation des fonctions pour un actionneur de type KIND_RX24_SPEED.
 * @author Valentin
 */

// Les différents includes nécessaires...
#include "impl_rx24_speed.h"

#include "../QS/QS_CANmsgList.h"
#include "../QS/QS_actuator/QS_rx24.h"
#include "../QS/QS_lowLayer/QS_can.h"
#include "../act_queue_utils.h"
#include "../ActManager.h"
#include "../queue.h"
#include "../../actuator_servo.h"
#include "impl_rx24_pos.h"  // utile pour les warners en position

// Les différents define pour le verbose sur uart
#define LOG_PREFIX "[RX24_SPEED]  "
#define LOG_COMPONENT OUTPUT_LOG_COMPONENT_IMPL_RX24_SPEED
#include "../QS/QS_outputlog.h"

// Les fonctions internes au fonctionnement de l'actionneur
static void IMPL_RX24_SPEED_init_servo(Actuator_servo_t* actuator);
static void IMPL_RX24_SPEED_command_init(Actuator_servo_t* actuator);
static void IMPL_RX24_SPEED_command_run(Actuator_servo_t* actuator);
//static void IMPL_RX24_SPEED_check_warner(Actuator_servo_t* actuator, Uint16 pos);


/**************************************************************************/
/**	Fonctions usuelles de gestion de l'actionneur                        **/
/**************************************************************************/

/**
 * @brief Fonction permettant d'initialiser le servo.
 * Elle initialise le bus de communication ainsi que les différents registres du servo.
 * Cette fonction est appelée au lancement de la carte via ActManager.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
 */
void IMPL_RX24_SPEED_init(void* actuator_gen_ptr) {
	Actuator_servo_t* actuator = (Actuator_servo_t*) actuator_gen_ptr;

	if(actuator->is_initialized)
		return;

	RX24_init();
	//IMPL_RX24_SPEED_init_servo(actuator); // Inutile
}

/**
 * @brief Fonction qui réinitialise la config du servo.
 * Fonction appellée si la carte IHM a détecté une grosse chutte de la tension d'alimentation des actionneurs.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
 */
void IMPL_RX24_SPEED_reset_config(void* actuator_gen_ptr) {
	Actuator_servo_t* actuator = (Actuator_servo_t*) actuator_gen_ptr;
	actuator->is_initialized = FALSE;
	IMPL_RX24_SPEED_init_servo(actuator);
}

/**
 * @brief Fonction qui initialise les registres du servo avec la config souhaitee.
 * Initialise le servo s'il n'était pas alimente lors d'initialisations precedentes, si deja initialise, ne fait rien.
 * @param actuator_ l'instance de l'actionneur.
 */
static void IMPL_RX24_SPEED_init_servo(Actuator_servo_t* actuator) {
	if(actuator->is_initialized == FALSE && RX24_is_ready(actuator->servo.id) == TRUE) {
		actuator->is_initialized = TRUE;

		RX24_set_wheel_mode_enabled(actuator->servo.id, TRUE); // Set the servo in wheel mode

		RX24_config_set_lowest_voltage(actuator->servo.id, RX24_MIN_VOLTAGE);
		RX24_config_set_highest_voltage(actuator->servo.id, RX24_MAX_VOLTAGE);
		RX24_set_torque_limit(actuator->servo.id, actuator->servo.max_torque);
		RX24_config_set_temperature_limit(actuator->servo.id, actuator->servo.max_temperature);

		RX24_config_set_error_before_led(actuator->servo.id, RX24_BEFORE_LED);
		RX24_config_set_error_before_shutdown(actuator->servo.id, RX24_BEFORE_SHUTDOWN);
		debug_printf("RX24 n°%u init config DONE\n", actuator->servo.id);
	}else if(actuator->is_initialized == FALSE)
		debug_printf("RX24 n°%u init config FAIL\n", actuator->servo.id);
}

/**
 * Fonction appellée pour initialiser en position le servo dès l'arrivée de l'alimentation via ActManager.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
 * @return le status d'initialisation.
 */
error_e IMPL_RX24_SPEED_init_pos(void *actuator_gen_ptr) {
	Actuator_servo_t* actuator = (Actuator_servo_t*) actuator_gen_ptr;
	IMPL_RX24_SPEED_init_servo(actuator);

	// Pour la simulation
	actuator->servo.simu.current_pos = actuator->servo.init_pos;
	actuator->servo.simu.current_speed = actuator->servo.speed;

	if(actuator->is_initialized == FALSE)
		return END_OK;

	if(!RX24_set_speed_percentage(actuator->servo.id, actuator->servo.init_pos))
		debug_printf("RX24 n°%u is not here\n", actuator->servo.id);
	else
		debug_printf("RX24 n°%u has been initialized in position\n", actuator->servo.id);

	return END_OK;
}

/**
 * @brief Fonction pour stopper l'asservissement du servo.
 * Fonction appellée à la fin du match via ActManager.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
 */
void IMPL_RX24_SPEED_stop(void *actuator_gen_ptr) {
	Actuator_servo_t* actuator = (Actuator_servo_t*) actuator_gen_ptr;
	RX24_set_torque_enabled(actuator->servo.id, FALSE);
}

/**
 * @brief Fonction de dispatch des messages CAN pour les ordres actionneurs.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
 * @param msg le message CAN reçu.
 * @return TRUE si le message CAN a ete traite et FALSE sinon.
 */
bool_e IMPL_RX24_SPEED_CAN_process_msg(void *actuator_gen_ptr, CAN_msg_t* msg) {
	Actuator_servo_t* actuator = (Actuator_servo_t*) actuator_gen_ptr;
	if(msg->sid == actuator->sid){
		//IMPL_RX24_SPEED_init_servo(actuator); //Inutile

		Sint16 param = 0;
		bool_e order_is_valid = ACT_get_command_value(&param, msg->data.act_order.order, actuator->servo.commands, actuator->servo.nb_commands);

		if(order_is_valid) {
			ACTQ_push_operation_from_msg(msg, actuator->queue_id, &IMPL_RX24_SPEED_run_command, param, TRUE);
		} else {
			component_printf(LOG_LEVEL_Warning, "invalid CAN msg order=%u for sid=0x%x!\n", msg->data.act_order.order, actuator->sid);
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
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
 * @param msg le message CAN reçu.
 */
void IMPL_RX24_SPEED_set_config(void *actuator_gen_ptr, CAN_msg_t* msg) {
	Actuator_servo_t* actuator = (Actuator_servo_t*) actuator_gen_ptr;
	ACTMGR_config_RX24(&(actuator->servo), msg);
}

/**
 * @brief Fonction permettant d'obtenir des infos sur la config du servo comme la vitesse ou le couple.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
 * @param incoming_msg le message CAN reçu.
 */
void IMPL_RX24_SPEED_get_config(void *actuator_gen_ptr, CAN_msg_t *incoming_msg) {
	Actuator_servo_t* actuator = (Actuator_servo_t*) actuator_gen_ptr;
	bool_e error = FALSE;
	CAN_msg_t msg;
	msg.sid = ACT_GET_CONFIG_ANSWER;
	msg.size = SIZE_ACT_GET_CONFIG_ANSWER;
	msg.data.act_get_config_answer.act_sid = 0xFF & (actuator->sid);
	msg.data.act_get_config_answer.config = incoming_msg->data.act_set_config.config;

	switch(incoming_msg->data.act_set_config.config){
		case POSITION_CONFIG:
			// Pas d'ordre lié à des positions, on renvoie ACT_DEFAULT_STOP comme valeur par défaut
			msg.data.act_get_config_answer.act_get_config_data.act_get_config_pos_answer.order = ACT_DEFAULT_STOP;
			if(global.flags.virtual_mode) {
				msg.data.act_get_config_answer.act_get_config_data.act_get_config_pos_answer.pos = 0;
			} else {
				msg.data.act_get_config_answer.act_get_config_data.act_get_config_pos_answer.pos = RX24_get_position(actuator->servo.id);
			}
			break;
		case TORQUE_CONFIG:
			if(global.flags.virtual_mode) {
				msg.data.act_get_config_answer.act_get_config_data.torque = 0;
			} else {
				msg.data.act_get_config_answer.act_get_config_data.torque = RX24_get_speed_percentage(actuator->servo.id);
			}
			break;
		case TEMPERATURE_CONFIG:
			if(global.flags.virtual_mode) {
				msg.data.act_get_config_answer.act_get_config_data.temperature = 0;
			} else {
				msg.data.act_get_config_answer.act_get_config_data.temperature = RX24_get_temperature(actuator->servo.id);
			}
			break;
		case LOAD_CONFIG:
			if(global.flags.virtual_mode) {
				msg.data.act_get_config_answer.act_get_config_data.load = 0;
			} else {
				msg.data.act_get_config_answer.act_get_config_data.load = RX24_get_load_percentage(actuator->servo.id);
			}
			break;
		default:
			error = TRUE;
			warn_printf("This config is not available\n");
	}

	if(!error){
		CAN_send(&msg); // Envoi du message CAN
	}
}


/**************************************************************************/
/**	Fonctions pour la gestion de l'asservissement en vitesse             **/
/**************************************************************************/

/**
 * @brief Fonction appellée par la queue pendant tout le temps d'execution de la commande en cours.
 * Le booleen init est a TRUE au premier lancement de la commande.
 * @param queueId l'id de la queue.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
 * @param init TRUE au premier appel pour initiliser la commande et FALSE ensuite.
 */
void IMPL_RX24_SPEED_run_command(queue_id_t queueId, void* actuator_gen_ptr, bool_e init) {
	Actuator_servo_t* actuator = (Actuator_servo_t*) actuator_gen_ptr;
	if(QUEUE_has_error(queueId)) {
		QUEUE_behead(queueId);
		return;
	}

	// Gestion des mouvements de l'actionneur
	if(queueId == actuator->queue_id) {
		if(init)
			IMPL_RX24_SPEED_command_init(actuator);
		else
			IMPL_RX24_SPEED_command_run(actuator);
	}
}

/**
 * @brief Fonction permettant d'initialiser une commande.
 * @param actuator l'instance de l'actionneur.
 */
static void IMPL_RX24_SPEED_command_init(Actuator_servo_t* actuator) {
	ACT_order_e order = QUEUE_get_arg(actuator->queue_id)->command;
	Sint16 goal_speed = 0;
	IMPL_RX24_SPEED_init_servo(actuator);

	if(order == ACT_DEFAULT_STOP) {
		if(!global.flags.virtual_mode) {
			RX24_set_torque_enabled(actuator->servo.id, FALSE); //Stopper l'asservissement de l'RX24
		}
		QUEUE_next(actuator->queue_id, ACT_RESULT_DONE, ACT_RESULT_ERROR_OK, __LINE__);
		return;
	} else {
		// On obtient une valeur correcte car on a vérifié précédemment que l'ordre existait.
		goal_speed = QUEUE_get_arg(actuator->queue_id)->param;
	}

	if(!global.flags.virtual_mode && actuator->is_initialized == FALSE){
		error_printf("Impossible to set the actuator sid=0x%x at desired speed, it is not initialized\n", actuator->sid);
		QUEUE_next(actuator->queue_id, ACT_RESULT_NOT_HANDLED, ACT_RESULT_ERROR_NOT_INITIALIZED, __LINE__);
		return;
	}

	if(goal_speed == ERROR_ACT_VALUE) {
		error_printf("Invalid RX24 speed for order: %u, code is broken !\n", order);
		QUEUE_next(actuator->queue_id, ACT_RESULT_NOT_HANDLED, ACT_RESULT_ERROR_LOGIC, __LINE__);
		return;
	}

	if(global.flags.virtual_mode) {
		// Calcul du temps d'execution de la commande (en ms)
		actuator->servo.simu.exec_finished_time = global.absolute_time; // Temps théorique de réponse quasiment nul
		debug_printf("Simulation: RX24 n°%u take %d ms to move.\n", actuator->servo.id, 0);
	} else {
		RX24_reset_last_error(actuator->servo.id); //Sécurité anti terroriste. Nous les parano on aime pas voir des erreurs là ou il n'y en a pas.
		RX24_set_torque_enabled(actuator->servo.id, TRUE); // Activation de l'asservissement de l'RX24
		if(!RX24_set_speed_percentage(actuator->servo.id, goal_speed)) {	//Si la commande n'a pas été envoyée correctement et/ou que l'RX24 ne répond pas a cet envoi, on l'indique à la carte stratégie
			error_printf("RX24 n°%u don't answered when setting speed: error=0x%x\n",  actuator->servo.id, RX24_get_last_error(actuator->servo.id).error);
			QUEUE_next(actuator->queue_id, ACT_RESULT_FAILED, ACT_RESULT_ERROR_NOT_HERE, __LINE__);
			return;
		}
	}
	//La commande a été envoyée et l'RX24 l'a bien reçu
	debug_printf("Move RX24 n°%u at speed %d\n", actuator->servo.id, goal_speed);
}

/**
 * @brief Fonction qui vérifie l'execution de la commande en cours.
 * @param actuator l'instance de l'actionneur.
 */
static void IMPL_RX24_SPEED_command_run(Actuator_servo_t* actuator) {
	Uint8 result = 0, errorCode = 0;
	Uint16 line = 0;
	Sint16 pos = 0;
	Uint16 wanted_speed = QUEUE_get_arg(actuator->queue_id)->param;

	if(global.flags.virtual_mode) {
		if(global.absolute_time > actuator->servo.simu.exec_finished_time) {
			actuator->servo.simu.current_speed = wanted_speed;
			QUEUE_next(actuator->queue_id, ACT_RESULT_DONE, ACT_RESULT_ERROR_OK, __LINE__);
		}
	} else {
		// L'asservissement en vitesse n'est pas top, du coup dès que le servo se met à bouger on considère que c'est bon.
		Uint16 speed = RX24_get_speed_percentage(actuator->servo.id);
		if(wanted_speed != 0 && speed != 0)
			speed = wanted_speed;
		else
			speed = 0;

		if(ACTQ_check_status_rx24(actuator->queue_id, actuator->servo.id, QUEUE_get_arg(actuator->queue_id)->param, speed, actuator->servo.epsilon, actuator->servo.timeout, actuator->servo.large_epsilon, &result, &errorCode, &line)) {
			QUEUE_next(actuator->queue_id, result, errorCode, line);
		}
	}

    // On ne surveille le warner que si il est activé
	if(actuator->warner.activated){
		pos = RX24_get_position(actuator->servo.id);
		IMPL_RX24_POS_check_warner(actuator, pos);
	}
}


/**************************************************************************/
/**	Fonctions pour la gestion des warners                                **/
/**************************************************************************/

/**
 * @brief Fonction permettant d'activer un warner sur une position du servo.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
 * @param msg le message CAN reçu.
 */
void IMPL_RX24_SPEED_set_warner(void* actuator_gen_ptr, CAN_msg_t *msg) {
	IMPL_RX24_POS_set_warner(actuator_gen_ptr, msg);
}



