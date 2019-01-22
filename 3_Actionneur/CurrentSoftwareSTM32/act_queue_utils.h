/*
 *	Club Robot ESEO 2012 - 2013
 *	Krusty & Tiny
 *
 *	Fichier : act_queue_utils.h
 *	Package : Carte Actionneur
 *	Description : Propose des fonctions pour g�rer les actions avec la pile.
 *	Auteur : Alexis
 *	Version 20130420
 */

#ifndef ACT_QUEUE_UTILS_H
#define	ACT_QUEUE_UTILS_H
	#include "QS/QS_all.h"
	#include "queue.h"
	#include "QS/QS_CANmsgList.h"
	#include "QS/QS_actuator/QS_DCMotorSpeed.h"
	#include "actuators/actuator.h"

	//Met sur la pile une action qui sera g�r�e par act_function_ptr avec en param�tre param. L'action est prot�g�e par semaphore avec act_id
	//Cette fonction est appel�e par les fonctions de traitement des messages CAN de chaque actionneur.
	//act_function_ptr est une des fonctions ACTQ_finish_* ci-dessous dans ce fichier.
	//Quand une action est termin�e, il faut appeler QUEUE_next.
	void ACTQ_push_operation_from_msg(CAN_msg_t* msg, QUEUE_act_e queue_id, action_t act_function_ptr, Sint16 param, bool_e send_result);

	//Gestion des �tats des actionneurs (ax12 / DCMotor)
	//Retourne TRUE si l'action en cours n'est pas termin�e, sinon FALSE

	/** Verifie l'�tat de l'ax12 et g�re la file en cons�quence.
	 *
	 * @param queue_id l'identifiant de la file
	 * @param ax12_id l'identifiant de l'ax12 � g�rer
	 * @param wanted_goal position voulue en degr� ou vitesse voulue en %
	 * @param current_goal position actuelle du servomoteur ou vitesse actuelle du servomoteur
	 * @param epsilon pr�cision de la position ou de la vitesse voulue, tant qu'on � pas atteint wanted_goal +/- epsilon on consid�re qu'on est pas rendu � la bonne position
	 * @param timeout_ms_ timeout en msec
	 * @param large_epsilon comme pos_epsilon mais utilis� apr�s un timeout. Apr�s un timeout, on v�rifie si on est proche de la position voulue (utile pour des pinces par ex), si oui il n'y a pas d'erreur
	 *
	 * @param result pointeur vers une variable qui contiendra le r�sultat de l'op�ration si la fonction retourne TRUE (ACT_RESULT*)
	 * @param error_code pointeur vers une variable qui contiendra le l'erreur li� a l'op�ration si la fonction retourne TRUE (ACT_RESULT_ERROR*)
	 * @param line pointeur vers une variable qui contiendra la ligne qui affecte l'erreur dans les variables pointeurs
	 * @return TRUE si l'ax12 a fini sa commande, FALSE sinon
	 */
	bool_e ACTQ_check_status_ax12(queue_id_t queue_id, Uint8 ax12_id, Sint16 wanted_goal, Sint16 current_goal, Uint16 epsilon, Uint16 timeout_ms, Uint16 large_epsilon, Uint8* result, Uint8* error_code, Uint16* line);

	/** Verifie l'�tat du rx24 et g�re la file en cons�quence.
	 *
	 * @param queue_id l'identifiant de la file
	 * @param rx24_id l'identifiant du rx24 � g�rer
	 * @param wanted_goal position voulue en degr� ou vitesse voulue en %
	 * @param current_goal position actuelle du servomoteur ou vitesse actuelle du servomoteur
	 * @param epsilon pr�cision de la position ou de la vitesse voulue, tant qu'on � pas atteint wanted_goal +/- epsilon on consid�re qu'on est pas rendu � la bonne position ou vitesse
	 * @param timeout_ms_ timeout en msec
	 * @param large_epsilon comme pos_epsilon mais utilis� apr�s un timeout. Apr�s un timeout, on v�rifie si on est proche de la position voulue (utile pour des pinces par ex), si oui il n'y a pas d'erreur
	 *
	 * @param result pointeur vers une variable qui contiendra le r�sultat de l'op�ration si la fonction retourne TRUE (ACT_RESULT*)
	 * @param error_code pointeur vers une variable qui contiendra le l'erreur li� a l'op�ration si la fonction retourne TRUE (ACT_RESULT_ERROR*)
	 * @param line pointeur vers une variable qui contiendra la ligne qui affecte l'erreur dans les variables pointeurs
	 * @return TRUE si le rx24 a fini sa commande, FALSE sinon
	 */
	bool_e ACTQ_check_status_rx24(queue_id_t queue_id, Uint8 rx24_id, Sint16 wanted_goal, Sint16 current_goal, Uint16 epsilon, Uint16 timeout_ms, Uint16 large_epsilon, Uint8* result, Uint8* error_code, Uint16* line);


	/** Verifie l'�tat d'un moteur DC et g�re la file en cons�quence.
	 *
	 * @param dcmotor_id identifiant du moteur (pass� en parametre des fonctions de DCMotor2.h)
	 * @param timeout_is_ok TRUE si lors d'un timeout, le resultat doit �tre Ok, FALSE si on doit indiquer une erreur. (Mettre � TRUE pour des moteur qui devrons forcer en continu sur qquechose et qui n'atteindront jamais la position voulue)
	 *
	 * @param result pointeur vers une variable qui contiendra le r�sultat de l'op�ration si la fonction retourne TRUE (ACT_RESULT*)
	 * @param error_code pointeur vers une variable qui contiendra le l'erreur li� a l'op�ration si la fonction retourne TRUE (ACT_RESULT_ERROR*)
	 * @param line pointeur vers une variable qui contiendra la ligne qui affecte l'erreur dans les variables pointeurs
	 * @return TRUE si le moteur a fini sa commande, FALSE sinon
	 */
	bool_e ACTQ_check_status_dcmotor(Uint8 dcmotor_id, bool_e timeout_is_ok, Uint8* result, Uint8* error_code, Uint16* line);

