/*
 *	Club Robot ESEO 2013
 *
 *	Fichier : LCD_interface.c
 *	Package : Supervision
 *	Description : implémentation de l'interface de l'écran LCD MCCOG42005A6W-BNMLWI
 *	Licence : CeCILL-C
 *	Auteur : HERZAEONE
 *	Version 201308
 */

#ifndef LCD_INTERFACE_H
	#define	LCD_INTERFACE_H

	#include "../QS/QS_all.h"

	#if USE_LCD

		void LCD_init(void);
		void LCD_processMain(void);
		void LCD_IHM_control(bool_e control);

		void LCD_setBatteryCritical(void);

		void LCD_setDebugMenu(void);

		void LCD_setOdometryCoef(Sint32 coef);
		void LCD_setOdometryError(Sint32 error);
		void LCD_setOdometryRound(Uint8 round);
		void LCD_setOdometrySuccess(bool_e success);
		void LCD_setOdometryWriteState(bool_e state);
		#endif

		/*
		 * Ecrit une ligne définie par l'utilisateur à la position demandée (et bascule le LCD en menu utilisateur !)
		 * Line doit être entre 0 et 3 inclus.
		 * La ligne 0 correspond à la dernière ligne du menu principal (INFOS)
		 * Les 3 autres lignes correspondent aux lignes du menu utilisateur.
		 */
		void LCD_printf(Uint8 line, bool_e switch_on_menu, bool_e log_on_sd, char * chaine, ...) __attribute__((format (printf, 4, 5)));

#endif	/* LCD_INTERFACE_H */

