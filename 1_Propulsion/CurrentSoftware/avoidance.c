/*
*  Club Robot ESEO 2014
*
*  Fichier : avoidance.c
*  Package : Propulsion
*  Description : Analyse la trajectoire que va emprunter le robot par rapport au robot adversaire, afin de savoir si le chemin est r�alisable
*  Auteur : Arnaud
*  Version 201203
*/


#include "avoidance.h"
#include "detection.h"
#include "QS/QS_maths.h"
#include "QS/QS_who_am_i.h"
#include "QS/QS_lowLayer/QS_timer.h"
#include "QS/QS_outputlog.h"
#include "QS/QS_maths.h"
#include "copilot.h"
#include "buffer.h"
#include "secretary.h"
#include "pilot.h"

#define WAIT_TIME_DISPLAY_AVOID		50
#define NB_SAMPLE					30
#define SAMPLE_TIME					5		//(en ms)
#define AVOID_WAIT_TIME				2000

static Sint16 ecretage_debug_rect(Sint16 val);

static adversary_t adversary; // adversaire d�tect� stock� dans cette variable pour pouvoir envoyer l'information � la strat�gie
static bool_e adv_hokuyo;

typedef struct{
	Uint16 Xleft;
	Uint16 Xright;
	Uint16 Yfront;
	Uint16 Yback;
}offset_avoid_s;

static offset_avoid_s offset_avoid = {0};

static bool_e active_small_avoidance = FALSE;

void AVOIDANCE_init(){
	UNUSED_VAR(ecretage_debug_rect(0));
}

void AVOIDANCE_process_it(){
	order_t current_order = COPILOT_get_current_order();
	volatile order_t *buffer_order;

	if((current_order.trajectory == TRAJECTORY_AUTOMATIC_CURVE || current_order.trajectory == TRAJECTORY_TRANSLATION) &&
		current_order.avoidance != AVOID_DISABLED){

		if(AVOIDANCE_target_safe(current_order.way, FALSE)){
			if(current_order.avoidance == AVOID_ENABLED){

				// Si on rencontre un adversaire on s'arrete
				ROADMAP_add_order(  TRAJECTORY_STOP,
									0,
									0,
									0,					//teta
									PROP_ABSOLUTE,		//relative
									PROP_NOW,			//maintenant
									ANY_WAY,			//sens de marche
									NOT_BORDER_MODE,	//mode bordure
									PROP_END_AT_POINT, //mode multipoints
									FAST,				//Vitesse
									ACKNOWLEDGE_ASKED,
									CORRECTOR_ENABLE,
									AVOID_DISABLED,
									current_order.idTraj
								);

				// Puis on avertie la strat�gie qu'il y a eu �vitement

				AVOIDANCE_said_foe_detected(FALSE, FALSE);

				COPILOT_set_avoid_in_rush(TRUE);

			}else if(current_order.avoidance == AVOID_ENABLED_AND_WAIT){

				//debug_printf("t : %ld      buffering !\n", global.absolute_time);

				// On met l'ordre actuel dans le buffer
				COPILOT_buffering_order();
				buffer_order = COPILOT_get_buffer_order();

				// On met � jours la variable qui contient le temps du d�but du wait
				buffer_order->wait_time_begin = global.absolute_time;

				order_t supp;
					supp.x = 0;
					supp.y = 0;
					supp.teta = 0;
					supp.relative = PROP_ABSOLUTE;
					supp.way = ANY_WAY;
					supp.border_mode = NOT_BORDER_MODE;
					supp.propEndCondition = PROP_END_AT_POINT;
					supp.speed = FAST;
					supp.acknowledge = NO_ACKNOWLEDGE;
					supp.corrector = CORRECTOR_ENABLE;
					supp.avoidance = AVOID_DISABLED;
					supp.total_wait_time = 0;
					supp.trajectory = WAIT_FOREVER;
					supp.idTraj = 0;
				BUFFER_add_begin(&supp);

				supp.trajectory = TRAJECTORY_STOP;
				BUFFER_add_begin(&supp);

				AVOIDANCE_said_foe_detected(FALSE, TRUE);

				ROADMAP_launch_next_order();

				COPILOT_set_avoid_in_rush(TRUE);
			}
		}
	}else if(current_order.trajectory == WAIT_FOREVER){

		buffer_order = COPILOT_get_buffer_order();

		// Si il y a timeout
		if(buffer_order->total_wait_time + global.absolute_time - buffer_order->wait_time_begin > AVOID_WAIT_TIME){

			//debug_printf("t : %ld      timeout !\n", global.absolute_time);

			// On remplace la trajectoire courante
			ROADMAP_add_order(  TRAJECTORY_STOP,
								0,
								0,
								0,					//teta
								PROP_ABSOLUTE,		//relative
								PROP_NOW,			//maintenant
								ANY_WAY,	//sens de marche
								NOT_BORDER_MODE,	//mode bordure
								PROP_END_AT_POINT, 	//mode multipoints
								FAST,				//Vitesse
								NO_ACKNOWLEDGE,
								CORRECTOR_ENABLE,
								AVOID_DISABLED,
								0
							);

			AVOIDANCE_said_foe_detected(TRUE, FALSE);

		}else if(AVOIDANCE_target_safe(buffer_order->way, FALSE) == FALSE){
			//debug_printf("t : %ld      free !\n", global.absolute_time);
			//debug_printf("Rien sur la trajectoire %dx %dy\n", buffer_order->x, buffer_order->y);
			buffer_order->total_wait_time += global.absolute_time - buffer_order->wait_time_begin;
			ROADMAP_add_simple_order(*buffer_order, TRUE, FALSE, TRUE);
			ROADMAP_launch_next_order();
		}
	}
}

