/**
 *	Club Robot ESEO 2017 - 2018
 *
 * @file impl_ppm.c
 * @brief Implémentation des fonctions pour un actionneur de type KIND_PWM.
 * @author Valentin
 */

// Les différents includes nécessaires...
#include "impl_ppm.h"

#include "../QS/QS_CANmsgList.h"
#include "../QS/QS_lowLayer/QS_pwm.h"
#include "../QS/QS_lowLayer/QS_can.h"
#include "../act_queue_utils.h"
#include "../ActManager.h"
#include "../queue.h"
#include "../actuators/actuator.h"

// Les différents define pour le verbose sur uart
#define LOG_PREFIX "[PPM]         "
#define LOG_COMPONENT OUTPUT_LOG_COMPONENT_IMPL_PPM
#include "../QS/QS_outputlog.h"

// Les fonctions internes au fonctionnement de l'actionneur
static void IMPL_PPM_init_pwm(Actuator_ppm_t* actuator);
static void IMPL_PPM_command_init(Actuator_ppm_t* actuator);
static void IMPL_PPM_command_run(Actuator_ppm_t* actuator);
static void IMPL_PPM_do_order(Actuator_ppm_t* actuator, ACT_order_e order, Sint16 param);

/**
 * @brief Fonction permettant d'initialiser le servo.
 * Elle initialise le bus de communication ainsi que les différents registres du servo.
 * Cette fonction est appelée au lancement de la carte via ActManager.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
 */
void IMPL_PPM_init(void* actuator_gen_ptr) {
	Actuator_ppm_t* actuator = (Actuator_ppm_t*) actuator_gen_ptr;

	if(actuator->is_initialized)
		return;

	IMPL_PPM_init_pwm(actuator);
}

/**
 * @brief Fonction qui initialise les registres du servo avec la config souhaitee.
 * Initialise le servo s'il n'était pas alimente lors d'initialisations precedentes, si deja initialise, ne fait rien.
 * @param actuator_ l'instance de l'actionneur.
 */
static void IMPL_PPM_init_pwm(Actuator_ppm_t* actuator) {
	if(actuator->is_initialized == FALSE) {
		actuator->is_initialized = TRUE;
		PWM_add(actuator->ppm.tim_id, actuator->ppm.channel_id);
		PWM_set_frequency(actuator->ppm.tim_id, actuator->ppm.freq);
		PWM_run(actuator->ppm.init_pos, actuator->ppm.tim_id, actuator->ppm.channel_id);

		debug_printf("PWM n°%u init config DONE\n", actuator->ppm.channel_id);
	}
}

/**
 * Fonction appellée pour initialiser en position le servo dès l'arrivée de l'alimentation via ActManager.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
 * @return le status d'initialisation.
 */
error_e IMPL_PPM_init_pos(void *actuator_gen_ptr) {
	Actuator_ppm_t* actuator = (Actuator_ppm_t*) actuator_gen_ptr;
	PWM_stop(actuator->ppm.tim_id, actuator->ppm.channel_id);
	debug_printf("PPM (channel n°%u) has been initialized (stop mode)\n", actuator->ppm.channel_id);
	return END_OK;
}

/**
 * @brief Fonction pour stopper l'actionneur.
 * Fonction appellée à la fin du match via ActManager.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
 */
void IMPL_PPM_stop(void *actuator_gen_ptr) {
	Actuator_ppm_t* actuator = (Actuator_ppm_t*) actuator_gen_ptr;
	PWM_stop(actuator->ppm.tim_id, actuator->ppm.channel_id);
}

/**
 * @brief Fonction de dispatch des messages CAN pour les ordres actionneurs.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
 * @param msg le message CAN reçu.
 * @return TRUE si le message CAN a ete traite et FALSE sinon.
 */
bool_e IMPL_PPM_CAN_process_msg(void *actuator_gen_ptr, CAN_msg_t* msg) {
	Actuator_ppm_t* actuator = (Actuator_ppm_t*) actuator_gen_ptr;
	if(msg->sid == actuator->sid){
		Sint16 param = 0;
		bool_e order_is_valid = ACT_get_command_index(&param, msg->data.act_order.order, actuator->ppm.commands, actuator->ppm.nb_commands);

		if(order_is_valid) {
			ACTQ_push_operation_from_msg(msg, actuator->queue_id, &IMPL_PPM_run_command, param, TRUE);
		} else {
			component_printf(LOG_LEVEL_Warning, "invalid CAN msg order=%u for sid=%u!\n",  msg->data.act_order.order, actuator->sid);
		}
		return TRUE;
	}
	return FALSE;
}

