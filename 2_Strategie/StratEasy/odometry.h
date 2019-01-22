/*
 * odometry.h
 *
 *  Created on: 26 juin 2018
 *      Author: Nirgal
 */

#ifndef ODOMETRY_H_
#define ODOMETRY_H_
#include "QS/QS_all.h"

void ODOMETRY_set_position(int32_t x, int32_t y, int32_t teta);
void ODOMETRY_new_movement(int32_t translation, int32_t rotation);

#endif /* ODOMETRY_H_ */
