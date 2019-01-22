
#ifndef QS_UART_H
	#define QS_UART_H

	#include "QS_all.h"


	/*#if !((defined UART1_ON_PB6_PB7) ^ (defined UART1_ON_PA9_PA10))	//Un "non ou exclusif", ça c'est la classe !
		#error "Dans config.h -> vous devez choisir entre UART1_ON_PB6_PB7 et UART1_ON_PA9_PA10."
	#endif

	#if !((defined UART2_ON_PA2_PA3) ^ (defined UART2_ON_PD5_PD6))
		#error "Dans config.h -> vous devez choisir entre UART2_ON_PA2_PA3 et UART2_ON_PD5_PD6."
	#endif

	#if  !((defined UART3_ON_PB10_PB11) ^ (defined UART3_ON_PD8_PD9))
		#error "Dans config.h -> vous devez choisir entre UART3_ON_PB10_PB11 et UART3_ON_PD8_PD9."
	#endif*/

	#define debug_printf(text, ...)		UART_printf(UART_DEBUG, text, ##__VA_ARGS__, 0)


	typedef enum
	{
		UART1_ID = 0,
		UART2_ID,
		UART3_ID,
		UART_ID_NB
	}uart_id_e;

	/**
	 * @brief	Initialise l'USARTx - 8N1 - vitesse des bits (baudrate) indiqué en paramètre
	 * @func	void UART_init(uint8_t uart_id, uart_interrupt_mode_e mode)
	 * @param	uart_id est le numéro de l'UART concerné :
	 * 				UART1_ID
	 * 				UART2_ID
	 * 				UART3_ID
	 * @param	baudrate indique la vitesse en baud/sec
	 * 				115200	vitesse propos�e par défaut
	 * 				9600	vitesse couramment utilisée
	 * 				19200	...
	 * @post	Cette fonction initialise les broches suivante selon l'USART choisit en parametre :
	 * 				USART1 : Rx=PA10 et Tx=PA9 		ou avec remap : Rx=PB7 et Tx=PB6
	 * 				USART2 : Rx=PA3 et Tx=PA2 		ou avec remap : Rx=PD6 et Tx=PD5
	 * 				USART3 : Rx=PB11 et Tx=PB10 	ou avec remap : Rx=PD9 et Tx=PD8
	 * 				La gestion des envois et reception se fait en interruption.
	 *
	 */
	void UART_init(uart_id_e uart_id, uint32_t baudrate);

	/*
	 * @brief	D�initialise l'USARTx
	 * @param	uart_id est le numéro de l'UART concerné :	UART1_ID, UART2_ID, UART3_ID
	 */
	void UART_DeInit(uart_id_e uart_id);

	/**
	 * @func 	char UART_getc(uart_id_e uart_id))
	 * @brief	Fonction NON blocante qui retourne le dernier caractere re�u sur l'USARTx. Ou 0 si pas de caractere re�u.
	 * @param	UART_Handle : UART_Handle.Instance = USART1, USART2 ou USART6
	 * @post	Si le caractere reçu est 0, il n'est pas possible de faire la difference avec le cas où aucun caractere n'est re�u.
	 * @ret		Le caractere reçu, sur 8 bits.
	 */
	uint8_t UART_getc(uart_id_e uart_id);

	/*
	 * @func
	 * @brief	Lit "len" caract�res re�us, s'ils existent...
	 * @post	Fonction non blocante : s'il n'y a plus de caract�re re�u, cette fonction renvoit la main
	 * @ret		Le nombre de caract�res lus.
	 */
	uint32_t UART_gets(uart_id_e uart_id, uint8_t * datas, uint32_t len);

	/**
	 * @brief	Envoi un caractere sur l'USARTx. Fonction BLOCANTE si un caractere est deja en cours d'envoi.
	 * @func 	void UART_putc(UART_HandleTypeDef * UART_Handle, char c)
	 * @param	c : le caractere a envoyer
	 * @param	USARTx : USART1, USART2 ou USART6
	 */
	void UART_putc(uart_id_e uart_id, uint8_t c);

	/**
	 * @brief	Envoi une chaine de caractere sur l'USARTx. Fonction BLOCANTE si un caractere est deja en cours d'envoi.
	 * @func 	void UART_putc(UART_HandleTypeDef * UART_Handle, char c)
	 * @param	str : la chaine de caractere �  envoyer
	 * @param	USARTx : USART1, USART2 ou USART6
	 */
	void UART_puts(uart_id_e uart_id, uint8_t * str, uint32_t len);

	/**
	 * @brief	Envoi une chaine de caractere sur l'USARTx. Fonction BLOCANTE si un caractere est deja en cours d'envoi.
	 * @func 	void UART_printf(uart_id_e uart_id, const char *text, ...)
	 * @param	text : la chaine de caract�re � envoyer
	 * @param	USARTx : USART1, USART2 ou USART6
	 */
	void UART_printf(uart_id_e uart_id, const char *text, ...);

	/*
	 * @brief	Fonction permettant de savoir si le buffer de l'UART demand� est vide ou non.
	 * @ret		Retourne VRAI si un ou des caractères sont disponibles dans le buffer.
	 * @ret		Retourne FAUX si aucun caractère n'est disponible dans le buffer (le buffer est vide)
	 * @param	uart_id est le num�ro de l'UART concerné :	UART1_ID, UART2_ID, UART3_ID
	 */
	bool_e UART_data_ready(uart_id_e uart_id);

	/*
	 * @brief	Fonction permettant de récuperer le prochain caract�re re�u dans le buffer.
	 * @ret 	Retourne le prochain caractère reçu. Ou 0 si rien n'a été reçu.
	 * @post 	Le caractère renvoyé par cette fonction ne sera plus renvoyé.
	 */
	uint8_t UART_get_next_byte(uart_id_e uart_id);

	/*
	 * @brief Fonction blocante qui presente un exemple d'utilisation de ce module logiciel.
	 */
	void UART_test(void);


#endif /* ndef QS_UART_H */