GEOMETRY_point_t avoid_poly[4];

bool_e AVOIDANCE_target_safe(way_e way, bool_e verbose){
	Sint32 vtrans;		//[mm/4096/5ms]
	Sint16 teta;		//[rad/4096]
	Sint16 sin, cos;	//[/4096]
	Sint16 sinus, cosinus;	//[/4096]

	adversary_t *adversaries;
	Uint8 max_foes;
	Uint8 i;
	order_t current_order = COPILOT_get_current_order();

	Sint32 avoidance_rectangle_min_x;
	Sint32 avoidance_rectangle_max_x;
	Sint32 avoidance_rectangle_width_y_min;
	Sint32 avoidance_rectangle_width_y_max;

	Uint32 breaking_acceleration;
	Uint32 current_speed;
	Uint32 break_distance;
	Uint32 respect_distance;
	Uint32 slow_distance;

	Sint32 relative_foe_x;
	Sint32 relative_foe_y;

	static time32_t last_time_refresh_avoid_displayed = 0;
	UNUSED_VAR(last_time_refresh_avoid_displayed);

	vtrans = global.vitesse_translation;
	teta = global.position.teta;

	COS_SIN_4096_get(teta, &cos, &sin);
	adversaries = DETECTION_get_adversaries(&max_foes); // R�cup�ration des adversaires

	bool_e in_path = FALSE;	//On suppose que pas d'adversaire dans le chemin
	bool_e in_slow_zone = FALSE;

	/*	On d�finit un "rectangle d'�vitement" comme la somme :
	 * 		- du rectangle que le robot va recouvrir s'il d�cide de freiner maintenant.
	 *  	- du rectangle de "respect" qui nous s�pare de l'adversaire lorsqu'on se sera arret�
	 *  On calcule la position relative des robots adverses pour savoir s'ils se trouvent dans ce rectangle
	 */

	if(PILOT_get_in_rush()){
		/*[mm/4096/5ms/5ms]*/	breaking_acceleration = BIG_ACCELERATION_AVOIDANCE_RUSH;
		/*[mm/4096/5ms]*/		current_speed = (Uint32)(absolute(vtrans)*1);
		/*[mm]*/				break_distance = SQUARE(current_speed)/(2*breaking_acceleration) >> 12;	//distance que l'on va parcourir si l'on d�cide de freiner maintenant. (Division par 4096 car on calcule avec des variables /4096)
		/*[mm]*/				respect_distance = 300 + ((QS_WHO_AM_I_get() == SMALL_ROBOT)? SMALL_ROBOT_RESPECT_DIST_MIN : BIG_ROBOT_RESPECT_DIST_MIN);	//Distance � laquelle on souhaite s'arr�ter
		/*[mm]*/				slow_distance = ((QS_WHO_AM_I_get() == SMALL_ROBOT)? SMALL_ROBOT_DIST_MIN_SPEED_SLOW : BIG_ROBOT_DIST_MIN_SPEED_SLOW);	//Distance � laquelle on souhaite ralentir
	}else{
		/*[mm/4096/5ms/5ms]*/	breaking_acceleration = (QS_WHO_AM_I_get() == SMALL_ROBOT)?SMALL_ACCELERATION_AVOIDANCE : BIG_ACCELERATION_AVOIDANCE;
		/*[mm/4096/5ms]*/		current_speed = (Uint32)(absolute(vtrans)*1);
		/*[mm]*/				break_distance = SQUARE(current_speed)/(2*breaking_acceleration) >> 12;	//distance que l'on va parcourir si l'on d�cide de freiner maintenant. (Division par 4096 car on calcule avec des variables /4096)
		/*[mm]*/				respect_distance = ((QS_WHO_AM_I_get() == SMALL_ROBOT)? SMALL_ROBOT_RESPECT_DIST_MIN : BIG_ROBOT_RESPECT_DIST_MIN);	//Distance � laquelle on souhaite s'arr�ter
		/*[mm]*/				slow_distance = ((QS_WHO_AM_I_get() == SMALL_ROBOT)? SMALL_ROBOT_DIST_MIN_SPEED_SLOW : BIG_ROBOT_DIST_MIN_SPEED_SLOW);	//Distance � laquelle on souhaite ralentir

		// Correction d�geu � J-1 de la coupe
		//TODO elle est belle celle ci !!!
		if(!HOKUYO_isWorkingWell() && I_AM_SMALL()){
			break_distance += 100;
		}
	}

	if(active_small_avoidance == FALSE){ // Evitement normal

		avoidance_rectangle_width_y_min = -((FOE_SIZE + ((QS_WHO_AM_I_get() == SMALL_ROBOT)?SMALL_ROBOT_WIDTH:BIG_ROBOT_WIDTH))/2 + offset_avoid.Xright);
		avoidance_rectangle_width_y_max = (FOE_SIZE + ((QS_WHO_AM_I_get() == SMALL_ROBOT)?SMALL_ROBOT_WIDTH:BIG_ROBOT_WIDTH))/2 + offset_avoid.Xleft;

		if(way == FORWARD || way == ANY_WAY)	//On avance
			avoidance_rectangle_max_x = break_distance + respect_distance + offset_avoid.Yfront;
		else
			avoidance_rectangle_max_x = -100;

		if(way == BACKWARD || way == ANY_WAY)	//On recule
			avoidance_rectangle_min_x = -(break_distance + respect_distance + offset_avoid.Yback);
		else
			avoidance_rectangle_min_x = 100;

	}else{
		//En mode small avoidance, on consid�re que la demi-taille de l'adversaire est FOE_SIZE/2
		avoidance_rectangle_width_y_min = -(FOE_SIZE/2 + ((QS_WHO_AM_I_get() == SMALL_ROBOT)?SMALL_ROBOT_WIDTH:BIG_ROBOT_WIDTH))/2;
		avoidance_rectangle_width_y_max = (FOE_SIZE/2 + ((QS_WHO_AM_I_get() == SMALL_ROBOT)?SMALL_ROBOT_WIDTH:BIG_ROBOT_WIDTH))/2;

		if(way == FORWARD || way == ANY_WAY)	//On avance
			avoidance_rectangle_max_x = break_distance + respect_distance;
		else
			avoidance_rectangle_max_x = -100;

		if(way == BACKWARD || way == ANY_WAY)	//On recule
			avoidance_rectangle_min_x = -(break_distance + respect_distance);
		else
			avoidance_rectangle_min_x = 100;

	}

#if DISPLAY_AVOIDANCE_POLY
	if(global.absolute_time - last_time_refresh_avoid_displayed > WAIT_TIME_DISPLAY_AVOID){

		Sint16 angle[4];
		angle[0] = global.position.teta + atan2(avoidance_rectangle_width_y_max, avoidance_rectangle_max_x)*4096;
		angle[1] = global.position.teta + atan2(avoidance_rectangle_width_y_min, avoidance_rectangle_max_x)*4096;
		angle[2] = global.position.teta + atan2(avoidance_rectangle_width_y_min, avoidance_rectangle_min_x)*4096;
		angle[3] = global.position.teta + atan2(avoidance_rectangle_width_y_max, avoidance_rectangle_min_x)*4096;

		Uint16 longueur[4];
		longueur[0] = GEOMETRY_distance((GEOMETRY_point_t){0, 0}, (GEOMETRY_point_t){avoidance_rectangle_width_y_max, avoidance_rectangle_max_x});
		longueur[1] = GEOMETRY_distance((GEOMETRY_point_t){0, 0}, (GEOMETRY_point_t){avoidance_rectangle_width_y_min, avoidance_rectangle_max_x});
		longueur[2] = GEOMETRY_distance((GEOMETRY_point_t){0, 0}, (GEOMETRY_point_t){avoidance_rectangle_width_y_min, avoidance_rectangle_min_x});
		longueur[3] = GEOMETRY_distance((GEOMETRY_point_t){0, 0}, (GEOMETRY_point_t){avoidance_rectangle_width_y_max, avoidance_rectangle_min_x});

		avoid_poly[0] = (GEOMETRY_point_t){ecretage_debug_rect(global.position.x+cos4096(angle[0])*longueur[0]), ecretage_debug_rect(global.position.y+sin4096(angle[0])*longueur[0])};
		avoid_poly[1] = (GEOMETRY_point_t){ecretage_debug_rect(global.position.x+cos4096(angle[1])*longueur[1]), ecretage_debug_rect(global.position.y+sin4096(angle[1])*longueur[1])};
		avoid_poly[2] = (GEOMETRY_point_t){ecretage_debug_rect(global.position.x+cos4096(angle[2])*longueur[2]), ecretage_debug_rect(global.position.y+sin4096(angle[2])*longueur[2])};
		avoid_poly[3] = (GEOMETRY_point_t){ecretage_debug_rect(global.position.x+cos4096(angle[3])*longueur[3]), ecretage_debug_rect(global.position.y+sin4096(angle[3])*longueur[3])};

		Uint8 nb_point = 4;
		Uint16 max = AROUND_UP((nb_point+1)/3.);
		CAN_msg_t msg;
		msg.sid = DEBUG_AVOIDANCE_POLY;
		msg.size = SIZE_DEBUG_AVOIDANCE_POLY;
		for(i=0;i<max;i++){
			Uint8 num = 0;
			if(i==0)
				msg.data.debug_avoidance_poly.new_polygone = TRUE;
			else
				msg.data.debug_avoidance_poly.new_polygone = FALSE;

			if(i*3 < nb_point){
				num++;
				msg.data.debug_avoidance_poly.point[0].x = (Uint8)(avoid_poly[i*3].x >> 4);
				msg.data.debug_avoidance_poly.point[0].y = (Uint8)(avoid_poly[i*3].y >> 4);
			}else if(i*3 == nb_point){
				num++;
				msg.data.debug_avoidance_poly.point[0].x = (Uint8)(avoid_poly[0].x >> 4);
				msg.data.debug_avoidance_poly.point[0].y = (Uint8)(avoid_poly[0].y >> 4);
			}

			if(i*3+1 < nb_point){
				num++;
				msg.data.debug_avoidance_poly.point[1].x = (Uint8)(avoid_poly[i*3+1].x >> 4);
				msg.data.debug_avoidance_poly.point[1].y = (Uint8)(avoid_poly[i*3+1].y >> 4);
			}else if(i*3+1 == nb_point){
				num++;
				msg.data.debug_avoidance_poly.point[1].x = (Uint8)(avoid_poly[0].x >> 4);
				msg.data.debug_avoidance_poly.point[1].y = (Uint8)(avoid_poly[0].y >> 4);
			}

			if(i*3+2 < nb_point){
				num++;
				msg.data.debug_avoidance_poly.point[2].x = (Uint8)(avoid_poly[i*3+2].x >> 4);
				msg.data.debug_avoidance_poly.point[2].y = (Uint8)(avoid_poly[i*3+2].y >> 4);
			}else if(i*3+2 == nb_point){
				num++;
				msg.data.debug_avoidance_poly.point[2].x = (Uint8)(avoid_poly[0].x >> 4);
				msg.data.debug_avoidance_poly.point[2].y = (Uint8)(avoid_poly[0].y >> 4);
			}

			msg.data.debug_avoidance_poly.first_point_number = num;
			SECRETARY_send_canmsg_from_it(&msg);
		}

		last_time_refresh_avoid_displayed = global.absolute_time;
	}
#endif

	for(i=0; i<max_foes; i++){

		if(adversaries[i].enable){
			COS_SIN_4096_get(adversaries[i].angle, &cosinus, &sinus);
			relative_foe_x = (((Sint32)(cosinus)) * adversaries[i].dist) >> 12;		//[rad/4096] * [mm] / 4096 = [mm]
			relative_foe_y = (((Sint32)(sinus))   * adversaries[i].dist) >> 12;		//[rad/4096] * [mm] / 4096 = [mm]

			if(		relative_foe_y > avoidance_rectangle_width_y_min && 	relative_foe_y < avoidance_rectangle_width_y_max
				&& 	relative_foe_x > avoidance_rectangle_min_x 		 && 	relative_foe_x < avoidance_rectangle_max_x)
				{
					in_path = TRUE;	//On est dans le rectangle d'�vitement !!!
					adversary = adversaries[i]; // On sauvegarde l'adversaire nous ayant fait �vit�
					adv_hokuyo = i < HOKUYO_MAX_FOES;
				}
		}
	}

	if(in_path == FALSE && current_order.trajectory == TRAJECTORY_TRANSLATION){

		avoidance_rectangle_width_y_min = -((FOE_SIZE + ((QS_WHO_AM_I_get() == SMALL_ROBOT)?SMALL_ROBOT_WIDTH:BIG_ROBOT_WIDTH))/2 + offset_avoid.Xright + 50);
		avoidance_rectangle_width_y_max = (FOE_SIZE + ((QS_WHO_AM_I_get() == SMALL_ROBOT)?SMALL_ROBOT_WIDTH:BIG_ROBOT_WIDTH))/2 + offset_avoid.Xleft + 50;

		if(way == FORWARD || way == ANY_WAY)	//On avance
			avoidance_rectangle_max_x = break_distance + slow_distance + offset_avoid.Yfront;
		else
			avoidance_rectangle_max_x = 0;

		if(way == BACKWARD || way == ANY_WAY)	//On recule
			avoidance_rectangle_min_x = -(break_distance + slow_distance + offset_avoid.Yback);
		else
			avoidance_rectangle_min_x = 0;

		for(i=0; i<max_foes; i++){

			if(adversaries[i].enable){
				COS_SIN_4096_get(adversaries[i].angle, &cosinus, &sinus);
				relative_foe_x = (((Sint32)(cosinus)) * adversaries[i].dist) >> 12;		//[rad/4096] * [mm] / 4096 = [mm]
				relative_foe_y = (((Sint32)(sinus))   * adversaries[i].dist) >> 12;		//[rad/4096] * [mm] / 4096 = [mm]

				if(		relative_foe_y > avoidance_rectangle_width_y_min && 	relative_foe_y < avoidance_rectangle_width_y_max
					&& 	relative_foe_x > avoidance_rectangle_min_x 		 && 	relative_foe_x < avoidance_rectangle_max_x)
					{
						PILOT_set_speed(SLOW_TRANSLATION_AND_FAST_ROTATION);
						in_slow_zone = TRUE;
					}
			}
		}
		if(in_slow_zone == FALSE)
			PILOT_set_speed(current_order.speed);
	}

	return in_path;
}


