/*  Club Robot ESEO 2012 - 2013
 *
 *	Fichier : ActManager.h
 *	Package : Carte actionneur
 *	Description : Gestion des actionneurs
 *  Auteur : Alexis, remake Arnaud, reremake Valentin
 *  Version 20130227
 */

#ifndef ACT_MANAGER_H
	#define	ACT_MANAGER_H

	#include "QS/QS_all.h"
	#include "QS/QS_CANmsgList.h"
	#include "actuators/actuator_functions.h"
	#include "actuators/actuator_servo.h"
	#include "queue.h"

	/**
	 * Definition de la structure ACTMGR_actuator_t qui comprend :
	 * - l'id de la queue
	 * - l'instance de l'actionneur
	 *
	 * Cette structure permet de faire la mapping entre une queue et son instance d'actionneur.
	 */
	typedef struct {
		queue_id_t queue_id;
		void* actuator;
		ACT_functions_t* functions;
	}ACTMGR_actuator_t;

	/**
	 * @brief Fonction d'initialisation de ActManager. Initialise les actionneurs en position.
	 */
	void ACTMGR_init();

	/**
	 * @brief Fonction qui réinitialise tous les actionneurs en position.
	 */
	void ACTMGR_reset_act();

	/**
	 * @brief Fonction pour initialiser les actionneurs particulier
	 * A compléter en fonction des années
	 */
	error_e ACTMGR_special_init();

	/**
	 * @brief Fonction qui stoppe tous les actionneurs (asservissement et puissance).
	 */
	void ACTMGR_stop();

	/**
	 * @brief Fonction permettant de réinitialiser la config de tous les actionneurs.
	 */
	void ACTMGR_reset_config();

	/**
	 * @brief Fonction qui dispatch les messages CAN recus pour les ordres actionneurs.
	 * @param msg le message CAN a traiter.
	 * @return TRUE si le message a ete gere et FALSE sinon.
	 */
	bool_e ACTMGR_process_msg(CAN_msg_t* msg);

	/**
	 * @brief Fonction qui execute le code d'IT de tous les actionneurs.
	 * Cette fonction est appelé dans clock.c
	 */
	void ACTMGR_process_it();

	/**
	 * @brief Fonction qui execute le code de tâche de fond de tous les actionneurs.
	 * Cette fonction est appelé dans main.c
	 */
	void ACTMGR_process_main();

	/**
	 * @brief Fonction qui dispatch les messages CAN recus pour le get config.
	 */
	void ACTMGR_get_config(CAN_msg_t* msg);

	/**
	 * @brief Fonction qui dispatch les messages CAN recus pour le set config.
	 */
	void ACTMGR_set_config(CAN_msg_t* msg);

	/**
	 * @brief Fonction qui dispatch les messages CAN recus pour les warners.
	 */
	void ACTMGR_set_warner(CAN_msg_t* msg);

	/**
	 * @brief Fonction permettant de chercher l'index d'un actionneur dans le tableau actuators[] a partir de son sid.
	 * @param sid le sid de l'actionneur a chercher.
	 * @return l'index de l'actionneur dans le tableau actuators[] ou -1 s'il n'a pas ete trouve.
	 */
	Sint16 ACTMGR_get_actuator_index_by_sid(Uint11 sid);

	/**
	 * @brief Fonction de configuration des AX12.
	 * @param servo l'instance du servo concerne (ici juste Actuator_servo_data_t suffit).
	 * @param msg le message CAN contenant la nouvelle configuration.
	 */
	void ACTMGR_config_AX12(Actuator_servo_data_t * servo, CAN_msg_t* msg);

	/**
	 * @brief Fonction de configuration des AX12.
	 * @param servo l'instance du servo concerne (ici juste Actuator_servo_data_t suffit).
	 * @param msg le message CAN contenant la nouvelle configuration.
	 */
	void ACTMGR_config_RX24(Actuator_servo_data_t * servo, CAN_msg_t* msg);

	/**
	 * @brief Permet de savoir si la première séquence d'initialisation des actionneurs a été effectuée.
	 * @return TRUE si la première séquence d'initialisation est terminée ou FALSE sinon.
	 */
	bool_e ACTMRG_is_first_init_pos_done();

#endif	/* ACT_MANAGER_H */
