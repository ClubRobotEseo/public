/*
 *	Club Robot ESEO 2017 - 2018
 *
 *	Fichier : led_manager.h
 *	Package : Carte Strat�gie
 *	Description : Traitement des informations pour le panneau domotique LED (Ludicrous Environment Device)
 *	Auteur : Valentin
 *	Version 20180406
 */

#ifndef LED_MANAGER_H_
#define LED_MANAGER_H_

	#include "QS/QS_all.h"

	/**
	 * Machine a �tat permettant l'envoi des messages XBee � intervalles de temps r�guliers.
	 */
	void LED_process_main();


	/**
	 * Machine a �tat permettant l'envoi des messages Xbee.
	 */
	void LED_send_xbee_messages(Uint11 sid);

	/**
	 * d�clare qu'on a re�u un message en provenance du panneau LED.
	 */
	void LED_is_alive(void);


#endif /* LED_MANAGER_H_ */
