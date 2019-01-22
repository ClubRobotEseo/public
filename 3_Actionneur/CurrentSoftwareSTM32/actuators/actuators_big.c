/*
 *	Club Robot ESEO 2017 - 2018
 *
 *	@file : actuators_big.c
 *	@brief : Actionneurs du gros robot
 *  @author : Valentin
 */

#include "actuators_big.h"
#include "../queue.h"
#include "../QS/QS_lowLayer/QS_qei.h"

/********************************************************************************************************
								Guide d'implémentation d'un actionneur
 ********************************************************************************************************

	1) Ajouter la déclaration de l'actionneur dans ce fichier.
	2) Ajouter la définition de la variable de l'actionneur au fichier actuators_big.h
	3) Ajouter un #define EXAMPLE_ID au fichier actuators_big.h.
	4) Déclarer le sid de l'actionneur dans QS/QS_CANmsgList.h (tout à la fin du fichier)
	5) Ajouter une valeur dans l'énumération de la queue dans config/config_global_vars_types.h
			Formatage : QUEUE_ACT_EXAMPLE
		Si l'actionneur a besoin d'une queue (servos et moteurs), cette valeur doit être déclarée avant NB_QUEUE.
		Si l'actionneur n'a pas besoin d'une queue, cette valeur doit être déclarée après NB_QUEUE
		(Cela sert juste à compter le nombre d'actionneurs de façon statique).
	6) Ajouter la déclaration de l'actionneur dans ActManager dans la fonction ACTMGR_declare_actuators()
			Formatage : ACT_DECLARE_ACTUATOR(nom_actionneur);
	7) Ajouter le pilotage via terminal dans le fichier term_io.c dans le tableau terminal_motor
			Formatage : ACT_DECLARE(EXAMPLE)
	8) Ajouter les postions dans QS_types.h dans l'énum ACT_order_e (avec "ACT_" et sans "_POS" à la fin)

	En stratégie
	1) Ajouter une d'une valeur dans le tableau act_link_SID_Queue du fichier act_functions.c/h
	2) (optionnel) Ajouter l'évitement actionneur dans act_avoidance.c/h si l'actionneur modifie l'évitement du robot
	3) (optionnel) Ajout de la verbosité dans QS/QS_can_verbose.h


 ********************************************************************************************************
												Exemples
 ********************************************************************************************************/

