/*
 * stepper_motors.h
 *
 *  Created on: 21 juin 2018
 *      Author: Nirgal
 */

#ifndef STEPPER_MOTORS_H_
#define STEPPER_MOTORS_H_

#include "QS/QS_all.h"


typedef struct
{
	trajectory_e trajectory;
	uint32_t nb_step;	//quantité de pas
	bool_e forward;
	int16_t max_speed;	//en mm/sec
}order_t;

bool_e STEPPER_MOTOR_state_machine(order_t * order);

#endif /* STEPPER_MOTORS_H_ */
