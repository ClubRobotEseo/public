/*
 *	Club Robot ESEO 2008 - 2010
 *	Archi'Tech
 *
 *	Fichier : Buffer.c
 *	Package : Supervision
 *	Description : impl�mentation du buffer circulaire qui sauvegarde les messages can �chang�s durant le match ainsi que la base de temps
 *	Auteur : Jixi
 *	Version 20090305
 */

#define BUFFER_C
#include "Buffer.h"

#include "../QS/QS_outputlog.h"
#include "../QS/QS_can_over_uart.h"
#include "../QS/QS_can_verbose.h"


typedef struct			//d�finition de la structure qui sera stock�e dans le buffer circulaire
{
	CAN_msg_t message;
	Uint16 temps;
} CANtime_t;

typedef struct		//d�finition de la structure du buffer
{
	CANtime_t tab[BUFFER_SIZE];
	Uint16 indiceDebut;
	Uint16 indiceFin;
} bufferCirculaire_t;

bufferCirculaire_t buffer;

void BUFFER_init()
{
	buffer.indiceDebut = 0;
	buffer.indiceFin = 0;
}

void BUFFER_add(CAN_msg_t * m)
{
	//Enregistrement du message dans le buffer...
	(buffer.tab[buffer.indiceFin]).message = *m;
	(buffer.tab[buffer.indiceFin]).temps =  (Uint16)(global.match_time/2); //le timer boucle toutes les 250 ms
	buffer.indiceFin++;

	if (buffer.indiceFin >= BUFFER_SIZE-1)		//Si jamais on d�passe la taille du buffer, on �crase le dernier message re�u, BUFFER_SIZE -1 car on incr�mente si la condition est vraie
		buffer.indiceFin=BUFFER_SIZE-1;
}


void BUFFER_flush()
{
	Uint16 index;
	CAN_msg_t * pmsg;
	for(index = buffer.indiceDebut; index < buffer.indiceFin; index++)
	{
		debug_printf("t=%.2d.%03ds ",buffer.tab[index].temps/500, ((buffer.tab[index].temps)%500)*2);
		pmsg = &(buffer.tab[index].message);
		QS_CAN_VERBOSE_can_msg_print(pmsg, VERB_LOG_MSG);
	}
}