/*
// Exemple AX12 asservi en position
Actuator_command_t example_commands[] = {
		{ACT_EXAMPLE_TOP, 	400},
		{ACT_EXAMPLE_MID, 	512},
		{ACT_EXAMPLE_BOT, 	624},
		{.order = ACT_EXAMPLE_TORQUE, .value = 630, .kind = CMD_ASSER_TORQUE, .param.torque.threshold = 20, .param.torque.way = WAY_ASSER_TORQUE_INC}
};
Actuator_servo_t example = {
	.name = "example",
	.kind = KIND_AX12_POS,
	.sid = ACT_EXAMPLE,
	.queue_id = QUEUE_ACT_EXAMPLE,
	.standard_init = TRUE,
	.servo.id = EXAMPLE_AX12_ID,
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

// Exemple AX12 asservi en vitesse
Actuator_command_t example_commands[] = {
		{ACT_EXAMPLE_NORMAL_SPEED, -100},
		{ACT_EXAMPLE_ZERO_SPEED, 	0}
};
Actuator_servo_t example = {
	.name = "example",
	.kind = KIND_AX12_SPEED,
	.sid = ACT_EXAMPLE,
	.queue_id = QUEUE_ACT_EXAMPLE,
	.standard_init = TRUE,
	.servo.id = EXAMPLE_AX12_ID,
	.servo.timeout = DEFAULT_SERVO_TIMEOUT,
	.servo.epsilon = DEFAULT_SERVO_POS_EPSILON,
	.servo.large_epsilon = DEFAULT_SERVO_POS_LARGE_EPSILON,
	.servo.max_torque = DEFAULT_SERVO_MAX_TORQUE_PERCENT,
	.servo.max_temperature = DEFAULT_SERVO_MAX_TEMPERATURE,
	.servo.speed = DEFAULT_SERVO_SPEED,
	.servo.min_angle = DEFAULT_SERVO_MIN_VALUE,
	.servo.max_angle = DEFAULT_SERVO_MAX_VALUE,
	.servo.init_pos = 0,	//					<------------ Position initiale
	.servo.commands = example_commands,
	.servo.nb_commands = NB_ACT_COMMANDS(example_commands)
};

// Exemple RX24 asservi en position
Actuator_command_t example_commands[] = {
		{ACT_EXAMPLE_TOP, 	400},
		{ACT_EXAMPLE_MID, 	512},
		{ACT_EXAMPLE_BOT, 	624},
		{.order = ACT_EXAMPLE_TORQUE, .value = 630, .kind = CMD_ASSER_TORQUE, .param.torque.threshold = 20, .param.torque.way = WAY_ASSER_TORQUE_INC}
};
Actuator_servo_t example = {
	.name = "example",
	.kind = KIND_RX24_POS,
	.sid = ACT_EXAMPLE,
	.queue_id = QUEUE_ACT_EXAMPLE,
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

// Exemple RX24 asservi en vitesse
Actuator_command_t example_commands[] = {
		{ACT_EXAMPLE_NORMAL_SPEED, -100},
		{ACT_EXAMPLE_ZERO_SPEED, 	0}
};
Actuator_servo_t example = {
	.name = "example",
	.kind = KIND_RX24_SPEED,
	.sid = ACT_EXAMPLE,
	.queue_id = QUEUE_ACT_EXAMPLE,
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
	.servo.init_pos = 0,	//					<------------ Position initiale
	.servo.commands = example_commands,
	.servo.nb_commands = NB_ACT_COMMANDS(example_commands)
};

// Exemple mosfet sur carte mosfet
Actuator_tor_t example = {
	.name = "example",
	.kind = KIND_MOSFET_ON_MOSFET_BOARD,
	.sid = ACT_EXAMPLE,
	.queue_id = NO_QUEUE_ID,
	.standard_init = TRUE,
};

// Exemple moteur asservi en vitesse (Adapter les pins + Déclarer la PWM et la comptage des ticks dans config/config_use.h)
Actuator_motor_speed_t example = {
	.name = "example",
	.kind = KIND_MOTOR_SPEED,
	.sid = ACT_EXAMPLE,
	.queue_id = QUEUE_ACT_EXAMPLE,
	.standard_init = TRUE,
	.motor.id = EXAMPLE_ID,
	.motor.activateRecovery = FALSE,
	.motor.speedRecovery = -10,
	.motor.timeRecovery = 1500,
	.motor.Kp = 100,
	.motor.Ki = 3000,
	.motor.Kd = 800,
	.motor.Kv = 177,
	// (Réduction / ConstanteVitesseMoteur * MaxPWM * 1024) / TensionAlimentation
	// Constante de vitesse moteur : 460
	// Réduction : 19:1
	.motor.epsilon = 2000,
	.motor.max_duty = 100,
	.motor.pwm_number = 3,
	.motor.simulateWay = TRUE,
	.motor.timeout = 2000,
	.motor.way_latch = GPIOC,
	.motor.way_bit_number = GPIO_Pin_11,
	.sensor.gpio = RPM_SENSOR_GPIO_A,
	.sensor.pin = RPM_SENSOR_PIN_7,
	.sensor.edge = RPM_SENSOR_Edge_Rising,
	.sensor.ticks_per_rev = 8
};


// Exemple de moteur asservi en position
// Exemple de moteur asservi en position
Actuator_command_t example_commands[] = {
		{ACT_CMD_EXAMPLE_BOT, 	0},
		{ACT_CMD_EXAMPLE_N0, 	3},
		{ACT_CMD_EXAMPLE_N1, 	(3 + 58 + 20)},
		{ACT_CMD_EXAMPLE_N2, 	(3 + 2 * 58 + 20)},
		{ACT_CMD_EXAMPLE_N3, 	(3 + 2 * 58 + 20)},
		{ACT_CMD_EXAMPLE_TOP, 	140}
};
Actuator_motor_pos_t example = {
	.name = "example",
 	.kind = KIND_MOTOR_POS,
 	.sid = ACT_EXAMPLE,
 	.queue_id = QUEUE_ACT_EXAMPLE,
	.standard_init = TRUE,
 	.motor.id = MOTOR_EXAMPLE_ID,
 	.motor.double_PID = FALSE,
 	.motor.Kp = 7000,
 	.motor.Ki = 3000,
 	.motor.Kd = 0,
 	.motor.Kp2 = 0,
 	.motor.Ki2 = 0,
 	.motor.Kd2 = 0,
 	.motor.coeff_pos_a = 1/1600., // 160000 impulsions pour 100mm
 	.motor.coeff_pos_b = 0,
 	.motor.pwm_tim_id = PWM_TIM_8,
	.motor.pwm_channel_id = PWM_CH_1,
 	.motor.way_latch = GPIOA,
 	.motor.way_bit_number = GPIO_Pin_8,
 	.motor.way0_max_duty = 70,
 	.motor.way1_max_duty = 30,
 	.motor.timeout = 2000,
 	.motor.dead_zone = 0,
 	.motor.epsilon = 10,
 	.motor.large_epsilon = 500,
 	.motor.inverse_way = TRUE,
 	.encoder.coeff = 0x0001,
 	.encoder.get = &QEI1_get_count,
 	.encoder.set = &QEI1_set_count,
 	.sensor_fdc.way_latch = GPIOC,
 	.sensor_fdc.way_bit_number = GPIO_Pin_2,
 	.init_pos = 0,
 	.init_duty_cycle = 50,
	.init_mode = MOTOR_INIT_SPEED_MODE,
	.init_speed = 15000,
	.init_speed_Kv = 0.0028,
	.init_speed_Kp = 0.0015,
 	.commands = small_elevator_commands,
 	.nb_commands = NB_ACT_COMMANDS(example_commands)
};


// Exemple de pompe (Changer la pin en fonction de la channel de PWM choisi + Déclarer la PWM dans config/config_use.h)
Actuator_pwm_t example = {
	.name = "example",
	.kind = KIND_PWM,
	.sid = ACT_EXAMPLE,
	.queue_id = QUEUE_ACT_EXAMPLE,
	.standard_init = TRUE,
	.pwm.pwm_number = EXAMPLE_PWM_NUM,
	.pwm.gpio = GPIOC,
	.pwm.pin = GPIO_Pin_11
};

// Exemple de PPM (Changer la pin en fonction de la channel de PWM choisi + Déclarer la PWM dans config/config_use.h)
 Actuator_command_t example_commands[] = {
		{ACT_CMD_EXAMPLE_LOCK,		6},
		{ACT_CMD_EXAMPLE_UNLOCK,	9}
};
Actuator_ppm_t example = {
	.name = "example",
 	.kind = KIND_PPM,
 	.sid = ACT_EXAMPLE,
 	.queue_id = QUEUE_ACT_EXAMPLE,
	.standard_init = TRUE,
 	.ppm.tim_id = PWM_TIM_1,
	.ppm.channel_id = PWM_CH_2,
	.ppm.freq = 50,
	.ppm.init_pos = 6,	//					<------------ Position initiale
	.ppm.commands = small_ball_adv_liberator_commands,
	.ppm.nb_commands = NB_ACT_COMMANDS(example_commands)
};

 ********************************************************************************************************/


