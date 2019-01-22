/*
 * memory.h
 *
 *  Created on: 20 sept. 2018
 *      Author: a.guilmet
 */

#ifndef MEMORY_H_
	#define MEMORY_H_

	#include "QS/QS_all.h"

	bool_e MEMORY_init(void);

	bool_e MEMORY_getCoefOdometry(Sint32 * value, PROPULSION_coef_e coef);
	bool_e MEMORY_setCoefOdometry(Sint32 value, PROPULSION_coef_e coef);

	bool_e MEMORY_getRobotDistance(Sint64 * value);
	bool_e MEMORY_setRobotDistance(Sint64 value);

#endif /* MEMORY_H_ */
