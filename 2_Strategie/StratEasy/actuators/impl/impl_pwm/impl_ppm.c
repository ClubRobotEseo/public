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
#include "../actuators/act_utils.h"
#include "../actuators/actuator.h"

// Les différents define pour le verbose sur uart
#define LOG_PREFIX "[PPM]         "
#define LOG_COMPONENT OUTPUT_LOG_COMPONENT_IMPL_PPM
#include "../QS/QS_outputlog.h"

// Les fonctions internes au fonctionnement de l'actionneur
static void IMPL_PPM_init_pwm(Actuator_ppm_t* actuator);
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

	actuator->cmd_launched = FALSE;
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
 * @brief Fonction permettant d'exécuter une commande.
 * Fonction utilisée pour les commandes de position.
 * @param actuator_gen_ptr l'instance de l'actionneur.
 * @param order l'ordre à exécuter.
 * @param duty_cycle le pourcentage cyclique à appliquer.
 * @return l'état de l'initialisation de la commande.
 */
act_result_state_e IMPL_PPM_run_order(void* actuator_gen_ptr, ACT_order_e order, Sint32 param) {
	Actuator_ppm_t* actuator = (Actuator_ppm_t*) actuator_gen_ptr;
	Sint16 cmd_index = 0;
	Sint16 duty_cycle = 0;
	act_result_state_e result = ACT_RESULT_IN_PROGRESS;

	bool_e order_is_valid = ACT_get_command_index(&cmd_index, order, actuator->ppm.commands, actuator->ppm.nb_commands);

	if(order_is_valid == FALSE) {
		ACT_printResult((void*) actuator, order, ACT_RESULT_FAILED, ACT_RESULT_ERROR_INVALID_ARG, __LINE__);
		return ACT_RESULT_NOT_HANDLED;
	}

	// On enregistre l'ordre courante et l'index de la commande correspondant
	actuator->current_order = order;
	actuator->ppm.current_cmd_index = cmd_index;

	if(order == ACT_DEFAULT_STOP) {
		if(!global.flags.virtual_mode) {
			PWM_stop(actuator->ppm.tim_id, actuator->ppm.channel_id);
		}
		result = ACT_RESULT_DONE;
		ACT_printResult((void*) actuator, order, ACT_RESULT_DONE, ACT_RESULT_ERROR_OK, __LINE__);
		debug_printf("Apply 0 percent of PWM on actuator id=0x%x\n", actuator->id);
	} else {
		if(cmd_index < actuator->ppm.nb_commands) {
			duty_cycle = actuator->ppm.commands[cmd_index].value;
			IMPL_PPM_do_order(actuator, order, duty_cycle);
			actuator->cmd_launched = TRUE;
			result = ACT_RESULT_DONE;
			debug_printf("Apply %u percent of PWM on actuator id=0x%x\n", duty_cycle, actuator->id);
		} else {
			result = ACT_RESULT_NOT_HANDLED;
			ACT_printResult((void*) actuator, order, ACT_RESULT_NOT_HANDLED, ACT_RESULT_ERROR_LOGIC, __LINE__);
			debug_printf("Invalid command : command %d does not exist\n", cmd_index);
		}
	}

	return result;
}

/**
 * @brief Fonction qui vérifie l'exécution de la commande en cours.
 * Fonction utilisée pour les commandes de position.
 * @param actuator_gen_ptr l'instance de l'actionneur.
 * @return l'état d'exécution de la commande.
 */
act_result_state_e IMPL_PPM_check_order(void* actuator_gen_ptr) {
	Actuator_ppm_t* actuator = (Actuator_ppm_t*) actuator_gen_ptr;
	act_result_state_e result = ACT_RESULT_IN_PROGRESS;
	ACT_order_e order = actuator->current_order;
	Uint8 cmd_index = actuator->ppm.current_cmd_index;
	Sint16 duty_cycle = actuator->ppm.commands[cmd_index].value;

	if(actuator->cmd_launched == FALSE) {
		//ACT_printResult((void*) actuator, actuator->current_order, ACT_RESULT_FAILED, ACT_RESULT_ERROR_INVALID_ARG, __LINE__);
		actuator->cmd_launched = FALSE;
		return ACT_RESULT_NOT_HANDLED;
	}

	if(global.flags.virtual_mode || duty_cycle == PWM_get_duty(actuator->ppm.tim_id, actuator->ppm.channel_id)) {
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
