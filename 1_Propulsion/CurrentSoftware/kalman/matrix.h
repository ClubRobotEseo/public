/**
 * Club Robot ESEO 2018-2019
 *
 * @file matrix.h
 * @brief Op�rations sur les matrices.
 * @author Valentin
 */

#ifndef KALMAN_MATRIX_H_
#define KALMAN_MATRIX_H_

#include "../QS/QS_all.h"

/**
 * Somme de deux matrices (mat1 + mat2).
 * @param matR la matrice r�sultat.
 * @param mat1 la premi�re matrice.
 * @param mat2 la deuxi�me matrice.
 * @param lines le nombre de lignes.
 * @param columns le nombre de colonnes.
 */
void MATRIX_sum(Sint32 **matR, Sint32 **mat1, Sint32 **mat2, Uint8 lines, Uint8 columns);

/**
 * Diff�rence de deux matrices (mat1 - mat2).
 * @param matR la matrice r�sultat.
 * @param mat1 la premi�re matrice.
 * @param mat2 la deuxi�me matrice.
 * @param lines le nombre de lignes.
 * @param columns le nombre de colonnes.
 */
void MATRIX_substract(Sint32 **matR, Sint32 **mat1, Sint32 **mat2, Uint8 lines, Uint8 columns);

/**
 * Produit de deux matrices (mat1 * mat2).
 * @param matR la matrice r�sultat.
 * @param mat1 la premi�re matrice.
 * @param lines1 le nombre de lignes de mat1.
 * @param columns1 le nombre de colonnes de mat1.
 * @param mat2 la deuxi�me matrice.
 * @param lines2 le nombre de lignes de mat2.
 * @param columns2 le nombre de colonnes de mat2.
 */
void MATRIX_multiply(Sint32 **matR, Sint32 **mat1, Uint8 lines1, Uint8 columns1, Sint32 **mat2, Uint8 lines2, Uint8 columns2);

/**
 * Produit de deux matrices dont la seconde est transpos�e (mat1 * transposed(mat2))
 * @param matR la matrice r�sultat.
 * @param mat1 la premi�re matrice.
 * @param lines1 le nombre de lignes de mat1.
 * @param columns1 le nombre de colonnes de mat1.
 * @param mat2 la deuxi�me matrice.
 * @param lines2 le nombre de lignes de mat2.
 * @param columns2 le nombre de colonnes de mat2.
 */
void MATRIX_multiplyByTransposed(Sint32 **matR, Sint32 **mat1, Uint8 lines1, Uint8 columns1, Sint32 **mat2, Uint8 lines2, Uint8 columns2);

/**
 * Inversion d'une matrice 2x2.
 * @param matR la matrice r�sultat.
 * @param mat la matrice op�rande.
 */
void MATRIX_reverse2x2(Sint32 **matR, Sint32 **mat);

/**
 * Affichage d'une matrice.
 * @param mat la matrice � afficher.
 * @param lines le nombre de lignes.
 * @param columns le nombre de colonnes.
 * @param le nom de la matrice.
 */
void MATRIX_display(Sint32 **mat, Uint8 lines, Uint8 columns, char *name);

#endif /* KALMAN_MATRIX_H_ */
