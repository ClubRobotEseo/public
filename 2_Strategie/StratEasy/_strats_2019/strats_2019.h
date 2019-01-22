/*
 * strats_2019.h
 *
 *  Created on: 1 nov. 2018
 *      Author: Admin
 */

#ifndef STRATS_2019_H_
#define STRATS_2019_H_

#include "../QS/QS_all.h"
#include "../prop.h"
#include "../QS/QS_stateMachineHelper.h"
#include "../QS/QS_lowLayer/QS_can.h"
#include "../actions.h"
#include "../odometry.h"
#include "../stepper_motors.h"

error_e strat2019(void);
error_e testActuators2019(void);
error_e sub_mark_ground_atoms_on_floor(void);
error_e sub_dispenser_south(void);
error_e sub_rush(void);
error_e sub_recup_distrbuteur_pente(void);
#endif /* STRATS_2019_H_ */
