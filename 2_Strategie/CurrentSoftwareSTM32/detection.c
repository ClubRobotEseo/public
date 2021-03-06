/*
 *	Club Robot ESEO 2009 - 2010
 *	PACMAN
 *
 *	Fichier : detection.c
 *	Package : Carte Principale
 *	Description : Traitement des informations pour d�tection
 *	Auteur : Jacen (Modifi� par Ronan)
 *	Version 20110417
 */

#define DETECTION_C
#include "detection.h"
#include "QS/QS_CANmsgList.h"
#include "QS/QS_maths.h"
#include "QS/QS_who_am_i.h"
#include "QS/QS_lowLayer/QS_can.h"
#include "environment.h"
#include "Supervision/Buzzer.h"

#define LOG_PREFIX "detection: "
#define LOG_COMPONENT OUTPUT_LOG_COMPONENT_DETECTION
#include "QS/QS_outputlog.h"

#define BEACON_FAR_THRESHOLD	1000

#define MINIMUM_TIME_FOR_BEACON_SYNCRONIZATION 260 // 260 ms

#define MAXIMUM_TIME_FOR_BEACON_REFRESH 300 // 500ms

#define BORDER_DELTA 50 // 50 mm

typedef struct
{
	Sint16 angle;
	Sint16 dist;
	Sint16 x;
	Sint16 y;
	Uint8 fiability_error;
	bool_e enable;					//cet objet est activ�.
	bool_e enable_for_avoidance;	//Objet activ� pour l'�vitement
	bool_e enable_for_zoning;		//Objet aciv� pour le zoning
	time32_t update_time;
}adversary_t;


typedef enum
{
	DETECTION_REASON_PROCESS_MAIN = 0,		//Process main...
	DETECTION_REASON_DATAS_RECEIVED_FROM_PROPULSION,
	DETECTION_REASON_DATAS_RECEIVED_FROM_BEACON_IR
}detection_reason_e;		//Raison de l'appel � la fonction DETECTION_compute

static void DETECTION_compute(detection_reason_e reason);

#define FOE_DATA_LIFETIME	250		//[ms] Dur�e de vie des donn�es envoy�es par la propulsion et par la balise

volatile adversary_t adversaries[MAX_HOKUYO_FOES];	//Ce tableau se construit progressivement, quand on a toutes les donn�es, on peut les traiter et renseigner le tableau des positions adverses.
volatile Uint8 hokuyo_objects_number = 0;	//Nombre d'objets hokuyo

volatile adversary_t beacon_ir_objects[MAX_BEACON_FOES];
static volatile time32_t data_from_propulsion_update_time = 0;

void DETECTION_init(void)
{
	static bool_e initialized = FALSE;
	if(initialized)
		return;
	hokuyo_objects_number = 0;

}

/*	mise � jour de l'information de d�tection avec le contenu
	courant de l'environnement */
void DETECTION_clean(void)
{
	//Necessaire pour des match infini de test, on reactive les balises toutes les 90sec
	static time32_t next_beacon_activate_msg = 1000;	//Prochain instant d'envoi de message.
	if(!global.flags.match_over && global.absolute_time > next_beacon_activate_msg)
	{
		next_beacon_activate_msg += 90000;
		CAN_send_sid(BEACON_ENABLE_PERIODIC_SENDING);
	}

	DETECTION_compute(DETECTION_REASON_PROCESS_MAIN);
}

#define BEACON_IR_SIZE_FILTER 3
static Sint16 beacon_ir_distance_filter(bool_e enable, Uint8 foe_id, Sint16 new_distance)
{
	static Uint8 index[MAX_BEACON_FOES];
	static Uint8 nb_datas[MAX_BEACON_FOES];
	static Sint16 previous_distances[MAX_BEACON_FOES][BEACON_IR_SIZE_FILTER];
	Uint8 i;
	Sint32 sum;
	assert(foe_id < MAX_BEACON_FOES);

	if(enable)
	{
		assert(index[foe_id] < BEACON_IR_SIZE_FILTER);
		previous_distances[foe_id][index[foe_id]] = new_distance;	//Ajout de la nouvelle distance dans le tableau de filtrage
		if(nb_datas[foe_id] < BEACON_IR_SIZE_FILTER)
			nb_datas[foe_id]++;							//le nombre de donn�es est entre 1 et 3
		index[foe_id] = (index[foe_id]<BEACON_IR_SIZE_FILTER-1) ? (index[foe_id]+1) : 0;	//l'index de la prochaine donn�e � �crire
		sum = 0;
		for(i=0;i<nb_datas[foe_id];i++)
		{
			sum += previous_distances[foe_id][i];
		}
		assert(nb_datas[foe_id] != 0);	//n'est jamais sens� se produire.
		return (Sint16)(sum/nb_datas[foe_id]);	//on renvoie la moyenne des distances enregistr�es dans le tableau
	}
	else
	{
		index[foe_id] = 0;
		nb_datas[foe_id] = 0;
		return new_distance;
	}
}

