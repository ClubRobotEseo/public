/*
 *  Club Robot ESEO 2008 - 2014
 *
 *  $Id: config_global_vars_types.h 6407 2018-04-17 12:26:20Z robot_pc_droit $
 *
 *  Package : Carte Strategie
 *  Description : Définition de types pour les variables globales
				définies specifiquement pour ce code.
 *  Auteur : Jacen
 */

#ifndef CONFIG_GLOBAL_VARS_TYPES_H
	#define CONFIG_GLOBAL_VARS_TYPES_H

	#include "../QS/QS_types.h"
	#include "../QS/QS_maths.h"
	#include "../QS/QS_CANmsgList.h"

	typedef struct
	{
		int32_t x;		//mm
		int32_t y;		//mm
		int32_t teta;	//rad4096
	}position_t;




	 typedef enum
	 {
		 ACTUATOR_ARM_LEFT,
		 ACTUATOR_ARM_RIGHT,
		 ACTUATOR_PUMP_LEFT,
		 ACTUATOR_PUMP_RIGHT,
		 ACTUATOR_SOLENOID_VALVE_LEFT,
		 ACTUATOR_SOLENOID_VALVE_RIGHT,
		 ACTUATOR_NB
	 }actuator_e;


#endif /* ndef CONFIG_GLOBAL_VARS_TYPES_H */
