/*
 *  Club Robot ESEO 2013 - 2014
 *
 *  $Id: config_use.h 6850 2018-05-16 19:24:50Z aguilmet $
 *
 *  Package : Carte Actionneur
 *  Description : Activation de modules et fonctionnalités
 *  Auteur : Jacen, Alexis
 */

#ifndef CONFIG_USE_H
#define CONFIG_USE_H
#include "config_global.h"

//////////////////////////////////////////////////////////////////
//-------------------------MODE ET USE--------------------------//
//////////////////////////////////////////////////////////////////

#define MODE_TEST_GPIO	0
#if MODE_TEST_GPIO
	#warning 'ATTENTION CE MODE EST STRICTEMENT INTERDIT EN MATCH NE SOYEZ PAS INCONSCIENT!'
#endif

#define DISABLE_CAN		0	//N'utilise pas le bus CAN
#if DISABLE_CAN
	#warning 'ATTENTION CE MODE EST STRICTEMENT INTERDIT EN MATCH NE SOYEZ PAS INCONSCIENT!'
#endif

#define FAST_COS_SIN			0	//Calcul rapide des cos et sin à l'aide d'un GRAND tableau de valeur

#define VERBOSE_MODE			//Activation du verbose

#define CAN_VERBOSE_MODE		//Activation de la verbosité des messages CAN

#define DISABLE_SECURE_GPIO_INIT		0
#if DISABLE_SECURE_GPIO_INIT
	#warning 'ATTENTION SECURITE DU GPIO DESACTIVER'
#endif

//////////////////////////////////////////////////////////////////
//----------------------------QS--------------------------------//
//////////////////////////////////////////////////////////////////

// Définir les configurations QS propre à chaque robot dans config_big/config_use.h ou config_small/config_use.h !


/* Define pour identifier la carte */
	#define I_AM_CARTE_ACT

/* Il faut choisir à quelle frequence on fait tourner le PIC */
	#define HCLK_FREQUENCY_HZ		168000000	//168Mhz, Max: 168Mhz
	#define PCLK1_FREQUENCY_HZ		42000000	//42Mhz,  Max: 42Mhz
	#define PCLK2_FREQUENCY_HZ		84000000	//84Mhz,  Max: 84Mhz
	#define CPU_EXTERNAL_CLOCK_HZ	8000000		//8Mhz,   Fréquence de l'horloge externe

/* IT */
	#define IT_USE_WATCHDOG
	#define IT_TIMER_ID 4
	#define IT_UPDATE_BUTTONS_PRESS_TIME

/* Réglages UART */
	#define USE_UART1
		#define UART1_BAUDRATE				230400

		#define USE_UART1_TX_BUFFER
			#define UART1_TX_BUFFER_SIZE 	128

		#define USE_UART1_RX_BUFFER
			#define UART1_RX_BUFFER_SIZE 	32

//	#define USE_UART6
//		#define UART6_BAUDRATE				19200
//
//		#define USE_UART6_TX_BUFFER
//			#define UART6_TX_BUFFER_SIZE 	128
//
//		#define USE_UART6_RX_BUFFER
//			#define UART6_RX_BUFFER_SIZE 	128

/* CAN */
	#define USE_CAN
	#define CAN_BUF_SIZE	32
	#define CAN_SEND_TIMEOUT_ENABLE
	#define QS_CAN_RX_IT_PRI 2

/* Bouton */
	#define I_ASSUME_I_WILL_CALL_BUTTONS_PROCESS_IT_ON_MY_OWN //Fait par clock.h/c
	#define USE_BUTTONS

/* Réglages watchdog */
	#define USE_WATCHDOG
	#define WATCHDOG_TIMER 3
	#define WATCHDOG_MAX_COUNT 5
	#define WATCHDOG_QUANTUM 1

/* Servo-Moteurs AX12 */
	#define USE_AX12_SERVO
	#define AX12_NUMBER 80
	#define AX12_INSTRUCTION_BUFFER_SIZE 200
	#define AX12_TIMER_ID 2
	#define AX12_STATUS_RETURN_MODE AX12_STATUS_RETURN_ALWAYS	//Permet de savoir quand l'AX12 n'est pas bien connecté ou ne répond pas.
	#define AX12_STATUS_RETURN_CHECK_CHECKSUM
	#define AX12_UART_ID 3

/* Servo-Moteurs RX24 */
	#define USE_RX24_SERVO
	#define RX24_NUMBER 80
	#define RX24_INSTRUCTION_BUFFER_SIZE 200
	#define RX24_TIMER_ID 1
	#define RX24_STATUS_RETURN_MODE RX24_STATUS_RETURN_ALWAYS	//Permet de savoir quand le RX24 n'est pas bien connecté ou ne répond pas.
	#define RX24_STATUS_RETURN_CHECK_CHECKSUM
	#define RX24_UART_ID 2

/*External IT */
	#define USE_EXTERNAL_IT

/*RPM Sensors */
	#define USE_RPM_SENSOR
		#define RPM_SENSOR_NB_SENSOR	1

/* Réglages mosfet */
//	#define USE_MOSFETS_MODULE
//		#define USE_MOSFET_1
//		#define USE_MOSFET_2
//		#define USE_MOSFET_3
//		#define USE_MOSFET_4
//		#define USE_MOSFET_5
//		#define USE_MOSFET_6
//		#define USE_MOSFET_7
//		#define USE_MOSFET_8
		//#define USE_MOSFET_MULTI

/* Réglages ADC */
	#define USE_AN9
	#define ADC_MOSFETS_5V		ADC_9

/* Réglages PWM */
	#define USE_PWM_MODULE
		#define PWM_FREQ	50000

/* Réglages DC Speed */
	#define USE_DC_MOTOR_SPEED
		#define DC_MOTOR_SPEED_NUMBER 		1
		#define DC_MOTOR_SPEED_TIME_PERIOD	10
	#define USE_DCMOTOR2
		#define DCM_NUMBER					1
		#define DCM_TIME_PERIOD				10
		#define DCMOTOR_NB_POS 				7


/* Réglages QEI */
	#define USE_QUEI1

/* Asservissement en position/vitesse de moteurs CC */
	/* déclarer l'utilisation du pilote */
	//#define USE_DCMOTOR2
	/* définir le nombre d'actionneurs asservis */
	//#define DCM_NUMBER			0
	/* Période d'asservisement (en ms) */
	//#define DCM_TIME_PERIOD		10
	/* nombre maximum de positions à gérer par moteur */
	//#define DCMOTOR_NB_POS		1


/* Récapitulatif TIMERs :
 * TIMER 1 : RX24			(QS_rx24.c/h)
 * TIMER 2 : AX12			(QS_ax12.c/h)
 * TIMER 3 : Watchdog		(QS_watchdog.c/h)
 * TIMER 4 : Clock			(clock.c/h)
 * TIMER 5 : Servo			(QS_servo.c/h)
 */

/* Récapitulatif IT priorité :
 * 15 : TIMER_5
 * 14 : TIMER_4
 * 13 : TIMER_3
 * 11 : TIMER_2
 *  9 : TIMER_1
 *  3 : USART_1
 *  3 : USART_2
 *  3 : USART_3
 *  2 : CAN
 *  0 : Systick
 */

#endif /* CONFIG_USE_H */
