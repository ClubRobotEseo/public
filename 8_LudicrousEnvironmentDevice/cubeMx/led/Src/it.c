/*
 *	Club Robot ESEO 2018
 *
 *	Fichier : it.c
 *	Package : Carte LED
 *	Description : Fonctions de traitement des interruptions.
 *  Auteur : Valentin
 *  Version 20180407
 */

#include "main.h"
#include "stm32h7xx_hal.h"
#include "QS/QS_all.h"
#include "QS/QS_can_over_xbee.h"

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	// Interruption toutes les 1 s
	if(htim->Instance == TIM3) {
		// Gestion du temps de match
		if(global.match_time != 0) {
			global.match_time--;
		}
		HAL_GPIO_TogglePin(blue_led_GPIO_Port, blue_led_Pin);
	}

	// Interruption toutes les 1 ms
	if(htim->Instance == TIM4) {
		// Gestion du XBee
		CAN_over_XBee_process_ms();
	}
}
