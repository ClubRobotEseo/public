/*
 * prop.h
 *
 *  Created on: 14 juin 2018
 *      Author: Nirgal
 */

#ifndef PROP_H_
#define PROP_H_

#include "QS/QS_all.h"

	bool_e PROP_go(int32_t x, int32_t y, way_e way);
	bool_e PROP_go_angle(int32_t teta);
	void PROP_set_max_speed(int16_t speed);
	void PROP_stop(void);

	#define COLOR_Y(y)		((global.color == 0) ? (y) : (3000 - (y)))
	#define COLOR_ANGLE(a)	((global.color == 0) ? (a) : (-(a)))

#endif /* PROP_H_ */
