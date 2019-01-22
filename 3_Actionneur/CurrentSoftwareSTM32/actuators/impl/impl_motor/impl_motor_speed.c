/**
 *	Club Robot ESEO 2017 - 2018
 *
 * @file impl_motor_speed.c
 * @brief Implémentation des fonctions pour un actionneur de type KIND_MOTOR_SPEED.
 * @author Valentin
 */

// Les différents includes nécessaires...
#include "impl_motor_speed.h"

#include "../QS/QS_CANmsgList.h"
#include "../QS/QS_lowLayer/QS_pwm.h"
#include "../QS/QS_lowLayer/QS_can.h"
#include "../QS/QS_sensor/QS_rpm_sensor.h"
#include "../QS/QS_actuator/QS_DCMotorSpeed.h"
#include "../act_queue_utils.h"
#include "../ActManager.h"
#include "../queue.h"
#include "../actuators/actuator.h"

// Les différents define pour le verbose sur uart
#define LOG_PREFIX "[MOTOR_SPEED] "
#define LOG_COMPONENT OUTPUT_LOG_COMPONENT_IMPL_MOTOR_SPEED
#include "../QS/QS_outputlog.h"

// Les fonctions internes au fonctionnement de l'actionneur
static void IMPL_MOTOR_SPEED_command_init(Actuator_motor_speed_t* actuator);
static void IMPL_MOTOR_SPEED_command_run(Actuator_motor_speed_t* actuator);

/**
 * @brief Fonction permettant d'initialiser le moteur.
 * Elle initialise la pwm et l'asservissement du moteur.
 * Cette fonction est appelée au lancement de la carte via ActManager.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
 */
void IMPL_MOTOR_SPEED_init(void* actuator_gen_ptr) {
	Actuator_motor_speed_t* actuator = (Actuator_motor_speed_t*) actuator_gen_ptr;

	if(actuator->is_initialized)
			return;
	actuator->is_initialized = TRUE;

	// Vérifications (Si un des ces assert survient, votre actionneur est mal configuré.
	assert(IS_GPIO_ALL_PERIPH(actuator->motor.way_latch));
	assert(IS_GET_GPIO_PIN(actuator->motor.way_bit_number));

	DC_MOTOR_SPEED_init();
	RPM_SENSOR_init();
	actuator->sensor.id = RPM_SENSOR_addSensor(actuator->sensor.gpio, actuator->sensor.pin, actuator->sensor.edge, actuator->sensor.ticks_per_rev);
	actuator->motor.sensorId = actuator->sensor.id; //On transfert l'id du sensor dans la structure du moteur (necessaire pour QS_DCMotorSpeed).
	actuator->motor.sensorRead = &RPM_SENSOR_getSpeed;
	PWM_add(actuator->motor.pwm_tim_id, actuator->motor.pwm_channel_id);
	DC_MOTOR_SPEED_config(actuator->sensor.id, &(actuator->motor));
}

/**
 * Fonction appellée pour initialiser le moteur dès l'arrivée de l'alimentation via ActManager.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
 * @return le status d'initialisation.
 */
error_e IMPL_MOTOR_SPEED_init_pos(void *actuator_gen_ptr) {
	Actuator_motor_speed_t* actuator = (Actuator_motor_speed_t*) actuator_gen_ptr;
	DC_MOTOR_SPEED_stop(actuator->motor.id);
	debug_printf("Motor n°%u has been initialized (stop mode)\n", actuator->motor.id);
	return END_OK;
}

/**
 * @brief Fonction pour stopper l'actionneur.
 * Fonction appellée à la fin du match via ActManager.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
 */
void IMPL_MOTOR_SPEED_stop(void *actuator_gen_ptr) {
	Actuator_motor_speed_t* actuator = (Actuator_motor_speed_t*) actuator_gen_ptr;
	DC_MOTOR_SPEED_stop(actuator->motor.id);
}

/**
 * @brief Fonction de dispatch des messages CAN pour les ordres actionneurs.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
 * @param msg le message CAN reçu.
 * @return TRUE si le message CAN a ete traite et FALSE sinon.
 */
bool_e IMPL_MOTOR_SPEED_CAN_process_msg(void *actuator_gen_ptr, CAN_msg_t* msg) {
	Actuator_motor_speed_t* actuator = (Actuator_motor_speed_t*) actuator_gen_ptr;
	if(msg->sid == actuator->sid){
		ACT_order_e order = msg->data.act_order.order;

		if(order == ACT_MOTOR_STOP || order == ACT_MOTOR_RUN) {
			ACTQ_push_operation_from_msg(msg, actuator->queue_id, &IMPL_MOTOR_SPEED_run_command, msg->data.act_order.act_optionnal_data[0], TRUE);
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
void IMPL_MOTOR_SPEED_run_command(queue_id_t queueId, void* actuator_gen_ptr, bool_e init) {
	Actuator_motor_speed_t* actuator = (Actuator_motor_speed_t*) actuator_gen_ptr;
	if(QUEUE_has_error(queueId)) {
		QUEUE_behead(queueId);
		return;
	}

	// Gestion des mouvements de l'actionneur
	if(queueId == actuator->queue_id) {
		if(init)
			IMPL_MOTOR_SPEED_command_init(actuator);
		else
			IMPL_MOTOR_SPEED_command_run(actuator);
	}
}

/**
 * @brief Fonction permettant d'initialiser une commande.
 * @param actuator l'instance de l'actionneur.
 */
static void IMPL_MOTOR_SPEED_command_init(Actuator_motor_speed_t* actuator) {
	ACT_order_e order = QUEUE_get_arg(actuator->queue_id)->command;
	Sint16 dcm_goal_speed = QUEUE_get_arg(actuator->queue_id)->param;

	if(dcm_goal_speed != 0 && order != ACT_MOTOR_STOP){
		if(global.flags.virtual_mode) {
			actuator->simu.exec_finished_time = global.absolute_time + actuator->simu.theoretical_exec_time;
			debug_printf("Simulation: motor n°%u take %ld ms to move.\n", actuator->motor.id, actuator->simu.theoretical_exec_time);
		} else {
			DC_MOTOR_SPEED_setSpeed(actuator->motor.id, dcm_goal_speed);
			DC_MOTOR_SPEED_restart(actuator->motor.id);
		}
		debug_printf("Update speed of DC motor (%d rpm)\n", dcm_goal_speed);
	}else{
		if(!global.flags.virtual_mode) {
			DC_MOTOR_SPEED_stop(actuator->motor.id);
		}
		QUEUE_next(actuator->queue_id, ACT_RESULT_DONE, ACT_RESULT_ERROR_OK, __LINE__);
		debug_printf("Stop DC motor sid=Ox%x\n", actuator->sid);
	}
}

/**
 * @brief Fonction qui vérifie l'execution de la commande en cours.
 * @param actuator l'instance de l'actionneur.
 */
static void IMPL_MOTOR_SPEED_command_run(Actuator_motor_speed_t* actuator) {
	Uint8 result = 0, error_code = 0;
	Uint16 line = 0;

	if(global.flags.virtual_mode) {
		if(global.absolute_time > actuator->simu.exec_finished_time) {
			QUEUE_next(actuator->queue_id, ACT_RESULT_DONE, ACT_RESULT_ERROR_OK, __LINE__);
		}
	} else {
		if(ACTQ_check_status_dcMotorSpeed(actuator->motor.id, &result, &error_code, &line)){
			QUEUE_next(actuator->queue_id, result, error_code, line);
		}
	}
}

