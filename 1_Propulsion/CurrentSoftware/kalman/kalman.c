/**
 * Club Robot ESEO 2018-2019
 *
 * @file kalman.c
 * @brief Implémentation d'un filtre de kalman pour le repérage en position du robot sur le terrain.
 * @author Valentin
 */

#include "kalman.h"

#if USE_KALMAN_FILTER

#include "matrix.h"
#include "../QS/QS_maths.h"
#include "../QS/QS_outputlog.h"
#include "../odometry.h"

#define SIZE_X	(3)			// Taille du vecteur d'état X.
#define SIZE_Y	(2)			// Taille du vecteur de mesures Y.

// TODO Régler les ecart-types pour les matrices de covariance
#define DEVIATION_X_ODOMETRY	(1)		// Ecart-type du bruit des roues codeuses en X	(mm)
#define DEVIATION_Y_ODOMETRY	(1)		// Ecart-type du bruit des roues codeuses en Y	(mm)
#define DEVIATION_TETA_ODOMETRY	(10)	// Ecart-type du bruit des roues codeuses en teta (PI4096)
#define DEVIATION_DIST_HOKUYO	(5)		// Ecart-type du bruit de mesure des balises en distance (mm)
#define DEVIATION_TETA_HOKUYO	(20)	// Ecart-type du bruit de mesure des balises en angle (PI4096)


// Variable locales
static Sint32 klm_X[SIZE_X] = {0}; // Vecteur d'état (x, y, teta)
static Sint32 klm_Xp[SIZE_X] = {0}; // Prédiction du vecteur d'état (x, y, teta)
static Sint32 klm_Xp_copy[SIZE_X] = {0}; // Copie de Xp au moment de la mesure hokuyo
static Sint32 klm_A[SIZE_X][SIZE_X] = {{0}}; // Matrice de transition reliant l'état courant à l'état précédent ( X(k+1) = A.X(k) )
static Sint32 klm_Q[SIZE_X][SIZE_X] = {{0}}; // Covariance du bruit de mesures (odométrie)
static Sint32 klm_P[SIZE_X][SIZE_X] = {{0}}; // Covariance de l'erreur d'estimation
static Sint32 klm_Pp[SIZE_X][SIZE_X] = {{0}}; // Prédiction de la covariance de l'erreur d'estimation
static Sint32 klm_Pp_copy[SIZE_X][SIZE_X] = {{0}}; // Copie de Pp au moment de la mesure hokuyo

static Sint32 klm_Y[SIZE_Y] = {0}; // Vecteur de mesure (dist, alpha)
static Sint32 klm_Yi[SIZE_Y] = {0}; // Innovation du vecteur de mesure (dist, alpha)
static Sint32 klm_S[SIZE_Y][SIZE_Y] = {0}; // Covariance de l'innovation
static Sint32 klm_H[SIZE_Y][SIZE_X] = {{0}}; // Matrice d'observation reliant l'état à la mesure ( Y = H.X + B )
static Sint32 klm_R[SIZE_Y][SIZE_Y] = {{0}}; // Covariance du bruit de mesures (mesures hokuyo)
static Sint32 klm_K[SIZE_X][SIZE_Y] = {{0}}; // Gain de Kalman

static Sint32 klm_I[SIZE_X][SIZE_X] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}; // Matrice identité

// Fonctions privées
static void KALMAN_update_A();
static void KALMAN_update_H(Beacon_t *beacon);

/**
 * Initialisation du filtre de Kalman.
 */
void KALMAN_init() {
	// Initialisation des balises
	BEACON_init();

	// Initialisation de la matrice de covariance pour les erreurs d'odométrie
	klm_Q[0][0] = SQUARE(DEVIATION_X_ODOMETRY);
	klm_Q[1][1] = SQUARE(DEVIATION_Y_ODOMETRY);
	klm_Q[2][2] = SQUARE(DEVIATION_TETA_ODOMETRY);

	// Initialisation de la matrice de covariance pour les erreurs de mesures hokuyo
	klm_R[0][0] = SQUARE(DEVIATION_DIST_HOKUYO);
	klm_R[1][1] = SQUARE(DEVIATION_TETA_HOKUYO);

	// Initialisation de la matrice de transition A
	klm_A[0][0] = 1;
	klm_A[1][1] = 1;
	klm_A[2][2] = 1;

	// Initialisation de la matrice d'observation H
	klm_H[0][2] = 0;
	klm_H[1][2] = -1;
}

/**
 * Initialisation du vecteur d'état X avec la position initiale.
 * @param x_init l'abscisse initiale.
 * @param y_init l'ordonnée initiale.
 * @param angle_init l'angle initial.
 */
void KALMAN_init_pos(Sint16 x_init, Sint16 y_init, Sint16 angle_init) {
	// Affectation de la position initiale: X = X0
	klm_X[0] = x_init;
	klm_X[1] = y_init;
	klm_X[2] = angle_init;
}

