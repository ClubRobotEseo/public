/*
 * prop.c
 *
 *  Created on: 14 juin 2018
 *      Author: Nirgal
 */
#include "QS/QS_all.h"
#include "prop.h"
#include "QS/QS_stateMachineHelper.h"
#include "config.h"
#include "QS/QS_maths.h"
#include "stepper_motors.h"
#include "QS/QS_outputlog.h"
#include "toolkit.h"

static void PROP_compute_destination_angle(Sint16 x, Sint16 y, Sint16 * teta_todo, way_e way, way_e * chosen_way);
static volatile int16_t max_speed = SPEED_TRANSLATION_DEFAULT;

void PROP_set_max_speed(int16_t speed)
{
	if(speed == 0)
		max_speed = SPEED_TRANSLATION_DEFAULT;
	else
		max_speed = speed;
}

bool_e PROP_go_angle(int32_t teta)
{
	CREATE_MAE(
			IDLE,
			RUNNING);
	bool_e ret = FALSE;
	static order_t order;
	static int32_t teta_todo;
	switch(state)
	{
		case IDLE:
			teta_todo = GEOMETRY_modulo_angle(teta - global.pos.teta);

			if(teta_todo < 0)
			{
				order.forward = FALSE;
				teta_todo = -teta_todo;
			}
			else
			{
				order.forward = TRUE;
			}
			order.nb_step = (uint32_t)((double)(teta_todo)*NB_MICROSTEP_BY_RAD4096);
			order.max_speed = SPEED_ROTATION_DEFAULT;
			order.trajectory = TRAJECTORY_ROTATION;
			if(order.nb_step > 0)
				state = RUNNING;
			else
			{
				ret = TRUE;
			}
			break;
		case RUNNING:
			if(STEPPER_MOTOR_state_machine(&order))
			{
				TOOLKIT_set_flag_update_position();
				debug_printf("  > fin de rotation           | teta=%ld=%ld°\tt=%ld\n", global.pos.teta, ((int32_t)(global.pos.teta)*180)/PI4096, global.match_time);
				ret = TRUE;
				state = IDLE;
			}
			break;
		default:
			break;
	}
	return ret;
}

bool_e PROP_go(int32_t x, int32_t y, way_e way)
{
	CREATE_MAE(
			IDLE,
			ROTATION_BEFORE_TRANSLATION,
			RUNNING);
	bool_e ret = FALSE;
	static order_t order_rotation;
	static order_t order_translation;
	static int16_t teta;
	static way_e chosen_way;
	switch(state)
	{
		case IDLE:
			//Calcul de la quantité de mouvement et de l'angle.
			//Calcul de l'angle de vue
			PROP_compute_destination_angle(x, y, &teta, way, &chosen_way);
			order_translation.forward = (chosen_way==BACKWARD)?FALSE:TRUE;
			order_translation.nb_step = GEOMETRY_distance((GEOMETRY_point_t){global.pos.x, global.pos.y}, (GEOMETRY_point_t){x, y})*NB_MICROSTEP_BY_MM;
			order_translation.trajectory = TRAJECTORY_TRANSLATION;
			order_translation.max_speed = max_speed;
			if(order_translation.nb_step > 1)		//translation minimum = 1 microstep
			{
				if(absolute(teta) > 1)				//Si l'angle est faible (0 ou 1 rad4096) -> rotation préalable
				{
					order_rotation.forward = (teta>0)?TRUE:FALSE;
					order_rotation.nb_step = (double)(absolute(teta))*NB_MICROSTEP_BY_RAD4096;
					order_rotation.trajectory = TRAJECTORY_ROTATION;
					order_rotation.max_speed = SPEED_ROTATION_DEFAULT;
					state = ROTATION_BEFORE_TRANSLATION;
				}
				else
					state = RUNNING;
			}
			else
			{
				ret = TRUE;
				debug_printf("trajectoire nulle -> abandonnée\n");
			}
			break;
		case ROTATION_BEFORE_TRANSLATION:
			if(STEPPER_MOTOR_state_machine(&order_rotation))
			{
				debug_printf("  > fin de rotation préalable | x=%ld\ty=%ld\tteta=%ld\tt=%ld\n", global.pos.x, global.pos.y, global.pos.teta, global.match_time);
				state = RUNNING;
			}
			break;
		case RUNNING:
			if(STEPPER_MOTOR_state_machine(&order_translation))
			{
				ret = TRUE;
				TOOLKIT_set_flag_update_position();
				debug_printf("  > fin de translation        | x=%ld\ty=%ld\tteta=%ld\tt=%ld\n", global.pos.x, global.pos.y, global.pos.teta, global.match_time);
				state = IDLE;
			}
			break;
		default:
			break;
	}
	return ret;
}

void PROP_stop(void)
{

}



static void PROP_compute_destination_angle(Sint16 x, Sint16 y, Sint16 * teta_todo, way_e way, way_e * chosen_way)
{
	int16_t teta_destination;
	//supposons qu'on soit en marche avant...
	teta_destination = GEOMETRY_viewing_angle(global.pos.x, global.pos.y, x, y);


	if(way == FORWARD)
	{
		*chosen_way = FORWARD;
	}

	if(way == BACKWARD)
	{
		teta_destination += PI4096;
		teta_destination = GEOMETRY_modulo_angle(teta_destination);
		*chosen_way = BACKWARD;
	}

	//on cherche l'angle que nous devons parcourir pour joindre cette destination
	*teta_todo = GEOMETRY_modulo_angle(teta_destination - global.pos.teta);

	if(way == ANY_WAY)
	{
		//supposons que l'on soit en marche avant...
		*chosen_way = FORWARD;

		//Si l'angle a parcourir est plus grand que PI/2... on aurait mieux fait de choisir la marche arrière !
		//l'angle a parcourir sera recalculé ensuite...
		if(absolute(*teta_todo) > PI4096/2)
		{
			//et on recalcule tout...
			*chosen_way = BACKWARD;
			teta_destination = GEOMETRY_modulo_angle(teta_destination + PI4096);
			*teta_todo = GEOMETRY_modulo_angle(teta_destination - global.pos.teta);
		}
	}
}
