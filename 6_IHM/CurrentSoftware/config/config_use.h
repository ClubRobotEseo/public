/*
 *  Club Robot ESEO 2013 - 2014
 *
 *  $Id: config_use.h 2706 2014-10-04 13:06:44Z aguilmet $
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

#define MODE_TEST_GPIO		0
#if MODE_TEST_GPIO
	#warning 'ATTENTION CE MODE EST STRICTEMENT INTERDIT EN MATCH NE SOYEZ PAS INCONSCIENT!'
#endif

#define VERBOSE_MODE			//Activation du verbose

#define CAN_VERBOSE_MODE		//Activation de la verbosit� des messages CAN

//////////////////////////////////////////////////////////////////
//----------------------------QS--------------------------------//
//////////////////////////////////////////////////////////////////

/* Define pour identifier la carte */
	#define I_AM_CARTE_IHM

/* Il faut choisir � quelle frequence on fait tourner le PIC */
	#define HCLK_FREQUENCY_HZ		168000000	//168Mhz, Max: 168Mhz
	#define PCLK1_FREQUENCY_HZ		42000000	//42Mhz,  Max: 42Mhz
	#define PCLK2_FREQUENCY_HZ		84000000	//84Mhz,  Max: 84Mhz
	#define CPU_EXTERNAL_CLOCK_HZ	8000000		//8Mhz,   Fr�quence de l'horloge externe

/* CAN */
	#define USE_CAN
		#define CAN_BUF_SIZE		32	//Nombre de messages CAN conserv�s pour traitement hors interuption
		#define CAN_SEND_TIMEOUT_ENABLE

/* R�glages LCD_OVER_UART */
	#define USE_LCD_OVER_UART		1
		#define LCD_OVER_UART__UART_ID 3
		#define LCD_OVER_UART__IHM_MODE


/* UART */
	#define USE_UART1
		#define UART1_BAUDRATE				230400

		#define USE_UART1_TX_BUFFER
			#define UART1_TX_BUFFER_SIZE 	128

		#define USE_UART1_RX_BUFFER
			#define UART1_RX_BUFFER_SIZE 	32


	#if USE_LCD_OVER_UART
		#define USE_UART3
			#define UART3_BAUDRATE				9600

			#define USE_UART3_TX_BUFFER
				#define UART3_TX_BUFFER_SIZE 	128

			#define USE_UART3_RX_BUFFER
				#define UART3_RX_BUFFER_SIZE 	512
	#endif


/* R�glages SPI */
	#define USE_SPI2
		#define SPI2_ON_DMA

/* R�glages watchdog */
	#define USE_WATCHDOG
		#define WATCHDOG_TIMER 3
		#define WATCHDOG_MAX_COUNT 5
		#define WATCHDOG_QUANTUM 1

/* R�glages entr�es analogiques */
	#define USE_AN5		// Mesure 12V hokuyo
	#define USE_AN6		// Mesure 24V puissance
	#define USE_AN10	// Mesure 24V permanence

/* R�glages LCD */
	#define LCD_SPI SPI2
	#define LCD_TOUCH_SPI SPI2
	#define USE_LCD_DMA
	#define USE_IRQ_TOUCH_VALIDATION

/* R�capitulatif TIMERs :
 * TIMER 1 : IT				(it.c/h)
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
 *  9 : TIMER_1
 *  3 : USART_1
 *  1 : CAN
 *  0 : Systick
 */

#endif /* CONFIG_USE_H */
