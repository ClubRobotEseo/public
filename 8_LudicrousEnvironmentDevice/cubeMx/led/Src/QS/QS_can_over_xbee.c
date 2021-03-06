/*
 *	Club Robot ESEO 2011-2012
 *	Shark & Fish
 *
 *	Fichier : QS_can_over_xbee.c
 *	Package : Qualit� Soft
 *	Description : fonctions d'encapsulation des messages CAN
					pour envoi via module XBEE configur� en mode API
 *	Auteur : Nirgal
 *	Licence : CeCILL-C (voir LICENCE.txt)
 *	Version 20120224
 */


#define QS_CAN_OVER_XBEE_C
#include "QS_can_over_xbee.h"
#include "QS_all.h"
#if USE_XBEE_OLD
	#include "QS_uart.h"

	#define XBEE_PING_PERIOD	500	//ms

	#include <stdio.h>

	volatile static CAN_over_XBee_callback_action_t CAN_over_XBee_send_callback = NULL;
	void XBEE_buffer_state_machine(void);

		#ifdef XBEE_PLUGGED_ON_UART1
			#define XBee_putc(c) UART1_putc(UART1_ID, c)
		#endif
		#ifdef XBEE_PLUGGED_ON_UART2
			#define XBee_putc(c) UART_putc(UART2_ID, c)
		#endif



		#ifdef XBEE_PLUGGED_ON_UART1
			#define XBee_data_ready() UART_data_ready(UART1_ID)
		#endif
		#ifdef XBEE_PLUGGED_ON_UART2
			#define XBee_data_ready() UART_data_ready(UART2_ID)
		#endif


		#ifdef XBEE_PLUGGED_ON_UART1
			#define XBee_get_next_msg() UART_getc(UART1_ID)
		#endif
		#ifdef XBEE_PLUGGED_ON_UART2
			#define XBee_get_next_msg() UART_getc(UART2_ID)
		#endif


	volatile static bool_e initialized = FALSE;
	volatile static bool_e XBee_ready_to_talk = FALSE;
	volatile static bool_e module_reachable[MODULE_NUMBER];	//Etat des autres modules (joignables ou non...)
	volatile static Uint16 t = 0;
	volatile static bool_e ping_pong_enable = FALSE;

	void CAN_over_XBee_process_ms(void)
	{
		if(t)
			t--;
	}

	volatile module_id_e XBee_i_am_module = BIG_ROBOT_MODULE;
	volatile module_id_e XBee_module_id_destination = SMALL_ROBOT_MODULE;


	void CAN_over_XBee_init(module_id_e me, module_id_e destination)
	{
		module_id_e module;
		assert(me<MODULE_NUMBER);
		assert(destination<MODULE_NUMBER);
		XBee_i_am_module = me;
		XBee_module_id_destination = destination;
		for(module = 0; module<MODULE_NUMBER; module++)
			 module_reachable[module] = FALSE;		//Tout les modules sont injoignables.
		assert(XBee_i_am_module < MODULE_NUMBER);
		module_reachable[XBee_i_am_module] = TRUE;	//En principe, on a pas besoin de v�rifier qu'on peut se parler � soit-m�me.
		ping_pong_enable = TRUE; //On active le ping_pong...
		initialized = TRUE;
	}



	void XBee_Pong(module_id_e module)
	{
		CAN_msg_t msg;
		if(module >= MODULE_NUMBER)
			return;
		msg.sid = XBEE_PONG;
		msg.size = SIZE_XBEE_PONG;
		msg.data.xbee_pong.module_id = XBee_i_am_module;
		CANMsgToXBeeDestination(&msg, module);
	}

	void XBee_Ping(module_id_e  module)
	{
		CAN_msg_t msg;
		if(module >= MODULE_NUMBER)
			return;
		msg.sid = XBEE_PING;
		msg.size = SIZE_XBEE_PING;
		msg.data.xbee_ping.module_id = XBee_i_am_module;
		CANMsgToXBeeDestination(&msg, module);
	}



	/*
	@function 	Fonction d'initialisation du dialogue avec le module XBee
	@pre		Cette fonction doit �tre appel�e p�riodiquement jusqu'� ce qu'elle renvoit TRUE !
	*/
	void CAN_over_XBee_process_main(void)
	{
		bool_e everyone_is_reachable;
		typedef enum
		{
			INIT = 0,					//init variables et lancement du reset du module XBee
			WAIT_BEFORE_RELEASE_RESET,	//Reset du module XBee
			WAIT_FOR_NETWORK,			//Attente que le module XBee soit pr�t
			PING_PONG,					//Etat d'�changes de ping et de pong entre les diff�rents modules
			IDLE						//R�seau en fonctionnement
		}XBee_state_e;
		static XBee_state_e state = INIT;
		module_id_e module;
		//Note : la mise en place d'un PING au niveau applicatif peut �tre sympatique... et peut permettre de d�clencher l'envoi des messages !
		//D�s que je re�ois un ping, je r�ponds pong, et je m'autorise � contacter ce correspondant !
		//Tant que je n'ai pas re�u de ping de tous les correspondants et que je n'ai pas envoy� un pong � tout les correspondants, je leur envoie un ping p�riodiquement (1 seconde par exemple)
		//A terme, il faudrait traiter l'acquittement des messages envoy�s pour maintenir � jour la connaissance du fonctionnement du r�seau...
		//Et pouvoir reprendre les ping envers un destinataire absent�... ! (Donc g�r� dans un �tat IDLE normal)

		XBEE_buffer_state_machine();

		switch(state)
		{
			case INIT:
				if(initialized == FALSE)
				{
					debug_printf("ERREUR : VOUS DEVEZ APPELER CAN_over_XBee_init() AVANT d'APPELER LE PROCESS MAIN...\n");
					return;
				}
				//Transition imm�diate.
				HAL_GPIO_WritePin(XBEE_RESET, 1); //RESET du module XBee
				t = 2;	//2ms
				state = WAIT_BEFORE_RELEASE_RESET;
			break;
			case WAIT_BEFORE_RELEASE_RESET:
				if(!t)
				{
					HAL_GPIO_WritePin(XBEE_RESET, 0);
					t = 3000;
					state = WAIT_FOR_NETWORK;
				}
			break;
			case WAIT_FOR_NETWORK:
				if(!t)
				{
					XBee_ready_to_talk = TRUE;
					state = PING_PONG;
				}
			break;
			case PING_PONG:
				if(!t)
				{
					everyone_is_reachable = TRUE;	//on suppose que tout le monde est joignable
					for(module = 0; module<MODULE_NUMBER; module++)
					{	//Pour tout les modules non reachable, on envoi un ping !
						if(module_reachable[module] == FALSE)
						{
							everyone_is_reachable = FALSE;	//si quelqu'un n'est pas joignable, alors tout le monde ne l'est pas !
							XBee_Ping(module);
							//debug_printf("ping %d->%d\n",XBee_i_am_module, module);
						}
					}
					if(everyone_is_reachable || !ping_pong_enable)
						state = IDLE;
					else
						t = XBEE_PING_PERIOD;
				}
			break;
			case IDLE:
				//Soit le match � commenc�, soit tout le monde a �t� joint -> on arr�te les pings...
				//TOUT LES MESSAGES CAN ENVOYES SONT ALORS CONSECUTIFS A UNE DEMANDE PROVENANT DU DESTINATAIRE DES MESSAGES !
				//ces demandes sont p�rissables en quelques secondes. Ceci afin d'�viter qu'un flux de donn�e ne soit actif longtemps vers un destinataire qui a �t� �teint.

				if(ping_pong_enable)
					state = PING_PONG;
			break;
			default:
			break;
		}
	}


	//Typiquement, lorsque le match commence, on cesse les ping pong...
	void XBEE_ping_pong_enable(bool_e enable)
	{
		ping_pong_enable = enable;
	}


	bool_e process_received_can_msg(CAN_msg_t * msg)
	{
		switch(msg->sid)	//Messages CAN re�us, premier filtrage.
		{
			case XBEE_PONG:
				if(msg->data.xbee_pong.module_id < MODULE_NUMBER)
					module_reachable[msg->data.xbee_pong.module_id] = TRUE;
				return FALSE;	//Le message ne sera pas transmis au reste du code.
			break;
			case XBEE_PING:
				if(msg->data.xbee_ping.module_id < MODULE_NUMBER)
				{
					module_reachable[msg->data.xbee_ping.module_id] = TRUE;
					XBee_Pong(msg->data.xbee_ping.module_id);
					debug_printf("pong %d->%d\n",XBee_i_am_module, msg->data.xbee_ping.module_id);
				}
				return FALSE;	//Le message ne sera pas transmis au reste du code.
			break;
			default:
			break;
		}
		return TRUE;	//Le message sera transmis au reste du code.
	}

	typedef enum
	{
		HEADER,
		SID_MSB,
		SID_LSB,
		DATA0, DATA1, DATA2, DATA3, DATA4, DATA5, DATA6, DATA7,
		SIZE_FIELD,
		FOOTER
	}can_msg_on_char_array_fields_e;

	#define CAN_MSG_LENGTH	11

	bool_e APIFrameToCANmsg(Uint8 * frame, CAN_msg_t * dest)
	{
		/*
		 *	cette fonction lit un octet dans le tableau frame
		 *	et construit un message CAN.  Elle renvoie ensuite si
		 *	oui ou non elle a trouv� un message CAN. Elle v�rifie
		 *  aussi si le message est bien conforme au protocole de communication
		 *  (cf QS)
		 */
		can_msg_on_char_array_fields_e next_byte_to_read=0;
		Uint8 byte_read;

		for(next_byte_to_read=0;next_byte_to_read<13;next_byte_to_read++)	//C'est � ces indices que le message CAN peut �tre r�cup�r�. Voir doc XBEE...	ou commentaires plus bas.
		{
			byte_read = frame[next_byte_to_read];

			switch (next_byte_to_read)
			{
				case HEADER:
					if(byte_read != SOH)
						return FALSE;
					break;
				case SID_MSB:		/*lecture du MSB du sid */
					dest->sid = (Uint16)byte_read <<8;
					break;
				case SID_LSB:		/*lecture du LSB du sid */
					dest->sid |= (Uint16)byte_read;
					break;
				case SIZE_FIELD:	/*lecture du champs size */
					dest->size = byte_read;
					if(dest->size > 8)
						return FALSE;
					break;
				case FOOTER:
					if(byte_read != EOT)
						return FALSE;
					else
						return TRUE;
					break;

				default:	/*lecture d'un octet de data */
					dest->data.raw_data[next_byte_to_read - DATA0]=byte_read;
					break;
			}
		}
		return FALSE;
	}

	/*
		Une frame de "donn�es re�ues" renvoy�e par le module XBee est de la forme :
		fr. delimiter	SizeMSB	SizeLSB		DataReceived	@64bits				@network	ACK'ed	SOH	 SID	DATAS			   SIZE	EOT		Checksum
		0x7E			0x00	25			0x90			0xAAAAAAAAAAAAAAAA	0xNNNN		0x01	0x01 0x0123 0x1122334455667788 0x08 0x04	0x??
					length = 25				<---------------------------------------------------------------------------------------------->
						indice frame		0				  1      �       8    9 10      11      12   13 14   15      �      22   23   24
														le message can est donc de  12  � 24 inclus:<-------------------------------------->
						le checksum porte sur les 25 octets du length, plus le checksum : le total doit valoir 0xFF
	*/
	typedef enum
	{
		START_DELIMITER = 0,
		LENGTH_MSB,
		LENGTH_LSB,
		HEADER_SIZE
	}api_frame_on_char_array_fields_e;
	#define FRAME_API_BUF_SIZE 32

	bool_e XBeeToCANmsg (CAN_msg_t* dest)
	{

		static api_frame_on_char_array_fields_e next_byte_to_read=0;
		static Uint16 length;
		static Uint8 checksum;
		static Uint8 frame_api[FRAME_API_BUF_SIZE];
		Uint8 byte_read;


		while(XBee_data_ready())
		{
			byte_read = XBee_get_next_msg();

			switch (next_byte_to_read)
			{
				case START_DELIMITER:
					checksum = 0;
					if(byte_read != 0x7E)
					{
						next_byte_to_read = 0;	//Ce n'est pas le d�but d'une trame... on refuse de continuer..
						debug_printf("XBee : invalid start 0x%02x != 0x7E\n",byte_read);
						continue;	//Prochain caract�re !
					}
					break;
				case LENGTH_MSB:
					length = (Uint16)byte_read <<8;
					break;
				case LENGTH_LSB:
					length |= (Uint16)byte_read;
					if(length != 25)
					{
						next_byte_to_read = 0;
						debug_printf("XBee : invalid length %d != 25\n",length);
						continue;	//Prochain caract�re !
					}
					break;
				default:
					//R�ception des datas...
					if(next_byte_to_read - HEADER_SIZE < FRAME_API_BUF_SIZE)
					{
						frame_api[next_byte_to_read-HEADER_SIZE] = byte_read;
						checksum += byte_read;
					}
					else
					{
						//Il y a un probl�me, on a explos� le buffer de r�ception...
						next_byte_to_read = 0;
						debug_printf("XBee : I do not want to explode buffer...\n");
						continue;	//Prochain caract�re !
					}

				break;
			}
			if(next_byte_to_read == length + HEADER_SIZE)	//Dernier octet (qui est d'ailleurs le checksum)
			{
				next_byte_to_read = 0;	//Pour le prochain passage...
				if(checksum	== 0xFF)	//Tout va bien !
				{
					if(frame_api[0] != 0x90)
						continue;

					if(APIFrameToCANmsg(frame_api+12, dest))	//Recherche d'un message can dans la frame re�ue.
					{
						//On a re�u un message CAN !!!!!!!
						return  process_received_can_msg(dest);	//Si cette fonction traite un message qui n'est destin� qu'� ce fichier, elle renvoie false, et le message n'est pas remontt� � l'applicatif !
					}
				}
			}
			else
			{
				next_byte_to_read++;
			}
		}

		return FALSE;
	}



	/*
		Une frame de "donn�es � envoyer" pour le module XBee est de la forme :
		fr. delimiter	SizeMSB	SizeLSB		DataToSend		needAck		@64bits				@network	Hope	Unicast	SOH	 SID	DATAS			   SIZE	EOT		Checksum
		0x7E			0x00	27			0x10			i or 0		0xAAAAAAAAAAAAAAAA	0xFFFE		0x00	0x00	0x01 0x0123 0x1122334455667788 0x08 0x04	0x??
					length = 27				<---------------------------------------------------------------------------------------------->
						indice frame		0				  1      	  2	 	 �       9   10 11      12      13  	  14  15 16	  17      �     24   25   26
																								le message can est ici :<-------------------------------------->
						le checksum porte sur les 27 octets du length, plus le checksum : le total doit valoir 0xFF
						l'adresse 64 bits doit �tre connue (correspond au num�ro de s�rie du module destinataire !)
						l'adresse network est d�cid�e par le coordinateur, donc variable. 0xFFFE permet d'indiquer qu'on ne la connait pas (le module d�terminera tout seul cette adresse !)
	*/


		#define SEND(x)	XBee_putc(x); cs+=x

	#if SD_ENABLE
		#include "../Supervision/SD/SD.h"
	#endif

	void CANMsgToXBeeDestination(CAN_msg_t * src, module_id_e module_dest)
	{
		Uint8 cs;
		Uint8 i;

	#if XBEE_SIMULATION
		if((src->sid & 0xF00) == XBEE_FILTER && src->sid != XBEE_PING && (module_dest == SMALL_ROBOT_MODULE || module_dest == BIG_ROBOT_MODULE)) // Envoie seulement les messages XBEE
			CAN_send(src);
	#endif

	#if SD_ENABLE
		if(src->sid != XBEE_PING)
			SD_new_event(TO_XBEE_DESTINATION,src,NULL,TRUE);
	#endif

		if(!XBee_ready_to_talk)
			return;

		if(src->sid != XBEE_PING && module_reachable[module_dest] == FALSE)	//Module non atteignable
				return;	//On se refuse d'envoyer un message si le module n'est pas joignable, sauf s'il s'agit d'un ping !

		XBee_putc(0x7E);	//D�but de l'ordre API.
		XBee_putc(0x00);
		XBee_putc(27); //SIZE

		cs = 0x00;
		SEND(0x10);	//API identifier pour TRANSMIT REQUEST

		#if XBEE_ASK_FOR_ACK			//Acquittement au niveau UART.. (pour avoir un retour et savoir si le message est arriv�... � impl�menter un jour ... ???)
			#warning "fonctionnalit� non test�e !"
			static Uint8 ack = 0;
			ack++;
			if(ack==0)
				ack=1;	//car � 0 on a pas d'acquittement.
			SEND(ack);	//msg id (pour acknoledge). 0 pour ne pas demander d'acknowledge.
		#else
			SEND(0x00);
		#endif
		for(i=0; i<8; i++)
		{	//Les accollades sont importantes (� cause du fait que SEND est une macro � deux instructions.
			SEND(module_address[module_dest][i]);	//@ du module de destination. Voir le header de ce fichier .c !!!
		}

		SEND(0xFF);	//Network address : on ne la connait pas (variable). 0xFFFE indique au module de la d�terminer par lui m�me...
		SEND(0xFE);	//
		SEND(0x01);	//Nombre max de noeud que le message peut traverser (Si 0 : valeur par d�faut = 10)
		if(src->sid != XBEE_PING)
		{	//Les accollades sont importantes (� cause du fait que SEND est une macro � deux instructions.
			SEND(0x00);	//Retries enable
		}
		else
		{	//Les accollades sont importantes (� cause du fait que SEND est une macro � deux instructions.
			SEND(0x01);	//Retries disable (utile quand on se fiche que le message passe... par exemple pour un ping)
		}

		//Datas
		SEND(SOH);
		SEND((Uint8)(src->sid >>8));
		SEND((Uint8)src->sid);
		for (i=0; i<src->size && i<8; i++)
		{	//Les accollades sont importantes (� cause du fait que SEND est une macro � deux instructions.
			SEND(src->data.raw_data[i]);
		}
		for (i=src->size; i<8; i++)
		{	//Les accollades sont importantes (� cause du fait que SEND est une macro � deux instructions.
			SEND(0xFF);
		}

		SEND(src->size);
		SEND(EOT);

		XBee_putc(0xFF-cs); //checksum
	}



