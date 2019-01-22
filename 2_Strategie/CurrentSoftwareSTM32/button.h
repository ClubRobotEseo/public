/*
 *	Club Robot ESEO 2008 - 2010
 *	Archi'Tech, PACMAN
 *
 *	Fichier : button.h
 *	Package : Carte Principale
 *	Description : 	Fonctions de gestion du bouton et de la biroute
 *	Auteur : Jacen & Ronan
 *	Version 20100408
 */

#include "QS/QS_all.h"


#ifndef BUTTON_H
	#define BUTTON_H
	#include "QS/QS_all.h"

	void BUTTON_init();
	void BUTTON_update();
	void BUTTON_start();
	void BUTTON_rotation();
	void BUTTON_servo();
	void BUTTON_pi_rotation();
	void BUTTON_translation();
	void SWITCH_bascule(void);
	void BUTTON_go_to_home();

#endif /* ndef BUTTON_H */
