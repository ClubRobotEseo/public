/**
 *	Club Robot ESEO 2017 - 2018
 *
 * @file impl_ax12_pos_double.h
 * @brief Implémentation des fonctions pour un actionneur de type KIND_AX12_POS_DOUBLE.
 * @author Valentin
 */

#ifndef IMPL_AX12_POS_DOUBLE_H_
#define IMPL_AX12_POS_DOUBLE_H_


	#include "../QS/QS_all.h"
	#include "../QS/QS_CANmsgList.h"
	#include "../queue.h"
	#include "../../actuator_servo.h"

	/**
	 * @brief Fonction permettant d'initialiser le servo.
	 * Elle initialise le bus de communication ainsi que les différents registres du servo.
	 * Cette fonction est appelée au lancement de la carte via ActManager.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
	 */
	void IMPL_AX12_POS_DOUBLE_init(void* actuator_gen_ptr);

	/**
	 * Fonction appellée pour initialiser en position le servo dès l'arrivée de l'alimentation via ActManager.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
	 * @return le status d'initialisation.
	 */
	error_e IMPL_AX12_POS_DOUBLE_init_pos(void *actuator_gen_ptr);

	/**
	 * @brief Fonction qui réinitialise la config du servo.
	 * Fonction appellée si la carte IHM a détecté une grosse chute de la tension d'alimentation des actionneurs.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
	 */
	void IMPL_AX12_POS_DOUBLE_reset_config(void* actuator_gen_ptr);

	/**
	 * @brief Fonction pour stopper l'asservissement du servo.
	 * Fonction appellée à la fin du match via ActManager.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
	 */
	void IMPL_AX12_POS_DOUBLE_stop(void *actuator_gen_ptr);

	/**
	 * @brief Fonction de dispatch des messages CAN pour les ordres actionneurs.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
	 * @param msg le message CAN reçu.
	 * @return TRUE si le message CAN a été traité et FALSE sinon.
	 */
	bool_e IMPL_AX12_POS_DOUBLE_CAN_process_msg(void *actuator_gen_ptr, CAN_msg_t* msg);

	/**
	 * @brief Fonction permettant d'obtenir des infos sur la config du servo comme la vitesse ou le couple.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
	 * @param incoming_msg le message CAN reçu.
	 */
	void IMPL_AX12_POS_DOUBLE_get_config(void *actuator_gen_ptr, CAN_msg_t *incoming_msg);

	/**
	 * @brief Fonction permettant de modifier la config du servo comme la vitesse ou le couple.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
	 * @param msg le message CAN reçu.
	 */
	void IMPL_AX12_POS_DOUBLE_set_config(void *actuator_gen_ptr, CAN_msg_t* msg);

	/**
	 * @brief Fonction permettant d'activer un warner sur une position du servo.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
	 * @param msg le message CAN reçu.
	 */
	void IMPL_AX12_POS_DOUBLE_set_warner(void* actuator_gen_ptr, CAN_msg_t *msg);

	/**
	 * @brief Fonction d'interruption d'un servo.
	 * Cette fonction permet d'effectuer l'asservissement en couple d'un servo.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
	 */
	void IMPL_AX12_POS_DOUBLE_process_it(void* actuator_gen_ptr);

#endif /* IMPL_AX12_POS_DOUBLE_H_ */
