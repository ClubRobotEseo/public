/*
 *	Club Robot ESEO 2017 - 2018
 *
 *	@file : actuators_stratEasy.c
 *	@brief : Actionneurs de StartEasy
 *  @author : Valentin
 */

#include "actuators_stratEasy.h"


/********************************************************************************************************
								Guide d'implémentation d'un actionneur
 ********************************************************************************************************

	1) Ajouter la déclaration de l'actionneur dans ce fichier.
	2) Ajouter la définition de la variable de l'actionneur au fichier actuators_stratEasy.h
	3) Ajouter un #define EXAMPLE_ID au fichier actuators_stratEasy.h.
	4) Déclarer l'id de l'actionneur dans actuators.c
	5) Ajouter la déclaration de l'actionneur dans actuators.c dans la fonction ACTUATOR_declare_actuators()
			Formatage : ACT_DECLARE_ACTUATOR(nom_actionneur);
	6) Ajouter les postions dans QS_types.h dans l'énum ACT_order_e (avec "ACT_" et sans "_POS" à la fin)

 *//********************************************************************************************************
												Exemples
 ********************************************************************************************************

// Exemple RX24 asservi en position
Actuator_command_t example_commands[] = {
		{ACT_EXAMPLE_UP, 	400},
		{ACT_EXAMPLE_MID, 	512},
		{ACT_EXAMPLE_DOWN, 	624},
};
Actuator_servo_t example = {
	.name = "example",
	.kind = KIND_RX24_POS,
	.id = ACT_EXAMPLE,
	.standard_init = TRUE,
	.servo.id = EXAMPLE_RX24_ID,
	.servo.timeout = DEFAULT_SERVO_TIMEOUT,
	.servo.epsilon = DEFAULT_SERVO_POS_EPSILON,
	.servo.large_epsilon = DEFAULT_SERVO_POS_LARGE_EPSILON,
	.servo.max_torque = DEFAULT_SERVO_MAX_TORQUE_PERCENT,
	.servo.max_temperature = DEFAULT_SERVO_MAX_TEMPERATURE,
	.servo.speed = DEFAULT_SERVO_SPEED,
	.servo.min_angle = DEFAULT_SERVO_MIN_VALUE,
	.servo.max_angle = DEFAULT_SERVO_MAX_VALUE,
	.servo.init_pos = 512,	//					<------------ Position initiale
	.servo.torque_asser.Kp = DEFAULT_SERVO_KP,
	.servo.torque_asser.activated = FALSE,
	.servo.commands = example_commands,
	.servo.nb_commands = NB_ACT_COMMANDS(example_commands)
};

 ********************************************************************************************************/


Actuator_command_t arm_left_commands[] = {
		{ACT_STRATEASY_ARM_INIT, 			824},
		{ACT_STRATEASY_ARM_DISTRI_SOUTH, 	690},
		{ACT_STRATEASY_ARM_ACCELERATOR_UP, 	391},
		{ACT_STRATEASY_ARM_ACCELERATOR_DOWN, 	536},
		{ACT_STRATEASY_ARM_UP, 			515},
		{ACT_STRATEASY_ARM_MID, 			516},
		{ACT_STRATEASY_ARM_DOWN, 			517},

};
Actuator_servo_t arm_left = {
	.name = "ARM_LEFT_RX24",
	.kind = KIND_RX24_POS,
	.id = ACTUATOR_ARM_LEFT,
	.standard_init = TRUE,
	.servo.id = ARM_LEFT_RX24_ID,
	.servo.timeout = DEFAULT_SERVO_TIMEOUT,
	.servo.epsilon = DEFAULT_SERVO_POS_EPSILON,
	.servo.large_epsilon = DEFAULT_SERVO_POS_LARGE_EPSILON,
	.servo.max_torque = DEFAULT_SERVO_MAX_TORQUE_PERCENT,
	.servo.max_temperature = DEFAULT_SERVO_MAX_TEMPERATURE,
	.servo.speed = DEFAULT_SERVO_SPEED,
	.servo.min_angle = DEFAULT_SERVO_MIN_VALUE,
	.servo.max_angle = DEFAULT_SERVO_MAX_VALUE,
	.servo.init_pos = 824,	//					<------------ Position initiale
	.servo.commands = arm_left_commands,
	.servo.nb_commands = NB_ACT_COMMANDS(arm_left_commands)
};


Actuator_command_t arm_right_commands[] = {
		{ACT_STRATEASY_ARM_INIT, 			200},
		{ACT_STRATEASY_ARM_ORIENTATION, 	600},
		{ACT_STRATEASY_ARM_DISTRI_SOUTH, 	364},
		{ACT_STRATEASY_ARM_ACCELERATOR_UP, 	505},
		{ACT_STRATEASY_ARM_ACCELERATOR_DOWN, 	361},
		{ACT_STRATEASY_ARM_UP, 			511},
		{ACT_STRATEASY_ARM_MID, 			513},
		{ACT_STRATEASY_ARM_DOWN, 			514},
};
Actuator_servo_t arm_right = {
	.name = " ARM_RIGHT_RX24",
	.kind = KIND_RX24_POS,
	.id = ACTUATOR_ARM_RIGHT,
	.standard_init = TRUE,
	.servo.id = ARM_RIGHT_RX24_ID,
	.servo.timeout = DEFAULT_SERVO_TIMEOUT,
	.servo.epsilon = DEFAULT_SERVO_POS_EPSILON,
	.servo.large_epsilon = DEFAULT_SERVO_POS_LARGE_EPSILON,
	.servo.max_torque = DEFAULT_SERVO_MAX_TORQUE_PERCENT,
	.servo.max_temperature = DEFAULT_SERVO_MAX_TEMPERATURE,
	.servo.speed = DEFAULT_SERVO_SPEED,
	.servo.min_angle = DEFAULT_SERVO_MIN_VALUE,
	.servo.max_angle = DEFAULT_SERVO_MAX_VALUE,
	.servo.init_pos = 200,	//					<------------ Position initiale
	.servo.commands = arm_right_commands,
	.servo.nb_commands = NB_ACT_COMMANDS(arm_right_commands)
};

