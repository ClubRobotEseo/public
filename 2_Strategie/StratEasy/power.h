/*
 * power.h
 *
 *  Created on: 15 nov. 2018
 *      Author: Nirgal
 */

#ifndef POWER_H_
#define POWER_H_
#include "../QS/QS_all.h"

void POWER_process_ms(void);

void POWER_process_main(void);

bool_e POWER_get_state(void);

uint32_t POWER_get_vbat_mv(void);

#endif /* POWER_H_ */