Actuator_command_t big_elevator_front_right_commands[] = {
		{ACT_BIG_ELEVATOR_FRONT_RIGHT_IDLE, 		355},
		{ACT_BIG_ELEVATOR_FRONT_RIGHT_VERY_BOT, 	355},
		{ACT_BIG_ELEVATOR_FRONT_RIGHT_BOT, 			343},
		{ACT_BIG_ELEVATOR_FRONT_RIGHT_ALMOST_BOT, 	300},
		{ACT_BIG_ELEVATOR_FRONT_RIGHT_ACCELERATOR_BOT, 	84},
		{ACT_BIG_ELEVATOR_FRONT_RIGHT_TOP, 			24},
};
Actuator_servo_t big_elevator_front_right = {
	.name = "big_elevator_front_right",
	.kind = KIND_RX24_POS,
	.sid = ACT_BIG_ELEVATOR_FRONT_RIGHT,
	.queue_id = QUEUE_ACT_BIG_ELEVATOR_FRONT_RIGHT,
	.standard_init = TRUE,
	.servo.id = ACT_BIG_ELEVATOR_FRONT_RIGHT_RX24_ID,
	.servo.timeout = DEFAULT_SERVO_TIMEOUT,
	.servo.epsilon = DEFAULT_SERVO_POS_EPSILON,
	.servo.large_epsilon = DEFAULT_SERVO_POS_LARGE_EPSILON,
	.servo.max_torque = DEFAULT_SERVO_MAX_TORQUE_PERCENT,
	.servo.max_temperature = DEFAULT_SERVO_MAX_TEMPERATURE,
	.servo.speed = DEFAULT_SERVO_SPEED,
	.servo.min_angle = DEFAULT_SERVO_MIN_VALUE,
	.servo.max_angle = DEFAULT_SERVO_MAX_VALUE,
	.servo.init_pos = 355,	//					<------------ Position initiale
	.servo.torque_asser.Kp = DEFAULT_SERVO_KP,
	.servo.torque_asser.activated = FALSE,
	.servo.commands = big_elevator_front_right_commands,
	.servo.nb_commands = NB_ACT_COMMANDS(big_elevator_front_right_commands)
};

