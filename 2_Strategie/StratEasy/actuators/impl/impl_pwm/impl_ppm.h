/**
 *	Club Robot ESEO 2017 - 2018
 *
 * @file impl_ppm.h
 * @brief Impl�mentation des fonctions pour un actionneur de type KIND_PWM.
 * @author Valentin
 */

#ifndef IMPL_PPM_H
	#define	IMPL_PPM_H

	#include "../QS/QS_all.h"
	#include "../QS/QS_CANmsgList.h"
	#include "../../actuator_pwm.h"

	/**
	 * @brief Fonction permettant d'initialiser le servo.
	 * Elle initialise le bus de communication ainsi que les diff�rents registres du servo.
	 * Cette fonction est appel�e au lancement de la carte via ActManager.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
	 */
	void IMPL_PPM_init(void* actuator_gen_ptr);

	/**
	 * Fonction appell�e pour initialiser en position le servo d�s l'arriv�e de l'alimentation via ActManager.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
	 * @return le status d'initialisation.
	 */
	error_e IMPL_PPM_init_pos(void *actuator_gen_ptr);

	/**
	 * @brief Fonction qui r�initialise la config du servo.
	 * Fonction appell�e si la carte IHM a d�tect� une grosse chutte de la tension d'alimentation des actionneurs.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
	 */
	//void IMPL_PPM_reset_config(void* actuator_gen_ptr);

	/**
	 * @brief Fonction pour stopper l'actionneur.
	 * Fonction appell�e � la fin du match via ActManager.
	 * @param actuator_gen_ptr l'instance de l'actionneur (sous forme de pointeur generique).
	 */
	void IMPL_PPM_stop(void *actuator_gen_ptr);

	/**
	 * @brief Fonction permettant d'ex�cuter une commande.
	 * Fonction utilis�e pour les commandes de position.
	 * @param actuator_gen_ptr l'instance de l'actionneur.
	 * @param order l'ordre � ex�cuter.
	 * @param duty_cycle le pourcentage cyclique � appliquer.
	 * @return l'�tat de l'initialisation de la commande.
	 */
	act_result_state_e IMPL_PPM_run_order(void* actuator_gen_ptr, ACT_order_e order, Sint32 duty_cycle);

	/**
	 * @brief Fonction qui v�rifie l'ex�cution de la commande en cours.
	 * Fonction utilis�e pour les commandes de position.
	 * @param actuator_gen_ptr l'instance de l'actionneur.
	 * @return l'�tat d'ex�cution de la commande.
	 */
	act_result_state_e IMPL_PPM_check_order(void* actuator_gen_ptr);

#endif	/* IMPL_PPM_H */

