/*
 *	Club Robot ESEO 2014
 *
 *	Fichier : led.h
 *	Package : Carte IHM
 *	Description : Contr�le les leds
 *	Auteur : Anthony
 *	Version 2012/01/14
 */

#ifndef LED_H
#define LED_H

#include "QS/QS_all.h"

#include "QS/QS_IHM.h"
#include "QS/QS_CANmsgList.h"

void LEDS_init();
void LEDS_process_it(Uint8 ms);
void LEDS_get_msg(CAN_msg_t *msg);
void LEDS_set_error(ihm_error_e ihm_error, bool_e state);
void LEDS_set_COLOR(led_color_e color);


#endif /* ndef LED_H */
