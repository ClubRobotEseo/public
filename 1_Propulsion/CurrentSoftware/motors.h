 /*
 *  Club Robot ESEO 2006 - 2012
 *
 *  Fichier : motors.h
 *  Package : Propulsion
 *  Description : Gestion des moteurs.
 *  Auteur : Nirgal 2009
 *  Version 201203
 */
#ifndef _MOTORS_H
	#define _MOTORS_H

	#include "QS/QS_all.h"

		#define CLIPPING_DUTY 99
		#define MOTOR_BACKWARD Bit_SET
		#define MOTOR_FORWARD Bit_RESET

	void MOTORS_init(void);

	void MOTORS_enable(bool_e enable);

	void MOTORS_update(Sint16 duty_left, Sint16 duty_right);

	void MOTORS_reset(void);

	void MOTORS_set_border_mode(bool_e border_mode_wanted);



#endif
