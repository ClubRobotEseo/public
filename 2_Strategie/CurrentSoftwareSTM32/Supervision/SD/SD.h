#ifndef SD_H
#define SD_H

#include "../../QS/QS_all.h"

void SD_process_1ms(void);
void SD_process_main(void);
void SD_init(void);
void SD_print_previous_match(void);
void SD_print_match(Uint16 nb_match);

typedef enum
{
	FROM_SOFT = 0,	//Evenement envoy� par le code... (changement d'�tat... information...)
	FROM_BUS_CAN,
	FROM_UART1,
	FROM_UART2,
	FROM_XBEE,
	TO_BUSCAN,	//Envoy� par nous, sur le bus can
	TO_UART1,	//Envoy� par nous, sur l'uart1
	TO_UART2,	//Envoy� par nous, sur l'uart2
	TO_XBEE_BROADCAST,		//Envoy� par nous, sur le XBee, en Broadcast
	TO_XBEE_DESTINATION		//Envoy� par nous, sur le XBee, � la destination par d�faut
}source_e;


void SD_new_event(source_e source, CAN_msg_t * can_msg, char * user_string, bool_e insert_time);
int printf_SD_verbose(bool_e verbose, const char *format, ...);
Uint16 SD_get_match_id();

bool_e SD_isOK();

#define SD_printf(...) printf_SD_verbose(FALSE, __VA_ARGS__);


#endif /* SD_H */
