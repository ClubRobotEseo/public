/**
 * Club Robot ESEO 2018-2019
 *
 * @file beacon.c
 * @brief Detection des positions des balises fixes
 * @author Valentin
 */

#include "beacon.h"
#include "../odometry.h"
#include "../QS/QS_outputlog.h"

/**
 * Marge de détection des balises.
 * Un point ayant une distance inférieure à cette marge est considéré valide.
 */
#define BEACON_DETECTION_MARGIN		(300)

/**
 * Nombre de points minimal pour considérer qu'une balise est détectée.
 */
#define BEACON_NB_MIN_POINTS		(5)

/**
 * Distance rajoutée la moyenne des points mesurée pour obtenir le centre (ici pour des balises de 60mm de diamètre).
 */
#define BEACON_DELTA_CENTRE (22)

/**
 * Nombre de points qui doivent être vérifiés pour éliminer les effets de bord.
 */
#define BEACON_SIDE_EFFECT_NB_POINTS (2)

/**
 * Distance maximale entre deux points mesurés. Au-delà de cette distance, on considère qu'il y a un effet de bord.
 */
#define BEACON_SIDE_EFFECT_MAX_DIST (30)

/**
 * Coefficient pour le calcul de la position du robot pendant la mesure.
 * Ce coefficient est appliqué à la position du robot avant la mesure.
 */
#define BEACON_COEF_BEFORE 	(0.6)

/**
 * Coefficient pour le calcul de la position du robot pendant la mesure.
 * Ce coefficient est appliqué à la position du robot après la mesure.
 */
#define BEACON_COEF_AFTER 	(1 - BEACON_COEF_BEFORE)


// Variables globales
static Beacon_t beacons[NB_BEACONS];	// Déclaration des trois balises fixes.
static position_t robot_pos_before;		// Position du robot avant la mesure.
static position_t robot_pos_after;		// Position du robot après la mesure.

// Fonctions privées
static void BEACON_compute(Beacon_t * beacon, position_t robot, position_t hokuyo);
static Beacon_point_t BEACON_find_center(Beacon_point_t points[], Uint16 nb_points, position_t hokuyo);
static GEOMETRY_point_t BEACON_compute_point(Sint32 dist, Sint16 angle_rel, position_t hokuyo);
static position_t BEACON_compute_hokuyo_pos(position_t robot);
static position_t BEACON_compute_robot_pos();
static Sint16 BEACON_modulo_centered_on_reference(Sint16 angle, Sint16 reference);


void BEACON_init() {
	color_e color = ODOMETRY_get_color();

	// Initialisation des positions théoriques
	if(color == BOT_COLOR) {
		debug_printf("Init with bot color");
		beacons[0].pos_theo = (GEOMETRY_point_t) {1000, 3094};
		beacons[1].pos_theo = (GEOMETRY_point_t) {50, -94};
		beacons[2].pos_theo = (GEOMETRY_point_t) {1950, -94};
	} else {
		debug_printf("Init with top color");
		beacons[0].pos_theo = (GEOMETRY_point_t) {1000, -94};
		beacons[1].pos_theo = (GEOMETRY_point_t) {50, 3094};
		beacons[2].pos_theo = (GEOMETRY_point_t) {1950, 3094};
	}
}


void BEACON_start_process_points(position_t robot_pos_before_measure, position_t robot_pos_after_measure) {
	Uint8 i;
	for(i = 0; i < NB_BEACONS; i++) {
		beacons[i].nb_points = 0;
		beacons[i].detected = FALSE;
	}

	robot_pos_before = robot_pos_before_measure;
	robot_pos_after = robot_pos_after_measure;
}


void BEACON_process_points(HOKUYO_point_position point, Sint16 teta_relative, Sint32 distance) {
	Uint8 i;
	Uint16 index;
	for(i = 0; i < NB_BEACONS; i++) {
		if(beacons[i].nb_points < NB_MAX_POINTS_PER_BEACON
		&& is_in_circle((GEOMETRY_point_t) {point.coordX, point.coordY}, (GEOMETRY_circle_t) {beacons[i].pos_theo, BEACON_DETECTION_MARGIN})) {
			index = beacons[i].nb_points;
			beacons[i].points[index].dist = distance;
			beacons[i].points[index].angle = teta_relative;
			beacons[i].points[index].pos = (GEOMETRY_point_t) {point.coordX, point.coordY};
			beacons[i].nb_points++;
		}
	}
}


