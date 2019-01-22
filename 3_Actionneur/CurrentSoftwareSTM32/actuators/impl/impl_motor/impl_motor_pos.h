/**
 *	Club Robot ESEO 2017 - 2018
 *
 * @file impl_motor_pos.c
 * @brief Implémentation des fonctions pour un actionneur de type KIND_MOTOR_POS.
 * @author Valentin
 */

#ifndef IMPL_MOTOR_POS_H
	#define	IMPL_MOTOR_POS_H

	#include "../QS/QS_all.h"
	#include "../QS/QS_CANmsgList.h"
	#include "../queue.h"
	#include "../../actuator_motor.h"

	/**
	 * @brief Fonction permettant d'initialiser le moteur.
	 * Elle initialise la pwm et l'asservissement du moteur.
	 * Cette fonction est appelée au lancement de la carte via ActManager.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
	 */
	void IMPL_MOTOR_POS_init(void* actuator_gen_ptr);

	/**
	 * Fonction appellée pour initialiser le moteur dès l'arrivée de l'alimentation via ActManager.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
	 * @return le status d'initialisation.
	 */
	error_e IMPL_MOTOR_POS_init_pos(void *actuator_gen_ptr);

	/**
	 * @brief Fonction qui réinitialise la config du servo.
	 * Fonction appellée si la carte IHM a détecté une grosse chutte de la tension d'alimentation des actionneurs.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
	 */
	//void IMPL_MOTOR_POS_reset_config(void* actuator_gen_ptr);

	/**
	 * @brief Fonction pour stopper l'actionneur.
	 * Fonction appellée à la fin du match via ActManager.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
	 */
	void IMPL_MOTOR_POS_stop(void *actuator_gen_ptr);

	/**
	 * @brief Fonction de dispatch des messages CAN pour les ordres actionneurs.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
	 * @param msg le message CAN reçu.
	 * @return TRUE si le message CAN a ete traite et FALSE sinon.
	 */
	bool_e IMPL_MOTOR_POS_CAN_process_msg(void *actuator_gen_ptr, CAN_msg_t* msg);

	/**
	 * @brief Fonction appellée par la queue pendant tout le temps d'execution de la commande en cours.
	 * Le booleen init est a TRUE au premier lancement de la commande.
	 * @param queueId l'id de la queue.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
	 * @param init TRUE au premier appel pour initiliser la commande et FALSE ensuite.
	 */
	void IMPL_MOTOR_POS_run_command(queue_id_t queueId, void* actuator_gen_ptr, bool_e init);

	/**
	 * @brief Fonction permettant de modifier la config du servo comme la vitesse ou le couple.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
	 * @param msg le message CAN reçu.
	 */
	void IMPL_MOTOR_POS_set_config(void *actuator_gen_ptr, CAN_msg_t* msg);

	/**
	 * @brief Fonction permettant d'obtenir des infos sur la config du moteur comme la position.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
	 * @param incoming_msg le message CAN reçu.
	 */
	void IMPL_MOTOR_POS_get_config(void *actuator_gen_ptr, CAN_msg_t *incoming_msg);

	/**
	 * @brief Fonction d'interruption d'un moteur.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
	 */
	void IMPL_MOTOR_POS_process_it(void* actuator_gen_ptr);

	/**
	 * @brief Fonction de tache de fond d'un moteur.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
	 */
	void IMPL_MOTOR_POS_process_main(void* actuator_gen_ptr);

#endif	/* IMPL_MOTOR_POS_H */

