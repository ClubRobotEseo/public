/**
 *	Club Robot ESEO 2017 - 2018
 *
 * @file impl_ax12_pos.h
 * @brief Implémentation des fonctions pour un actionneur de type KIND_AX12_POS.
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
	 * Elle initialise le bus de communication ainsi que les différents registres du servo.
	 * Cette fonction est appelée au lancement de la carte via ActManager.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
	 */
	void IMPL_AX12_POS_init(void* actuator_gen_ptr);

	/**
	 * Fonction appellée pour initialiser en position le servo dès l'arrivée de l'alimentation via ActManager.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
	 * @return le status d'initialisation.
	 */
	error_e IMPL_AX12_POS_init_pos(void *actuator_gen_ptr);

	/**
	 * @brief Fonction qui réinitialise la config du servo.
	 * Fonction appellée si la carte IHM a détecté une grosse chute de la tension d'alimentation des actionneurs.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
	 */
	void IMPL_AX12_POS_reset_config(void* actuator_gen_ptr);

	/**
	 * @brief Fonction pour stopper l'asservissement du servo.
	 * Fonction appellée à la fin du match via ActManager.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
	 */
	void IMPL_AX12_POS_stop(void *actuator_gen_ptr);

	/**
	 * @brief Fonction permettant d'exécuter une commande.
	 * Fonction utilisée pour les commandes de position.
	 * @param actuator_gen_ptr l'instance de l'actionneur.
	 * @param order l'ordre à exécuter.
	 * @param param un paramètre (inutilisé ici)
	 * @return l'état de l'initialisation de la commande.
	 */
	act_result_state_e IMPL_AX12_POS_run_order(void* actuator_gen_ptr, ACT_order_e order, Sint32 param);

	/**
	 * @brief Fonction qui vérifie l'exécution de la commande en cours.
	 * Fonction utilisée pour les commandes de position.
	 * @param actuator_gen_ptr l'instance de l'actionneur.
	 * @return l'état d'exécution de la commande.
	 */
	act_result_state_e IMPL_AX12_POS_check_order(void* actuator_gen_ptr);

	/**
	 * @brief Fonction permettant d'obtenir des infos sur la config du servo comme la vitesse ou le couple.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
	 * @param id l'id de l'actionneur concerné.
	 * @return la valeur de la config demandée.
	 */
	Uint16 IMPL_AX12_POS_get_config(void *actuator_gen_ptr, act_config_e config);

	/**
	 * @brief Fonction permettant de modifier la config du servo comme la vitesse ou le couple.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur générique).
	 * @param config la configuration à changer.
	 * @param value la nouvelle valeur de la configuration
	 */
	void IMPL_AX12_POS_set_config(void *actuator_gen_ptr, act_config_e config, Uint16 value);

	#endif /* USE_AX12_SERVO */

#endif	/* IMPL_AX12_POS_H */

