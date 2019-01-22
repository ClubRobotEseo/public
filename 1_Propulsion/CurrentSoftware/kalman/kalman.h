/**
 * Club Robot ESEO 2018-2019
 *
 * @file kalman.h
 * @brief Implémentation d'un filtre de kalman pour le repérage en position du robot sur le terrain.
 * @author Valentin
 */

#ifndef KALMAN_KALMAN_H_
#define KALMAN_KALMAN_H_

#include "../QS/QS_all.h"

#if USE_KALMAN_FILTER

#include "beacon.h"

/**
 * Initialisation du filtre de Kalman.
 */
void KALMAN_init();

/**
 * Initialisation du vecteur d'état X avec la position initiale.
 * @param x_init l'abscisse initiale.
 * @param y_init l'ordonnée initiale.
 * @param angle_init l'angle initial.
 */
void KALMAN_init_pos(Sint16 x_init, Sint16 y_init, Sint16 angle_init);

/**
 * Phase de prédiction du filtre de Kalman.
 */
void KALMAN_predict_position();

/**
 * Phase de mise à jour du filtre de Kalman.
 */
void KALMAN_update_position();

/**
 * Application du filtre de Kalman séquentiellement pour chaque balise.
 * @param beacons les balises considérées.
 */
void KALMAN_process_main(Beacon_t * beacons);

/**
 * Affichage des matrices du filtre de Kalman.
 */
void KALMAN_display();

#endif /* USE_KALMAN_FILTER */

#endif /* KALMAN_KALMAN_H_ */