//Cette fonction utilise les donn�es accumul�es... selon un algo tr�s sophistiqu�... et d�termine les positions adverses.
static void DETECTION_compute(detection_reason_e reason)
{
	Uint8 i,j, j_min;
	Sint16 dist_min;
	bool_e objects_chosen[MAX_HOKUYO_FOES];

	//On se contente de choisir les NB_FOES objets hokuyo les plus proches observ�s et de les enregistrer dans le tableau d'adversaires.
	//Si pas de donn�es venant de la propulsion (hokuyo ou adversaire virtuel parfait) -> on prend les donn�es IR.


	switch(reason)
	{
		case DETECTION_REASON_PROCESS_MAIN:		//CLEAN
			global.flags.foes_updated_for_lcd = FALSE;
			for(i=0;i<MAX_NB_FOES;i++)
			{
				global.foe[i].updated = FALSE;		//On baisse le flag updated � chaque tour de boucle.

				if((global.foe[i].enable == TRUE) && (global.absolute_time - global.foe[i].update_time > FOE_DATA_LIFETIME))
				{
					global.flags.foes_updated_for_lcd = TRUE;
					global.foe[i].enable = FALSE;
				}
			}
			break;
		case DETECTION_REASON_DATAS_RECEIVED_FROM_BEACON_IR:
			global.flags.foes_updated_for_lcd = TRUE;
			for(i=0;i<MAX_BEACON_FOES;i++)
			{
				global.foe[MAX_HOKUYO_FOES+i].fiability_error = beacon_ir_objects[i].fiability_error;
				if(beacon_ir_objects[i].enable)	//Si les donn�es sont coh�rentes... (signal vu...)
				{
					//Si je n'ai pas de donn�es en provenance de la propulsion depuis un moment... j'utilise les donn�es de la BEACON_IR.
					// Ou bien si l'objet observ� est dans l'angle mort de l'hokuyo...
					if	(global.absolute_time - data_from_propulsion_update_time > FOE_DATA_LIFETIME
						|| 	(		beacon_ir_objects[i].angle > PI4096/2-PI4096/3		//Angle entre 30� et 150� --> angle mort hokuyo + marge de 15� de chaque cot�.
								&& 	beacon_ir_objects[i].angle < PI4096/2+PI4096/3 )
						)
					{
						global.foe[MAX_HOKUYO_FOES+i].x 			= beacon_ir_objects[i].x;
						global.foe[MAX_HOKUYO_FOES+i].y 			= beacon_ir_objects[i].y;
						global.foe[MAX_HOKUYO_FOES+i].angle 		= beacon_ir_objects[i].angle;
						global.foe[MAX_HOKUYO_FOES+i].dist 			= beacon_ir_objects[i].dist;
						global.foe[MAX_HOKUYO_FOES+i].update_time 	= beacon_ir_objects[i].update_time;
						global.foe[MAX_HOKUYO_FOES+i].enable 		= TRUE;
						global.foe[MAX_HOKUYO_FOES+i].updated 		= TRUE;
						global.foe[MAX_HOKUYO_FOES+i].from 			= DETECTION_FROM_BEACON_IR;
					}
				}
				else
				{
					global.foe[MAX_HOKUYO_FOES+i].enable = FALSE;
					global.foe[MAX_HOKUYO_FOES+i].updated = FALSE;
				}

			}


			break;
		case DETECTION_REASON_DATAS_RECEIVED_FROM_PROPULSION:		//Cette source d'info est prioritaire...
			global.flags.foes_updated_for_lcd = TRUE;
			// Emet un bip sonore lors de la premiere initialisation de l'hokuyo (r�ception des premi�res donn�es)
			if(global.absolute_time - data_from_propulsion_update_time > FOE_DATA_LIFETIME
			   && global.flags.match_started == FALSE) {
				BUZZER_play(1000, DEFAULT_NOTE, 1);
			}
			data_from_propulsion_update_time = global.absolute_time;
			//debug_printf("Compute :");
			for(j = 0; j < hokuyo_objects_number; j++)
				objects_chosen[j] = FALSE;			//init, aucun des objets n'est choisi

			for(i = 0 ; i < MAX_HOKUYO_FOES ; i++)	//Pour chaque case du tableau d'adversaires qu'on doit remplir
			{
				dist_min = 0x7FFF;
				j_min = 0xFF;		//On suppose qu'il n'y a pas d'objet hokuyo.
				for(j = 0; j < hokuyo_objects_number; j++)	//Pour tout les objets hokuyos recus
				{
					if(adversaries[i].enable && objects_chosen[j] == FALSE && dist_min > adversaries[j].dist)	//Pour tout objet restant (activ�, non choisi)
					{
						j_min = j;
						dist_min = adversaries[j].dist;		//On cherche la distance mini parmi les objet restant
					}
				}
				if(j_min != 0xFF)									//Si on a trouv� un objet
				{
					objects_chosen[j_min] = TRUE;					//On "consomme" cet objet
					global.foe[i].x 			= adversaries[j_min].x;	//On enregistre cet objet � la case i.
					global.foe[i].y 			= adversaries[j_min].y;
					global.foe[i].angle 		= adversaries[j_min].angle;
					global.foe[i].dist 			= adversaries[j_min].dist;
					global.foe[i].update_time 	= adversaries[j_min].update_time;
					global.foe[i].enable 		= TRUE;
					global.foe[i].updated 		= TRUE;
					global.foe[i].from 			= DETECTION_FROM_PROPULSION;
					//debug_printf("%d:x=%4d\ty=%4d\ta=%5d\td=%4d\t|", i, hokuyo_objects[j_min].x, hokuyo_objects[j_min].y, hokuyo_objects[j_min].angle, hokuyo_objects[j_min].dist);
				}
				else
					global.foe[i].enable = FALSE;				//Plus d'objet dispo... on vide la case i.

				//TODO le tableau de foe devrait plutot contenir d'autres types d'infos utiles..... revoir leur type..
			}
			break;
		default:
			break;
	}




	//S'il y a plus d'objets hokuyo que d'adversaires possibles dans notre �vitement, on choisit les plus proches.
	//debug_printf("\n");
}


