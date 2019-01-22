/*
 * pm.c
 *
 *  Created on: 29 april 2014
 *      Author: Arnaud
 */
#include "pm.h"

#include <errno.h>
#include "../term_commands_utils.h"
#include "../../../QS/QS_all.h"
#include "../SD.h"

const char term_cmd_pm_brief[] = "Affiche le numéro du match voulu (Print Match)";
const char term_cmd_pm_help[] =
		"Utilisation : pm nb_match\n"
		"OU sans argument pour afficher le match précédent : pm\n"
		"nb_match       numéro du match voulu\n";


int term_cmd_pm(int argc, const char *argv[]) {

	Sint32 nb_match;

	if(argc > 1)
		return EINVAL;

	if(argc < 1)
	{
		//aucun patch précisé, on prend le précédent...
		SD_print_previous_match();
	}
	else
	{
		if(!argtolong(argv[0], 10, &nb_match))
			return EINVAL;
		SD_print_match(nb_match);
	}
	return 0;
}




