/**
 *	Club Robot ESEO 2017 - 2018
 *
 * @file impl_motor_pos.c
 * @brief Implémentation des fonctions pour un actionneur de type KIND_MOTOR_POS.
 * @author Valentin
 */

// Les différents includes nécessaires...
#include "IMPL_MOTOR_POS.h"

#include "../QS/QS_CANmsgList.h"
#include "../QS/QS_lowLayer/QS_pwm.h"
#include "../QS/QS_lowLayer/QS_can.h"
#include "../QS/QS_sensor/QS_rpm_sensor.h"
#include "../QS/QS_actuator/QS_DCMotor2.h"
#include "../QS/QS_lowLayer/QS_qei.h"
#include "../act_queue_utils.h"
#include "../ActManager.h"
#include "../queue.h"
#include "../actuators/actuator.h"

// Les différents define pour le verbose sur uart
#define LOG_PREFIX "[MOTOR_POS]   "
#define LOG_COMPONENT OUTPUT_LOG_COMPONENT_IMPL_MOTOR_POS
#include "../QS/QS_outputlog.h"

// Les fonctions internes au fonctionnement de l'actionneur
static void IMPL_MOTOR_POS_command_init(Actuator_motor_pos_t* actuator);
static void IMPL_MOTOR_POS_command_run(Actuator_motor_pos_t* actuator);
static Sint32 IMPL_MOTOR_POS_get_position(DCMotor_config_t* motor);
static void IMPL_MOTOR_POS_state_machine(Actuator_motor_pos_t* actuator);


/**
 * @brief Fonction permettant d'initialiser le moteur.
 * Elle initialise la pwm et l'asservissement du moteur.
 * Cette fonction est appelée au lancement de la carte via ActManager.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
 */
void IMPL_MOTOR_POS_init(void* actuator_gen_ptr) {
	Actuator_motor_pos_t* actuator = (Actuator_motor_pos_t*) actuator_gen_ptr;

	if(actuator->is_initialized)
			return;
	actuator->is_initialized = TRUE;

	// Vérifications (Si un des ces assert survient, votre actionneur est mal configuré.
	assert(IS_GPIO_ALL_PERIPH(actuator->motor.way_latch));
	assert(IS_GET_GPIO_PIN(actuator->motor.way_bit_number));
	assert(IS_GPIO_ALL_PERIPH(actuator->sensor_fdc.way_latch));
	assert(IS_GET_GPIO_PIN(actuator->sensor_fdc.way_bit_number));
	assert(actuator->encoder.get != NULL);
	assert(actuator->encoder.set != NULL);

	actuator->ready = FALSE;
	actuator->init_first_order = FALSE;

	// Encoder
	actuator->encoder.ready = FALSE;
	actuator->encoder.count = 0;

	// State machine
	actuator->sm.state = INIT;
	actuator->sm.last_state = DEACTIVATE;
	actuator->sm.begin_detection = 0;
	actuator->sm.begin_wait = 0;
	actuator->sm.last_alim = FALSE;

	// Moteur
	actuator->motor.position = 0;
	actuator->motor.sensor_read = &IMPL_MOTOR_POS_get_position;

	DCM_init();
	QEI_init();

	PWM_add(actuator->motor.pwm_tim_id, actuator->motor.pwm_channel_id);
	DCM_config(actuator->motor.id, &(actuator->motor));
	DCM_stop(actuator->motor.id);
}

/**
 * Fonction appellée pour initialiser le moteur dès l'arrivée de l'alimentation via ActManager.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
 * @return le status d'initialisation.
 */
error_e IMPL_MOTOR_POS_init_pos(void *actuator_gen_ptr) {
	Actuator_motor_pos_t* actuator = (Actuator_motor_pos_t*) actuator_gen_ptr;
	// Ignoré ici car géré en IT avec une machine à état
	debug_printf("Motor n°%u will be initialized in position in IT\n", actuator->motor.id);
	return END_OK;
}

/**
 * @brief Fonction pour stopper l'actionneur.
 * Fonction appellée à la fin du match via ActManager.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
 */
void IMPL_MOTOR_POS_stop(void *actuator_gen_ptr) {
	Actuator_motor_pos_t* actuator = (Actuator_motor_pos_t*) actuator_gen_ptr;
	DCM_stop(actuator->motor.id);
}

/**
 * @brief Fonction de dispatch des messages CAN pour les ordres actionneurs.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
 * @param msg le message CAN reçu.
 * @return TRUE si le message CAN a ete traite et FALSE sinon.
 */