Actuator_command_t big_elevator_front_middle_commands[] = {
		{ACT_BIG_ELEVATOR_FRONT_MIDDLE_IDLE, 		446},
		{ACT_BIG_ELEVATOR_FRONT_MIDDLE_VERY_BOT, 		452},
		{ACT_BIG_ELEVATOR_FRONT_MIDDLE_BOT, 		446},
		{ACT_BIG_ELEVATOR_FRONT_MIDDLE_ALMOST_BOT, 	370},
		{ACT_BIG_ELEVATOR_FRONT_MIDDLE_ACCELERATOR_BOT, 	172},
		{ACT_BIG_ELEVATOR_FRONT_MIDDLE_TOP, 		102},
};
Actuator_servo_t big_elevator_front_middle = {
	.name = "big_elevator_front_middle",
	.kind = KIND_RX24_POS,
	.sid = ACT_BIG_ELEVATOR_FRONT_MIDDLE,
	.queue_id = QUEUE_ACT_BIG_ELEVATOR_FRONT_MIDDLE,
	.standard_init = TRUE,
	.servo.id = ACT_BIG_ELEVATOR_FRONT_MIDDLE_RX24_ID,
	.servo.timeout = DEFAULT_SERVO_TIMEOUT,
	.servo.epsilon = DEFAULT_SERVO_POS_EPSILON,
	.servo.large_epsilon = DEFAULT_SERVO_POS_LARGE_EPSILON,
	.servo.max_torque = DEFAULT_SERVO_MAX_TORQUE_PERCENT,
	.servo.max_temperature = DEFAULT_SERVO_MAX_TEMPERATURE,
	.servo.speed = DEFAULT_SERVO_SPEED,
	.servo.min_angle = DEFAULT_SERVO_MIN_VALUE,
	.servo.max_angle = DEFAULT_SERVO_MAX_VALUE,
	.servo.init_pos = 446,	//					<------------ Position initiale
	.servo.torque_asser.Kp = DEFAULT_SERVO_KP,
	.servo.torque_asser.activated = FALSE,
	.servo.commands = big_elevator_front_middle_commands,
	.servo.nb_commands = NB_ACT_COMMANDS(big_elevator_front_middle_commands)
};