#define BUFFERIZE_XBEE_MSG		1

#define PERIOD_MSG_XBEE	200
#define BUFFER_MSG_SIZE	32
	typedef struct
	{
		CAN_msg_t msg;
		bool_e broadcast;
	}buffer_msg_t;
	static buffer_msg_t buffer_msgs[BUFFER_MSG_SIZE];
	static uint16_t index_read;
	static uint16_t index_write;
	static uint16_t index_nb;

	void XBEE_buffer_state_machine(void)
	{
		typedef enum
		{
			INIT = 0,
			RUN
		}state_e;
		static state_e state = INIT;
		static time32_t previous_send_time;
		switch(state)
		{
			case INIT:
				index_read = 0;
				index_write = 0;
				index_nb = 0;
				previous_send_time = 0;
				state = RUN;
				break;
			case RUN:
				if(global.absolute_time - previous_send_time > PERIOD_MSG_XBEE)	//Si on a le droit de causer sur le XBEE...
				{
					if(index_nb)	//Il y a des message � envoyer.
					{
						previous_send_time = global.absolute_time;		//On reset le compteur de temps � chaque compteur de msg.
						if(buffer_msgs[index_read].broadcast)
						{
							module_id_e module;
							for(module = 0; module<MODULE_NUMBER; module++)
							{
								if(module != XBee_i_am_module)
									CANMsgToXBeeDestination(&(buffer_msgs[index_read].msg), module);
							}
						}
						else
							CANMsgToXBeeDestination(&(buffer_msgs[index_read].msg), XBee_module_id_destination);

						if(CAN_over_XBee_send_callback)
							(*CAN_over_XBee_send_callback)(&(buffer_msgs[index_read].msg));

						//m�j index
						index_read = (index_read + 1) % BUFFER_MSG_SIZE;
						index_nb--;
					}
				}
				break;
		}
	}





	void CANMsgToXbee(CAN_msg_t * src, bool_e broadcast)
	{
		#if BUFFERIZE_XBEE_MSG
			if(index_nb < BUFFER_MSG_SIZE)
			{
				buffer_msgs[index_write].msg = *src;
				buffer_msgs[index_write].broadcast = broadcast;
				index_write = (index_write + 1) % BUFFER_MSG_SIZE;
				index_nb++;
			}
			else
				debug_printf("buffer_xbee plein.\n");
		#else
			module_id_e module;
			if(broadcast)
			{
				for(module = 0; module<MODULE_NUMBER; module++)
				{
					if(module != XBee_i_am_module)
						CANMsgToXBeeDestination(src, module);
				}
			}
			else
				CANMsgToXBeeDestination(src, XBee_module_id_destination);

			if(CAN_over_XBee_send_callback)
				(*CAN_over_XBee_send_callback)(src);
		#endif
	}




	void XBEE_send_sid(Uint11 sid, bool_e broadcast)
	{
		CAN_msg_t msg;
		msg.sid = sid;
		msg.size = 0;
		CANMsgToXbee(&msg, broadcast);

	}

	bool_e XBee_is_destination_reachable(void)
	{
		if(initialized)
			return module_reachable[XBee_module_id_destination];
		return FALSE;
	}

	bool_e XBee_is_module_reachable(module_id_e module)
	{
		if(initialized)
			return module_reachable[module];
		return FALSE;
	}

	void XBee_set_module_reachable(module_id_e module, bool_e state)
	{
		module_reachable[module] = state;
	}

	void CAN_over_XBee_set_send_callback(CAN_over_XBee_callback_action_t action)
	{
		CAN_over_XBee_send_callback = action;
	}


#endif //USE_XBEE_OLD


