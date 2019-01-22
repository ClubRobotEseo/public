/*
 * actuators.h
 *
 *  Created on: 14 juin 2018
 *      Author: Nirgal
 */

#ifndef ACTUATORS_H_
#define ACTUATORS_H_
#include "QS/QS_all.h"

/**
	 * @brief Fonction d'initialisation de ActManager. Initialise les actionneurs en position.
	 */
	void ACTUATOR_init();

	/**
	 * @brief Fonction qui réinitialise tous les actionneurs en position.
	 */
	void ACTUATOR_reset_act();

	/**
	 * @brief Fonction qui réinitialise tous les actionneurs en position.
	 * (tourne en tâche de fond)
	 */
	void ACTUATOR_run_reset_act();

	/**
	 * @brief Fonction qui stoppe tous les actionneurs (asservissement et puissance).
	 */
	void ACTUATOR_stop();

	/**
	 * @brief Fonction permettant de réinitialiser la config de tous les actionneurs.
	 */
	void ACTUATOR_reset_config();

	/**
	 * @brief Fonction qui permet de récupérer la valeur d'une config sur un actionneur.
	 * @param id l'id de l'actionneur concerné.
	 * @return la valeur de la config demandée.
	 */
	Uint16 ACTUATOR_get_config(actuator_e id, act_config_e config);

	/**
	 * @brief Fonction qui permet de setter une config sur un actionneur.
	 * @param id l'id de l'actionneur concerné.
	 * @param config la configuration à changer.
	 * @param value la nouvelle valeur de la configuration (Pour les servos: Si value est égale à 0, cela reprend la valeur par défaut).
	 */
	void ACTUATOR_set_config(actuator_e id, act_config_e config, Uint16 value);

	/**
	 * @brief Envoie un ordre à un actionneur.
	 * @param id l'id de lactionneur concerné.
	 * @param param un paramètre.
	 * @param order l'ordre demandé.
	 */
	void ACTUATOR_go(actuator_e id,  ACT_order_e order, Sint32 param);

	/**
	 * @brief Permet de vérifier si un actionneur est arrivé.
	 * @param id l'id de l'actionneur concerné.
	 * @return TRUE si l'actionneur est arrivé et FALSE sinon.
	 */
	bool_e ACTUATOR_is_arrived(actuator_e id);

	/**
	 * @brief Permet de vérifier si un actionneur est en timeout.
	 * @param id l'id de l'actionneur concerné.
	 * @return TRUE si l'actionneur est en timeout et FALSE sinon.
	 */
	bool_e ACTUATOR_is_in_timeout(actuator_e id);

	/**
	 * @brief Fonction de traitement des messages CAN.
	 * @param msg le message CAN à traiter.
	 */
	void ACTUATOR_process_msg(CAN_msg_t * msg);

	/**
	 * @brief Fonction exécuter en tâche de fond.
	 */
	void ACTUATOR_process_main(void);

#endif /* ACTUATORS_H_ */
