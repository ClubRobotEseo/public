/*
 *	Club Robot ESEO 2008 - 2009
 *	Archi'Tech
 *
 *	Fichier : clock.c
 *	Package : Carte Principale
 *	Description : 	Fonctions de gestion de l'horloge
 *	Auteur : Jacen
 *	Version 20090322
 */


#include "clock.h"
#include "QS/QS_can_over_xbee.h"
#include "QS/QS_IHM.h"
#include "QS/QS_lowLayer/QS_ports.h"
#include "QS/QS_lowLayer/QS_timer.h"

void CLOCK_run();

void CLOCK_init()
{
	static bool_e initialized = FALSE;
	if(initialized)
		return;
	initialized = TRUE;

	TIMER_init();
	TIMER1_stop();
	CLOCK_run();	//Lancement du timer pour utilisation avant le début du match.
	global.match_time = 0;
	global.absolute_time = 0;
	global.calibrate_time_x=0;
	global.calibrate_time_x=0;
}

void CLOCK_run()
{
	TIMER1_run_us(1000);
}


void CLOCK_run_match()
{
	TIMER1_stop();
	TIMER1_run_us(1000);
}

#define MATCH_DURATION	100000

void _ISR _T1Interrupt()
{
	global.absolute_time++;

	if(global.flags.match_started && !global.flags.match_over && !global.flags.match_suspended)	//Match commencé et NON terminé
	{
		//Pendant le match.
		global.match_time++;

		if(global.match_time & 0x100)
			toggle_led(LED_USER);

		if(global.match_time >= MATCH_DURATION)
		{
			global.flags.match_over = TRUE;
			GPIO_WriteBit(LED_USER, 1);
		}
	}

	process_ms();
	TIMER1_AckIT();
}
