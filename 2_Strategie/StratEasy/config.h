/*
 * config.h
 *
 *  Created on: 25 juin 2018
 *      Author: Nirgal
 */

#ifndef CONFIG_H_
#define CONFIG_H_

//Coefs à régler :
#define PI							((double)(3.1415654))
#define DISTANCE_BETWEEN_WHEELS		((double)(178.0))			//mm
#define WHEEL_DIAMETER_MM			((double)(72.4000))				//[mm]  (73mm - écrasement d'1mm de chaque côté)

#define NB_MICROSTEP_BY_WHEELTURN	(200*32)
#define MM_BY_MICROSTEP				((double)(WHEEL_DIAMETER_MM*PI/NB_MICROSTEP_BY_WHEELTURN))	//environ 0.035 mm par microstep

#define NB_MICROSTEP_BY_MM			(double)(NB_MICROSTEP_BY_WHEELTURN/(WHEEL_DIAMETER_MM*PI))	//nombre de pulses par mm  (environ 28,3)
#define NB_MICROSTEP_BY_ROBOT_TURN	((double)(DISTANCE_BETWEEN_WHEELS*NB_MICROSTEP_BY_WHEELTURN/WHEEL_DIAMETER_MM)) 	//90mm*2*pi/nb_microstep_by_mm = environ 16000 !
#define NB_MICROSTEP_BY_RAD4096		((double)(NB_MICROSTEP_BY_ROBOT_TURN/2/PI/4096))

#define ACCELERATION				1000.0		//[mm/s²]
#define BRAKING_ACCELERATION		(ACCELERATION*2)
#define SPEED_MAX					1024		//[mm/s]
#define INITIAL_SPEED				4		//mm/s		//un classeur excel permet ce dimensionnement assez finot. But : courbe régulière.

#define SPEED_ROTATION_DEFAULT		12868			//rad4096/s
#define	SPEED_TRANSLATION_DEFAULT	SPEED_MAX		//mm/s

#endif /* CONFIG_H_ */
