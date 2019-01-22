/*
 *  Club Robot ESEO 2013 - 2017
 *
 *  $Id: config_use.h 6878 2018-06-02 09:25:06Z vbesnard $
 *
 *  Package : Carte Strategie
 *  Description : Activation de modules et fonctionnalités
 *  Auteur : Jacen, Alexis
 */

#ifndef CONFIG_USE_H
#define CONFIG_USE_H

//////////////////////////////////////////////////////////////////
//-------------------------MODE ET USE--------------------------//
//////////////////////////////////////////////////////////////////

#define SIMULATION_VIRTUAL_PERFECT_ROBOT		0	// Afin que les actionneurs et certaines fonction en strat renvoi true
#if SIMULATION_VIRTUAL_PERFECT_ROBOT
	#define ACT_NO_ERROR_HANDLING		1
	#warning 'ATTENTION CE MODE EST STRICTEMENT INTERDIT EN MATCH NE SOYEZ PAS INCONSCIENT!'
#else
	#define ACT_NO_ERROR_HANDLING		0
#endif

#define DISABLE_CAN					0		// N'utilise pas le bus CAN
#if DISABLE_CAN
	#warning 'ATTENTION CE MODE EST STRICTEMENT INTERDIT EN MATCH NE SOYEZ PAS INCONSCIENT!'
#endif

//Désactive la détection du robot sur laquelle se situe la carte.
//Décommenter l'une de ces deux lignes pour forcer la détection du robot.
#define DISABLE_WHO_AM_I			SMALL_ROBOT
//#define DISABLE_WHO_AM_I			BIG_ROBOT


#define XBEE_SIMULATION 	0 // Répete les messages XBEE sur le CAN
#if XBEE_SIMULATION
	#warning 'ATTENTION CE MODE EST STRICTEMENT INTERDIT EN MATCH NE SOYEZ PAS INCONSCIENT!'
#endif

#define DISABLE_SECURE_GPIO_INIT		1	//Peut être utile sur d'anciens FdP.

#define MODE_TEST_GPIO		0
#if MODE_TEST_GPIO
	#warning 'ATTENTION CE MODE EST STRICTEMENT INTERDIT EN MATCH NE SOYEZ PAS INCONSCIENT!'
#endif

#define FAST_COS_SIN		0	//Calcul rapide des cos et sin à l'aide d'un GRAND tableau de valeur

#define USE_LCD				0	//Activation de l'écran LCD

#define USE_RTC				0	//Activation de la RTC

#define VERBOSE_MODE			//Activation du verbose

#define USE_FOE_ANALYSER	0

#define USE_ASTAR			0		//Activation de l'algorithme ASTAR, avec pathfind par polygones

#define SD_ENABLE			0		//Activation de la carte SD

#define USE_SYNC_ELEMENTS	0
#define USE_HARDFLAGS		0

#define BELGIUM_CUP			0

#define FINAL_PHASIS	0		//Mettre 1 ou 0...

#define USE_SCAN_DISTRI 0

#define USE_SCAN_HOKUYO	0

#define SOUTH_PROTECTION_WITHOUT_ZONE 	0

/* Servo-Moteurs RX24 */
	#define USE_RX24_SERVO	1
	#define RX24_NUMBER 	80
	#define RX24_INSTRUCTION_BUFFER_SIZE 200
	#define RX24_TIMER_ID 4
	#define RX24_STATUS_RETURN_MODE RX24_STATUS_RETURN_ALWAYS	//Permet de savoir quand le RX24 n'est pas bien connecté ou ne répond pas.
	#define RX24_STATUS_RETURN_CHECK_CHECKSUM
	#define RX24_UART_ID 3

//////////////////////////////////////////////////////////////////
//----------------------------QS--------------------------------//
//////////////////////////////////////////////////////////////////

/* Define pour identifier la carte */
	#define I_AM_CARTE_STRATEASY

