/*
 *	Club Robot ESEO 2008 - 2009
 *	Archi'Tech
 *
 *	Fichier : Generic_functions.h
 *	Package : Carte Principale
 *	Description : 	Fonction generique pour piles de gestion
 *					des actionneurs
 *	Auteur : Jacen
 *	Version 20090111
 */

#define GENERIC_FUNCTIONS_C

#include "generic_functions.h"
#include "../propulsion/movement.h"
#include "../QS/QS_outputlog.h"

Uint8 wait_time(time32_t time, Uint8 in_progress, Uint8 success_state){
	typedef enum
	{
		IDLE,
		WAIT,
		END
	}state_e;
	static state_e state = IDLE;
	static time32_t timeEnd;
	switch(state){

		case IDLE :
			timeEnd = global.match_time + time;
			state = WAIT;
			break;

		case WAIT:
			if(timeEnd <= global.match_time)
				state = END;
			break;

		case END:
			state = IDLE;
			return success_state;
	}
	return in_progress;
}

void on_turning_point(){
	if(!is_possible_point_for_rotation(&((GEOMETRY_point_t){global.pos.x, global.pos.y})))
		debug_printf(" !! NOT ON TURNING POINT !!\n");
}

//Un point est-t-il une position permettant les rotations sans tapper dans un élément de jeu.
bool_e is_possible_point_for_rotation(GEOMETRY_point_t * p)
{
	Uint8 widthRobot;
	widthRobot =  (QS_WHO_AM_I_get() == BIG_ROBOT)? (BIG_ROBOT_WIDTH/2) : (SMALL_ROBOT_WIDTH/2);
	widthRobot += 50;	//Marge !

	// Spécifique Terrain 2019
	if(
			!is_in_square(0 + widthRobot, 2000 - widthRobot, 0 + widthRobot, 3000 - widthRobot, *p)	// Hors Terrain
		|| 	is_in_square(1540 - widthRobot, 2000, 450 - widthRobot, 2550 + widthRobot,*p)			// Zones des pentes
		||	is_in_square(1178 - widthRobot, 2000, 1480 - widthRobot, 1520 + widthRobot,*p)			// Tasseau central entre les pentes

	  )
		return FALSE;

	return  TRUE;
}

//Le point passé en paramètre permet-il une extraction ?
bool_e is_possible_point_for_dodge(GEOMETRY_point_t * p)
{
	Uint8 widthRobot;
	widthRobot =  (QS_WHO_AM_I_get() == BIG_ROBOT)? (BIG_ROBOT_WIDTH/2) : (SMALL_ROBOT_WIDTH/2);
	widthRobot += 100;	//Marge !

	// Spécifique Terrain 2019
	if(
			!is_in_square(0 + widthRobot, 2000-widthRobot, 0+widthRobot, 3000-widthRobot, *p)	// Hors Terrain
		||  is_in_square(300 - widthRobot, 1200 + widthRobot, COLOR_Y(2650 - widthRobot), COLOR_Y(3000), *p)  // Tableau périodique adverse (n'empêche pas de tourner mais on ne doit pas s'extraire dedans)
		||  is_in_square(1540 - widthRobot, 2000, 450 - widthRobot, 2550 + widthRobot, *p)		// Zone des pentes
		||	is_in_square(1178 - widthRobot, 2000, 1480 - widthRobot, 1520 + widthRobot, *p)		// Tasseau central entre les pentes
		|| (is_in_square(1178 - widthRobot, 1600, 0, 1500, *p) && global.pos.y > 1500)			// Tasseau central entre les pentes (point de dodge côté jaune et robot côté violet)
		|| (is_in_square(1178 - widthRobot, 1600, 1500, 3000, *p) && global.pos.y < 1500)		// Tasseau central entre les pentes (point de dodge côté violet et robot côté jaune)

	)
		return FALSE;

	return  TRUE;
}

bool_e i_am_in_square_color(Sint16 x1, Sint16 x2, Sint16 y1, Sint16 y2){
	return is_in_square(x1, x2, COLOR_Y(y1), COLOR_Y(y2), (GEOMETRY_point_t){global.pos.x, global.pos.y});
}

bool_e i_am_in_square(Sint16 x1, Sint16 x2, Sint16 y1, Sint16 y2){
	return is_in_square(x1, x2, y1, y2, (GEOMETRY_point_t){global.pos.x, global.pos.y});
}

bool_e is_in_square_color(Sint16 x1, Sint16 x2, Sint16 y1, Sint16 y2, GEOMETRY_point_t current){
	return is_in_square(x1, x2, COLOR_Y(y1), COLOR_Y(y2), current);
}

