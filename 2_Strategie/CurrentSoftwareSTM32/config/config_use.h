/*
 *  Club Robot ESEO 2013 - 2017
 *
 *  $Id: config_use.h 7084 2018-10-20 19:12:54Z vbesnard $
 *
 *  Package : Carte Strategie
 *  Description : Activation de modules et fonctionnalit�s
 *  Auteur : Jacen, Alexis
 */

#ifndef CONFIG_USE_H
#define CONFIG_USE_H

//////////////////////////////////////////////////////////////////
//-------------------------MODE ET USE--------------------------//
//////////////////////////////////////////////////////////////////


#define ACT_NO_ERROR_HANDLING				0	// Pas de gestion d'erreurs des actionneurs (pas besoin de carte actionneur dans ce mode)

#define DISABLE_CAN					0		// N'utilise pas le bus CAN
#if DISABLE_CAN
	#warning 'ATTENTION CE MODE EST STRICTEMENT INTERDIT EN MATCH NE SOYEZ PAS INCONSCIENT!'
#endif

//D�sactive la d�tection du robot sur laquelle se situe la carte.
//D�commenter l'une de ces deux lignes pour forcer la d�tection du robot.
//#define DISABLE_WHO_AM_I			SMALL_ROBOT
//#define DISABLE_WHO_AM_I			BIG_ROBOT
#ifdef DISABLE_WHO_AM_I
	#warning 'ATTENTION CE MODE EST STRICTEMENT INTERDIT EN MATCH NE SOYEZ PAS INCONSCIENT!'
#endif

#define XBEE_SIMULATION 	0 // R�pete les messages XBEE sur le CAN
#if XBEE_SIMULATION
	#warning 'ATTENTION CE MODE EST STRICTEMENT INTERDIT EN MATCH NE SOYEZ PAS INCONSCIENT!'
#endif

#define DISABLE_SECURE_GPIO_INIT		0	//Peut �tre utile sur d'anciens FdP.
#if DISABLE_SECURE_GPIO_INIT
	#warning 'ATTENTION SECURITE DU GPIO DESACTIVEE'
#endif

#define MODE_TEST_GPIO		0
#if MODE_TEST_GPIO
	#warning 'ATTENTION CE MODE EST STRICTEMENT INTERDIT EN MATCH NE SOYEZ PAS INCONSCIENT!'
#endif

#define FAST_COS_SIN		0	//Calcul rapide des cos et sin � l'aide d'un GRAND tableau de valeur

#define USE_LCD				1	//Activation de l'�cran LCD

#define USE_RTC				1	//Activation de la RTC

#define VERBOSE_MODE			//Activation du verbose

#define USE_FOE_ANALYSER	0

#define USE_ASTAR			1		//Activation de l'algorithme ASTAR, avec pathfind par polygones

#define SD_ENABLE			1		//Activation de la carte SD

#define USE_SYNC_ELEMENTS	1
#define USE_HARDFLAGS		1

#define BELGIUM_CUP			0

#define FINAL_PHASIS	0		//Mettre 1 ou 0...

#define USE_SCAN_DISTRI 1

#define USE_SCAN_HOKUYO	0

#define SOUTH_PROTECTION_WITHOUT_ZONE 	1

//////////////////////////////////////////////////////////////////
//----------------------------QS--------------------------------//
//////////////////////////////////////////////////////////////////

/* Define pour identifier la carte */
	#define I_AM_CARTE_STRAT

/* Il faut choisir � quelle frequence on fait tourner la STM32 */
	#define HCLK_FREQUENCY_HZ		168000000	//168Mhz, Max: 168Mhz
	#define PCLK1_FREQUENCY_HZ		42000000	//42Mhz,  Max: 42Mhz
	#define PCLK2_FREQUENCY_HZ		84000000	//84Mhz,  Max: 84Mhz
	#define CPU_EXTERNAL_CLOCK_HZ	8000000		//8Mhz,   Fr�quence de l'horloge externe

/* R�glages WATCHDOG */
	#define USE_WATCHDOG
		#define WATCHDOG_TIMER 3
		#define WATCHDOG_MAX_COUNT 5
		#define WATCHDOG_QUANTUM 1

/* R�glages SPI */
	#define USE_SPI2

/* R�glages I2C */
	#define USE_I2C2

/* R�glages CAN */
	#define USE_CAN
		#define CAN_BUF_SIZE		32		//Nombre de messages CAN conserv�s pour traitement hors interuption
		#define CAN_SEND_TIMEOUT_ENABLE

/* R�glages UART */
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

	#if USE_LCD
		#define USE_UART3
			#define UART3_BAUDRATE				9600

			#define USE_UART3_TX_BUFFER
				#define UART3_TX_BUFFER_SIZE 	128

			#define USE_UART3_RX_BUFFER
				#define UART3_RX_BUFFER_SIZE	256
	#endif

/* R�glages Boutons */
	#define USE_BUTTONS
	//#define BUTTONS_TIMER 2			//Utilise le timer 2 pour les boutons
	#define BUTTONS_TIMER_USE_WATCHDOG

/* R�glages PWM */
	#define USE_PWM_MODULE
		#define PWM_FREQ	50000
		#define PWM_TIM_BUZZER	PWM_TIM_8
		#define PWM_CH_BUZZER	PWM_CH_4

/* R�glages de la carte Mosfets */
	#define USE_MOSFETS_MODULE
	//	#define USE_MOSFET_1

/* R�glages LCD via UART */
	#if USE_LCD
		#define USE_LCD_OVER_UART		1
			#define LCD_OVER_UART__UART_ID	3
	#endif



/* R�glages entr�es analogiques */
	#define USE_AN8				// Capteur mesure 24V
		#define ADC_CHANNEL_MEASURE24		ADC_8

/* R�glages XBEE */
	#define USE_XBEE_OLD		1
		#define XBEE_PLUGGED_ON_UART2

/* R�glages FIFO */
	#define USE_FIFO

/* R�glages Capteur couleur */
	#define USE_CW_SENSOR

#define USE_MCP23017

/* R�capitulatif TIMERs :
 * TIMER 1 : Clock			(clock.c/h)
 * TIMER 2 : ...
 * TIMER 3 : Watchdog		(QS_watchdog.c/h)
 * TIMER 4 : ...
 * TIMER 5 : ...
 */

/* R�capitulatif IT priorit� :
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
