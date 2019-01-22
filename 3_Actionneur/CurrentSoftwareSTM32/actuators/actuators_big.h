/*
 *	Club Robot ESEO 2017 - 2018
 *
 *	@file : actuators_big.h
 *	@brief : Actionneurs du gros robot
 *  @author : Valentin
 */

#ifndef ACTUATORS_ACTUATORS_BIG_H_
#define ACTUATORS_ACTUATORS_BIG_H_

#include "actuator.h"
#include "actuator_servo.h"
#include "actuator_tor.h"
#include "actuator_pwm.h"
#include "actuator_motor.h"

//------------------ Configuration des ID actionneurs BIG_ROBOT ------------------

// ID des différents moteurs asservis en position
//#define EXEMPLE_ID			0

// ID des différents moteurs asservis en vitesse
//#define EXEMPLE_ID			0

// ID des différents AX12 (L'id doit être inférieur à 50)
//#define EXEMPLE_ID			0

// ID des différents RX24 (L'id doit être inférieur à 50)
//#define EXEMPLE_ID			0
#define ACT_BIG_ELEVATOR_FRONT_RIGHT_RX24_ID		27
#define ACT_BIG_ELEVATOR_FRONT_MIDDLE_RX24_ID		42
#define ACT_BIG_ELEVATOR_FRONT_LEFT_RX24_ID			34
#define ACT_BIG_LOCKER_FRONT_RIGHT_RX24_ID			30
#define ACT_BIG_LOCKER_FRONT_MIDDLE_RX24_ID			23
#define ACT_BIG_LOCKER_FRONT_LEFT_RX24_ID			25
#define ACT_BIG_LOCKER_BACK_RX24_ID					0
#define ACT_BIG_SLOPE_TAKER_BACK_RX24_ID			24
#define ACT_BIG_SORTING_BACK_VERY_RIGHT_RX24_ID		0
#define ACT_BIG_SORTING_BACK_RIGHT_RX24_ID			43
#define ACT_BIG_SORTING_BACK_LEFT_RX24_ID			28
#define ACT_BIG_SORTING_BACK_VERY_LEFT_RX24_ID		0


//------------------ Configuration des ID actionneurs BIG_ROBOT ------------------
// extern Actuator_servo_t exemple;
extern Actuator_servo_t big_elevator_front_right;
extern Actuator_servo_t big_elevator_front_middle;
extern Actuator_servo_t big_elevator_front_left;
extern Actuator_servo_t big_locker_front_right;
extern Actuator_servo_t big_locker_front_middle;
extern Actuator_servo_t big_locker_front_left;
extern Actuator_servo_t big_locker_back;
extern Actuator_servo_t big_slope_taker;
extern Actuator_servo_t big_sorting_back_very_right;
extern Actuator_servo_t big_sorting_back_right;
extern Actuator_servo_t big_sorting_back_left;
extern Actuator_servo_t big_sorting_back_very_left;

#endif /* ACTUATORS_ACTUATORS_BIG_H_ */
