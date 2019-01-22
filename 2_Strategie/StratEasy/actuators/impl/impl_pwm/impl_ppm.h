/**
 *	Club Robot ESEO 2017 - 2018
 *
 * @file impl_ppm.h
 * @brief Implémentation des fonctions pour un actionneur de type KIND_PWM.
 * @author Valentin
 */

#ifndef IMPL_PPM_H
	#define	IMPL_PPM_H

	#include "../QS/QS_all.h"
	#include "../QS/QS_CANmsgList.h"
	#include "../../actuator_pwm.h"

	/**
	 * @brief Fonction permettant d'initialiser le servo.
	 * Elle initialise le bus de communication ainsi que les différents registres du servo.
	 * Cette fonction est appelée au lancement de la carte via ActManager.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
	 */
	void IMPL_PPM_init(void* actuator_gen_ptr);

	/**
	 * Fonction appellée pour initialiser en position le servo dès l'arrivée de l'alimentation via ActManager.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
	 * @return le status d'initialisation.
	 */
	error_e IMPL_PPM_init_pos(void *actuator_gen_ptr);

	/**
	 * @brief Fonction qui réinitialise la config du servo.
	 * Fonction appellée si la carte IHM a détecté une grosse chutte de la tension d'alimentation des actionneurs.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
	 */
	//void IMPL_PPM_reset_config(void* actuator_gen_ptr);

	/**
	 * @brief Fonction pour stopper l'actionneur.
	 * Fonction appellée à la fin du match via ActManager.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
	 */
	void IMPL_PPM_stop(void *actuator_gen_ptr);

	/**
	 * @brief Fonction permettant d'exécuter une commande.
	 * Fonction utilisée pour les commandes de position.
	 * @param actuator_gen_ptr l'instance de l'actionneur.
	 * @param order l'ordre à exécuter.
	 * @param duty_cycle le pourcentage cyclique à appliquer.
	 * @return l'état de l'initialisation de la commande.
	 */
	act_result_state_e IMPL_PPM_run_order(void* actuator_gen_ptr, ACT_order_e order, Sint32 duty_cycle);

	/**
	 * @brief Fonction qui vérifie l'exécution de la commande en cours.
	 * Fonction utilisée pour les commandes de position.
	 * @param actuator_gen_ptr l'instance de l'actionneur.
	 * @return l'état d'exécution de la commande.
	 */
	act_result_state_e IMPL_PPM_check_order(void* actuator_gen_ptr);

#endif	/* IMPL_PPM_H */