bool_e IMPL_MOTOR_POS_CAN_process_msg(void *actuator_gen_ptr, CAN_msg_t* msg) {
	Actuator_motor_pos_t* actuator = (Actuator_motor_pos_t*) actuator_gen_ptr;
	if(msg->sid == actuator->sid){
		Sint16 param = 0;
		bool_e order_is_valid = ACT_get_command_index(&param, msg->data.act_order.order, actuator->commands, actuator->nb_commands);

		if(order_is_valid) {
			ACTQ_push_operation_from_msg(msg, actuator->queue_id, &IMPL_MOTOR_POS_run_command, param, TRUE);
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
void IMPL_MOTOR_POS_run_command(queue_id_t queueId, void* actuator_gen_ptr, bool_e init) {
	Actuator_motor_pos_t* actuator = (Actuator_motor_pos_t*) actuator_gen_ptr;
	if(QUEUE_has_error(queueId)) {
		QUEUE_behead(queueId);
		return;
	}

	if(!actuator->ready) {
		// Initialisation de la position du moteur lors du premier ordre.
		if(!global.flags.virtual_mode) {
			IMPL_MOTOR_POS_state_machine(actuator);
		} else {
			// Pour la simulation
			actuator->simu.current_pos = 0;
			actuator->ready = TRUE;
		}
		actuator->init_first_order = TRUE;
	} else {
		// Gestion des mouvements de l'actionneur
		if(queueId == actuator->queue_id) {
			if(init || actuator->init_first_order) {
				IMPL_MOTOR_POS_command_init(actuator);
				actuator->init_first_order = FALSE;
			} else {
				IMPL_MOTOR_POS_command_run(actuator);
			}
		}
	}
}

/**
 * @brief Fonction permettant d'initialiser une commande.
 * @param actuator l'instance de l'actionneur.
 */
static void IMPL_MOTOR_POS_command_init(Actuator_motor_pos_t* actuator) {
	ACT_order_e order = QUEUE_get_arg(actuator->queue_id)->command;
	Uint8 cmd_index = QUEUE_get_arg(actuator->queue_id)->param;
	Sint16 dcm_goal_position = 0;

	if(order == ACT_DEFAULT_STOP) {
		if(!global.flags.virtual_mode) {
			DCM_stop(actuator->motor.id);
		}
		QUEUE_next(actuator->queue_id, ACT_RESULT_DONE, ACT_RESULT_ERROR_OK, __LINE__);
		return;
	} else {
		// On obtient une valeur correcte car on a vérifié précédemment que l'ordre existait.
		cmd_index = QUEUE_get_arg(actuator->queue_id)->param;
		if(cmd_index < actuator->nb_commands) {
			dcm_goal_position = actuator->commands[cmd_index].value;
		} else {
			dcm_goal_position = ERROR_ACT_VALUE;
		}
	}

	if(actuator->ready == FALSE){
		error_printf("Impossible de mettre l'actionneur en position il n'est pas initialisé\n");
		QUEUE_next(actuator->queue_id, ACT_RESULT_FAILED, ACT_RESULT_ERROR_NOT_INITIALIZED, __LINE__);
		return;
	}

	if(dcm_goal_position == ERROR_ACT_VALUE) {
		error_printf("Invalid dcm position for command: %u, code is broken !\n", order);
		QUEUE_next(actuator->queue_id, ACT_RESULT_NOT_HANDLED, ACT_RESULT_ERROR_LOGIC, __LINE__);
		return;
	}

	if(global.flags.virtual_mode) {
		time32_t exec_time = absolute(actuator->simu.current_pos - dcm_goal_position) * actuator->simu.theoretical_speed;
		actuator->simu.exec_finished_time = global.absolute_time + exec_time;
		debug_printf("Simulation: motor n°%u take %ld ms to move.\n", actuator->motor.id, exec_time);
	} else {
		DCM_setPosValue(actuator->motor.id, 0, dcm_goal_position);
		DCM_goToPos(actuator->motor.id, 0);
		DCM_restart(actuator->motor.id);
	}
	debug_printf("Placement en position %d du moteur DC lancé\n", dcm_goal_position);
}

/**
 * @brief Fonction qui vérifie l'execution de la commande en cours.
 * @param actuator l'instance de l'actionneur.
 */
static void IMPL_MOTOR_POS_command_run(Actuator_motor_pos_t* actuator) {
	Uint8 result = 0, error_code = 0;
	Uint16 line = 0;

	if(global.flags.virtual_mode) {
		if(global.absolute_time > actuator->simu.exec_finished_time) {
			Uint8 cmd_index = QUEUE_get_arg(actuator->queue_id)->param;
			Sint16 wanted_goal = actuator->commands[cmd_index].value;
			actuator->simu.current_pos = wanted_goal;
			QUEUE_next(actuator->queue_id, ACT_RESULT_DONE, ACT_RESULT_ERROR_OK, __LINE__);
		}
	} else {
		if(ACTQ_check_status_dcmotor(actuator->motor.id, FALSE, &result, &error_code, &line)){
			QUEUE_next(actuator->queue_id, result, error_code, line);
		}
	}
}

/**
 * @brief Machine à état permettant de controler le démarrage du moteur et son initialisation.
 * @param actuator l'instance de l'actionneur.
 * @param msg le message CAN reçu.
 */
static void IMPL_MOTOR_POS_state_machine(Actuator_motor_pos_t* actuator) {
	Uint8 result, error_code;
	Uint16 line;
	bool_e entrance = (actuator->sm.state != actuator->sm.last_state)? TRUE : FALSE;

	if(entrance)
		debug_printf("%d -> %d\n", actuator->sm.last_state, actuator->sm.state);

	actuator->sm.last_state = actuator->sm.state;


	switch (actuator->sm.state) {
		case INIT:
			actuator->sm.begin_detection = 0;
			actuator->sm.begin_wait = 0;
			actuator->sm.last_alim = FALSE;

			IMPL_MOTOR_POS_init(actuator);
			actuator->sm.state = WAIT_FDC;
			break;

		case WAIT_FDC:
			if(entrance){
				if(actuator->motor.inverse_way) {
					GPIO_SetBits(actuator->motor.way_latch, actuator->motor.way_bit_number);
				} else {
					GPIO_ResetBits(actuator->motor.way_latch, actuator->motor.way_bit_number);
				}
				if(actuator->init_mode == MOTOR_INIT_NORMAL_MODE){
					PWM_run(actuator->init_duty_cycle, actuator->motor.pwm_tim_id, actuator->motor.pwm_channel_id);
				}
				actuator->sm.begin_detection = 0;
				actuator->sm.lastSpeedMeasure = global.absolute_time;
				actuator->sm.lastSpeed = 0;
			}

			if(!actuator->sm.last_alim && global.flags.power)
				actuator->sm.begin_wait = global.absolute_time;
			actuator->sm.last_alim = global.flags.power;


			if(actuator->init_mode == MOTOR_INIT_SPEED_MODE){


				if(global.absolute_time - actuator->sm.lastSpeedMeasure >= 25){

					Sint32 pulseCount = absolute(actuator->encoder.get());
					actuator->encoder.set(0);

					time32_t differentialTime = global.absolute_time - actuator->sm.lastSpeedMeasure;
					actuator->sm.lastSpeedMeasure = global.absolute_time;

					Sint32 motorSpeed = (pulseCount * 1000) / differentialTime;

					motorSpeed = motorSpeed * 0.5 + actuator->sm.lastSpeed * 0.5;
					actuator->sm.lastSpeed = motorSpeed;

					Sint32 dutyCycle = (actuator->init_speed * actuator->init_speed_Kv) + (((Sint32)actuator->init_speed - motorSpeed) * actuator->init_speed_Kp);

					if(dutyCycle < 0)
						dutyCycle = 0;
					else if(dutyCycle > actuator->init_duty_cycle)
						dutyCycle = actuator->init_duty_cycle;

					PWM_run(dutyCycle, actuator->motor.pwm_tim_id, actuator->motor.pwm_channel_id);
				}
			}

			if(!GPIO_ReadInputDataBit(actuator->sensor_fdc.way_latch, actuator->sensor_fdc.way_bit_number) && actuator->sm.begin_detection == 0) {
				actuator->sm.begin_detection = global.absolute_time;
			}

			if(!GPIO_ReadInputDataBit(actuator->sensor_fdc.way_latch, actuator->sensor_fdc.way_bit_number) && actuator->sm.begin_detection != 0 && global.absolute_time - actuator->sm.begin_detection > 50) {
				PWM_run(0, actuator->motor.pwm_tim_id, actuator->motor.pwm_channel_id);
				actuator->encoder.set(0);
				actuator->encoder.ready = TRUE;
				actuator->sm.state = INIT_POS;
			}else if(global.flags.power && global.absolute_time - actuator->sm.begin_wait > actuator->motor.timeout + 1000) {
				actuator->sm.state = DEACTIVATE;
			}

			if(GPIO_ReadInputDataBit(actuator->sensor_fdc.way_latch, actuator->sensor_fdc.way_bit_number)) {
				actuator->sm.begin_detection = 0;
			}
			break;

		case INIT_POS:
			DCM_setPosValue(actuator->motor.id, 0, actuator->init_pos);
			DCM_goToPos(actuator->motor.id, 0);
			DCM_restart(actuator->motor.id);
			actuator->sm.state = WAIT_POS;
			break;

		case WAIT_POS:
			if(ACTQ_check_status_dcmotor(actuator->motor.id, FALSE, &result, &error_code, &line)){
				if(result == ACT_RESULT_DONE){
					actuator->ready = TRUE;
					debug_printf("Win !\n");
					actuator->sm.state = RUN;
				}else{
					DCM_stop(actuator->motor.id);
					debug_printf("Fail !\n");
					actuator->sm.state = DEACTIVATE;
				}
			}
			break;

		case RUN:
			break;

		case DEACTIVATE:
			if(entrance) {
				debug_printf("Deactivate !\n");
				PWM_stop(actuator->motor.pwm_tim_id, actuator->motor.pwm_channel_id);
			}
			break;
	}
}

/**
 * @brief Fonction permettant de récupérer la position courante.
 * @param actuator l'instance de l'actionneur.
 * @param msg le message CAN reçu.
 */
static Sint32 IMPL_MOTOR_POS_get_position(DCMotor_config_t* motor){
	return (Sint32)((motor->position) * (motor->coeff_pos_a) + (motor->coeff_pos_b));

}


/**************************************************************************/
/**	Fonctions pour la gestion des configs                                **/
/**************************************************************************/

/**
 * @brief Fonction permettant de modifier la config du servo comme la vitesse ou le couple.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
 * @param msg le message CAN reçu.
 */
void IMPL_MOTOR_POS_set_config(void *actuator_gen_ptr, CAN_msg_t* msg) {
	Actuator_motor_pos_t* actuator = (Actuator_motor_pos_t*) actuator_gen_ptr;

	switch(msg->data.act_set_config.config){
		case SPEED_CONFIG:
			if(msg->data.act_set_config.data_config.raw_data != 0)
				DCM_setPwmWay(actuator->motor.id, msg->data.act_set_config.data_config.raw_data, msg->data.act_set_config.data_config.raw_data);
			else
				DCM_setPwmWay(actuator->motor.id, actuator->motor.way0_max_duty, actuator->motor.way1_max_duty);
			break;

		default :
			warn_printf("invalid CAN msg data[2]=%u (configuration impossible)!\n", msg->data.act_set_config.config);
	}
}

/**
 * @brief Fonction permettant d'obtenir des infos sur la config du moteur comme la position.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
 * @param incoming_msg le message CAN reçu.
 */
void IMPL_MOTOR_POS_get_config(void *actuator_gen_ptr, CAN_msg_t *incoming_msg) {
	Actuator_motor_pos_t* actuator = (Actuator_motor_pos_t*) actuator_gen_ptr;
	bool_e error = FALSE;
	Uint16 pos = 0;
	CAN_msg_t msg;
	msg.sid = ACT_GET_CONFIG_ANSWER;
	msg.size = SIZE_ACT_GET_CONFIG_ANSWER;
	msg.data.act_get_config_answer.act_sid = 0xFF & (actuator->sid);
	msg.data.act_get_config_answer.config = incoming_msg->data.act_set_config.config;

	switch(incoming_msg->data.act_set_config.config){
		case POSITION_CONFIG:
			pos = IMPL_MOTOR_POS_get_position(&(actuator->motor));
			msg.data.act_get_config_answer.act_get_config_data.act_get_config_pos_answer.order = 0;
			msg.data.act_get_config_answer.act_get_config_data.act_get_config_pos_answer.pos = pos;
			break;

		default:
			error = TRUE;
			error_printf("The config %u is not available\n", incoming_msg->data.act_set_config.config);
	}

	if(!error){
		CAN_send(&msg); // Envoi du message CAN
	}
}

/**************************************************************************/
/**	Fonction d'interruption                                              **/
/**************************************************************************/

/**
 * @brief Fonction d'interruption d'un moteur.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
 */
void IMPL_MOTOR_POS_process_it(void* actuator_gen_ptr) {
	Actuator_motor_pos_t* actuator = (Actuator_motor_pos_t*) actuator_gen_ptr;

	if(actuator->encoder.ready){
		Sint16 count = actuator->encoder.get();
		Sint32 delta = count - actuator->encoder.count;

		if(delta > 32000)
			delta -= 65536;
		else if(delta < -32000)
			delta += 65536;

		actuator->encoder.count = count;

		actuator->motor.position += delta * actuator->encoder.coeff;
	}
}

/**************************************************************************/
/**	Fonction de tache de fond                                            **/
/**************************************************************************/
/**
 * @brief Fonction de tache de fond d'un moteur.
 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
 */
void IMPL_MOTOR_POS_process_main(void* actuator_gen_ptr) {

}
