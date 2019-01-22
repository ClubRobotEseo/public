/**
 *	Club Robot ESEO 2017 - 2018
 *
 * @file impl_mosfet.c
 * @brief Implémentation des fonctions pour un actionneur mosfet de type KIND_TOR.
 * @author Valentin
 */

// Les différents includes nécessaires...
#include "impl_mosfet.h"

#include "../QS/QS_CANmsgList.h"
#include "../QS/QS_lowLayer/QS_can.h"
#include "../act_queue_utils.h"
#include "../ActManager.h"
#include "../queue.h"
#include "../actuators/actuator.h"

// Les différents define pour le verbose sur uart
#define LOG_PREFIX "[MOSFET]      "
#define LOG_COMPONENT OUTPUT_LOG_COMPONENT_IMPL_MOSFET
#include "../QS/QS_outputlog.h"

// Les fonctions internes au fonctionnement de l'actionneur
static void IMPL_MOSFET_do_order(Actuator_tor_t* actuator, ACT_order_e order);

/**
 * @brief Fonction permettant d'initialiser le servo.
 * Elle initialise le bus de communication ainsi que les différents registres du servo.
 * Cette fonction est appelée au lancement de la carte via ActManager.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
 */
void IMPL_MOSFET_init(void* actuator_gen_ptr) {
	Actuator_tor_t* actuator = (Actuator_tor_t*) actuator_gen_ptr;

	if(actuator->is_initialized)
		return;

	// Vérifications (Si un des ces assert survient, votre actionneur est mal configuré.
	assert(IS_GPIO_ALL_PERIPH(actuator->tor.gpio));
	assert(IS_GET_GPIO_PIN(actuator->tor.pin));

	actuator->is_initialized = TRUE;
}

/**
 * Fonction appellée pour initialiser en position le servo dès l'arrivée de l'alimentation via ActManager.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
 * @return le status d'initialisation.
 */
error_e IMPL_MOSFET_init_pos(void *actuator_gen_ptr) {
	Actuator_tor_t* actuator = (Actuator_tor_t*) actuator_gen_ptr;
	GPIO_ResetBits(actuator->tor.gpio, actuator->tor.pin);
	return END_OK;
}

/**
 * @brief Fonction pour stopper l'actionneur.
 * Fonction appellée à la fin du match via ActManager.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
 */
void IMPL_MOSFET_stop(void *actuator_gen_ptr) {
	Actuator_tor_t* actuator = (Actuator_tor_t*) actuator_gen_ptr;
	GPIO_ResetBits(actuator->tor.gpio, actuator->tor.pin);
}

/**
 * @brief Fonction de dispatch des messages CAN pour les ordres actionneurs.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
 * @param msg le message CAN reçu.
 * @return TRUE si le message CAN a ete traite et FALSE sinon.
 */
bool_e IMPL_MOSFET_CAN_process_msg(void *actuator_gen_ptr, CAN_msg_t* msg) {
	Actuator_tor_t* actuator = (Actuator_tor_t*) actuator_gen_ptr;
	if(msg->sid == actuator->sid){
		ACT_order_e order = msg->data.act_order.order;

		if(order == ACT_MOSFET_STOP || order == ACT_MOSFET_NORMAL) {
			IMPL_MOSFET_do_order(actuator, order);
		} else {
			component_printf(LOG_LEVEL_Warning, "invalid CAN msg order=%u for sid=%u!\n", msg->data.act_order.order, actuator->sid);
		}
		return TRUE;
	}
	return FALSE;
}

/**
 * @brief Fonction permettant d'effectuer un order.
 * @param actuator l'instance de l'actionneur.
 * @param order l'ordre à effectuer
 */
static void IMPL_MOSFET_do_order(Actuator_tor_t* actuator, ACT_order_e order) {
	bool_e order_is_valid = TRUE;

	switch(order) {
		case ACT_MOSFET_STOP:
			if(!global.flags.virtual_mode) {
				GPIO_ResetBits(actuator->tor.gpio, actuator->tor.pin);
			}
			debug_printf("Reset mosfet on actuator sid=0x%x\n", actuator->sid);
			break;
		case ACT_MOSFET_NORMAL:
			if(!global.flags.virtual_mode) {
				GPIO_SetBits(actuator->tor.gpio, actuator->tor.pin);
			}
			debug_printf("Set mosfet on actuator sid=0x%x\n", actuator->sid);
			break;
		default:
			debug_printf("Invalid CAN msg order=%u for sid=%u!\n", order, actuator->sid);
			order_is_valid = FALSE;
	}

	if(order_is_valid && (global.flags.virtual_mode || order == GPIO_ReadOutputDataBit(actuator->tor.gpio, actuator->tor.pin))) {
		ACTQ_finish_SendResult(actuator->queue_id, ACT_RESULT_DONE, ACT_RESULT_ERROR_OK, __LINE__);
	}else {
		ACTQ_finish_SendResult(actuator->queue_id, ACT_RESULT_FAILED, ACT_RESULT_ERROR_LOGIC, __LINE__);
	}
}
