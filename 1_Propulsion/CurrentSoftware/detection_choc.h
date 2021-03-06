/*
 *	Club Robot ESEO 2015 - 2016
 *	Black & Pearl
 *
 *	Fichier : detection_choc.c
 *	Package : Carte Stratégie
 *	Description : Detection de choc avec un robot adverse entrainant une perte de notre position
 *	Auteur :
 *	Version 2015/10/14
 */

#include "QS/QS_all.h"

#include "QS/QS_sensor/QS_accelero.h"

#ifndef DETECTION_CHOC_H
	#define  DETECTION_CHOC_H

void DETECTION_CHOC_init();
void DETECTION_CHOC_process_it_tim2();
void DETECTION_CHOC_process_it_tim5();
void DETECTION_CHOC_process_main();

#endif /* ndef  DETECTION_CHOC_H */