#ifdef USE_DC_MOTOR_SPEED

	/** Verifie l'�tat d'un moteur DC asservie en vitesse et g�re la file en cons�quence.
	 *
	 * @param id identifiant du moteur (pass� en parametre des fonctions de QS_DCMotorSpeed.h)
	 *
	 * @param result pointeur vers une variable qui contiendra le r�sultat de l'op�ration si la fonction retourne TRUE (ACT_RESULT*)
	 * @param error_code pointeur vers une variable qui contiendra le l'erreur li� a l'op�ration si la fonction retourne TRUE (ACT_RESULT_ERROR*)
	 * @param line pointeur vers une variable qui contiendra la ligne qui affecte l'erreur dans les variables pointeurs
	 * @return TRUE si le moteur a fini sa commande, FALSE sinon
	 */
	bool_e ACTQ_check_status_dcMotorSpeed(DC_MOTOR_SPEED_id id, Uint8* result, Uint8* error_code, Uint16* line);

#endif

	/** V�rifie si le temps timeout � �t� d�pass�
	 *
	 * @param queue_id l'identifiant de la file
	 * @param timeout_ms timeout en msec
	 * @return TRUE si le temps a �t� d�pass�, FALSE sinon
	 */
	bool_e ACTQ_check_timeout(queue_id_t queue_id, time32_t timeout_ms);

	//Callback
	//Renvoie un retour � la strat dans tous les cas
	bool_e ACTQ_finish_SendResult(queue_id_t queue_id, Uint8 result, Uint8 error_code, Uint16 param);

	//Retour � la strat seulement si l'op�ration � fail
	bool_e ACTQ_finish_SendResultIfFail(queue_id_t queue_id, Uint8 result, Uint8 error_code, Uint16 param);

	//Retour � la strat seulement si l'op�ration � reussi
	bool_e ACTQ_finish_SendResultIfSuccess(queue_id_t queue_id, Uint8 result, Uint8 error_code, Uint16 param);

	//Ne fait aucun retour
	bool_e ACTQ_finish_SendNothing(queue_id_t queue_id, Uint8 result, Uint8 error_code, Uint16 param);


	//Envoie le message CAN de retour � la strat (et affiche des infos de debuggage si activ�)
	void ACTQ_sendResult(Actuator_t * actuator, Uint8 original_command, Uint8 result, Uint8 error_code);

	//Comme CAN_sendResult mais ajoute un param�tre au message. Peut servir pour debuggage.
	void ACTQ_sendResultWithParam(Actuator_t * actuator, Uint8 original_command, Uint8 result, Uint8 error_code, Uint32 param);

	//Comme CAN_sendResultWithParam mais le param�tre est consid�r� comme �tant un num�ro de ligne.
	void ACTQ_sendResultWitExplicitLine(Actuator_t * actuator, Uint8 original_command, Uint8 result, Uint8 error_code, Uint16 lineNumber);

	//Macro pour avoir la ligne a laquelle cette macro est utilis� comme param�tre � CAN_sendResultWithParam
	#define ACTQ_sendResultWithLine(actuator, original_command, result, error_code) ACTQ_sendResultWitExplicitLine(actuator, original_command, result, error_code, __LINE__);

	void ACTQ_printResult(Actuator_t * actuator, Uint8 original_command, Uint8 result, Uint8 error_code, Uint16 param);


#endif	/* ACT_QUEUE_UTILS_H */
