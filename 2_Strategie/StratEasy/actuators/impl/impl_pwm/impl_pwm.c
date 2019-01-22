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
#include "../actuators/act_utils.h"
#include "../actuators/actuator.h"

// Les différents define pour le verbose sur uart
#define LOG_PREFIX "[PWM]         "
#define LOG_COMPONENT OUTPUT_LOG_COMPONENT_IMPL_PWM
#include "../QS/QS_outputlog.h"

// Les fonctions internes au fonctionnement de l'actionneur
static void IMPL_PWM_init_pwm(Actuator_pwm_t* actuator);
static void IMPL_PWM_do_order(Actuator_pwm_t* actuator, ACT_order_e order, Sint32 param);

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

	actuator->cmd_launched = FALSE;
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
 * @brief Fonction permettant d'exécuter une commande.
 * Fonction utilisée pour les commandes de position.
 * @param actuator_gen_ptr l'instance de l'actionneur.
 * @param order l'ordre à exécuter.
 * @param duty_cycle le pourcentage cyclique à appliquer.
 * @return l'état de l'initialisation de la commande.
 */
act_result_state_e IMPL_PWM_run_order(void* actuator_gen_ptr, ACT_order_e order, Sint32 duty_cycle) {
	Actuator_pwm_t* actuator = (Actuator_pwm_t*) actuator_gen_ptr;

	// On enregistre l'ordre courante et l'index de la commande correspondant
	actuator->current_order = order;
	actuator->current_param = duty_cycle;

	if(order == ACT_DEFAULT_STOP) {
		if(!global.flags.virtual_mode) {
			PWM_stop(actuator->pwm.tim_id, actuator->pwm.channel_id);
		}
		ACT_printResult((void*) actuator, order, ACT_RESULT_DONE, ACT_RESULT_ERROR_OK, __LINE__);
	} else {
		IMPL_PWM_do_order(actuator, order, duty_cycle);
		actuator->cmd_launched = TRUE;
	}

	debug_printf("Apply %lu percent of PWM on actuator id=0x%x\n", duty_cycle, actuator->id);
	return ACT_RESULT_DONE;
}

/**
 * @brief Fonction qui vérifie l'exécution de la commande en cours.
 * Fonction utilisée pour les commandes de position.
 * @param actuator_gen_ptr l'instance de l'actionneur.
 * @return l'état d'exécution de la commande.
 */
act_result_state_e IMPL_PWM_check_order(void* actuator_gen_ptr) {
	Actuator_pwm_t* actuator = (Actuator_pwm_t*) actuator_gen_ptr;
	act_result_state_e result = ACT_RESULT_IN_PROGRESS;
	ACT_order_e order = actuator->current_order;
	Uint8 duty_cycle = actuator->current_param;

	if(actuator->cmd_launched == FALSE) {
		//ACT_printResult((void*) actuator, actuator->current_order, ACT_RESULT_FAILED, ACT_RESULT_ERROR_INVALID_ARG, __LINE__);
		actuator->cmd_launched = FALSE;
		return ACT_RESULT_NOT_HANDLED;
	}

	if(global.flags.virtual_mode || duty_cycle == PWM_get_duty(actuator->pwm.tim_id, actuator->pwm.channel_id)) {
		ACT_printResult((void*) actuator, order, ACT_RESULT_DONE, ACT_RESULT_ERROR_OK, __LINE__);
		result = ACT_RESULT_DONE;
	} else {
		ACT_printResult((void*) actuator, order, ACT_RESULT_FAILED, ACT_RESULT_ERROR_LOGIC, __LINE__);
		result = ACT_RESULT_FAILED;
	}

	if(result != ACT_RESULT_IN_PROGRESS) {
		actuator->cmd_launched = FALSE;
	}

	return result;
}

/**
 * @brief Fonction qui execute l'ordre de la commande courante.
 * @param actuator l'instance de l'actionneur.
 * @param order l'ordre a executer
 * @param param le pourcentage de pwm à appliquer
 */
static void IMPL_PWM_do_order(Actuator_pwm_t* actuator, ACT_order_e order, Sint32 param) {
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
				debug_printf("Invalid order=%u for sid=0x%x\n", order, actuator->id);
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
		if(!global.flags.virtual_mode) {
			PWM_run(duty_cycle, actuator->pwm.tim_id, actuator->pwm.channel_id);
		}
	} else {
		if(!global.flags.virtual_mode) {
			PWM_stop(actuator->pwm.tim_id, actuator->pwm.channel_id);
		}
	}
}