Actuator_command_t big_elevator_front_left_commands[] = {
		{ACT_BIG_ELEVATOR_FRONT_LEFT_IDLE, 			660},
		{ACT_BIG_ELEVATOR_FRONT_LEFT_VERY_BOT, 		666},
		{ACT_BIG_ELEVATOR_FRONT_LEFT_BOT, 			660},
		{ACT_BIG_ELEVATOR_FRONT_LEFT_ALMOST_BOT, 	580},
		{ACT_BIG_ELEVATOR_FRONT_LEFT_ACCELERATOR_BOT, 	390},
		{ACT_BIG_ELEVATOR_FRONT_LEFT_TOP, 			330},
};
Actuator_servo_t big_elevator_front_left = {
	.name = "big_elevator_front_left",
	.kind = KIND_RX24_POS,
	.sid = ACT_BIG_ELEVATOR_FRONT_LEFT,
	.queue_id = QUEUE_ACT_BIG_ELEVATOR_FRONT_LEFT,
	.standard_init = TRUE,
	.servo.id = ACT_BIG_ELEVATOR_FRONT_LEFT_RX24_ID,
	.servo.timeout = DEFAULT_SERVO_TIMEOUT,
	.servo.epsilon = DEFAULT_SERVO_POS_EPSILON,
	.servo.large_epsilon = DEFAULT_SERVO_POS_LARGE_EPSILON,
	.servo.max_torque = DEFAULT_SERVO_MAX_TORQUE_PERCENT,
	.servo.max_temperature = DEFAULT_SERVO_MAX_TEMPERATURE,
	.servo.speed = DEFAULT_SERVO_SPEED,
	.servo.min_angle = DEFAULT_SERVO_MIN_VALUE,
	.servo.max_angle = DEFAULT_SERVO_MAX_VALUE,
	.servo.init_pos = 660,	//					<------------ Position initiale
	.servo.torque_asser.Kp = DEFAULT_SERVO_KP,
	.servo.torque_asser.activated = FALSE,
	.servo.commands = big_elevator_front_left_commands,
	.servo.nb_commands = NB_ACT_COMMANDS(big_elevator_front_left_commands)
};

Actuator_command_t big_locker_front_right_commands[] = {
		{ACT_BIG_LOCKER_FRONT_RIGHT_IDLE, 			486},
		{ACT_BIG_LOCKER_FRONT_RIGHT_VERY_LOCK, 		526},
		{ACT_BIG_LOCKER_FRONT_RIGHT_LOCK, 			530}, //check_out
		{ACT_BIG_LOCKER_FRONT_RIGHT_UNLOCK, 		800},
		{ACT_BIG_LOCKER_FRONT_RIGHT_VERY_UNLOCK, 	995},
		{ACT_BIG_LOCKER_FRONT_RIGHT_CHECK_IN, 		490},
};
Actuator_servo_t big_locker_front_right = {
	.name = "big_locker_front_right",
	.kind = KIND_RX24_POS,
	.sid = ACT_BIG_LOCKER_FRONT_RIGHT,
	.queue_id = QUEUE_ACT_BIG_LOCKER_FRONT_RIGHT,
	.standard_init = TRUE,
	.servo.id = ACT_BIG_LOCKER_FRONT_RIGHT_RX24_ID,
	.servo.timeout = DEFAULT_SERVO_TIMEOUT,
	.servo.epsilon = DEFAULT_SERVO_POS_EPSILON,
	.servo.large_epsilon = DEFAULT_SERVO_POS_LARGE_EPSILON,
	.servo.max_torque = DEFAULT_SERVO_MAX_TORQUE_PERCENT,
	.servo.max_temperature = DEFAULT_SERVO_MAX_TEMPERATURE,
	.servo.speed = 80,
	.servo.min_angle = DEFAULT_SERVO_MIN_VALUE,
	.servo.max_angle = DEFAULT_SERVO_MAX_VALUE,
	.servo.init_pos = 486,	//					<------------ Position initiale
	.servo.torque_asser.Kp = DEFAULT_SERVO_KP,
	.servo.torque_asser.activated = FALSE,
	.servo.commands = big_locker_front_right_commands,
	.servo.nb_commands = NB_ACT_COMMANDS(big_locker_front_right_commands)
};

