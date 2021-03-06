/*
 *	Club Robot ESEO 2008 - 2012
 *	Archi'Tech, CHOMP, CheckNorris, Shark & Fish
 *
 *	Fichier : brain.h
 *	Package : Carte Principale
 *	Description : 	Fonctions de g�n�ration des ordres
 *	Auteur : Jacen, modifi� par Gonsevi
 *	Version 2012/01/14
 */

#include "QS/QS_all.h"

#ifndef BRAIN_H
	#define BRAIN_H

	#include "QS/QS_all.h"

	/* Les procedures d'ia possibles : */
	typedef void(*ia_fun_t)(void);


	void BRAIN_init(void);

	/* 	execute un match de match_duration secondes � partir de la
		liberation de la biroute. Arrete le robot � la fin du match.
		Appelle une autre routine pour l'IA pendant le match.
	*/
	void any_match(void);


	bool_e BRAIN_get_strat_updated(void);
	char * BRAIN_get_current_strat_name(void);
	ia_fun_t BRAIN_get_current_strat_function(void);
	ia_fun_t BRAIN_get_displayed_strat_function(Uint8 i);
	char * BRAIN_get_displayed_strat_name(Uint8 i);

	Uint8 BRAIN_get_number_of_displayed_strategy();

	void BRAIN_set_strategy_index(Uint8 i);

	time32_t BRAIN_getMatchDuration(void);

	void BRAIN_start(void);


#endif /* ndef BRAIN_H */