/**
 * Phase de prédiction du filtre de Kalman.
 */
void KALMAN_predict_position() {
	// Mise à jour de la matrice de transition A
	KALMAN_update_A();

	// Prédiction de la position du robot via odométrie: Xp = A.X
	MATRIX_multiply((Sint32 **) &klm_Xp, (Sint32 **) klm_A, SIZE_X, SIZE_X, (Sint32 **) &klm_X, SIZE_X, 1);

	// Prédiction de l'incertitude de la position du robot: Pp = A.P.transposed(A) + Q
	Sint32 tmp_Pp_1[SIZE_X][SIZE_X] = {{0}};
	MATRIX_multiply((Sint32 **) tmp_Pp_1, (Sint32 **) klm_A, SIZE_X, SIZE_X, (Sint32 **) klm_P, SIZE_X, SIZE_X);
	Sint32 tmp_Pp_2[SIZE_X][SIZE_X] = {{0}};
	MATRIX_multiplyByTransposed((Sint32 **) tmp_Pp_2, (Sint32 **) tmp_Pp_1, SIZE_X, SIZE_X, (Sint32 **) klm_A, SIZE_X, SIZE_X);
	MATRIX_sum((Sint32 **) klm_Pp, (Sint32 **) tmp_Pp_2, (Sint32 **) klm_Q, SIZE_X, SIZE_X);

	// Mise à jour de la position
	global.position.x = klm_Xp[0];
	global.position.y = klm_Xp[1];
	global.position.teta = klm_Xp[2];
}

/**
 * Phase de mise à jour du filtre de Kalman.
 */
void KALMAN_update_position(Beacon_t *beacon) {
	// Mise à jour de la matrice d'observation H
	KALMAN_update_H(beacon);

	// Calcul de l'innovation: Yi = Y - H.Xp
	Sint32 tmp_Yi[SIZE_Y] = {0}; // H.X
	MATRIX_multiply((Sint32 **) &tmp_Yi, (Sint32 **) klm_H, SIZE_Y, SIZE_X, (Sint32 **) &klm_X, SIZE_X, 1);
	MATRIX_substract((Sint32 **) klm_Yi, (Sint32 **) &klm_Y, (Sint32 **) &tmp_Yi, SIZE_Y, 1);

	// Calul de la covariance de l'innovation: S = reverse(H.Pp.transposed(H) + R)
	Sint32 tmp_S_1[SIZE_Y][SIZE_X] = {{0}}; // H.Pp
	MATRIX_multiply((Sint32 **) tmp_S_1, (Sint32 **) klm_H, SIZE_Y, SIZE_X, (Sint32 **) klm_Pp, SIZE_X, SIZE_X);
	Sint32 tmp_S_2[SIZE_Y][SIZE_Y] = {{0}}; // H.Pp.transposed(H)
	MATRIX_multiplyByTransposed((Sint32 **) tmp_S_2, (Sint32 **) tmp_S_1, SIZE_Y, SIZE_X, (Sint32 **) klm_H, SIZE_Y, SIZE_X);
	MATRIX_sum((Sint32 **) klm_S, (Sint32 **) tmp_S_2, (Sint32 **) klm_R, SIZE_Y, SIZE_Y);

	// Calcul du gain de Kalman: K = Pp.transposed(H).reverse(S)
	Sint32 tmp_K_1[SIZE_Y][SIZE_Y] = {{0}}; // reverse(S)
	MATRIX_reverse2x2((Sint32 **) tmp_K_1, (Sint32 **) klm_S);
	Sint32 tmp_K_2[SIZE_X][SIZE_Y] = {{0}}; // Pp.transposed(H)
	MATRIX_multiplyByTransposed((Sint32 **) tmp_K_2, (Sint32 **) klm_Pp, SIZE_X, SIZE_X, (Sint32 **) klm_H, SIZE_Y, SIZE_X);
	MATRIX_multiply((Sint32 **) klm_K, (Sint32 **) tmp_K_2, SIZE_X, SIZE_Y, (Sint32 **) tmp_K_1, SIZE_Y, SIZE_Y);

	// Mise à jour position: X = Xp + K.Yi
	Sint32 tmp_X_1[SIZE_Y][SIZE_X] = {{0}};
	MATRIX_multiply((Sint32 **) tmp_X_1, (Sint32 **) klm_K, SIZE_X, SIZE_Y, (Sint32 **) klm_Yi, SIZE_Y, 1);
	MATRIX_sum((Sint32 **) klm_X, (Sint32 **) klm_Xp, (Sint32 **) tmp_X_1, SIZE_X, 1);

	// Mise à jour de l'incertitude de la position: P = (I - K.H).Pp
	Sint32 tmp_P_1[SIZE_X][SIZE_X] = {{0}}; // K.H
	MATRIX_multiply((Sint32 **) tmp_P_1, (Sint32 **) klm_K, SIZE_X, SIZE_Y, (Sint32 **) klm_H, SIZE_X, SIZE_X);
	Sint32 tmp_P_2[SIZE_X][SIZE_X] = {{0}}; // I - K.H
	MATRIX_substract((Sint32 **) tmp_P_2, (Sint32 **) klm_I, (Sint32 **) tmp_P_1, SIZE_X, SIZE_X);
	MATRIX_multiply((Sint32 **) klm_P, (Sint32 **) tmp_P_2, SIZE_X, SIZE_X, (Sint32 **) klm_Pp, SIZE_X, SIZE_X);
}