Actuator_command_t big_locker_front_middle_commands[] = {
		{ACT_BIG_LOCKER_FRONT_MIDDLE_IDLE, 		4},
		{ACT_BIG_LOCKER_FRONT_MIDDLE_LOCK, 		182},
		{ACT_BIG_LOCKER_FRONT_MIDDLE_UNLOCK, 	4},
};
Actuator_servo_t big_locker_front_middle = {
	.name = "big_locker_front_middle",
	.kind = KIND_RX24_POS,
	.sid = ACT_BIG_LOCKER_FRONT_MIDDLE,
	.queue_id = QUEUE_ACT_BIG_LOCKER_FRONT_MIDDLE,
	.standard_init = TRUE,
	.servo.id = ACT_BIG_LOCKER_FRONT_MIDDLE_RX24_ID,
	.servo.timeout = DEFAULT_SERVO_TIMEOUT,
	.servo.epsilon = DEFAULT_SERVO_POS_EPSILON,
	.servo.large_epsilon = DEFAULT_SERVO_POS_LARGE_EPSILON,
	.servo.max_torque = DEFAULT_SERVO_MAX_TORQUE_PERCENT,
	.servo.max_temperature = DEFAULT_SERVO_MAX_TEMPERATURE,
	.servo.speed = DEFAULT_SERVO_SPEED,
	.servo.min_angle = DEFAULT_SERVO_MIN_VALUE,
	.servo.max_angle = DEFAULT_SERVO_MAX_VALUE,
	.servo.init_pos = 4,	//					<------------ Position initiale
	.servo.torque_asser.Kp = DEFAULT_SERVO_KP,
	.servo.torque_asser.activated = FALSE,
	.servo.commands = big_locker_front_middle_commands,
	.servo.nb_commands = NB_ACT_COMMANDS(big_locker_front_middle_commands)
};

Actuator_command_t big_locker_front_left_commands[] = {
		{ACT_BIG_LOCKER_FRONT_LEFT_IDLE, 		687},
		{ACT_BIG_LOCKER_FRONT_LEFT_VERY_LOCK, 	646},
		{ACT_BIG_LOCKER_FRONT_LEFT_LOCK, 		642}, //check_out
		{ACT_BIG_LOCKER_FRONT_LEFT_UNLOCK, 		365},
		{ACT_BIG_LOCKER_FRONT_LEFT_VERY_UNLOCK, 225},
		{ACT_BIG_LOCKER_FRONT_LEFT_CHECK_IN, 	680},
};
Actuator_servo_t big_locker_front_left = {
	.name = "big_locker_front_left",
	.kind = KIND_RX24_POS,
	.sid = ACT_BIG_LOCKER_FRONT_LEFT,
	.queue_id = QUEUE_ACT_BIG_LOCKER_FRONT_LEFT,
	.standard_init = TRUE,
	.servo.id = ACT_BIG_LOCKER_FRONT_LEFT_RX24_ID,
	.servo.timeout = DEFAULT_SERVO_TIMEOUT,
	.servo.epsilon = DEFAULT_SERVO_POS_EPSILON,
	.servo.large_epsilon = DEFAULT_SERVO_POS_LARGE_EPSILON,
	.servo.max_torque = DEFAULT_SERVO_MAX_TORQUE_PERCENT,
	.servo.max_temperature = DEFAULT_SERVO_MAX_TEMPERATURE,
	.servo.speed = 80,
	.servo.min_angle = DEFAULT_SERVO_MIN_VALUE,
	.servo.max_angle = DEFAULT_SERVO_MAX_VALUE,
	.servo.init_pos = 687,	//					<------------ Position initiale
	.servo.torque_asser.Kp = DEFAULT_SERVO_KP,
	.servo.torque_asser.activated = FALSE,
	.servo.commands = big_locker_front_left_commands,
	.servo.nb_commands = NB_ACT_COMMANDS(big_locker_front_left_commands)
};

Actuator_command_t big_locker_back_commands[] = {
		{ACT_BIG_LOCKER_BACK_IDLE, 			400},
		{ACT_BIG_LOCKER_BACK_VERY_LOCK, 	512},
		{ACT_BIG_LOCKER_BACK_LOCK, 			624},
		{ACT_BIG_LOCKER_BACK_UNLOCK, 		624},
};
Actuator_servo_t big_locker_back = {
	.name = "big_locker_back",
	.kind = KIND_RX24_POS,
	.sid = ACT_BIG_LOCKER_BACK,
	.queue_id = QUEUE_ACT_BIG_LOCKER_BACK,
	.standard_init = TRUE,
	.servo.id = ACT_BIG_LOCKER_BACK_RX24_ID,
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
	.servo.commands = big_locker_back_commands,
	.servo.nb_commands = NB_ACT_COMMANDS(big_locker_back_commands)
};


