/*
 *	Club Robot ESEO 2018 - 2019
 *
 *	Fichier : act_utils.h
 *	Package : Carte Actionneur
 *	Description : Propose des fonctions génériques pour gérer les actionneurs.
 *	Auteur : Valentin
 */

#ifndef ACT_QUEUE_UTILS_H
#define	ACT_UTILS_H

	#include "../QS/QS_all.h"
	#include "../QS/QS_CANmsgList.h"
	#include "actuator_servo.h"

	/** Verifie l'état de l'ax12 et gère la file en conséquence.
	 *
	 * @param ax12Id l'identifiant de l'ax12 à gérer
	 * @param wantedGoal position voulue en degré ou vitesse voulue en %
	 * @param currentGoal position actuelle du servomoteur ou vitesse actuelle du servomoteur
	 * @param epsilon précision de la position ou de la vitesse voulue, tant qu'on à pas atteint wantedGoal +/- epsilon on considère qu'on est pas rendu à la bonne position
	 * @param start_time_ms le temps auquel la commande a été envoyée en ms
	 * @param timeout_ms_ timeout en msec
	 * @param large_epsilon comme pos_epsilon mais utilisé après un timeout. Après un timeout, on vérifie si on est proche de la position voulue (utile pour des pinces par ex), si oui il n'y a pas d'erreur
	 *
	 * @param result pointeur vers une variable qui contiendra le résultat de l'opération si la fonction retourne TRUE (ACT_RESULT*)
	 * @param error_code pointeur vers une variable qui contiendra le l'erreur lié a l'opération si la fonction retourne TRUE (ACT_RESULT_ERROR*)
	 * @param line pointeur vers une variable qui contiendra la ligne qui affecte l'erreur dans les variables pointeurs
	 * @return TRUE si l'ax12 a fini sa commande, FALSE sinon
	 */
	bool_e ACTQ_check_status_ax12(Uint8 ax12Id, Sint16 wantedGoal, Sint16 currentGoal, Uint16 epsilon, time32_t start_time_ms, Uint16 timeout_ms, Uint16 large_epsilon, Uint8* result, Uint8* error_code, Uint16* line);

	/** Verifie l'état du rx24 et gère la file en conséquence.
	 *
	 * @param rx24Id l'identifiant du rx24 à gérer
	 * @param wantedGoal position voulue en degré ou vitesse voulue en %
	 * @param currentGoal position actuelle du servomoteur ou vitesse actuelle du servomoteur
	 * @param epsilon précision de la position ou de la vitesse voulue, tant qu'on à pas atteint wantedGoal +/- epsilon on considère qu'on est pas rendu à la bonne position ou vitesse
	 * @param start_time_ms le temps auquel la commande a été envoyée en ms
	 * @param timeout_ms_ timeout en msec
	 * @param large_epsilon comme pos_epsilon mais utilisé après un timeout. Après un timeout, on vérifie si on est proche de la position voulue (utile pour des pinces par ex), si oui il n'y a pas d'erreur
	 *
	 * @param result pointeur vers une variable qui contiendra le résultat de l'opération si la fonction retourne TRUE (ACT_RESULT*)
	 * @param error_code pointeur vers une variable qui contiendra le l'erreur lié a l'opération si la fonction retourne TRUE (ACT_RESULT_ERROR*)
	 * @param line pointeur vers une variable qui contiendra la ligne qui affecte l'erreur dans les variables pointeurs
	 * @return TRUE si le rx24 a fini sa commande, FALSE sinon
	 */
	bool_e ACTQ_check_status_rx24(Uint8 rx24Id, Sint16 wantedGoal, Sint16 currentGoal, Uint16 epsilon, time32_t start_time_ms, Uint16 timeout_ms, Uint16 large_epsilon, Uint8* result, Uint8* error_code, Uint16* line);

	/**
	 * @brief Affichage du résultat d'un ordre actionneur.
	 * @param actuator l'actionneur concerné.
	 * @param order l'ordre effectué.
	 * @param result le résultat de l'exécution.
	 * @param errorCode le code d'erreur associé au résultat de l'exécution.
	 * @param param un paramètre (e.g., le numéro de ligne)
	 */
	void ACT_printResult(Actuator_t* actuator, Uint8 order, Uint8 result, Uint8 errorCode, Uint16 param);


#endif	/* ACT_UTILS_H */
