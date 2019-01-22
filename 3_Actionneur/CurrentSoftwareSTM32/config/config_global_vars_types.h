/*
 *  Club Robot ESEO 2008 - 2014
 *
 *  $Id: config_global_vars_types.h 7236 2019-01-14 16:19:41Z gdupoiron $
 *
 *  Package : Carte Actionneur
 *  Description : D�finition de types pour les variables globales
				d�finies specifiquement pour ce code.
 *  Auteur : Jacen
 */


#ifndef CONFIG_GLOBAL_VARS_TYPES_H
	#define CONFIG_GLOBAL_VARS_TYPES_H


	#include "config_global.h"

	typedef struct{
		volatile Sint16 angle;
		volatile Sint16 x;
		volatile Sint16 y;
		volatile bool_e updated;
	}position_t;

	typedef enum {
		// Queues communes aux deux robots
		QUEUE_RESET_ACT,

		// Queues pour BIG_ROBOT
		//QUEUE_ACT_EXEMPLE,
		QUEUE_ACT_BIG_ELEVATOR_FRONT_RIGHT,
		QUEUE_ACT_BIG_ELEVATOR_FRONT_MIDDLE,
		QUEUE_ACT_BIG_ELEVATOR_FRONT_LEFT,
		QUEUE_ACT_BIG_LOCKER_FRONT_RIGHT,
		QUEUE_ACT_BIG_LOCKER_FRONT_MIDDLE,
		QUEUE_ACT_BIG_LOCKER_FRONT_LEFT,
		QUEUE_ACT_BIG_LOCKER_BACK,
		QUEUE_ACT_BIG_SLOPE_TAKER_BACK,
		QUEUE_ACT_BIG_SORTING_BACK_VERY_RIGHT,
		QUEUE_ACT_BIG_SORTING_BACK_RIGHT,
		QUEUE_ACT_BIG_SORTING_BACK_LEFT,
		QUEUE_ACT_BIG_SORTING_BACK_VERY_LEFT,

		NB_QUEUES_BIG,


		MARKER_ACTUATORS_BIG,


		// Queues pour SMALL_ROBOT
		//QUEUE_ACT_SMALL_EXEMPLE = 0,
		QUEUE_ACT_SMALL_LOCKER_BACK,
		QUEUE_ACT_SMALL_LOCKER_FRONT_LEFT,
		QUEUE_ACT_SMALL_LOCKER_FRONT_RIGHT,
		QUEUE_ACT_SMALL_SORTING_BACK_RIGHT,
		QUEUE_ACT_SMALL_SORTING_BACK_MIDDLE,
		QUEUE_ACT_SMALL_SORTING_BACK_LEFT,
		QUEUE_ACT_SMALL_ELEVATOR_FRONT_LEFT,
		QUEUE_ACT_SMALL_ELEVATOR_FRONT_RIGHT,

		NB_QUEUES_SMALL,



		MARKER_ACTUATORS_SMALL
	} QUEUE_act_e;

	// Nombre d'actionneurs sur BIG_ROBOT
	#define NB_ACTUATORS_BIG  	(MARKER_ACTUATORS_BIG - 2)

	// Nombre d'actionneurs sur SMALL_ROBOT
	#define NB_ACTUATORS_SMALL 	(MARKER_ACTUATORS_SMALL - MARKER_ACTUATORS_BIG - 2)

	// Macro correspondant au nombre maximal d'actionneurs (= MAX(NB_ACTUATORS_BIG, NB_ACTUATORS_SMALL)
	#define NB_ACTUATORS  MAX(NB_ACTUATORS_BIG, NB_ACTUATORS_SMALL)

#endif /* ndef CONFIG_GLOBAL_VARS_TYPES_H */