Actuator_command_t big_slope_taker_commands[] = {
		{ACT_BIG_SLOPE_TAKER_BACK_IDLE, 	45},
		{ACT_BIG_SLOPE_TAKER_BACK_IN, 	45},
		{ACT_BIG_SLOPE_TAKER_BACK_OUT, 	360},
};
Actuator_servo_t big_slope_taker = {
	.name = "big_slope_taker",
	.kind = KIND_RX24_POS,
	.sid = ACT_BIG_SLOPE_TAKER_BACK,
	.queue_id = QUEUE_ACT_BIG_SLOPE_TAKER_BACK,
	.standard_init = TRUE,
	.servo.id = ACT_BIG_SLOPE_TAKER_BACK_RX24_ID,
	.servo.timeout = DEFAULT_SERVO_TIMEOUT,
	.servo.epsilon = DEFAULT_SERVO_POS_EPSILON,
	.servo.large_epsilon = DEFAULT_SERVO_POS_LARGE_EPSILON,
	.servo.max_torque = DEFAULT_SERVO_MAX_TORQUE_PERCENT,
	.servo.max_temperature = DEFAULT_SERVO_MAX_TEMPERATURE,
	.servo.speed = DEFAULT_SERVO_SPEED,
	.servo.min_angle = DEFAULT_SERVO_MIN_VALUE,
	.servo.max_angle = DEFAULT_SERVO_MAX_VALUE,
	.servo.init_pos = 45,	//					<------------ Position initiale
	.servo.torque_asser.Kp = DEFAULT_SERVO_KP,
	.servo.torque_asser.activated = FALSE,
	.servo.commands = big_slope_taker_commands,
	.servo.nb_commands = NB_ACT_COMMANDS(big_slope_taker_commands)
};

Actuator_command_t big_sorting_back_very_right_commands[] = {
		{ACT_BIG_SORTING_BACK_VERY_RIGHT_IDLE, 				400},
		{ACT_BIG_SORTING_BACK_VERY_RIGHT_BOT, 				512},
		{ACT_BIG_SORTING_BACK_VERY_RIGHT_DISPENSOR_LEVEL, 	624},
		{ACT_BIG_SORTING_BACK_VERY_RIGHT_TOP, 				624},
};
Actuator_servo_t big_sorting_back_very_right = {
	.name = "big_sorting_back_very_right",
	.kind = KIND_RX24_POS,
	.sid = ACT_BIG_SORTING_BACK_VERY_RIGHT,
	.queue_id = QUEUE_ACT_BIG_SORTING_BACK_VERY_RIGHT,
	.standard_init = TRUE,
	.servo.id = ACT_BIG_SORTING_BACK_VERY_RIGHT_RX24_ID,
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
	.servo.commands = big_sorting_back_very_right_commands,
	.servo.nb_commands = NB_ACT_COMMANDS(big_sorting_back_very_right_commands)
};

Actuator_command_t big_sorting_back_right_commands[] = {
		{ACT_BIG_SORTING_BACK_RIGHT_IDLE, 				288},
		{ACT_BIG_SORTING_BACK_RIGHT_ACCELERATOR_TOP, 	400},  // 440
		{ACT_BIG_SORTING_BACK_RIGHT_ACCELERATOR_BOT, 	570},  // 533
		{ACT_BIG_SORTING_BACK_RIGHT_DISPENSOR_LEVEL, 	683},
		{ACT_BIG_SORTING_BACK_RIGHT_GOLDENIUM_LEVEL, 	528},
		{ACT_BIG_SORTING_BACK_RIGHT_BLUEIUM_LEVEL, 		568},
		{ACT_BIG_SORTING_BACK_RIGHT_SCALE, 				440},
		{ACT_BIG_SORTING_BACK_RIGHT_TOP, 				288},
};
Actuator_servo_t big_sorting_back_right = {
	.name = "big_sorting_back_right",
	.kind = KIND_RX24_POS,
	.sid = ACT_BIG_SORTING_BACK_RIGHT,
	.queue_id = QUEUE_ACT_BIG_SORTING_BACK_RIGHT,
	.standard_init = TRUE,
	.servo.id = ACT_BIG_SORTING_BACK_RIGHT_RX24_ID,
	.servo.timeout = DEFAULT_SERVO_TIMEOUT,
	.servo.epsilon = DEFAULT_SERVO_POS_EPSILON,
	.servo.large_epsilon = DEFAULT_SERVO_POS_LARGE_EPSILON,
	.servo.max_torque = DEFAULT_SERVO_MAX_TORQUE_PERCENT,
	.servo.max_temperature = DEFAULT_SERVO_MAX_TEMPERATURE,
	.servo.speed = DEFAULT_SERVO_SPEED,
	.servo.min_angle = DEFAULT_SERVO_MIN_VALUE,
	.servo.max_angle = DEFAULT_SERVO_MAX_VALUE,
	.servo.init_pos = 288,	//					<------------ Position initiale
	.servo.torque_asser.Kp = DEFAULT_SERVO_KP,
	.servo.torque_asser.activated = FALSE,
	.servo.commands = big_sorting_back_right_commands,
	.servo.nb_commands = NB_ACT_COMMANDS(big_sorting_back_right_commands)
};

