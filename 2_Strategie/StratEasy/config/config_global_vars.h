/*
 *  Club Robot ESEO 2008 - 2014
 *
 *  $Id: config_global_vars.h 6859 2018-05-22 15:14:26Z spoiraud $
 *
 *  Package : Carte Strategie
 *  Description : Variables globales définies specifiquement pour
					le code de la carte l'executant.
 *  Auteur : Jacen
 */

#ifndef CONFIG_GLOBAL_VARS_H
	#define CONFIG_GLOBAL_VARS_H

	#include "../QS/QS_types.h"
	#include "config_global_vars_types.h"

	typedef struct{
		volatile bool_e match_started;
		volatile bool_e match_over;
		volatile bool_e match_suspended;
		volatile bool_e virtual_mode;
		volatile bool_e calibrated;
	}flag_list_t;

	typedef struct{
		volatile flag_list_t flags;				// Les flags

		volatile color_e color;

		volatile time32_t match_time;			//temps de match en ms.
		volatile time32_t absolute_time;		//temps depuis le lancement de la STM32
		volatile time32_t calibrate_time_x;		// temps depuis la dernière calibration en X
		volatile time32_t calibrate_time_y;		// temps depuis la dernière calibration en Y
		volatile time32_t calibrate_time_angle;	// temps depuis la dernière calibration en angle

		volatile position_t pos;
	}global_data_storage_t;

	extern global_data_storage_t global;

#endif /* ndef CONFIG_GLOBAL_VARS_H */
