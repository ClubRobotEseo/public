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

/** ----------------  Defines possibles  --------------------
 *	USE_XBEE_OLD					: Active QS_can_over_xbee
 *	XBEE_PLUGGED_ON_UART1		: Configuration de l'XBEE via l'UART 1
 *	XBEE_PLUGGED_ON_UART2		: Configuration de l'XBEE via l'UART 2
 *	XBEE_SIMULATION				: Envoi des messages CAN du XBEE sur le CAN
 *
 * ----------------  Choses � savoir  --------------------
 *	Il faut d�finir XBEE_PLUGGED_ON_UART1 ou 2 obligatoirement
 *	SD_ENABLE					: Sauvegarde sur la SD des messages transit�s sur le XBEE	(Ne marche que sur la carte strat�gie ayant une SD)
 */


#include "QS_all.h"
#include "QS_CANmsgList.h"

#ifndef QS_CAN_OVER_XBEE_H
	#define	QS_CAN_OVER_XBEE_H

	#if USE_XBEE_OLD

		#if !(defined XBEE_PLUGGED_ON_UART1 || defined XBEE_PLUGGED_ON_UART2)
			#error "Vous devez d�finir l'UART o� est branch� le XBEE"
			#error "Pour cela, d�finissez XBEE_PLUGGED_ON_UART1 ou XBEE_PLUGGED_ON_UART2"
		#endif

		#define SOH 0x01
		#define EOT 0x04
		#define STX 0x02
		#define ETX 0x03

		//Listez ici l'ensemble des modules utilis�s
		typedef enum
		{
			MODULE_C = 0,	//Coordinateur.
			MODULE_1,
			MODULE_2,
			MODULE_3,
			MODULE_NUMBER	//nombre de module... = forc�ment le nombre suivant dans l'�num�ration !
		}module_id_e;

		typedef void(*CAN_over_XBee_callback_action_t)(CAN_msg_t * can_msg);

		#ifdef QS_CAN_OVER_XBEE_C
			//ADRESSES PHYSIQUE DES MODULES XBEE UTILISES !!! (= num�ro de s�rie = on ne peut pas le changer)
			//Attention, l'ordre doit correspondre � l'�num�ration module_id_e !
			//L'ajout d'une gomette num�rot�e sur chaque module est vivement conseill� !!!
			const Uint8 module_address[MODULE_NUMBER][8] =
				{{0x00, 0x13, 0xA2, 0x00, 0x40, 0x61, 0x49, 0x66},	//COORDINATEUR
				 {0x00, 0x13, 0xA2, 0x00, 0x40, 0x5D, 0xFB, 0x8D},	//MODULE 1
				 {0x00, 0x13, 0xA2, 0x00, 0x40, 0x5D, 0xF9, 0xFB},	//MODULE 2
				 {0x00, 0x13, 0xA2, 0x00, 0x40, 0x9E, 0xAF, 0xA1}	//MODULE 3
				};
		#endif


		//Possibilit� pour faire correspondre l'applicatif aux modules utilis�s :
		#define BIG_ROBOT_MODULE	MODULE_C
		#define SMALL_ROBOT_MODULE	MODULE_1
		#define BALISE_MERE			MODULE_2
		#define LED_MODULE			MODULE_3

		void CAN_over_XBee_init(module_id_e me, module_id_e destination);

		/*
		@function	permettant de recevoir des messages CAN via une liaison ZigBee, et en provenance de notre module XBee
		@param		dest est un pointeur vers le message CAN � recevoir
		@return 	un bool�en qui indique si un message a �t� re�u. (si TRUE, on peut lire *dest sinon, *dest ne vaut rien d'int�ressant !!!)
		@pre		AU PLUS SOUVENT : on peut appeler cette fonction en boucle infinie
		@pre		AU MOINS SOUVENT : il faut appeler cette fonction � chaque caract�re re�u sur l'UARTxbee
		@post		Si une frame correcte est re�ue, cette fonction tente d'y trouver le message CAN.
		*/
		bool_e XBeeToCANmsg (CAN_msg_t* dest);


		/*
		@function 	permettant d'envoyer un message CAN via une liaison ZigBee au module XBee de destination par d�faut, ou a l'ensemble des modules joignables.
		@note		les messages CAN transmis respectent le protocole d�finit pour le transfert de message CAN over UART (SOH, EOT, et nombre de data constantes !)
					int�r�t : c'est plus simple !
		@param		src est un pointeur vers le message CAN � envoyer
		@param		broadcast : si TRUE, le msg est envoy� � tout les modules joignables, si FALSE, seulement au module par d�faut (configur� dans l'init)
		@post		cette fonction ne peut pas garantir que le message � �t� envoy�. Mais elle a tent� de transmettre la demande d'envoi au module XBee !
		@pre		le module XBee doit �tre allum�, et en �tat de fonctionnement r�seau. (une attente de quelques secondes apr�s reset est conseill�e avant d'envoyer des messages)
		*/
		void CANMsgToXbee(CAN_msg_t * src, bool_e broadcast);

		void XBEE_send_sid(Uint11 sid, bool_e broadcast);


		/*
		@function 	permettant d'envoyer un message CAN via une liaison ZigBee � un autre module XBee DONT LA DESTINATION EST CONNUE.
		@note		les messages CAN transmis respectent le protocole d�finit pour le transfert de message CAN over UART (SOH, EOT, et nombre de data constantes !)
					int�r�t : c'est plus simple !
		@param		src est un pointeur vers le message CAN � envoyer
		@param		module_dest est l'un des modules de destination connu. Voir QS_can_over_xbee.h pour une liste des modules connus !
		@post		cette fonction ne peut pas garantir que le message � �t� envoy�. Mais elle a tent� de transmettre la demande d'envoi au module XBee !
		@pre		le module XBee doit �tre allum�, et en �tat de fonctionnement r�seau. (une attente de quelques secondes apr�s reset est conseill�e avant d'envoyer des messages)
		*/
		void CANMsgToXBeeDestination(CAN_msg_t * src, module_id_e destination);

		void CAN_over_XBee_process_main(void);
		void CAN_over_XBee_process_ms(void);
		bool_e XBee_is_destination_reachable(void);
		bool_e XBee_is_module_reachable(module_id_e module);
		void XBee_set_module_reachable(module_id_e module, bool_e state);
		void XBEE_ping_pong_enable(bool_e enable);
		void XBee_Pong(module_id_e module);
		void XBee_Ping(module_id_e module);

		//Enregistre un pointeur sur fonction qui sera appel� � chaque message CAN envoy�.
		void CAN_over_XBee_set_send_callback(CAN_over_XBee_callback_action_t action);


	#endif // USE_XBEE_OLD

#endif /* ifndef QS_CAN_OVER_XBEE_H */
