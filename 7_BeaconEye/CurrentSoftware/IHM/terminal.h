/*
 * terminal.h
 *
 *  Created on: 14 f�vr. 2017
 *      Author: guill
 */

#ifndef IHM_TERMINAL_H_
#define IHM_TERMINAL_H_

#include "../QS/QS_all.h"

void TERMINAL_init();
void TERMINAL_processMain();
void TERMINAL_launchCounter();
void TERMINAL_resetCounter();
void TERMINAL_puts(char *str);
void TERMINAL_printf(const char *format, ...);

#endif /* IHM_TERMINAL_H_ */
