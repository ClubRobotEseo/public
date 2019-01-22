/**
 *	Club Robot ESEO 2017 - 2018
 *
 * @file actuator_pwm.h
 * @brief Definition d'un actionneur PWM.
 * @author Valentin
 */

#ifndef ACTUATORS_ACTUATOR_PWM_H_
#define ACTUATORS_ACTUATOR_PWM_H_

#include "actuator.h"
#include "../QS/QS_lowLayer/QS_pwm.h"

/**
 * Definition de la structure de données PWM.
 */
typedef struct {
	PWM_TIM_id_e tim_id;
	PWM_CH_id_e channel_id;
	Uint16 freq;
	bool_e way_used;
	GPIO_TypeDef* gpio_port_way;
	Uint16 gpio_pin_way;
} Actuator_pwm_data_t;

/**
 * Definition d'un actionneur PWM.
 */
typedef struct {
	ABSTRACT_Actuator_t;
	Actuator_pwm_data_t pwm;
} Actuator_pwm_t;


/**
 * Valeurs maximales et minimales de PPM à appliquer
 */
#define MIN_PPM_DUTY_CYCLE	(5)
#define MAX_PPM_DUTY_CYCLE	(10)


/**
 * Definition de la structure de données PPM.
 */
typedef struct {
	PWM_TIM_id_e tim_id;
	PWM_CH_id_e channel_id;
	Uint16 freq;
	Sint16 init_pos;
	Actuator_command_t * commands;
	Uint8 nb_commands;
} Actuator_ppm_data_t;

/**
 * Definition d'un actionneur PPM.
 */
typedef struct {
	ABSTRACT_Actuator_t;
	Actuator_ppm_data_t ppm;
} Actuator_ppm_t;

#endif /* ACTUATORS_ACTUATOR_PWM_H_ */
