#include <stdarg.h>
#include <string.h>
#include "QS_uart.h"
#include "QS_macro.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_uart.h"

//Les buffers de réception accumulent les données reçues, dans la limite de leur taille.
//Les emplacement occupés par les octets reçus sont libérés dès qu'on les consulte.
#define BUFFER_RX_SIZE	128

static UART_HandleTypeDef UART_HandleStructure[UART_ID_NB];	//Ce tableau contient les structures qui sont utilisées pour piloter chaque UART avec la librairie HAL.
static const USART_TypeDef * instance_array[UART_ID_NB] = {USART1, USART2, USART3};
static const IRQn_Type nvic_irq_array[UART_ID_NB] = {USART1_IRQn, USART2_IRQn, USART3_IRQn};

//Buffers
static uint8_t buffer_rx[UART_ID_NB][BUFFER_RX_SIZE];
static uint8_t buffer_rx_write_index[UART_ID_NB] = {0};
static uint32_t buffer_rx_read_index[UART_ID_NB] = {0};
static volatile bool_e buffer_rx_data_ready[UART_ID_NB];


/**
 * @brief	Initialise l'USARTx - 8N1 - vitesse des bits (baudrate) indiqué en paramètre
 * @func	void UART_init(uint8_t uart_id, uart_interrupt_mode_e mode)
 * @param	uart_id est le numéro de l'UART concerné :
 * 				UART1_ID
 * 				UART2_ID
 * 				UART3_ID
 * @param	baudrate indique la vitesse en baud/sec
 * 				115200	vitesse proposée par défaut
 * 				9600	vitesse couramment utilisée
 * 				19200	...
 * @post	Cette fonction initialise les broches suivante selon l'USART choisit en parametre :
 * 				USART1 : Rx=PA10 et Tx=PA9 		ou avec remap : Rx=PB7 et Tx=PB6
 * 				USART2 : Rx=PA3 et Tx=PA2 		ou avec remap : Rx=PD6 et Tx=PD5
 * 				USART3 : Rx=PB11 et Tx=PB10 	ou avec remap : Rx=PD9 et Tx=PD8
 * 				La gestion des envois et reception se fait en interruption.
 *
 */
void UART_init(uart_id_e uart_id, uint32_t baudrate)
{
	assert(baudrate > 1000);
	assert(uart_id < UART_ID_NB);

	buffer_rx_read_index[uart_id] = 0;
	buffer_rx_write_index[uart_id] = 0;
	buffer_rx_data_ready[uart_id] = FALSE;
	/* USARTx configured as follow:
		- Word Length = 8 Bits
		- One Stop Bit
		- No parity
		- Hardware flow control disabled (RTS and CTS signals)
		- Receive and transmit enabled
		- OverSampling: enable
	*/
	UART_HandleStructure[uart_id].Instance = (USART_TypeDef*)instance_array[uart_id];
	UART_HandleStructure[uart_id].Init.BaudRate = baudrate;
	UART_HandleStructure[uart_id].Init.WordLength = UART_WORDLENGTH_8B;//
	UART_HandleStructure[uart_id].Init.StopBits = UART_STOPBITS_1;//
	UART_HandleStructure[uart_id].Init.Parity = UART_PARITY_NONE;//
	UART_HandleStructure[uart_id].Init.HwFlowCtl = UART_HWCONTROL_NONE;//
	UART_HandleStructure[uart_id].Init.Mode = UART_MODE_TX_RX;//
	UART_HandleStructure[uart_id].Init.OverSampling = UART_OVERSAMPLING_16;//

	/*On applique les parametres d'initialisation ci-dessus */
	HAL_UART_Init(&UART_HandleStructure[uart_id]);

	/*Activation de l'UART */
	__HAL_UART_ENABLE(&UART_HandleStructure[uart_id]);

	// On fixe les priorités des interruptions de l'usart PreemptionPriority = 0, SubPriority = 1 et on autorise les interruptions
	HAL_NVIC_SetPriority(nvic_irq_array[uart_id] , 1, 1);
	HAL_NVIC_EnableIRQ(nvic_irq_array[uart_id]);
	HAL_UART_Receive_IT(&UART_HandleStructure[uart_id],&buffer_rx[uart_id][buffer_rx_write_index[uart_id]],1);	//Activation de la réception d'un caractère

	//Config LibC: no buffering
	setvbuf(stdout, NULL, _IONBF, 0 );
	setvbuf(stderr, NULL, _IONBF, 0 );
	setvbuf(stdin, NULL, _IONBF, 0 );
}


