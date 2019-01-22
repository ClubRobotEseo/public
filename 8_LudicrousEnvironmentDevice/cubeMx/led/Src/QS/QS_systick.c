/*
 *	Club Robot ESEO 2014 - 2015
 *	Holly & Wood
 *
 *	Fichier : QS_systick.c
 *	Package : Qualite Soft
 *	Description : Configuration de systick pour gérer une l'horloge du projet
 *	Auteur : Arnaud
 *	Version 20100421
 */

#include <stdlib.h>
#include "QS_systick.h"

static volatile time32_t* systick_counter = NULL;

void SYSTICK_init(volatile time32_t* systick_ptr){
	systick_counter = systick_ptr;
}

void HAL_SYSTICK_Callback(void) {
	if(systick_counter)
		(*systick_counter)++;
}