/*bool_e AVOIDANCE_target_safe_curve(way_e way, bool_e verbose){
	Sint32 vtrans;		//[mm/4096/5ms]
	Sint32 vrot;		//[rad/4096/1024/5ms]
	Sint16 teta;		//[rad/4096]
	Sint16 px;			//[mm]
	Sint16 py;			//[mm]

	adversary_t *adversaries;
	Uint8 max_foes;
	Uint8 i;
	order_t current_order = COPILOT_get_current_order();

	Sint32 avoidance_rectangle_min_x;
	Sint32 avoidance_rectangle_max_x;
	Sint32 avoidance_rectangle_width_y_min;
	Sint32 avoidance_rectangle_width_y_max;

	Uint32 breaking_acceleration;
	Uint32 current_speed;
	Uint32 break_distance;
	Uint32 respect_distance;

	GEOMETRY_point_t pts[NB_SAMPLE];

	static time32_t last_time_refresh_avoid_displayed = 0;

	vrot = global.vitesse_rotation;
	vtrans = global.vitesse_translation/12;
	teta = global.position.teta;

	adversaries = DETECTION_get_adversaries(&max_foes); // R�cup�ration des adversaires

	bool_e in_path = FALSE;	//On suppose que pas d'adversaire dans le chemin


	breaking_acceleration = (QS_WHO_AM_I_get() == SMALL_ROBOT)?SMALL_ROBOT_ACCELERATION_NORMAL:BIG_ROBOT_ACCELERATION_NORMAL;
	current_speed = (Uint32)(absolute(vtrans)*1);
	break_distance = SQUARE(current_speed)/(4*breaking_acceleration);	//distance que l'on va parcourir si l'on d�cide de freiner maintenant.
	respect_distance = (QS_WHO_AM_I_get() == SMALL_ROBOT)?SMALL_ROBOT_RESPECT_DIST_MIN:BIG_ROBOT_RESPECT_DIST_MIN;	//Distance � laquelle on souhaite s'arr�ter

	avoidance_rectangle_width_y_min = -((FOE_SIZE + ((QS_WHO_AM_I_get() == SMALL_ROBOT)?SMALL_ROBOT_WIDTH:BIG_ROBOT_WIDTH))/2 + offset_avoid.Xright);
	avoidance_rectangle_width_y_max = (FOE_SIZE + ((QS_WHO_AM_I_get() == SMALL_ROBOT)?SMALL_ROBOT_WIDTH:BIG_ROBOT_WIDTH))/2 + offset_avoid.Xleft;

	if(way == FORWARD || way == ANY_WAY)	//On avance
		avoidance_rectangle_max_x = break_distance + respect_distance + offset_avoid.Yfront;
	else
		avoidance_rectangle_max_x = 0;

	if(way == BACKWARD || way == ANY_WAY)	//On recule
		avoidance_rectangle_min_x = -(break_distance + respect_distance + offset_avoid.Yback);
	else
		avoidance_rectangle_min_x = 0;

	#if DISABLE_CAN

		if(global.absolute_time - last_time_refresh_avoid_displayed > WAIT_TIME_DISPLAY_AVOID){
				last_time_refresh_avoid_displayed = global.absolute_time;
			}

	#endif

	for(i=0; i<max_foes; i++){

		if(adversaries[i].enable){

		}
	}

	return in_path;
}*/