void BEACON_finish_process_points() {
	Uint8 i;
	static time32_t local_time = 0;
	bool_e hasToDisplay = FALSE;

	if(global.absolute_time > local_time + 500) {
		local_time = global.absolute_time;
		hasToDisplay = TRUE;
	}

	// Calcul de la position du robot
	position_t robot_pos_during = BEACON_compute_robot_pos();

	// Calcul de la position de l'hokuyo
	position_t hokuyo_pos = BEACON_compute_hokuyo_pos(robot_pos_during);

	for(i = 0; i < NB_BEACONS; i++) {
		if(hasToDisplay) {
			debug_printf("beacon %d: nb_points=%d\n", i, beacons[i].nb_points);
			if(beacons[i].nb_points >= BEACON_NB_MIN_POINTS) {
				beacons[i].detected = TRUE;
				debug_printf("Robot before=(%d,  %d,  %d)\n", robot_pos_before.x, robot_pos_before.y, robot_pos_before.teta);
				debug_printf("Robot before=(%d,  %d,  %d)\n", robot_pos_after.x, robot_pos_after.y, robot_pos_after.teta);
				debug_printf("Balise theo=(%d, %d)\n", beacons[i].pos_theo.x, beacons[i].pos_theo.y);
				BEACON_compute(&(beacons[i]), robot_pos_during, hokuyo_pos);
			}
		}
	}
}



/**
 * Calcul des données necessaires au traitement d'une balise (centre, distance, angle).
 * @param beacon la balise concernée
 * @param robot la position du robot pendant la mesure.
 * @param hokuyo la position de l'hokuyo durant la mesure.
 */
static void BEACON_compute(Beacon_t * beacon, position_t robot, position_t hokuyo) {

	for(Uint8 i = 0; i < beacon->nb_points; i++) {
		debug_printf("> %4d,  %4d,  %4d,  %5ld\n", beacon->points[i].pos.x, beacon->points[i].pos.y, beacon->points[i].angle, beacon->points[i].dist);
	}

	// Calcul de la position réelle de la balise (position absolue)
	Beacon_point_t centre = BEACON_find_center(beacon->points, beacon->nb_points, hokuyo);

	// Affectation des données calculées dans l'instance de la balise.
	beacon->pos_real = centre.pos;
	beacon->dist = centre.dist;

	// Calcul de l'angle relatif de la balise par rapport au robot
	beacon->angle = robot.teta - atan2_4096(beacon->pos_real.y - robot.y, beacon->pos_real.x - robot.x);

	debug_printf("Balise theo=(%d, %d) real=(%d, %d) dist=%d angle=%d\n", beacon->pos_theo.x, beacon->pos_theo.y, beacon->pos_real.x, beacon->pos_real.y, beacon->dist, beacon->angle);
}

/**
 * Calcul du centre d'une balise.
 * @param points les points mesurés.
 * @param nb_points le nombre de points mesurés.
 * @param hokuyo la position de l'hokuyo durant la mesure.
 * @return la position du centre de la balise.
 */
static Beacon_point_t BEACON_find_center(Beacon_point_t points[], Uint16 nb_points, position_t hokuyo) {
	Beacon_point_t centre;
	Uint64 sum_dist = 0;
	Sint64 sum_angle = 0;
	Uint16 i;

	Uint16 ind_start = 0; 				// Index de la première mesure prise en compte.
	Uint16 ind_end = nb_points - 1;		// Index de la dernière mesure prise en compte.

	// Vérification des effets de bord
	if(nb_points > 3 * BEACON_SIDE_EFFECT_NB_POINTS) {
		// Calcul de l'index de début.
		for(Uint16 i = 0; i < BEACON_SIDE_EFFECT_NB_POINTS; i++) {
			int dist = GEOMETRY_distance(points[i].pos, points[i+1].pos);
			if(dist > BEACON_SIDE_EFFECT_MAX_DIST) {
				ind_start = i + 1;
			}
		}

		// Calcul de l'index de fin.
		for(i = nb_points - 1; i > nb_points - 1 - BEACON_SIDE_EFFECT_NB_POINTS; i--) {
			int dist = GEOMETRY_distance(points[i].pos, points[i-1].pos);
			if(dist > BEACON_SIDE_EFFECT_MAX_DIST) {
				ind_end = i - 1;
			}
		}
	}

	assert(ind_start < ind_end);

	// Calcul des moyennes de distance et d'angle des points mesurés et pris en compte
	for(i = ind_start; i <= ind_end; i++) {
		sum_dist += points[i].dist;
		sum_angle += BEACON_modulo_centered_on_reference(points[i].angle, points[ind_start].angle);
	}

	Uint16 nb_points_considered = ind_end - ind_start + 1;
	Uint32 avg_dist = sum_dist / nb_points_considered;
	Sint16 avg_angle = sum_angle / nb_points_considered;

	avg_dist = avg_dist + BEACON_DELTA_CENTRE;
	avg_angle = GEOMETRY_modulo_angle(avg_angle);

	centre.dist = avg_dist;
	centre.angle = avg_angle;
	centre.pos = BEACON_compute_point(avg_dist, avg_angle, hokuyo);

	return centre;
}


