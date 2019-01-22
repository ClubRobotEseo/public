/**
 *	Club Robot ESEO 2017 - 2018
 *
 * @file actuator_servo.c
 * @brief Definition d'un actionneur servomoteur.
 * @author Valentin
 */

#include "actuator_servo.h"
#include "../QS/QS_outputlog.h"

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
		case KIND_AX12_POS_DOUBLE:
			theoretical_speed = AX12_THEORETICAL_SPEED;
			break;
		case KIND_RX24_POS:
		case KIND_RX24_POS_DOUBLE:
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
