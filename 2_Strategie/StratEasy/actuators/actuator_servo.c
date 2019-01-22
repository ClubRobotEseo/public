/**
 *	Club Robot ESEO 2017 - 2018
 *
 * @file actuator_servo.c
 * @brief Definition d'un actionneur servomoteur.
 * @author Valentin
 */

#include "actuator_servo.h"
#include "../QS/QS_outputlog.h"
#include "../QS/QS_actuator/QS_ax12.h"
#include "../QS/QS_actuator/QS_rx24.h"

/**
 * @brief Calcul du temps mis par un servo pour effectuer une commande d'asservissement en position.
 * @param kind le type de l'actionneur.
 * @param servo_speed la vitesse du servo.
 * @param goal_position la nouvelle position demandé.
 * @param current_position la position courante du servo.
 * @return le temps thérotique mis par le servo pour effectuer la commande (en ms).
 */
time32_t ACT_SERVO_compute_cmd_exec_time(Actuator_kind_e kind, Uint8 servo_speed, Sint16 goal_position, Sint16 current_position) {
	Uint16 theoretical_speed = 0;
	time32_t exec_time = 0;

	switch(kind) {
		case KIND_AX12_POS:
//		case KIND_AX12_POS_DOUBLE:
			theoretical_speed = AX12_THEORETICAL_SPEED;
			break;
		case KIND_RX24_POS:
//		case KIND_RX24_POS_DOUBLE:
			theoretical_speed = RX24_THEORETICAL_SPEED;
			break;
		default:
			error_printf("ACT_SERVO_compute_cmd_exec_time: This kind of actuator cannot be used\n");
	}

	if(theoretical_speed != 0) {
		double rpm_speed = ((double)(theoretical_speed * servo_speed)) / 100; // Vitesse (en rpm)
		double distance = ((double) absolute(goal_position - current_position) * 300) / 1024;
		// Explication: temps = vitesse / distance
		// movement / 360 = nombre de tours
		// rpm_speed / 60 = vitesse en tours par secondes (on en prend ici l'inverse)
		// division par 1000 pour avoir le temps en ms
		exec_time = (distance / 360) * (60 / rpm_speed) * 1000; // Temps d'execution de la commande (en ms)
	}
	return exec_time;
}

#ifdef USE_AX12_SERVO
/**
 * @brief Fonction de configuration des AX12.
 * @param servo l'instance du servo concerne (ici juste Actuator_servo_data_t suffit).
 * @param config la configuration à changer.
 * @param value la nouvelle valeur de la configuration (Si égale à 0, cela reprend la valeur par défaut).
 */
void ACT_SERVO_config_AX12(Actuator_servo_data_t * servo, act_config_e config, Uint16 value) {
	Uint8 percentage;
	switch(config){
		case SPEED_CONFIG : // Configuration de la vitesse
			if(AX12_is_wheel_mode_enabled(servo->id)){
				if(value == 0){
					percentage = servo->speed;
				}else{
					percentage = value;
				}
				servo->simu.current_speed = percentage;
				if(!global.flags.virtual_mode) {
					AX12_set_speed_percentage(servo->id, percentage);
				}
				debug_printf("Configuration de la vitesse (wheel mode) de l'AX12 %d avec une valeur de %d\n", servo->id, percentage);
			}else{
				if(value == 0){
					percentage = servo->speed;
				}else{
					percentage = value;
				}
				servo->simu.current_speed = percentage;
				if(!global.flags.virtual_mode) {
					AX12_set_move_to_position_speed(servo->id, percentage);
				}
				debug_printf("Configuration de la vitesse (position mode) de l'AX12 %d avec une valeur de %d\n", servo->id, percentage);
			}
			break;

		case TORQUE_CONFIG : // Configuration du couple
			if(value == 0){
				percentage = servo->max_torque;
			}else{
				percentage = value;
			}
			if(!global.flags.virtual_mode) {
				AX12_set_torque_limit(servo->id, percentage);
			}
			debug_printf("Configuration du couple de l'AX12 %d avec une valeur de %d\n", servo->id, percentage);
			break;

		default :
			warn_printf("invalid CAN msg data[2]=%u (configuration impossible)!\n", config);
	}
}
#endif /* USE_RX12_SERVO */

#ifdef USE_RX24_SERVO
/**
 * @brief Fonction de configuration des RX24.
 * @param servo l'instance du servo concerne (ici juste Actuator_servo_data_t suffit).
 * @param config la configuration à changer.
 * @param value la nouvelle valeur de la configuration (Si égale à 0, cela reprend la valeur par défaut).
 */
void ACT_SERVO_config_RX24(Actuator_servo_data_t * servo, act_config_e config, Uint16 value) {
	Uint8 percentage;
	switch(config){
		case SPEED_CONFIG : // Configuration de la vitesse
			if(RX24_is_wheel_mode_enabled(servo->id)){
				if(value == 0){
					percentage = servo->speed;
				}else{
					percentage = value;
				}
				servo->simu.current_speed = percentage;
				if(!global.flags.virtual_mode) {
					RX24_set_speed_percentage(servo->id, percentage);
				}
				debug_printf("Configuration de la vitesse (wheel mode) du RX24 %d avec une valeur de %d\n", servo->id, percentage);
			}else{
				if(value == 0){
					percentage = servo->speed;
				}else{
					percentage = value;
				}
				servo->simu.current_speed = percentage;
				if(!global.flags.virtual_mode) {
					RX24_set_move_to_position_speed(servo->id, percentage);
				}
				debug_printf("Configuration de la vitesse (position mode) du RX24 %d avec une valeur de %d\n", servo->id, percentage);
			}
			break;

		case TORQUE_CONFIG : // Configuration du couple
			if(value == 0){
				percentage = servo->max_torque;
			}else{
				percentage = value;
			}
			if(!global.flags.virtual_mode) {
				RX24_set_torque_limit(servo->id, percentage);
			}
			debug_printf("Configuration du couple du RX24 %d avec une valeur de %d\n", servo->id, percentage);
			break;

		default :
			warn_printf("invalid CAN msg data[2]=%u (configuration impossible)!\n", config);
	}
}
#endif /* USE_RX24_SERVO */
