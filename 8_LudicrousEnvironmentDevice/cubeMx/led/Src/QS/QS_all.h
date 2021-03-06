/*
 *	Club Robot ESEO 2008 - 2010
 *	Archi-Tech', PACMAN
 *
 *	Fichier : QS_all.h
 *	Package : Qualite Soft
 *	Description : Header necessaire pour tout code du robot
 *	Auteur : Jacen, Gwenn
 *	Version 20100418
 */

#ifndef QS_ALL_H
	#define QS_ALL_H

	/*
	 * - config_global.h est inclu par tout le monde,
	 *    Il devrait y avoir le moins de chose possible.
	 *
	 * - config_use.h contient des #define activant ou non des parties du code de la carte.
	 *               (Les #define g�rant du code de debug ne sont pas ici)
	 *
	 * - config_debug.h contient les #define li�s � l'activation de debug
	 *
	 * Les configurations li� � un fichier (comme STACK_SIZE) doivent �tre mis dans le .h correspondant (comme Stack.h)
	 *
	 */

	#include "stm32h7xx_hal.h"					/* 	Toujours utile...								*/
	#define _ISR								/*  attribut d'interruption non utilis� sous STM32  */

	#include "../config/config_global.h"
	#include "../config/config_use.h"
	#include "../config/config_debug.h"
	#include "../config/config_global_vars.h"

	#include "QS_types.h"					/* Nos types										*/
	#include "QS_macro.h"					/* Quelques macros pratiques						*/

	#ifdef VERBOSE_MODE
		#include <stdio.h>					/* pour beneficier de printf en test				*/
	#endif /* def VERBOSE_MODE */

	#ifndef NULL
		#define NULL 0
	#endif

#endif /* ndef QS_ALL_H */
