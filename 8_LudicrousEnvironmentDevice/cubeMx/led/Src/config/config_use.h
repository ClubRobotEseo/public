/*
 *  Club Robot ESEO 2013 - 2017
 *
 *  $Id: config_use.h 6328 2018-03-27 15:40:09Z spoiraud $
 *
 *  Package : Carte Strategie
 *  Description : Activation de modules et fonctionnalités
 *  Auteur : Jacen, Alexis
 */

#ifndef CONFIG_USE_H
#define CONFIG_USE_H


#define CAN_VERBOSE_MODE


/* UART */
#define UART_DEBUG (UART1_ID)
#define UART_DEBUG_BAUDRATE (230400)

#define UART_XBEE (UART2_ID)
#define UART_XBEE_BAUDRATE (115200)

/* CAN over Xbee */
#define USE_XBEE_OLD	1
#define XBEE_PLUGGED_ON_UART2
#define XBEE_RESET 		GPIOD, GPIO_PIN_7


#endif /* CONFIG_USE_H */
