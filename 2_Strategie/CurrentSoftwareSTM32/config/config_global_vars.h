/*
 *  Club Robot ESEO 2008 - 2014
 *
 *  $Id: config_global_vars.h 6976 2018-08-24 13:06:17Z spoiraud $
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
		volatile bool_e initial_position_received;

		volatile bool_e ask_start;
		volatile bool_e ask_suspend_match;

		volatile bool_e match_started;
		volatile bool_e match_over;
		volatile bool_e match_suspended;

		volatile bool_e match_paused;

		volatile bool_e alim;
		volatile bool_e aru;

		volatile bool_e foes_updated_for_lcd;

		volatile bool_e go_to_home;

		volatile bool_e virtual_mode;

	}flag_list_t;

	typedef struct{
		volatile flag_list_t flags;				// Les flags

		volatile color_e color;

		volatile Uint16 alim_value;				// en mV

		volatile foe_t foe[MAX_NB_FOES];		//l'ensemble des adversaires vus sur le terrain
		//Attention, un même adversaire peut être mis à jour sur plusieurs cases différents du tableau de foe !

		volatile GEOMETRY_point_t destination;
		volatile communication_t com;

		volatile time32_t match_time;			//temps de match en ms.
		volatile time32_t absolute_time;		//temps depuis le lancement de la STM32
		volatile time32_t calibrate_time_x;		// temps depuis la dernière calibration en X
		volatile time32_t calibrate_time_y;		// temps depuis la dernière calibration en Y
		volatile time32_t calibrate_time_angle;	// temps depuis la dernière calibration en angle

		volatile debug_t debug;
		volatile prop_env_t prop;
		volatile position_t pos;

		volatile time32_t friend_position_lifetime;	//Durée de vie restante pour les données de position de notre autre robot.
		volatile GEOMETRY_point_t friend_pos;
	}global_data_storage_t;

	extern global_data_storage_t global;

#endif /* ndef CONFIG_GLOBAL_VARS_H */
