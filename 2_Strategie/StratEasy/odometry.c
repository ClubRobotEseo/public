/*
 * odometry.c
 *
 *  Created on: 14 juin 2018
 *      Author: Nirgal
 */

#include "config.h"
#include "odometry.h"


typedef struct
{
	double x;		//mm
	double y;		//mm
	double teta;	//rad
}precise_position_t;

static precise_position_t precise_position;	//x et y en mm4096 ! teta en rad4096*1024

void ODOMETRY_set_position(int32_t x, int32_t y, int32_t teta)
{
	precise_position.x = (double)x;
	precise_position.y = (double)y;
	precise_position.teta = (double)(teta)/4096;
	global.pos.x = (int32_t)(precise_position.x);
	global.pos.y = (int32_t)(precise_position.y);
	global.pos.teta = (int32_t)(precise_position.teta*4096);
}


//translation et rotation en nombre de pas !!!
void ODOMETRY_new_movement(int32_t translation, int32_t rotation)
{
	int16_t cos, sin;

	if(rotation)
	{
		precise_position.teta += (((double)(rotation))*2*PI/NB_MICROSTEP_BY_ROBOT_TURN);	//RAD
		if(precise_position.teta > 2*PI)
			precise_position.teta -= 2*PI;
		else if(precise_position.teta < -2*PI)
			precise_position.teta += 2*PI;
	}

	if(translation)
	{
		COS_SIN_4096_get((int16_t)(precise_position.teta*4096), &cos, &sin);
		precise_position.x += (((double)(translation*cos))/4096)*MM_BY_MICROSTEP;
		precise_position.y += (((double)(translation*sin))/4096)*MM_BY_MICROSTEP;
	}


	global.pos.x = (int32_t)(precise_position.x);
	global.pos.y = (int32_t)(precise_position.y);
	global.pos.teta = (int32_t)(precise_position.teta*4096);
}


