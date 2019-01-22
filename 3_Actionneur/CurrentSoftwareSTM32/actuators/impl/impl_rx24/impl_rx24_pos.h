/**
 *	Club Robot ESEO 2017 - 2018
 *
 * @file impl_rx24_pos.h
 * @brief Impl�mentation des fonctions pour un actionneur de type KIND_RX24_POS.
 * @author Valentin
 */

#ifndef IMPL_RX24_POS_H
	#define	IMPL_RX24_POS_H

	#include "../QS/QS_all.h"
	#include "../QS/QS_CANmsgList.h"
	#include "../queue.h"
	#include "../../actuator_servo.h"

	/**
	 * @brief Fonction permettant d'initialiser le servo.
	 * Elle initialise le bus de communication ainsi que les diff�rents registres du servo.
	 * Cette fonction est appel�e au lancement de la carte via ActManager.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur g�n�rique).
	 */
	void IMPL_RX24_POS_init(void* actuator_gen_ptr);

	/**
	 * Fonction appell�e pour initialiser en position le servo d�s l'arriv�e de l'alimentation via ActManager.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur g�n�rique).
	 * @return le status d'initialisation.
	 */
	error_e IMPL_RX24_POS_init_pos(void *actuator_gen_ptr);

	/**
	 * @brief Fonction qui r�initialise la config du servo.
	 * Fonction appell�e si la carte IHM a d�tect� une grosse chute de la tension d'alimentation des actionneurs.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
	 */
	void IMPL_RX24_POS_reset_config(void* actuator_gen_ptr);

	/**
	 * @brief Fonction pour stopper l'asservissement du servo.
	 * Fonction appell�e � la fin du match via ActManager.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur g�n�rique).
	 */
	void IMPL_RX24_POS_stop(void *actuator_gen_ptr);

	/**
	 * @brief Fonction de dispatch des messages CAN pour les ordres actionneurs.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur g�n�rique).
	 * @param msg le message CAN re�u.
	 * @return TRUE si le message CAN a �t� trait� et FALSE sinon.
	 */
	bool_e IMPL_RX24_POS_CAN_process_msg(void *actuator_gen_ptr, CAN_msg_t* msg);

	/**
	 * @brief Fonction permettant d'obtenir des infos sur la config du servo comme la vitesse ou le couple.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur g�n�rique).
	 * @param incoming_msg le message CAN re�u.
	 */
	void IMPL_RX24_POS_get_config(void *actuator_gen_ptr, CAN_msg_t *incoming_msg);

	/**
	 * @brief Fonction permettant de modifier la config du servo comme la vitesse ou le couple.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur g�n�rique).
	 * @param msg le message CAN re�u.
	 */
	void IMPL_RX24_POS_set_config(void *actuator_gen_ptr, CAN_msg_t* msg);

	/**
	 * @brief Fonction permettant d'activer un warner sur une position du servo.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur g�n�rique).
	 * @param msg le message CAN re�u.
	 */
	void IMPL_RX24_POS_set_warner(void* actuator_gen_ptr, CAN_msg_t *msg);

	/**
	 * @brief Fonction permettant de v�rifier si le warner est franchi lors du mouvement du servo.
	 * @param actuator l'instance de l'actionneur.
	 * @param pos la position courante du servo.
	 */
	void IMPL_RX24_POS_check_warner(Actuator_servo_t* actuator, Uint16 pos);

	/**
	 * @brief Fonction d'interruption d'un servo.
	 * Cette fonction permet d'effectuer l'asservissement en couple d'un servo.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
	 */
	void IMPL_RX24_POS_process_it(void* actuator_gen_ptr);

#endif	/* IMPL_RX24_POS_H */