/**
 * @brief Fonction appellée par la queue pendant tout le temps d'execution de la commande en cours.
 * Le booleen init est a TRUE au premier lancement de la commande.
 * @param queueId l'id de la queue.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
 * @param init TRUE au premier appel pour initiliser la commande et FALSE ensuite.
 */
void IMPL_PPM_run_command(queue_id_t queueId, void* actuator_gen_ptr, bool_e init) {
	Actuator_ppm_t* actuator = (Actuator_ppm_t*) actuator_gen_ptr;
	if(QUEUE_has_error(queueId)) {
		QUEUE_behead(queueId);
		return;
	}

	// Gestion des mouvements de l'actionneur
	if(queueId == actuator->queue_id) {
		if(init)
			IMPL_PPM_command_init(actuator);
		else
			IMPL_PPM_command_run(actuator);
	}
}

/**
 * @brief Fonction permettant d'initialiser une commande.
 * @param actuator l'instance de l'actionneur.
 */
static void IMPL_PPM_command_init(Actuator_ppm_t* actuator) {
	ACT_order_e order = QUEUE_get_arg(actuator->queue_id)->command;
	Uint8 cmd_index = 0;
	Sint16 duty_cycle = 0;

	if(order == ACT_DEFAULT_STOP) {
		if(!global.flags.virtual_mode) {
			PWM_stop(actuator->ppm.tim_id, actuator->ppm.channel_id);
		}
		QUEUE_next(actuator->queue_id, ACT_RESULT_DONE, ACT_RESULT_ERROR_OK, __LINE__);
		debug_printf("Apply 0 percent of PWM on actuator sid=0x%x\n", actuator->sid);
	} else {
		// On obtient une valeur correcte car on a vérifié précédemment que l'ordre existait.
		cmd_index = QUEUE_get_arg(actuator->queue_id)->param;
		if(cmd_index < actuator->ppm.nb_commands) {
			duty_cycle = actuator->ppm.commands[cmd_index].value;
			IMPL_PPM_do_order(actuator, order, duty_cycle);
			debug_printf("Apply %u percent of PWM on actuator sid=0x%x\n", duty_cycle, actuator->sid);
		} else {
			QUEUE_next(actuator->queue_id, ACT_RESULT_FAILED, ACT_RESULT_ERROR_LOGIC, __LINE__);
			debug_printf("Invalid command : command %d does not exist\n", cmd_index);
		}
	}
}

/**
 * @brief Fonction qui vérifie l'execution de la commande en cours.
 * @param actuator l'instance de l'actionneur.
 */
static void IMPL_PPM_command_run(Actuator_ppm_t* actuator) {
	Uint8 cmd_index = QUEUE_get_arg(actuator->queue_id)->param;
	Sint16 duty_cycle = actuator->ppm.commands[cmd_index].value;

	if(global.flags.virtual_mode || duty_cycle == PWM_get_duty(actuator->ppm.tim_id, actuator->ppm.channel_id)) {
		QUEUE_next(actuator->queue_id, ACT_RESULT_DONE, ACT_RESULT_ERROR_OK, __LINE__);
	} else {
		QUEUE_next(actuator->queue_id, ACT_RESULT_FAILED, ACT_RESULT_ERROR_LOGIC, __LINE__);
	}
}

/**
 * @brief Fonction qui execute l'ordre de la commande courante.
 * @param actuator l'instance de l'actionneur.
 * @param order l'ordre a executer
 * @param param le pourcentage de pwm à appliquer
 */
static void IMPL_PPM_do_order(Actuator_ppm_t* actuator, ACT_order_e order, Sint16 param) {
	Uint8 duty_cycle;
	bool_e order_is_valid = TRUE;

	if(order_is_valid) {
		if(param > MAX_PPM_DUTY_CYCLE){
			duty_cycle = MAX_PPM_DUTY_CYCLE;
		} else if(param < MIN_PPM_DUTY_CYCLE) {
			duty_cycle = MIN_PPM_DUTY_CYCLE;
		} else {
			duty_cycle = param;
		}
		if(!global.flags.virtual_mode) {
			PWM_run(duty_cycle, actuator->ppm.tim_id, actuator->ppm.channel_id);
		}
	} else {
		if(!global.flags.virtual_mode) {
			PWM_stop(actuator->ppm.tim_id, actuator->ppm.channel_id);
		}
	}
}
