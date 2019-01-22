/*
 *	Club Robot ESEO 2018
 *
 *	Fichier : environment.h
 *	Package : Carte LED
 *	Description : Fonctions de traitement des données de l'environnement
 *  Auteur : Valentin
 *  Version 20180407
 */

#ifndef ENVIRONMENT_H_
#define ENVIRONMENT_H_

	#include "QS/QS_all.h"
	#include "QS/QS_CANmsgList.h"

	typedef struct
	{
		Uint8 foe_ball_placed;
		Uint8 our_ball_placed;
		Uint8 water_getted;
		Uint8 bee_placed;
		Uint8 bursted_balloon;
		Uint8 first_stage;
		Uint8 second_stage;
		Uint8 third_stage;
		Uint8 fourth_stage;
		Uint8 fifth_stage;
		Uint8 well_done_building;
		Uint8 domotic_placed;
		Uint8 switch_activated;
	} ScoreEventCounter;

	void ENV_process_main();

	void ENV_process_color_combination(CAN_msg_t * msg);

	void ENV_process_score_estimation(CAN_msg_t * msg);

	/**
	 * Getter de la combinaison de couleurs.
	 * @param combination_ptr le pointeur ou stocker la combinaison de couleurs.
	 * @return TRUE si la combinaison est disponible et FALSE sinon.
	 */
	bool_e ENV_get_color_combination(CUBE_color_e ** combination_ptr);

	Uint16 ENV_get_score_estimation(robot_id_e robot_id);

	Uint16 ENV_get_total_score_estimation();

	ScoreEventCounter ENV_get_event_counter(robot_id_e robot_id);


#endif /* ENVIRONMENT_H_ */