/*
 * @brief	Déinitialise l'USARTx
 * @param	uart_id est le numéro de l'UART concerné :	UART1_ID, UART2_ID, UART3_ID
 */
void UART_DeInit(uart_id_e uart_id)
{
	assert(uart_id < UART_ID_NB);
	HAL_UART_DeInit(&UART_HandleStructure[uart_id]);
}

/*
 * @brief	Fonction permettant de savoir si le buffer de l'UART demandé est vide ou non.
 * @ret		Retourne VRAI si un ou des caractères sont disponibles dans le buffer.
 * @ret		Retourne FAUX si aucun caractère n'est disponible dans le buffer (le buffer est vide)
 * @param	uart_id est le numéro de l'UART concerné :	UART1_ID, UART2_ID, UART3_ID
 */
bool_e UART_data_ready(uart_id_e uart_id)
{
	assert(uart_id < UART_ID_NB);
	return buffer_rx_data_ready[uart_id];
}

/*
 * @brief	Fonction permettant de récupérer le prochain caractère reçu dans le buffer.
 * @ret 	Retourne le prochain caractère reçu. Ou 0 si rien n'a été reçu.
 * @post 	Le caractère renvoyé par cette fonction ne sera plus renvoyé.
 */
uint8_t UART_get_next_byte(uart_id_e uart_id)
{
	uint8_t ret;
	assert(uart_id < UART_ID_NB);

	if(!buffer_rx_data_ready[uart_id])	//N'est jamais sensé se produire si l'utilisateur vérifie que UART_data_ready() avant d'appeler UART_get_next_byte()
		return 0;

	ret =  buffer_rx[uart_id][buffer_rx_read_index[uart_id]];
	buffer_rx_read_index[uart_id] = (buffer_rx_read_index[uart_id] + 1) % BUFFER_RX_SIZE;

	//Section critique durant laquelle on désactive les interruptions... pour éviter une mauvaise préemption.
	NVIC_DisableIRQ(nvic_irq_array[uart_id]);
	if (buffer_rx_write_index[uart_id] == buffer_rx_read_index[uart_id])
		buffer_rx_data_ready[uart_id] = FALSE;
	NVIC_EnableIRQ(nvic_irq_array[uart_id]);
	return ret;
}


/**
 * @func 	char UART_getc(uart_id_e uart_id))
 * @brief	Fonction NON blocante qui retourne le dernier caractere reçu sur l'USARTx. Ou 0 si pas de caractere reçu.
 * @param	UART_Handle : UART_Handle.Instance = USART1, USART2 ou USART6
 * @post	Si le caractere reçu est 0, il n'est pas possible de faire la difference avec le cas où aucun caractere n'est reçu.
 * @ret		Le caractere reçu, sur 8 bits.
 */
uint8_t UART_getc(uart_id_e uart_id)
{
	return UART_get_next_byte(uart_id);
}


/*
 * @func
 * @brief	Lit "len" caractères reçus, s'ils existent...
 * @post	Fonction non blocante : s'il n'y a plus de caractère reçu, cette fonction renvoit la main
 * @ret		Le nombre de caractères lus.
 */
uint32_t UART_gets(uart_id_e uart_id, uint8_t * datas, uint32_t len)
{
	uint32_t i;
	for(i=0; i<len ; i++)
	{
		if(UART_data_ready(uart_id))
			datas[i] = UART_get_next_byte(uart_id);
		else
			break;
	}
	return i;
}

/**
 * @brief	Envoi un caractere sur l'USARTx. Fonction BLOCANTE si un caractere est deja en cours d'envoi.
 * @func 	void UART_putc(UART_HandleTypeDef * UART_Handle, char c)
 * @param	c : le caractere a envoyer
 * @param	USARTx : USART1, USART2 ou USART6
 */
void UART_putc(uart_id_e uart_id, uint8_t c)
{
	HAL_StatusTypeDef state;
	assert(uart_id < UART_ID_NB);
	do
	{
		NVIC_DisableIRQ(nvic_irq_array[uart_id]);
		state = HAL_UART_Transmit_IT(&UART_HandleStructure[uart_id], &c, 1);
		NVIC_EnableIRQ(nvic_irq_array[uart_id]);
	}while(state == HAL_BUSY);
}

