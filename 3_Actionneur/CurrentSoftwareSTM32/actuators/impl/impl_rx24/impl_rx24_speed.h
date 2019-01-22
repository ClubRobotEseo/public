/**
 *	Club Robot ESEO 2017 - 2018
 *
 * @file impl_rx24_speed.h
 * @brief Implémentation des fonctions pour un actionneur de type KIND_RX24_SPEED.
 * @author Valentin
 */

#ifndef IMPL_RX24_SPEED_H
	#define	IMPL_RX24_SPEED_H

	#include "../QS/QS_all.h"
	#include "../QS/QS_CANmsgList.h"
	#include "../queue.h"

	/**
	 * @brief Fonction permettant d'initialiser le servo.
	 * Elle initialise le bus de communication ainsi que les différents registres du servo.
	 * Cette fonction est appelée au lancement de la carte via ActManager.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
	 */
	void IMPL_RX24_SPEED_init(void* actuator_gen_ptr);

	/**
	 * Fonction appellée pour initialiser en position le servo dès l'arrivée de l'alimentation via ActManager.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
	 * @return le status d'initialisation
	 */
	error_e IMPL_RX24_SPEED_init_pos(void *actuator_gen_ptr);

	/**
	 * @brief Fonction qui réinitialise la config du servo.
	 * Fonction appellée si la carte IHM a détecté une grosse chutte de la tension d'alimentation des actionneurs.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
	 */
	void IMPL_RX24_SPEED_reset_config(void* actuator_gen_ptr);

	/**
	 * @brief Fonction pour stopper l'asservissement du servo.
	 * Fonction appellée à la fin du match via ActManager.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
	 */
	void IMPL_RX24_SPEED_stop(void *actuator_gen_ptr);

	/**
	 * @brief Fonction de dispatch des messages CAN pour les ordres actionneurs.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
	 * @param msg le message CAN reçu.
	 * @return TRUE si le message CAN a ete traite et FALSE sinon.
	 */
	bool_e IMPL_RX24_SPEED_CAN_process_msg(void *actuator_gen_ptr, CAN_msg_t* msg);

	/**
	 * @brief Fonction appellée par la queue pendant tout le temps d'execution de la commande en cours.
	 * Le booleen init est a TRUE au premier lancement de la commande.
	 * @param queueId l'id de la queue.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
	 * @param init TRUE au premier appel pour initiliser la commande et FALSE ensuite.
	 */
	void IMPL_RX24_SPEED_run_command(queue_id_t queueId, void* actuator_gen_ptr, bool_e init);

	/**
	 * @brief Fonction permettant d'obtenir des infos sur la config du servo comme la vitesse ou le couple.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
	 * @param incoming_msg le message CAN reçu.
	 */
	void IMPL_RX24_SPEED_get_config(void *actuator_gen_ptr, CAN_msg_t *incoming_msg);

	/**
	 * @brief Fonction permettant de modifier la config du servo comme la vitesse ou le couple.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
	 * @param msg le message CAN reçu.
	 */
	void IMPL_RX24_SPEED_set_config(void *actuator_gen_ptr, CAN_msg_t* msg);

	/**
	 * @brief Fonction permettant d'activer un warner sur une position du servo.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
	 * @param msg le message CAN reçu.
	 */
	void IMPL_RX24_SPEED_set_warner(void* actuator_gen_ptr, CAN_msg_t *msg);

#endif	/* IMPL_RX24_SPEED_H */