/**
 * Calcul de la position d'un point mesuré par l'hokuyo en prenant en compte le décalage de l'hokuyo par rapport au centre du robot.
 * @param dist la distance mesurée.
 * @param angle_rel l'angle relatif mesuré.
 * @param hokuyo la position de l'hokuyo pendant la mesure.
 * @return la position absolue du point.
 */
static GEOMETRY_point_t BEACON_compute_point(Sint32 dist, Sint16 angle_rel, position_t hokuyo) {
	GEOMETRY_point_t point;
	Sint16 cos;								// Cosinus de l'angle de mesure en cours
	Sint16 sin;								// Sinus de l'angle de mesure en cours

	// Calcul de la position du point
	Sint16 angle_abs = GEOMETRY_modulo_angle(hokuyo.teta + angle_rel);
	COS_SIN_4096_get(angle_abs, &cos, &sin);
	point.x = hokuyo.x + dist*cos/4096.;
	point.y = hokuyo.y + dist*sin/4096.;

	return point;
}


/**
 * Calcul de la position hokuyo pendant la mesure.
 * @param robot la position du robot pendant la mesure.
 */
static position_t BEACON_compute_hokuyo_pos(position_t robot) {
	position_t hokuyo;
	Sint16 offset_pos_x, offset_pos_y;		// Offset sur les valeurs de position sur le terrain par rapport au placement de l'hokuyo sur le robot
	Sint16 cos;						// Cosinus de l'angle du robot
	Sint16 sin;						// Sinus de l'angle du robot

	if(QS_WHO_AM_I_get() == BIG_ROBOT){
		offset_pos_x = HOKUYO_OFFSET_BIG_POS_X;
		offset_pos_y = HOKUYO_OFFSET_BIG_POS_Y;
	}else{
		offset_pos_x = HOKUYO_OFFSET_SMALL_POS_X;
		offset_pos_y = HOKUYO_OFFSET_SMALL_POS_Y;
	}

	// Calcul des offsets de la position hokuyo en X et Y
	COS_SIN_4096_get(robot.teta, &cos, &sin);
	hokuyo.x = robot.x - offset_pos_y*sin/4096. + offset_pos_x*cos/4096.;
	hokuyo.y = robot.y + offset_pos_y*cos/4096. + offset_pos_x*sin/4096.;
	hokuyo.teta = robot.teta;

	return hokuyo;
}


/**
 * Calcul de la position du robot pendant la mesure.
 */
static position_t BEACON_compute_robot_pos() {
	position_t robot_pos_during;

	// Recalcule de l'angle du robot après la mesure pour être sur d'être dans le même intervalle que l'angle avant la mesure.
	// Avoir cet angle à 2*pi près ne suffit pas !!!
	Sint16 teta_after_recomputed = BEACON_modulo_centered_on_reference(robot_pos_after.teta, robot_pos_before.teta);

	//Ici, 0.6 semble plus efficace de faire une moyenne pondéré (0.6 * pos_before + 0.4 * pos_after)
	// plutôt qu'une simple moyenne.
	robot_pos_during.x = BEACON_COEF_BEFORE * robot_pos_before.x + BEACON_COEF_AFTER * robot_pos_after.x;
	robot_pos_during.y = BEACON_COEF_BEFORE * robot_pos_before.y + BEACON_COEF_AFTER * robot_pos_after.y;
	robot_pos_during.teta =  BEACON_COEF_BEFORE * robot_pos_before.teta + BEACON_COEF_AFTER * teta_after_recomputed;
	return robot_pos_during;
}


/**
 * Calcul du modulo d'angle par rapport à un angle de référence.
 * Cette fonction doit retourner un angle dans l'intervalle ]refrence - PI4096 ; reference + PI4096].
 * @param angle l'angle dont on doit calculer le modulo.
 * @param reference l'angle de reference.
 * @return le modulo de l'angle dans l'intervalle ]refrence - PI4096 ; reference + PI4096].
 */
static Sint16 BEACON_modulo_centered_on_reference(Sint16 angle, Sint16 reference) {
	Sint16 result = angle;
	while(angle <= reference - PI4096) {
		result += 2*PI4096;
	}
	while(angle > reference + PI4096) {
		result -= 2*PI4096;
	}

	return result;
}


Beacon_t * BEACON_get_beacons() {
	return beacons;
}