/* Il faut choisir à quelle frequence on fait tourner la STM32 */
	#define HCLK_FREQUENCY_HZ		168000000	//168Mhz, Max: 168Mhz
	#define PCLK1_FREQUENCY_HZ		42000000	//42Mhz,  Max: 42Mhz
	#define PCLK2_FREQUENCY_HZ		84000000	//84Mhz,  Max: 84Mhz
	#define CPU_EXTERNAL_CLOCK_HZ	8000000		//8Mhz,   Fréquence de l'horloge externe

/* Réglages WATCHDOG */
	#define USE_WATCHDOG
		#define WATCHDOG_TIMER 3
		#define WATCHDOG_MAX_COUNT 5
		#define WATCHDOG_QUANTUM 1

/* Réglages SPI */
	#define USE_SPI2

/* Réglages I2C */
	#define USE_I2C2

/* Réglages CAN */
	#define USE_CAN
		#define CAN_BUF_SIZE		32		//Nombre de messages CAN conservés pour traitement hors interuption
		#define CAN_SEND_TIMEOUT_ENABLE

/* Réglages UART */
	#define USE_UART1
		#define UART1_BAUDRATE				230400

		#define USE_UART1_TX_BUFFER
			#define UART1_TX_BUFFER_SIZE 	1280

		#define USE_UART1_RX_BUFFER
			#define UART1_RX_BUFFER_SIZE 	32

	#define USE_UART2
		#define UART2_BAUDRATE				9600

		#define USE_UART2_TX_BUFFER
			#define UART2_TX_BUFFER_SIZE 	128

		#define USE_UART2_RX_BUFFER
			#define UART2_RX_BUFFER_SIZE 	32


//	#define USE_UART3
//		#define UART3_BAUDRATE				9600
//
//		#define USE_UART3_TX_BUFFER
//			#define UART3_TX_BUFFER_SIZE 	128
//
//		#define USE_UART3_RX_BUFFER
//			#define UART3_RX_BUFFER_SIZE	256


/* Réglages Boutons */
	#define USE_BUTTONS
	//#define BUTTONS_TIMER 2			//Utilise le timer 2 pour les boutons
	#define BUTTONS_TIMER_USE_WATCHDOG

/* Réglages PWM */
	#define USE_PWM_MODULE
		#define PWM_FREQ	50000
		#define PWM_TIM_BUZZER	PWM_TIM_8
		#define PWM_CH_BUZZER	PWM_CH_4

/* Réglages de la carte Mosfets */
	//#define USE_MOSFETS_MODULE
	//	#define USE_MOSFET_1

/* Réglages LCD via UART */
	#if USE_LCD
		#define USE_LCD_OVER_UART		0
			#define LCD_OVER_UART__UART_ID	3
	#endif



/* Réglages entrées analogiques */
	//#define USE_AN8				// Capteur mesure 24V

	//#define USE_AN12			// Couleur rouge

	//#define USE_AN14			// Couleur verte

	//#define USE_AN11			// Couleur bleue

	#define USE_AN14			//Mesure Vbat 24V

/* Réglages XBEE */
	#define USE_XBEE_OLD		0
		#define XBEE_PLUGGED_ON_UART2

/* Réglages FIFO */
	#define USE_FIFO

/* Réglages Capteur couleur */
	//#define USE_CW_SENSOR

//#define USE_MCP23017

/* Récapitulatif TIMERs :
 * TIMER 1 : Clock			(clock.c/h)
 * TIMER 2 : ...
 * TIMER 3 : Watchdog		(QS_watchdog.c/h)
 * TIMER 4 : ...
 * TIMER 5 : ...
 */

/* Récapitulatif IT priorité :
 * 15 : TIMER_5
 * 14 : TIMER_4
 * 13 : TIMER_3
 * 11 : TIMER_2
 * 10 : USART_3
 *  9 : TIMER_1
 *  7 : I2C
 *  3 : USART_2
 *  3 : USART_1
 *  1 : CAN
 */

#endif /* CONFIG_USE_H */
