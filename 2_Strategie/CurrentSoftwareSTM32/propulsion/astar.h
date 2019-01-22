/*
 *	Club Robot ESEO 2015 - 2017
 *
 *
 *	Fichier : astar.c
 *	Package : Strat�gie
 *	Description : 	Fonctions de g�n�ration des trajectoires
 *					par le biais d'un algo de type A*
 *	Auteurs : Valentin BESNARD
 *	Version 2
 */

#include "../QS/QS_all.h"

#if USE_ASTAR

	#ifndef _ASTAR_H_
		#define _ASTAR_H_

	#include "../QS/QS_who_am_i.h"
	#include "movement.h"



//--------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------- Fonctions importantes de l'algo A* ------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------

	// Initialisation de l'astar
	void ASTAR_init();

	// Machine � �tat r�alisant le try_going apr�s appel � l'algorithme astar
	Uint8 ASTAR_try_going(Uint16 x, Uint16 y, Uint8 in_progress, Uint8 success_state, Uint8 fail_state, PROP_speed_e speed, way_e way, avoidance_type_e avoidance, STRAT_endCondition_e end_condition);

//--------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------- Fonctions d'affichage ------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------

	// Proc�dure affichant la liste ouverte
	void ASTAR_print_opened_list();

	// Proc�dure affichant la liste ferm�e
	void ASTAR_print_closed_list();

	// Proc�dure affichant la liste des nodes et leurs caract�ristiques
	void ASTAR_print_nodes(bool_e with_neighbors);

	// Proc�dure affichant la liste des obstacles(zones interdites et hardlines)
	void ASTAR_print_obstacles();

//--------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------- Accesseurs -----------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------

	// Num�ro des polygones
	#define POLYGON_WATER_TREATMENT_PLANT		(0)
	#define POLYGON_ADV_START_ZONE				(1)
	#define POLYGON_OUR_BUILDING_ZONE			(2)
	#define POLYGON_ADV_BUILDING_ZONE			(3)
	#define POLYGON_CROSS_NO					(4)
	#define POLYGON_CROSS_O						(5)
	#define POLYGON_CROSS_SO					(6)
	#define POLYGON_CROSS_NE					(7)
	#define POLYGON_CROSS_E						(8)
	#define POLYGON_CROSS_SE					(9)

	// Proc�dure permettant d'activer un polygone
	void ASTAR_enable_polygon(Uint8 polygon_number);

	// Proc�dure permettant de d�sactiver un polygone
	void ASTAR_disable_polygon(Uint8 polygon_number);


	#endif /* ndef _ASTAR_H_ */
#endif /* USE_ASTAR */