/**
 * Sauvegarde de certaines données lors d'une nouvelle mesure hokuyo.
 */
void KALMAN_new_hokuyo_measure() {
	Uint8 i, j;

	// Copie de Xp
	for(i = 0; i < SIZE_X; i++) {
		klm_Xp_copy[i] = klm_Xp[i];
	}

	// Copie de Pp
	for(i = 0; i < SIZE_X; i++) {
		for(j = 0; j < SIZE_X; j++) {
			klm_Pp_copy[i][j] = klm_Pp[i][j];
		}
	}
}

/**
 * Application du filtre de Kalman séquentiellement pour chaque balise.
 * @param beacons les balises considérées.
 */
void KALMAN_process_main(Beacon_t * beacons) {
	Uint8 i;

	for(i = 0; i < NB_BEACONS; i++) {
		if(beacons[i].detected) {
			KALMAN_update_position(&(beacons[i]));
		}
	}
}

/**
 * Mise à jour de la matrice de transition A à partir du vecteur d'état X.
 */
static void KALMAN_update_A() {
	//Sint32 teta32 = ODOMETRY_get_teta22();
	//COS_SIN_16384_get(teta32 >> 8,&cos,&sin);

	Sint16 cos,sin;
	Sint32 cos32, sin32;

	Sint64 real_speed_translation = ODOMETRY_get_real_speed_translation();

	COS_SIN_4096_get(klm_X[3], &cos, &sin);

	cos32 = (Sint32)(cos);
	sin32 = (Sint32)(sin);

	klm_A[0][2] = (Sint32)((real_speed_translation * sin32) >> 16);
	klm_A[1][2] = - (Sint32)((real_speed_translation * cos32) >> 16);
}

/**
 * Mise à jour de la matrice d'observation H à partir de la prédiction du vecteur d'état Xp.
 * @param beacon la balise considérée.
 * @param robot_pos la position du robot.
 */
static void KALMAN_update_H(Beacon_t *beacon) {
	Sint32 dist = GEOMETRY_distance(beacon->pos_real, (GEOMETRY_point_t) {klm_Xp[0], klm_Xp[1]});
	Sint32 diff_x = beacon->pos_real.x - klm_Xp[0];
	Sint32 diff_y = beacon->pos_real.y - klm_Xp[1];
	double quotient = diff_y / diff_x;

	klm_H[0][0] = - ((double) diff_x) / dist;
	klm_H[0][1] = - ((double) diff_y) / dist;
	klm_H[1][0] =   ((double) diff_y) / (SQUARE(diff_x) * (1 + SQUARE(quotient)));
	klm_H[1][1] = - ((double)    1.0) / (SQUARE(diff_x) * (1 + SQUARE(quotient)));
}

/**
 * Affichage des matrices du filtre de Kalman.
 */
void KALMAN_display() {
	debug_printf("=============KALMAN============\n\n");

	MATRIX_display((Sint32 **) &klm_X, SIZE_X, 1, "X");
	MATRIX_display((Sint32 **) &klm_Xp, SIZE_X, 1, "Xp");
	MATRIX_display((Sint32 **) &klm_A, SIZE_X, SIZE_X, "A");
	MATRIX_display((Sint32 **) &klm_Q, SIZE_X, SIZE_X, "Q");
	MATRIX_display((Sint32 **) &klm_P, SIZE_X, SIZE_X, "P");
	MATRIX_display((Sint32 **) &klm_Pp, SIZE_X, SIZE_X, "Pp");

	MATRIX_display((Sint32 **) &klm_Y, SIZE_Y, 1, "Y");
	MATRIX_display((Sint32 **) &klm_Yi, SIZE_Y, 1, "Yi");
	MATRIX_display((Sint32 **) &klm_S, SIZE_Y, SIZE_Y, "S");
	MATRIX_display((Sint32 **) &klm_H, SIZE_Y, SIZE_X, "H");
	MATRIX_display((Sint32 **) &klm_R, SIZE_Y, SIZE_Y, "R");
	MATRIX_display((Sint32 **) &klm_K, SIZE_X, SIZE_Y, "K");
}

#endif /* USE_KALMAN_FILTER */