// Retourne si un adversaire est dans la chemin entre nous et la position
bool_e AVOIDANCE_foe_in_zone(bool_e verbose, Sint16 x, Sint16 y, bool_e check_on_all_traject){
	bool_e inZone;
	Uint8 i;
	Sint32 a, b, c; // avec a, b et c les coefficients de la droite entre nous et la cible
	Sint32 NCx, NCy, NAx, NAy;

	Sint16 px;			//[mm]
	Sint16 py;			//[mm]


	Uint32 width_distance;
	Uint32 length_distance;

	if(QS_WHO_AM_I_get() == BIG_ROBOT){
		width_distance = BIG_ROBOT_RESPECT_DIST_MIN;
		length_distance = FOE_SIZE + BIG_ROBOT_WIDTH;
	}else{
		width_distance = SMALL_ROBOT_RESPECT_DIST_MIN;
		length_distance = FOE_SIZE + SMALL_ROBOT_WIDTH;
	}

	adversary_t *adversaries;
	Uint8 max_foes;
	adversaries = DETECTION_get_adversaries(&max_foes); // R�cup�ration des adversaires

	px = global.position.x;
	py = global.position.y;

	a = y - py;
	b = -(x - px);
	c = -(Sint32)px*y + (Sint32)py*x;

	if(px == x && py == y)
		return FALSE;


	inZone = FALSE;	//On suppose que pas d'adversaire dans le chemin

	for (i=0; i<max_foes; i++)
	{
		if (adversaries[i].enable){
			// d(D, A) < L
			// D : droite que le robot empreinte pour aller au point
			// A : Point adversaire
			// L : Largeur du robot maximum * 2

			if(absolute((Sint32)a*adversaries[i].x + (Sint32)b*adversaries[i].y + c) / (Sint32)sqrt((Sint32)a*a + (Sint32)b*b) < length_distance){
				// /NC./NA � [0,NC*d]
				// /NC : Vecteur entre nous et le point cible
				// /NA : Vecteur entre nous et l'adversaire
				// NC : Distance entre nous et le point cible
				// d : Distance d'�vitement de l'adversaire (longueur couloir)

				NCx = x - px;
				NCy = y - py;

				NAx = adversaries[i].x - px;
				NAy = adversaries[i].y - py;

				Sint32 dist_point_to_point = GEOMETRY_distance((GEOMETRY_point_t) {px, py}, (GEOMETRY_point_t) {x, y});

				if((NCx*NAx + NCy*NAy) >= dist_point_to_point*100
						&& (
							(!check_on_all_traject &&(NCx*NAx + NCy*NAy) < dist_point_to_point*width_distance)
							||
							(check_on_all_traject &&(NCx*NAx + NCy*NAy) < SQUARE(dist_point_to_point)))){

					adversary = adversaries[i]; // On sauvegarde l'adversaire nous ayant fait �vit�
					adv_hokuyo = i < HOKUYO_MAX_FOES;
					inZone = TRUE;
				}
			}
		}
	}
	return inZone;
}

