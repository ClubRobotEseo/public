/**
 *	Club Robot ESEO 2017 - 2018
 *
 * @file impl_ax12_pos.h
 * @brief Impl�mentation des fonctions pour un actionneur de type KIND_AX12_POS.
 * @author Valentin
 */

#ifndef IMPL_AX12_POS_H
	#define	IMPL_AX12_POS_H

	#include "../QS/QS_all.h"
	#include "../QS/QS_CANmsgList.h"
	#include "../../actuator_servo.h"

	#ifdef USE_AX12_SERVO

	/**
	 * @brief Fonction permettant d'initialiser le servo.
	 * Elle initialise le bus de communication ainsi que les diff�rents registres du servo.
	 * Cette fonction est appel�e au lancement de la carte via ActManager.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur g�n�rique).
	 */
	void IMPL_AX12_POS_init(void* actuator_gen_ptr);

	/**
	 * Fonction appell�e pour initialiser en position le servo d�s l'arriv�e de l'alimentation via ActManager.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur g�n�rique).
	 * @return le status d'initialisation.
	 */
	error_e IMPL_AX12_POS_init_pos(void *actuator_gen_ptr);

	/**
	 * @brief Fonction qui r�initialise la config du servo.
	 * Fonction appell�e si la carte IHM a d�tect� une grosse chute de la tension d'alimentation des actionneurs.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
	 */
	void IMPL_AX12_POS_reset_config(void* actuator_gen_ptr);

	/**
	 * @brief Fonction pour stopper l'asservissement du servo.
	 * Fonction appell�e � la fin du match via ActManager.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur g�n�rique).
	 */
	void IMPL_AX12_POS_stop(void *actuator_gen_ptr);

	/**
	 * @brief Fonction permettant d'ex�cuter une commande.
	 * Fonction utilis�e pour les commandes de position.
	 * @param actuator_gen_ptr l'instance de l'actionneur.
	 * @param order l'ordre � ex�cuter.
	 * @param param un param�tre (inutilis� ici)
	 * @return l'�tat de l'initialisation de la commande.
	 */
	act_result_state_e IMPL_AX12_POS_run_order(void* actuator_gen_ptr, ACT_order_e order, Sint32 param);

	/**
	 * @brief Fonction qui v�rifie l'ex�cution de la commande en cours.
	 * Fonction utilis�e pour les commandes de position.
	 * @param actuator_gen_ptr l'instance de l'actionneur.
	 * @return l'�tat d'ex�cution de la commande.
	 */
	act_result_state_e IMPL_AX12_POS_check_order(void* actuator_gen_ptr);

	/**
	 * @brief Fonction permettant d'obtenir des infos sur la config du servo comme la vitesse ou le couple.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur g�n�rique).
	 * @param id l'id de l'actionneur concern�.
	 * @return la valeur de la config demand�e.
	 */
	Uint16 IMPL_AX12_POS_get_config(void *actuator_gen_ptr, act_config_e config);

	/**
	 * @brief Fonction permettant de modifier la config du servo comme la vitesse ou le couple.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur g�n�rique).
	 * @param config la configuration � changer.
	 * @param value la nouvelle valeur de la configuration
	 */
	void IMPL_AX12_POS_set_config(void *actuator_gen_ptr, act_config_e config, Uint16 value);

	#endif /* USE_AX12_SERVO */

#endif	/* IMPL_AX12_POS_H */

