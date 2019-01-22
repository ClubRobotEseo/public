/**
 *	Club Robot ESEO 2017 - 2018
 *
 * @file actuator_motor.h
 * @brief Definition d'un actionneur moteur.
 * @author Valentin
 */

#ifndef ACTUATORS_ACTUATOR_MOTOR_H_
#define ACTUATORS_ACTUATOR_MOTOR_H_

#include "actuator.h"
#include "../QS/QS_actuator/QS_DCMotor2.h"
#include "../QS/QS_actuator/QS_DCMotorSpeed.h"
#include "../QS/QS_sensor/QS_rpm_sensor.h"
#include "../QS/QS_lowLayer/QS_pwm.h"

/**
 * Definition de la structure de données pour la simulation d'un moteur asservi en position.
 */
typedef struct {
	time32_t exec_finished_time;
	Sint16 current_pos;
	Uint16 theoretical_speed;
} Actuator_motor_pos_simu_t;

/**
 * Definition de la structure de données pour la simulation d'un moteur asservi en vitesse.
 */
typedef struct {
	time32_t exec_finished_time;
	time32_t theoretical_exec_time;
} Actuator_motor_speed_simu_t;

/**
 * Definition de la structure de données d'un capteur.
 */
typedef struct {
	RPM_SENSOR_id_t id;
	RPM_SENSOR_port_e gpio;
	RPM_SENSOR_pin_e pin;
	RPM_SENSOR_edge_e edge;
	Uint8 ticks_per_rev;
} Actuator_sensor_data_t;

/**
 * Definition de l'énum des états pour la machine à états d'asservissement en position.
 */
typedef enum{
	INIT,
	WAIT_FDC,
	INIT_POS,
	WAIT_POS,
	RUN,
	DEACTIVATE
}Actuator_motor_pos_state_e;

/**
 * Definition de l'énum des différents mode à l'initialisation du moteur.
 */
typedef enum{
	MOTOR_INIT_NORMAL_MODE,
	MOTOR_INIT_SPEED_MODE
}Actuator_init_motor_mode_e;

/**
 * Definition de la structure de données pour la machine à état d'asservissement en position du moteur.
 */
typedef struct {
	Actuator_motor_pos_state_e state;
	Actuator_motor_pos_state_e last_state;
	time32_t begin_detection;
	time32_t begin_wait;
	bool_e last_alim;
	bool_e activated; // Permet de lancer l'exécution de la machine à état en IT.
	time32_t lastSpeedMeasure;
	Sint32 lastSpeed;
} Actuator_motor_pos_state_machine_t;

/**
 * Définition du type pointeur de fonction de lecture d'un codeur.
 */
typedef Sint16(*encoder_get_fun_t)(void);
typedef void(*encoder_set_fun_t)(Sint16);

/**
 * Definition de la structure de données pour un codeur.
 */
typedef struct {
	bool_e ready;
	Sint16 coeff;
	Sint16 count;
	encoder_get_fun_t get;
	encoder_set_fun_t set;
} Actuator_encoder_t;


/**
 * Definition de la structure de données pour un capteur de fin de course.
 */
typedef struct {
	GPIO_TypeDef* way_latch;
	Uint16 way_bit_number;
} Actuator_sensor_fdc_t;



/**
 * Definition d'un actionneur de type moteur avec asservissement en position.
 */
typedef struct {
	ABSTRACT_Actuator_t;
	DCMotor_config_t motor;
	bool_e ready;
	bool_e init_first_order;
	Sint16 init_pos;
	Sint8 init_duty_cycle;
	Actuator_init_motor_mode_e init_mode;
	Uint32 init_speed;							// Vitesse à l'initialisation en pusle / sec
	float init_speed_Kv;
	float init_speed_Kp;
	Actuator_motor_pos_state_machine_t sm;
	Actuator_sensor_fdc_t sensor_fdc;
	Actuator_encoder_t encoder;
	Actuator_command_t * commands;
	Uint8 nb_commands;
	Actuator_motor_pos_simu_t simu;
}Actuator_motor_pos_t;

/**
 * Definition d'un actionneur de type moteur avec asservissement en vitesse.
 */
typedef struct {
	ABSTRACT_Actuator_t;
	Actuator_sensor_data_t sensor;
	DC_MOTOR_SPEED_config_t motor;
	Actuator_motor_speed_simu_t simu;
}Actuator_motor_speed_t;


#endif /* ACTUATORS_ACTUATOR_MOTOR_H_ */
