/**
 *	Club Robot ESEO 2017 - 2018
 *
 * @file impl_pwm.c
 * @brief Implémentation des fonctions pour un actionneur de type KIND_PWM.
 * @author Valentin
 */

// Les différents includes nécessaires...
#include "impl_pwm.h"

#include "../QS/QS_CANmsgList.h"
#include "../QS/QS_lowLayer/QS_pwm.h"
#include "../QS/QS_lowLayer/QS_can.h"
#include "../act_queue_utils.h"
#include "../ActManager.h"
#include "../queue.h"
#include "../actuators/actuator.h"

// Les différents define pour le verbose sur uart
#define LOG_PREFIX "[PWM]         "
#define LOG_COMPONENT OUTPUT_LOG_COMPONENT_IMPL_PWM
#include "../QS/QS_outputlog.h"

// Les fonctions internes au fonctionnement de l'actionneur
static void IMPL_PWM_init_pwm(Actuator_pwm_t* actuator);
static void IMPL_PWM_command_init(Actuator_pwm_t* actuator);
static void IMPL_PWM_command_run(Actuator_pwm_t* actuator);
static void IMPL_PWM_do_order(Actuator_pwm_t* actuator, ACT_order_e order, Sint16 param);

/**
 * @brief Fonction permettant d'initialiser le servo.
 * Elle initialise le bus de communication ainsi que les différents registres du servo.
 * Cette fonction est appelée au lancement de la carte via ActManager.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
 */
void IMPL_PWM_init(void* actuator_gen_ptr) {
	Actuator_pwm_t* actuator = (Actuator_pwm_t*) actuator_gen_ptr;

	if(actuator->is_initialized)
		return;

	// Vérifications (Si un des ces assert survient, votre actionneur est mal configuré.
	if(actuator->pwm.way_used) {
		assert(IS_GPIO_ALL_PERIPH(actuator->pwm.gpio_port_way));
		assert(IS_GET_GPIO_PIN(actuator->pwm.gpio_pin_way));
	}

	IMPL_PWM_init_pwm(actuator);
}

/**
 * @brief Fonction qui initialise les registres du servo avec la config souhaitee.
 * Initialise le servo s'il n'était pas alimente lors d'initialisations precedentes, si deja initialise, ne fait rien.
 * @param actuator_ l'instance de l'actionneur.
 */
static void IMPL_PWM_init_pwm(Actuator_pwm_t* actuator) {
	if(actuator->is_initialized == FALSE) {
		actuator->is_initialized = TRUE;
		PWM_add(actuator->pwm.tim_id, actuator->pwm.channel_id);
		PWM_set_frequency(actuator->pwm.tim_id, actuator->pwm.freq);
		PWM_stop(actuator->pwm.tim_id, actuator->pwm.channel_id);

		debug_printf("PWM n°%u init config DONE\n", actuator->pwm.channel_id);
	}
}

/**
 * Fonction appellée pour initialiser en position le servo dès l'arrivée de l'alimentation via ActManager.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
 * @return le status d'initialisation.
 */
error_e IMPL_PWM_init_pos(void *actuator_gen_ptr) {
	Actuator_pwm_t* actuator = (Actuator_pwm_t*) actuator_gen_ptr;
	PWM_stop(actuator->pwm.tim_id, actuator->pwm.channel_id);
	debug_printf("PWM (channel n°%u) has been initialized (stop mode)\n", actuator->pwm.channel_id);
	return END_OK;
}

/**
 * @brief Fonction pour stopper l'actionneur.
 * Fonction appellée à la fin du match via ActManager.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
 */
void IMPL_PWM_stop(void *actuator_gen_ptr) {
	Actuator_pwm_t* actuator = (Actuator_pwm_t*) actuator_gen_ptr;
	PWM_stop(actuator->pwm.tim_id, actuator->pwm.channel_id);
}

/**
 * @brief Fonction de dispatch des messages CAN pour les ordres actionneurs.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
 * @param msg le message CAN reçu.
 * @return TRUE si le message CAN a ete traite et FALSE sinon.
 */
