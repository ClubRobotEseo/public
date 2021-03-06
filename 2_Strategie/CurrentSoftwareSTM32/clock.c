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
#include "Supervision/SD/SD.h"
#include "Supervision/RTC.h"
#include "Supervision/Supervision.h"
#include "Supervision/Selftest.h"
#include "QS/QS_can_over_xbee.h"
#include "Supervision/Buzzer.h"
#include "actuator/act_functions.h"
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
	CLOCK_run();	//Lancement du timer pour utilisation avant le d�but du match.
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


void _ISR _T1Interrupt()
{
	static Uint16 local_time = 0;
	static Uint16 count_1sec = 0;
	global.absolute_time++;

	if(global.flags.match_started && !global.flags.match_over && !global.flags.match_suspended  && !global.flags.match_paused)	//Match commenc� et NON termin�
	{
		//Pendant le match.
		global.match_time++;

		if(global.match_time & 0x100)
			toggle_led(LED_USER);

		if(global.friend_position_lifetime)
			global.friend_position_lifetime--;
	}

	local_time++;

	if(local_time == 500)
	{
		local_time = 0;
		SELFTEST_process_500ms();
	}

	count_1sec++;
	if(count_1sec >= 1000)
	{
		count_1sec = 0;
		RTC_process_it_1sec();
	}
	SD_process_1ms();
	BUZZER_process_1ms();
	Supervision_process_1ms();
	TIMER1_AckIT();
}