/* Message can recu avec des infos concernant les adversaires... */
void DETECTION_pos_foe_update (CAN_msg_t* msg)
{
	//bool_e slashn;
	Uint8 fiability;
	Uint8 adversary_nb, i;
	Sint16 cosinus, sinus;

	switch(msg->sid)
	{
		case BROADCAST_ADVERSARIES_POSITION:
			adversary_nb = msg->data.broadcast_adversaries_position.adversary_number;
			if(adversary_nb < MAX_HOKUYO_FOES)
			{
				fiability = msg->data.broadcast_adversaries_position.fiability;
				if(fiability)
				{
					adversaries[adversary_nb].enable = TRUE;
					adversaries[adversary_nb].update_time = global.absolute_time;
				}
				else
					adversaries[adversary_nb].enable = FALSE;
				if(fiability & ADVERSARY_DETECTION_FIABILITY_X)
					adversaries[adversary_nb].x = ((Sint16)msg->data.broadcast_adversaries_position.x)*20;
				if(fiability & ADVERSARY_DETECTION_FIABILITY_Y)
					adversaries[adversary_nb].y = ((Sint16)msg->data.broadcast_adversaries_position.y)*20;
				if(fiability)
				{
					if(fiability & ADVERSARY_DETECTION_FIABILITY_TETA)
						adversaries[adversary_nb].angle = msg->data.broadcast_adversaries_position.teta;
					else	//je dois calculer moi-m�me l'angle de vue relatif de l'adversaire
					{
						adversaries[adversary_nb].angle = GEOMETRY_viewing_angle(global.pos.x, global.pos.y,adversaries[adversary_nb].x, adversaries[adversary_nb].y);
						adversaries[adversary_nb].angle = GEOMETRY_modulo_angle(adversaries[adversary_nb].angle - global.pos.angle);
					}
					if(fiability & ADVERSARY_DETECTION_FIABILITY_DISTANCE)
						adversaries[adversary_nb].dist = ((Sint16)msg->data.broadcast_adversaries_position.dist)*20;
					else	//je dois calculer moi-m�me la distance de l'adversaire
						adversaries[adversary_nb].dist = GEOMETRY_distance(	(GEOMETRY_point_t){global.pos.x, global.pos.y},
																				(GEOMETRY_point_t){adversaries[adversary_nb].x, adversaries[adversary_nb].y}
																				);
				}
				if(msg->data.broadcast_adversaries_position.last_adversary)
				{
					if(adversary_nb == 0 && !fiability)
						hokuyo_objects_number = 0;				//On a des donn�es, qui nous disent qu'aucun adversaire n'est vu...
					else
						hokuyo_objects_number = adversary_nb + 1;
					DETECTION_compute(DETECTION_REASON_DATAS_RECEIVED_FROM_PROPULSION);
				}
			}

			break;
		case BROADCAST_BEACON_ADVERSARY_POSITION_IR:

			for(i = 0; i<MAX_BEACON_FOES; i++)
			{
				beacon_ir_objects[i].fiability_error = msg->data.broadcast_beacon_adversary_position_ir.adv[i].error;
				if((beacon_ir_objects[i].fiability_error & ~SIGNAL_INSUFFISANT & ~TACHE_TROP_GRANDE) == AUCUNE_ERREUR)	//Si je n'ai pas d'autre erreur que SIGNAL_INSUFFISANT... c'est bon
					beacon_ir_objects[i].enable = TRUE;
				else
					beacon_ir_objects[i].enable = FALSE;

				beacon_ir_objects[i].angle = GEOMETRY_modulo_angle(msg->data.broadcast_beacon_adversary_position_ir.adv[i].angle);
				beacon_ir_objects[i].dist = (Uint16)(msg->data.broadcast_beacon_adversary_position_ir.adv[i].dist)*20;

				if(beacon_ir_objects[i].fiability_error & TACHE_TROP_GRANDE)
					beacon_ir_objects[i].dist = 250;	//Lorsque l'on re�oit l'erreur TACHE TROP GRANDE, la distance est fausse, mais l'adversaire est probablement tr�s proche. On impose 25cm.

				//filtrage de la distance
				beacon_ir_objects[i].dist = beacon_ir_distance_filter(beacon_ir_objects[i].enable,i,beacon_ir_objects[i].dist);

				beacon_ir_objects[i].update_time = global.absolute_time;
				COS_SIN_4096_get(beacon_ir_objects[i].angle, &cosinus, &sinus);
				beacon_ir_objects[i].x = global.pos.x + (beacon_ir_objects[i].dist * ((float){((Sint32)(cosinus) * (Sint32)(global.pos.cosAngle) - (Sint32)(sinus) * (Sint32)(global.pos.sinAngle))}/(4096*4096)));
				beacon_ir_objects[i].y = global.pos.y + (beacon_ir_objects[i].dist * ((float){((Sint32)(cosinus) * (Sint32)(global.pos.sinAngle) + (Sint32)(sinus) * (Sint32)(global.pos.cosAngle))}/(4096*4096)));
			}

			DETECTION_compute(DETECTION_REASON_DATAS_RECEIVED_FROM_BEACON_IR);	//On pr�vient l'algo COMPUTE.




		/*	slashn = FALSE;
			if((msg->data[0] & 0xFE) == AUCUNE_ERREUR)	//Si l'octet de fiabilit� vaut SIGNAL_INSUFFISANT, on le laisse passer quand m�me
			{
				//slashn = TRUE;
				global.sensor[BEACON_IR_FOE_1].angle = U16FROMU8(msg->data[1],msg->data[2]);
				// Pour g�rer l'inversion de la balise
				//global.sensor[BEACON_IR_FOE_1].angle += (global.sensor[BEACON_IR_FOE_1].angle > 0)?-PI4096:PI4096;
				global.sensor[BEACON_IR_FOE_1].distance = (Uint16)(msg->data[3])*10;
				global.sensor[BEACON_IR_FOE_1].update_time = global.match_time;
				global.sensor[BEACON_IR_FOE_1].updated = TRUE;
				//debug_printf("IR1=%dmm", global.sensor[BEACON_IR_FOE_1].distance);
				//debug_printf("|%d", ((Sint16)((((Sint32)(global.sensor[BEACON_IR_FOE_1].angle))*180/PI4096))));
			} //else debug_printf("NO IR 1 err %d!\n", msg->data[0]);
			if((msg->data[4] & 0xFE) == AUCUNE_ERREUR)
			{
				//slashn = TRUE;
				global.sensor[BEACON_IR_FOE_2].angle = (Sint16)(U16FROMU8(msg->data[5],msg->data[6]));
				// Pour g�rer l'inversion de la balise
				//global.sensor[BEACON_IR_FOE_2].angle += (global.sensor[BEACON_IR_FOE_2].angle > 0)?-PI4096:PI4096;
				global.sensor[BEACON_IR_FOE_2].distance = (Uint16)(msg->data[7])*10;
				global.sensor[BEACON_IR_FOE_2].update_time = global.match_time;
				global.sensor[BEACON_IR_FOE_2].updated = TRUE;
				//debug_printf(" IR2=%dmm", global.sensor[BEACON_IR_FOE_2].distance);
				//debug_printf("|%d", ((Sint16)((((Sint32)(global.sensor[BEACON_IR_FOE_2].angle))*180/PI4096))));
			} //else debug_printf("NO IR 2 err %d!\n", msg->data[4]);
			if(slashn)
				debug_printf("\n");
				*/
			break;
		default:
			break;
	}
}