void AVOIDANCE_said_foe_detected(bool_e timeout, bool_e in_wait){
	SECRETARY_send_foe_detected(adversary.x, adversary.y, adversary.dist, adversary.angle, adv_hokuyo, timeout, in_wait);
}

void AVOIDANCE_process_CAN_msg(CAN_msg_t *msg){

	switch(msg->sid){
		case PROP_OFFSET_AVOID:
			#if USE_ACT_AVOID
				offset_avoid.Xleft = msg->data.prop_offset_avoid.x_left;
				offset_avoid.Xright = msg->data.prop_offset_avoid.x_right;
				offset_avoid.Yfront = msg->data.prop_offset_avoid.y_front;
				offset_avoid.Yback = msg->data.prop_offset_avoid.y_back;
			#endif
			break;

		case PROP_CUSTOM_AVOIDANCE:
			active_small_avoidance = msg->data.prop_custom_avoidance.active_small_avoidance;
			break;
	}
}

bool_e AVOIDANCE_foe_near(){
	Uint8 i;
	adversary_t *adversaries;
	Uint8 max_foes;
	bool_e foe_near = FALSE;
	adversaries = DETECTION_get_adversaries(&max_foes); // R�cup�ration des adversaires

	for(i=0; i<max_foes; i++){
		if(adversaries[i].enable && adversaries[i].dist <= DISTANCE_EVITEMENT_ROTATION)
				foe_near = TRUE;
	}
	return foe_near;
}

static Sint16 ecretage_debug_rect(Sint16 val){
	if(val < 0)
		return 0;
	else if(val > 2999)
		return 2999;
	else
		return val;
}
