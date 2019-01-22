/**
 * Club Robot ESEO 2018-2019
 *
 * @file beacon.h
 * @brief Detection des positions des balises fixes
 * @author Valentin
 */

#ifndef KALMAN_BEACON_H_
#define KALMAN_BEACON_H_

#include "../QS/QS_all.h"
#include "../QS/QS_hokuyo/hokuyo.h"
#include "../QS/QS_maths.h"

/**
 * Nombre de balises.
 */
#define NB_BEACONS 		(3)

/**
 * Nombre maximal de points qui peuvent être détecté par l'hukuyo lors d'un scan.
 */
#define NB_MAX_POINTS_PER_BEACON	(255)


typedef struct {
	GEOMETRY_point_t pos;
	Uint32 dist;
	Sint16 angle;
} Beacon_point_t;

typedef struct{
	GEOMETRY_point_t pos_theo;		// Position théorique de la balise
	GEOMETRY_point_t pos_real;		// Position réelle de la balise
	Uint16 dist;					// Distance de la balise par rapport au robot
	Sint16 angle;					// Angle relatif de la balise par rapport au robot
	Beacon_point_t points[NB_MAX_POINTS_PER_BEACON];		// Point scannés par l'hokuyo
	Uint8 nb_points;				// Nombre de points scannés par l'hokuyo
	bool_e detected;				// Statut de la balise (1:détecté, 0:non détectée)
} Beacon_t;


/**
 * Initialisation des balises.
 */
void BEACON_init();

/**
 * Début du traitement des points d'un nouveau scan hokuyo.
 * @param robot_pos_before_measure la position du robot avant la mesure.
 * @param robot_pos_after_measure la position du robot après la mesure.
 */
void BEACON_start_process_points(position_t robot_pos_before_measure, position_t robot_pos_after_measure);

/**
 * Traiement d'un point d'un nouveau scan hokuyo.
 * @param point un nouveau point à traiter.
 */
void BEACON_process_points(HOKUYO_point_position point, Sint16 teta_relative, Sint32 distance);

/**
 * Fin du traitement des points d'un nouveau scan hokuyo.
 * Calcul des positions réelles des balises par rapport au robot.
 */
void BEACON_finish_process_points();

/**
 * Getter of beacons.
 * @returns les instances des balises.
 */
Beacon_t * BEACON_get_beacons();

#endif /* KALMAN_BEACON_H_ */
