/*
 *	Club Robot ESEO 2010 - 2011
 *	Check Norris
 *
 *	Fichier : clock.c
 *	Package : Actionneur
 *	Description : Gestion des IT
 *  Auteurs : Jacen
 *  Version 20100412
 */

#include "it.h"
#include "ActManager.h"
#include "QS/QS_buttons.h"
#include "QS/QS_actuator/QS_DCMotor2.h"
#include "QS/QS_actuator/QS_DCMotorSpeed.h"
#include "QS/QS_lowLayer/QS_ports.h"
#include "QS/QS_sensor/QS_rpm_sensor.h"


#ifdef IT_TIMER_ID
	#define TIMER_SRC_TIMER_ID IT_TIMER_ID
#endif
#ifdef IT_USE_WATCHDOG
	#define TIMER_SRC_USE_WATCHDOG
#endif

#include "QS/QS_lowLayer/QS_setTimerSource.h"


#if defined(IT_UPDATE_BUTTONS_PRESS_TIME) && defined(BUTTONS_TIMER)
#error "IT ne doit pas mettre a jour les temps d'appui des boutons si un autre timer est utilisé !"
#endif


#define IT_TIME 10


void IT_init()
{
	static bool_e initialized = FALSE;
	if(initialized)
		return;
	initialized = TRUE;

#if defined(IT_UPDATE_BUTTONS_PRESS_TIME)
	BUTTONS_init();
#endif
	TIMER_SRC_TIMER_init();
	TIMER_SRC_TIMER_start_ms(IT_TIME);
}

#include "QS/QS_lowLayer/QS_uart.h"
void TIMER_SRC_TIMER_interrupt()
{
	#ifdef USE_DCMOTOR2
		DCM_process_it();
	#endif

	#ifdef USE_RPM_SENSOR
		RPM_SENSOR_process_it();
	#endif

	#ifdef USE_DC_MOTOR_SPEED
		DC_MOTOR_SPEED_process_it();
	#endif

	ACTMGR_process_it();

	toggle_led(LED_RUN);

	#if defined(IT_UPDATE_BUTTONS_PRESS_TIME)
		BUTTONS_process_it();
	#endif

	TIMER_SRC_TIMER_resetFlag();
}

void _T6Interrupt() {
	RPM_SENSOR_inc_counter_us();
	TIMER6_AckIT();
}