Actuator_command_t big_sorting_back_left_commands[] = {
		{ACT_BIG_SORTING_BACK_LEFT_IDLE, 				614},
		{ACT_BIG_SORTING_BACK_LEFT_ACCELERATOR_TOP, 	480},  // 440
		{ACT_BIG_SORTING_BACK_LEFT_ACCELERATOR_BOT, 	367},  // 377
		{ACT_BIG_SORTING_BACK_LEFT_DISPENSOR_LEVEL, 	222},
		{ACT_BIG_SORTING_BACK_LEFT_GOLDENIUM_LEVEL, 	386},
		{ACT_BIG_SORTING_BACK_LEFT_BLUEIUM_LEVEL, 		345},
		{ACT_BIG_SORTING_BACK_LEFT_SCALE, 				440},
		{ACT_BIG_SORTING_BACK_LEFT_TOP, 				614},
};
Actuator_servo_t big_sorting_back_left = {
	.name = "big_sorting_back_left",
	.kind = KIND_RX24_POS,
	.sid = ACT_BIG_SORTING_BACK_LEFT,
	.queue_id = QUEUE_ACT_BIG_SORTING_BACK_LEFT,
	.standard_init = TRUE,
	.servo.id = ACT_BIG_SORTING_BACK_LEFT_RX24_ID,
	.servo.timeout = DEFAULT_SERVO_TIMEOUT,
	.servo.epsilon = DEFAULT_SERVO_POS_EPSILON,
	.servo.large_epsilon = DEFAULT_SERVO_POS_LARGE_EPSILON,
	.servo.max_torque = DEFAULT_SERVO_MAX_TORQUE_PERCENT,
	.servo.max_temperature = DEFAULT_SERVO_MAX_TEMPERATURE,
	.servo.speed = DEFAULT_SERVO_SPEED,
	.servo.min_angle = DEFAULT_SERVO_MIN_VALUE,
	.servo.max_angle = DEFAULT_SERVO_MAX_VALUE,
	.servo.init_pos = 614,	//					<------------ Position initiale
	.servo.torque_asser.Kp = DEFAULT_SERVO_KP,
	.servo.torque_asser.activated = FALSE,
	.servo.commands = big_sorting_back_left_commands,
	.servo.nb_commands = NB_ACT_COMMANDS(big_sorting_back_left_commands)
};

Actuator_command_t big_sorting_back_very_left_commands[] = {
		{ACT_BIG_SORTING_BACK_VERY_LEFT_IDLE, 				400},
		{ACT_BIG_SORTING_BACK_VERY_LEFT_BOT, 				512},
		{ACT_BIG_SORTING_BACK_VERY_LEFT_DISPENSOR_LEVEL, 	624},
		{ACT_BIG_SORTING_BACK_VERY_LEFT_TOP, 				624},
};
Actuator_servo_t big_sorting_back_very_left = {
	.name = "big_sorting_back_very_left",
	.kind = KIND_RX24_POS,
	.sid = ACT_BIG_SORTING_BACK_VERY_LEFT,
	.queue_id = QUEUE_ACT_BIG_SORTING_BACK_VERY_LEFT,
	.standard_init = TRUE,
	.servo.id = ACT_BIG_SORTING_BACK_VERY_LEFT_RX24_ID,
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
	.servo.commands = big_sorting_back_very_left_commands,
	.servo.nb_commands = NB_ACT_COMMANDS(big_sorting_back_very_left_commands)
};


