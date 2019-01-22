/**
 * Club Robot ESEO 2018-2019
 *
 * @file matrix.c
 * @brief Opérations sur les matrices.
 * @author Valentin
 */

#include "matrix.h"
#include "../QS/QS_outputlog.h"

/**
 * Somme de deux matrices (mat1 + mat2).
 * @param matR la matrice résultat.
 * @param mat1 la première matrice.
 * @param mat2 la deuxième matrice.
 * @param lines le nombre de lignes.
 * @param columns le nombre de colonnes.
 */
void MATRIX_sum(Sint32 **matR, Sint32 **mat1, Sint32 **mat2, Uint8 lines, Uint8 columns) {
	Uint8 i, j;

	for(i = 0; i < lines; i++) {
		for(j = 0; j < columns; j++) {
			matR[i][j] = mat1[i][j] + mat2[i][j];
		}
	}
}


/**
 * Différence de deux matrices (mat1 - mat2).
 * @param matR la matrice résultat.
 * @param mat1 la première matrice.
 * @param mat2 la deuxième matrice.
 * @param lines le nombre de lignes.
 * @param columns le nombre de colonnes.
 */
void MATRIX_substract(Sint32 **matR, Sint32 **mat1, Sint32 **mat2, Uint8 lines, Uint8 columns) {
	Uint8 i, j;

	for(i = 0; i < lines; i++) {
		for(j = 0; j < columns; j++) {
			matR[i][j] = mat1[i][j] - mat2[i][j];
		}
	}
}

/**
 * Produit de deux matrices (mat1 * mat2).
 * @param matR la matrice résultat.
 * @param mat1 la première matrice.
 * @param lines1 le nombre de lignes de mat1.
 * @param columns1 le nombre de colonnes de mat1.
 * @param mat2 la deuxième matrice.
 * @param lines2 le nombre de lignes de mat2.
 * @param columns2 le nombre de colonnes de mat2.
 * @pre matR doit être différent de mat1 et mat2.
 */
void MATRIX_multiply(Sint32 **matR, Sint32 **mat1, Uint8 lines1, Uint8 columns1, Sint32 **mat2, Uint8 lines2, Uint8 columns2) {
	Uint8 i, j, k;
//	Sint32 result[lines1][columns2]; // La matrice résultat avant copie dans matR.

	if(matR == mat1 || matR == mat2) {
		debug_printf("Matrix multiplication: matR cannot be mat1 or mat2.\n");
	}

	if(columns1 != lines2) {
		debug_printf("Matrix multiplication: problem of dimensions.\n");
	}

	for(i = 0; i < lines1; i++) {
		for(j = 0; j < columns2; j++) {
			for(k = 0; k < columns1; k++) {
				matR[i][j] += mat1[i][k] * mat2[k][j];
			}
		}
	}

//	// Copie du résultat dans matR
//	if(matR != NULL) {
//		for(i = 0; i < lines1; i++) {
//			for(j = 0; j < columns2; j++) {
//				matR[i][j] = result[i][j];
//			}
//		}
//	}
}


/**
 * Produit de deux matrices dont la seconde est transposée (mat1 * transposed(mat2))
 * @param matR la matrice résultat.
 * @param mat1 la première matrice.
 * @param lines1 le nombre de lignes de mat1.
 * @param columns1 le nombre de colonnes de mat1.
 * @param mat2 la deuxième matrice.
 * @param lines2 le nombre de lignes de mat2.
 * @param columns2 le nombre de colonnes de mat2.
 * @pre matR doit être différent de mat1 et mat2.
 */
void MATRIX_multiplyByTransposed(Sint32 **matR, Sint32 **mat1, Uint8 lines1, Uint8 columns1, Sint32 **mat2, Uint8 lines2, Uint8 columns2) {
	Uint8 i, j, k;
//	Sint32 result[lines1][columns2]; // La matrice résultat avant copie dans matR.

	if(matR == mat1 || matR == mat2) {
		debug_printf("Matrix multiplication: matR cannot be mat1 or mat2.\n");
		return;
	}

	if(columns1 != lines2) {
		debug_printf("Matrix multiplication: problem of dimensions.\n");
		return;
	}

	for(i = 0; i < lines1; i++) {
		for(j = 0; j < columns2; j++) {
			for(k = 0; k < columns1; k++) {
				matR[i][j] += mat1[i][k] * mat2[j][k];
			}
		}
	}

//	// Copie du résultat dans matR
//	if(matR != NULL) {
//		for(i = 0; i < lines1; i++) {
//			for(j = 0; j < columns2; j++) {
//				matR[i][j] = result[i][j];
//			}
//		}
//	}
}


/**
 * Inversion d'une matrice 2x2.
 * @param matR la matrice résultat.
 * @param mat la matrice opérande.
 */
void MATRIX_reverse2x2(Sint32 **matR, Sint32 **mat) {
	Sint32 delta = mat[0][0] * mat[1][1] -  mat[0][1] * mat[1][0];
	double k = 1 / (double) delta;

	mat[0][0] = k * mat[1][1];
	mat[0][1] = - k * mat[1][0];
	mat[1][0] = - k * mat[0][1];
	mat[1][1] = k * mat[0][0];
}


/**
 * Affichage d'une matrice.
 * @param mat la matrice à afficher.
 * @param lines le nombre de lignes.
 * @param columns le nombre de colonnes.
 * @param le nom de la matrice.
 */
void MATRIX_display(Sint32 **mat, Uint8 lines, Uint8 columns, char *name) {
	Uint8 i, j;

	debug_printf("\nMatrice %s:\n", (name == NULL) ? "XXX":name);
	for(i = 0; i < lines; i++) {
		debug_printf("(");
		for(j = 0; j < columns; j++) {
			debug_printf(" %4ld ", mat[i][j]);
		}
		debug_printf(")");
	}

}
