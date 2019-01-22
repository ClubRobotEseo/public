/*
 *	Club Robot ESEO 2008 - 2009
 *	Archi'Tech
 *
 *	Fichier : clock.h
 *	Package : Carte Principale
 *	Description : 	Fonctions de gestion de l'horloge
 *	Auteur : Jacen
 *	Version 20090322
 */

#include "QS/QS_all.h"

#ifndef CLOCK_H
#define CLOCK_H

void CLOCK_init();

/* leve le drapeau de départ de match et lance le compteur de quarts de seconde */
void CLOCK_run_match();

extern void process_ms(void);

#endif /*ndef CLOCK_H */