/**
 * @brief	Envoi une chaine de caractere sur l'USARTx. Fonction BLOCANTE si un caractere est deja en cours d'envoi.
 * @func 	void UART_puts(uart_id_e uart_id, uint8_t * str, uint32_t len)
 * @param	str : la chaine de caractère à envoyer
 * @param	USARTx : USART1, USART2 ou USART6
 */
void UART_puts(uart_id_e uart_id, uint8_t * str, uint32_t len)
{
	HAL_StatusTypeDef state;
	HAL_UART_StateTypeDef uart_state;
	assert(uart_id < UART_ID_NB);
	if(!len)
		len = strlen((char*)str);
	do
	{
		NVIC_DisableIRQ(nvic_irq_array[uart_id]);
		state = HAL_UART_Transmit_IT(&UART_HandleStructure[uart_id], str, (uint16_t)len);
		NVIC_EnableIRQ(nvic_irq_array[uart_id]);
	}while(state == HAL_BUSY);

	do{
		uart_state = HAL_UART_GetState(&UART_HandleStructure[uart_id]);
	}while(uart_state == HAL_UART_STATE_BUSY_TX || uart_state == HAL_UART_STATE_BUSY_TX_RX);	//Blocant.
}

/**
 * @brief	Envoi une chaine de caractere sur l'USARTx. Fonction BLOCANTE si un caractere est deja en cours d'envoi.
 * @func 	void UART_printf(uart_id_e uart_id, const char *text, ...)
 * @param	text : la chaine de caractère à envoyer
 * @param	USARTx : USART1, USART2 ou USART6
 */
void UART_printf(uart_id_e uart_id, const char *text, ...) {
	char buffer[512];

	va_list args_list;
	va_start(args_list, text);
	vsnprintf(buffer, 512, text, args_list);
	va_end(args_list);

	UART_puts(uart_id, (uint8_t*)buffer, strlen(buffer));
}

/*
 * @brief Fonction blocante qui présente un exemple d'utilisation de ce module logiciel.
 */
void UART_test(void)
{
	UART_init(UART1_ID,115200);
	UART_init(UART2_ID,115200);
	UART_init(UART3_ID,115200);
	uint8_t c;
	while(1)
	{
		if(UART_data_ready(UART1_ID))
		{
			c = UART_get_next_byte(UART1_ID);
			UART_putc(UART1_ID,c);					//Echo du caractère reçu sur l'UART 1.
		}

		if(UART_data_ready(UART2_ID))
		{
			c = UART_get_next_byte(UART2_ID);
			UART_putc(UART2_ID,c);					//Echo du caractère reçu sur l'UART 2.
		}

		if(UART_data_ready(UART3_ID))
		{
			c = UART_get_next_byte(UART3_ID);
			UART_putc(UART3_ID,c);					//Echo du caractère reçu sur l'UART 3.
		}
	}
}

/////////////////  ROUTINES D'INTERRUPTION  //////////////////////////////

void USART1_IRQHandler(void)
{
	HAL_UART_IRQHandler(&UART_HandleStructure[UART1_ID]);
}

void USART2_IRQHandler(void)
{
	HAL_UART_IRQHandler(&UART_HandleStructure[UART2_ID]);
}

void USART3_IRQHandler(void)
{
	HAL_UART_IRQHandler(&UART_HandleStructure[UART3_ID]);
}



/*
 * @brief	Cette fonction est appelée en interruption UART par le module HAL_UART.
 * @post	L'octet reçu est rangé dans le buffer correspondant.
 * @post	La réception en IT du prochain octet est ré-activée.
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	uint8_t uart_id;
	if(huart->Instance == USART1)		uart_id = UART1_ID;
	else if(huart->Instance == USART2)	uart_id = UART2_ID;
	else if(huart->Instance == USART3)	uart_id = UART3_ID;
	else return;

	buffer_rx_data_ready[uart_id] = TRUE;	//Le buffer n'est pas (ou plus) vide.
	buffer_rx_write_index[uart_id] = (uint8_t)((buffer_rx_write_index[uart_id] + 1) % BUFFER_RX_SIZE);						//Déplacement pointeur en écriture
	HAL_UART_Receive_IT(&UART_HandleStructure[uart_id],&buffer_rx[uart_id][buffer_rx_write_index[uart_id]],1);	//Réactivation de la réception d'un caractère
}

/*
 * @brief	Cette fonction est appelée par la fonction d'initialisation HAL_Init().  => Generated by Cube MX
 * 			Selon le numéro de l'UART, on y defini la configuration des broches correspondantes (voir la doc)
 * @param	huart: uart handler utilisé
 */
