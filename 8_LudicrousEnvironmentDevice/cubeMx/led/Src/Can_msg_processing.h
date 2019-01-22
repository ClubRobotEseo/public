/*
 *	Club Robot ESEO 2018
 *
 *	Fichier : Can_msg_processing.h
 *	Package : Carte LED
 *	Description : Fonctions de traitement des messages CAN
 *  Auteur : Valentin
 *  Version 20180407
 */

#ifndef CAN_MSG_PROCESSING_H
	#define CAN_MSG_PROCESSING_H
	#include "QS/QS_all.h"
	#include "QS/QS_CANmsgList.h"

	//Traite les messages CAN reçus
	void CAN_process_msg(CAN_msg_t* msg_to_process);

	void QS_CAN_VERBOSE_can_msg_print(CAN_msg_t* msg);

#endif /* ndef CAN_MSG_PROCESSING_H */
