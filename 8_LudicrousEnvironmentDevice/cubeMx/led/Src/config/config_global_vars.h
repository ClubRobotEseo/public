/*
 *  Club Robot ESEO 2008 - 2014
 *
 *  $Id: config_global_vars.h 4988 2017-01-04 20:08:04Z aguilmet $
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
		volatile bool_e communication_available;
		volatile bool_e score_updated;
		volatile bool_e color_combination_updated;
	}flag_list_t;

	typedef struct{
		volatile flag_list_t flags;				// Les flags

		volatile time32_t match_time;			//temps de match en ms.
		volatile time32_t absolute_time;		//temps depuis le lancement de la STM32

	}global_data_storage_t;

	extern global_data_storage_t global;

#endif /* ndef CONFIG_GLOBAL_VARS_H */
