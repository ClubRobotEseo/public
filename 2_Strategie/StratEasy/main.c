/*
 *	Club Robot ESEO 2008 - 2012
 *	Archi'Tech, CHOMP, CheckNorris, Shark & Fish
 *
 *	Fichier : main.c
 *	Package : Carte Principale
 *	Description : Ordonnancement de la carte Principale
 *	Auteur : Jacen, modifié par Gonsevi
 *	Version 2012/01/14
 */

#include "main.h"
#include "QS/QS_IHM.h"

#ifdef STM32F40XX
	#include "QS/QS_lowLayer/QS_sys.h"
	#include "stm32f4xx.h"
	#include "stm32f4xx_gpio.h"
#endif

#include "QS/QS_lowLayer/QS_ports.h"
#include "QS/QS_lowLayer/QS_uart.h"
#include "QS/QS_outputlog.h"
#include "QS/QS_buttons.h"
#include "QS/QS_lowLayer/QS_adc.h"
#include "QS/QS_lowLayer/QS_rcc.h"
#include "QS/QS_who_am_i.h"

#include "clock.h"
#include "stepper_motors.h"
#include "config.h"
#include "remote.h"
#include "toolkit.h"
#include "actuators.h"
#include "secretary.h"
#include "strategy.h"
#include "actuators.h"
#include "power.h"

void tests(void)
{
	//test_bp_switchs();
	//test_leds();
}

int main(void){
	SYS_init();		// Init système

	#if MODE_TEST_GPIO
		WATCHDOG_init();
		do{
			TEST_gpioShortCircuit();
			bool_e flag;
			WATCHDOG_create_flag(10000, &flag);
			while(!flag);
		}while(1);
	#endif

	PORTS_init();	// Config des ports

	UART_init();
	//RCC_read();

	QS_WHO_AM_I_find();

	SECRETARY_init();

	GPIO_SetBits(LED_RUN);

	ADC_init();

	CLOCK_init();

	ACTUATOR_init();

	while(1)
	{
		//toggle_led(LED_RUN);
		//BUTTONS_update();

		GPIOD->BSRRL = GPIO_Pin_13;
		GPIOD->BSRRH = GPIO_Pin_13;
		BUTTONS_update();

		POWER_process_main();

		ACTUATOR_run_reset_act();

		strategy();

		REMOTE_process_main();

		ACTUATOR_process_main();

		SECRETARY_process_main();

		TOOLKIT_process_main();
	}
	return 0;
}


void process_ms(void)
{
	BUTTONS_process_it();
	//MOTOR_process_ms();
	POWER_process_ms();
}



