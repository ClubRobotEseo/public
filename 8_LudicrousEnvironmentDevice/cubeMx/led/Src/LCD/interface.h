/*
 *	Club Robot ESEO 2016 - 2017
 *
 *	Fichier : xpt2046.h
 *	Package : IHM
 *	Description : Driver du XPT2046
 *	Auteur : Arnaud Guilmet
 *  Licence : CeCILL-C (voir LICENCE.txt)
 *	Version 20100929
 */

/** ----------------  Defines possibles  --------------------
 *	IRQ_TOUCH					: GPIO de lecture de l'interruption
 *	USE_IRQ_TOUCH_VALIDATION	: Utiliser la broche d'interruption de tactile pour valider les donn�es
 *	LCD_CS_TOUCH				: GPIO d'�criture du chip select
 *
 *	TODO : Gestion de IRQPen en interruption
 */

#ifndef _H_INTERFACE
	#define _H_INTERFACE
	#include "../QS/QS_all.h"

	typedef enum{
		INTERFACE_IHM_WAIT,
		INTERFACE_IHM_MATCH,
		INTERFACE_IHM_CUSTOM,
		INTERFACE_IHM_USER_BUTTON,
	}INTERFACE_ihm_e;

	void INTERFACE_init(void);

	void INTERFACE_processMain(void);

	void INTERFACE_setInterface(INTERFACE_ihm_e ihm);

	INTERFACE_ihm_e INTERFACE_getInterface();

#endif //_H_INTERFACE