/*void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{

	if(huart->Instance == USART1)
	{
		#ifdef UART1_ON_PA9_PA10
			__HAL_RCC_GPIOA_CLK_ENABLE();		//Horloge des broches a utiliser
			BSP_GPIO_PinCfg(GPIOA, GPIO_PIN_9, GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH); //Configure Tx as AF
			BSP_GPIO_PinCfg(GPIOA, GPIO_PIN_10, GPIO_MODE_AF_INPUT, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH); //Configure Rx as AF
		#endif
		#ifdef UART1_ON_PB6_PB7
			//Remap :
			__HAL_RCC_AFIO_CLK_ENABLE();
			__HAL_RCC_GPIOB_CLK_ENABLE();		//Horloge des broches a utiliser
			BSP_GPIO_PinCfg(GPIOB, GPIO_PIN_6, GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH); //Configure Tx as AF
			BSP_GPIO_PinCfg(GPIOB, GPIO_PIN_7, GPIO_MODE_AF_INPUT, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH); //Configure Rx as AF
			__HAL_AFIO_REMAP_USART1_ENABLE();
		#endif
		__HAL_RCC_USART1_CLK_ENABLE();		//Horloge du peripherique UART
	}
	else if(huart->Instance == USART2)
	{
		#ifdef UART2_ON_PA2_PA3
			__HAL_RCC_GPIOA_CLK_ENABLE();		//Horloge des broches a utiliser
			BSP_GPIO_PinCfg(GPIOA, GPIO_PIN_2, GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH);	//Configure Tx as AF
			BSP_GPIO_PinCfg(GPIOA, GPIO_PIN_3, GPIO_MODE_AF_INPUT, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH); //Configure Rx as AF
		#endif
		#ifdef UART2_ON_PD5_PD6
			__HAL_RCC_AFIO_CLK_ENABLE();
			__HAL_RCC_GPIOD_CLK_ENABLE();
			BSP_GPIO_PinCfg(GPIOD, GPIO_PIN_5, GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH);	//Configure Tx as AF
			BSP_GPIO_PinCfg(GPIOD, GPIO_PIN_6, GPIO_MODE_AF_INPUT, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH); //Configure Rx as AF
			__HAL_AFIO_REMAP_USART2_ENABLE()
		#endif
		__HAL_RCC_USART2_CLK_ENABLE();		//Horloge du peripherique UART
	}
	else if(huart->Instance == USART3)
	{
		#ifdef  UART3_ON_PB10_PB11
			__HAL_RCC_GPIOB_CLK_ENABLE();		//Horloge des broches a utiliser
			BSP_GPIO_PinCfg(GPIOB, GPIO_PIN_10, GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH);	//Configure Tx as AF
			BSP_GPIO_PinCfg(GPIOB, GPIO_PIN_11, GPIO_MODE_AF_INPUT, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH); //Configure Rx as AF
		#endif
		#ifdef UART3_ON_PD8_PD9
			//Remap...
			__HAL_RCC_AFIO_CLK_ENABLE();
			__HAL_RCC_GPIOD_CLK_ENABLE();		//Horloge des broches a utiliser
			BSP_GPIO_PinCfg(GPIOD, GPIO_PIN_8, GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH);	//Configure Tx as AF
			BSP_GPIO_PinCfg(GPIOD, GPIO_PIN_9, GPIO_MODE_AF_INPUT, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH); //Configure Rx as AF
			__HAL_AFIO_REMAP_USART3_ENABLE();
		#endif
		__HAL_RCC_USART3_CLK_ENABLE();		//Horloge du peripherique UART
	}
}*/

/*
 * @brief	Function called when the uart throws an error
 * @param	huart handler used to throw errors
 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->ErrorCode == HAL_UART_ERROR_ORE){
        // remove the error condition
        huart->ErrorCode = HAL_UART_ERROR_NONE;
        // set the correct state, so that the UART_RX_IT works correctly
        huart->gState = HAL_UART_STATE_BUSY_RX;
    }

}

