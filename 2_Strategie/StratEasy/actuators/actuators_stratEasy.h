/*
 *	Club Robot ESEO 2017 - 2018
 *
 *	@file : actuators_stratEasy.h
 *	@brief : Actionneurs de StratEasy
 *  @author : Valentin
 */

#ifndef ACTUATORS_ACTUATORS_STRATEASY_H_
#define ACTUATORS_ACTUATORS_STRATEASY_H_

#include "actuator.h"
#include "actuator_servo.h"
#include "actuator_pwm.h"

//------------------ Configuration des ID actionneurs STRAT_EASY ------------------

// ID des différents AX12 (L'id doit être inférieur à 50)
//#define EXEMPLE_ID			0

// ID des différents RX24 (L'id doit être inférieur à 50)
//#define EXEMPLE_ID			0
#define ARM_LEFT_RX24_ID		29
#define ARM_RIGHT_RX24_ID		26


//------------------ Configuration des ID actionneurs STRAT_EASY  ------------------
// extern Actuator_servo_t exemple;

extern Actuator_servo_t arm_left;
extern Actuator_servo_t arm_right;


#endif /* ACTUATORS_ACTUATORS_STRATEASY_H_ */
