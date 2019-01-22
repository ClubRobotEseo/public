/*
 *	Club Robot ESEO 2017 - 2018
 *
 *	@file : actuators_small.h
 *	@brief : Actionneurs du petit robot
 *  @author : Valentin
 */

#ifndef ACTUATORS_ACTUATORS_SMALL_H_
#define ACTUATORS_ACTUATORS_SMALL_H_

#include "actuator.h"
#include "actuator_servo.h"
#include "actuator_tor.h"
#include "actuator_pwm.h"
#include "actuator_motor.h"

//------------------ Configuration des ID actionneurs SMALL_ROBOT ------------------

// ID des différents moteurs asservis en position
//#define EXEMPLE_ID			0

// ID des différents moteurs asservis en vitesse
//#define EXEMPLE_ID			0

// ID des différents AX12 (L'id doit être inférieur à 50)
//#define EXEMPLE_ID			0


// ID des différents RX24 (L'id doit être inférieur à 50)
//#define EXEMPLE_ID			0
#define ACT_SMALL_LOCKER_BACK_RX24_ID				44
#define ACT_SMALL_LOCKER_FRONT_LEFT_RX24_ID			46
#define ACT_SMALL_LOCKER_FRONT_RIGHT_RX24_ID		38
#define ACT_SMALL_SORTING_BACK_MIDDLE_RX24_ID		32
#define ACT_SMALL_SORTING_BACK_LEFT_RX24_ID			31
#define ACT_SMALL_SORTING_BACK_RIGHT_RX24_ID		33
#define ACT_SMALL_ELEVATOR_FRONT_LEFT_RX24_ID		40
#define ACT_SMALL_ELEVATOR_FRONT_RIGHT_RX24_ID		35


//------------------ Configuration des ID actionneurs SMALL_ROBOT ------------------
// extern Actuator_servo_t exemple;
extern Actuator_servo_t small_locker_back;
extern Actuator_servo_t small_locker_front_right;
extern Actuator_servo_t small_locker_front_left;
extern Actuator_servo_t small_sorting_back_left;
extern Actuator_servo_t small_sorting_back_middle;
extern Actuator_servo_t small_sorting_back_right;
extern Actuator_servo_t small_elevator_front_left;
extern Actuator_servo_t small_elevator_front_right;



#endif /* ACTUATORS_ACTUATORS_SMALL_H_ */