bool_e IMPL_PWM_CAN_process_msg(void *actuator_gen_ptr, CAN_msg_t* msg) {
	Actuator_pwm_t* actuator = (Actuator_pwm_t*) actuator_gen_ptr;
	if(msg->sid == actuator->sid){
		ACT_order_e order = msg->data.act_order.order;

		if(order == ACT_POMPE_STOP || order == ACT_POMPE_NORMAL ||  order == ACT_POMPE_REVERSE) {
			ACTQ_push_operation_from_msg(msg, actuator->queue_id, &IMPL_PWM_run_command, msg->data.act_order.act_optionnal_data[0], TRUE);
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
void IMPL_PWM_run_command(queue_id_t queueId, void* actuator_gen_ptr, bool_e init) {
	Actuator_pwm_t* actuator = (Actuator_pwm_t*) actuator_gen_ptr;
	if(QUEUE_has_error(queueId)) {
		QUEUE_behead(queueId);
		return;
	}

	// Gestion des mouvements de l'actionneur
	if(queueId == actuator->queue_id) {
		if(init)
			IMPL_PWM_command_init(actuator);
		else
			IMPL_PWM_command_run(actuator);
	}
}

/**
 * @brief Fonction permettant d'initialiser une commande.
 * @param actuator l'instance de l'actionneur.
 */
static void IMPL_PWM_command_init(Actuator_pwm_t* actuator) {
	ACT_order_e order = QUEUE_get_arg(actuator->queue_id)->command;
	Sint16 param = QUEUE_get_arg(actuator->queue_id)->param; // Rapport cyclique de PWM demandé

	if(order == ACT_DEFAULT_STOP) {
		if(!global.flags.virtual_mode) {
			PWM_stop(actuator->pwm.tim_id, actuator->pwm.channel_id);
		}
		QUEUE_next(actuator->queue_id, ACT_RESULT_DONE, ACT_RESULT_ERROR_OK, __LINE__);
	} else {
		IMPL_PWM_do_order(actuator, order, param);
	}

	debug_printf("Apply %u percent of PWM on actuator sid=0x%x\n", param, actuator->sid);
}

/**
 * @brief Fonction qui vérifie l'execution de la commande en cours.
 * @param actuator l'instance de l'actionneur.
 */
static void IMPL_PWM_command_run(Actuator_pwm_t* actuator) {
	Uint8 param = QUEUE_get_arg(actuator->queue_id)->param;
	if(global.flags.virtual_mode || param == PWM_get_duty(actuator->pwm.tim_id, actuator->pwm.channel_id)) {
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
static void IMPL_PWM_do_order(Actuator_pwm_t* actuator, ACT_order_e order, Sint16 param) {
	Uint8 duty_cycle;
	bool_e order_is_valid = TRUE;

	if(actuator->pwm.way_used) {
		switch(order) {
			case ACT_POMPE_NORMAL:
				if(!global.flags.virtual_mode) {
					GPIO_SetBits(actuator->pwm.gpio_port_way, actuator->pwm.gpio_pin_way);
				}
				break;
			case ACT_POMPE_REVERSE:
				if(!global.flags.virtual_mode) {
					GPIO_ResetBits(actuator->pwm.gpio_port_way, actuator->pwm.gpio_pin_way);
				}
				break;
			default:
				debug_printf("Invalid order=%u for sid=0x%x\n", order, actuator->sid);
				order_is_valid = FALSE;
		}
	}

	if(order_is_valid) {
		if(param > 100){
			duty_cycle = 100;
		} else if(param < 0) {
			duty_cycle = 0;
		} else {
			duty_cycle = param;
		}
		QUEUE_get_arg(actuator->queue_id)->param = duty_cycle;
		if(!global.flags.virtual_mode) {
			PWM_run(duty_cycle, actuator->pwm.tim_id, actuator->pwm.channel_id);
		}
	} else {
		if(!global.flags.virtual_mode) {
			PWM_stop(actuator->pwm.tim_id, actuator->pwm.channel_id);
		}
	}
}