time32_t DETECTION_get_last_time_since_hokuyo_date(){
	return global.absolute_time - data_from_propulsion_update_time;
}


//void DETECTION_update_foe_position(void)
//{


	/*
	static bool_e ultrasonic_fiable = TRUE;
	Sint16 beacon_foe_x, beacon_foe_y;
	bool_e update_dist_by_ir;
	Uint8 foe_id;
	 for (foe_id = 0; foe_id < MAX_NB_FOES; foe_id++)
	{
		if((global.match_time - global.sensor[BEACON_IR(foe_id)].update_time < MINIMUM_TIME_FOR_BEACON_SYNCRONIZATION) &&
		   (global.match_time - global.sensor[BEACON_US(foe_id)].update_time < MINIMUM_TIME_FOR_BEACON_SYNCRONIZATION) &&
		   (global.match_started == TRUE) &&
		   (ultrasonic_fiable == TRUE))
		{
			if(absolute(global.sensor[BEACON_IR(foe_id)].distance - global.sensor[BEACON_US(foe_id)].distance) > 1000) {
				ultrasonic_fiable = FALSE;
				CAN_msg_t msg;
				msg.sid = DEBUG_US_NOT_RELIABLE;
				msg.data[0] = HIGHINT(global.sensor[BEACON_IR(foe_id)].distance);
				msg.data[1] = LOWINT(global.sensor[BEACON_IR(foe_id)].distance);
				msg.data[2] = HIGHINT(global.sensor[BEACON_US(foe_id)].distance);
				msg.data[3] = LOWINT(global.sensor[BEACON_US(foe_id)].distance);
				msg.data[4] = foe_id;
				msg.size = 4;
				CAN_send(&msg);
			}
		}
#warning "DESACTIVATION MANUELLE DES US !!!"
		ultrasonic_fiable = FALSE;

		if(global.sensor[BEACON_IR(foe_id)].updated)
		{
			update_dist_by_ir = FALSE;

			if(global.match_time - global.sensor[BEACON_US(foe_id)].update_time > MAXIMUM_TIME_FOR_BEACON_REFRESH ||
				ultrasonic_fiable == FALSE) //Si la balise US n'a rien re�u depuis 500ms
			{
				global.foe[foe_id].dist = global.sensor[BEACON_IR(foe_id)].distance; //On met � jour la distance par infrarouge
				update_dist_by_ir = TRUE;
			}
			if(global.match_time - global.sensor[BEACON_US(foe_id)].update_time < MINIMUM_TIME_FOR_BEACON_SYNCRONIZATION || update_dist_by_ir)
			{
				//L'ancienne distance est conservee
				beacon_foe_x = (global.foe[foe_id].dist * cos4096(global.sensor[BEACON_IR(foe_id)].angle)) * global.pos.cosAngle
					- (global.foe[foe_id].dist * sin4096(global.sensor[BEACON_IR(foe_id)].angle)) * global.pos.sinAngle + global.pos.x;

				beacon_foe_y  = (global.foe[foe_id].dist * cos4096(global.sensor[BEACON_IR(foe_id)].angle)) * global.pos.sinAngle
					+ (global.foe[foe_id].dist * sin4096(global.sensor[BEACON_IR(foe_id)].angle)) * global.pos.cosAngle + global.pos.y;

				if(ENV_game_zone_filter(beacon_foe_x,beacon_foe_y,BORDER_DELTA))
				{
					global.foe[foe_id].x = beacon_foe_x;
					global.foe[foe_id].y = beacon_foe_y;
					global.foe[foe_id].update_time = global.match_time;
					global.foe[foe_id].enable = TRUE;
				}
			}
			global.foe[foe_id].angle = global.sensor[BEACON_IR(foe_id)].angle;
			//debug_printf("IR Foe_%d is x:%d y:%d d:%d a:%d\r\n",foe_id, global.foe[foe_id].x, global.foe[foe_id].y, global.foe[foe_id].dist, ((Sint16)(((Sint32)(global.foe[foe_id].angle))*180/PI4096)));
		}

		if(global.sensor[BEACON_US(foe_id)].updated && ultrasonic_fiable == TRUE)
		{
			// L'ancien angle est conserve
			if(global.match_time - global.sensor[BEACON_IR(foe_id)].update_time < MINIMUM_TIME_FOR_BEACON_SYNCRONIZATION)
			{
				beacon_foe_x = (global.sensor[BEACON_US(foe_id)].distance * cos4096(global.foe[foe_id].angle)) * global.pos.cosAngle
					- (global.sensor[BEACON_US(foe_id)].distance * sin4096(global.foe[foe_id].angle)) * global.pos.sinAngle + global.pos.x;

				beacon_foe_y  = (global.sensor[BEACON_US(foe_id)].distance * cos4096(global.foe[foe_id].angle)) * global.pos.sinAngle
					+ (global.sensor[BEACON_US(foe_id)].distance * sin4096(global.foe[foe_id].angle)) * global.pos.cosAngle + global.pos.y;

				if(ENV_game_zone_filter(beacon_foe_x,beacon_foe_y,BORDER_DELTA))
				{
					global.foe[foe_id].x = beacon_foe_x;
					global.foe[foe_id].y = beacon_foe_y;
					global.foe[foe_id].update_time = global.match_time;
					global.foe[foe_id].enable = TRUE;
				}
			}
			// On mets a jour la distance
			global.foe[foe_id].dist = global.sensor[BEACON_US(foe_id)].distance;
			//debug_printf("US Foe_%d is x:%d y:%d d:%d a:%d\r\n",foe_id, global.foe[foe_id].x, global.foe[foe_id].y, global.foe[foe_id].dist,((Sint16)(((Sint32)(global.foe[foe_id].angle))*180/PI4096)));
		}
	}

bool_e ENV_game_zone_filter(Sint16 x, Sint16 y, Uint16 delta)
{
	// D�limitation du terrain
	if(x < delta || y < delta || x > GAME_ZONE_SIZE_X - delta || y > GAME_ZONE_SIZE_Y - delta
	//|| (x > 875 - delta && x < 1125 + delta  && y > 975 - delta && y < 2025 + delta) // Pour supprimer la zone centrale (totem + palmier)
	|| (x > 1250 - delta && (y < 340 + delta || y > 2660 - delta))) //Pour supprimer les cales
	{
		return FALSE;
	}
	return TRUE;
}

	*/




//}


