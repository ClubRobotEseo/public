#include "rm.h"
#include <errno.h>
#include <string.h>
#include "../term_commands_utils.h"
#include "../../../QS/QS_all.h"
#include "../../../QS/QS_outputlog.h"
#include <ctype.h>
#include <stdlib.h>

const char term_cmd_rm_brief[] = "Supprime des fichiers";
const char term_cmd_rm_help[] =
		"Utilisation : rm FICHIER...\n"
		"OU si vous voulez supprimer un ou plusieurs match(s) : rm NUMERO_MATCH\n"
		"FICHIER... Fichiers à supprimer séparé par un espace\n"
		"Exemples :\n"
		"   rm index.inf\n"
		"   rm 4 12 54 15\n"
		"   rm 0498.MCH\n";

int term_cmd_rm(int argc, const char *argv[]) {
	int i;
	BYTE res;

	if(argc < 1)
		return EINVAL;

	for(i=0; i < argc; i++) {
		res = fatfs_err_to_errno(f_unlink(argv[i]));
		if(res)
		{
			debug_printf("Impossible de supprimer %s : %s\n", argv[i], strerror(res));
			if(isdigit(argv[i][0]))
			{
				Sint16 n;
				n = (Sint16)(strtol(argv[i], (char**)NULL, 10));
				if(n>=0)
				{
					char path[10];
					snprintf(path, 10, "%04d.MCH", n);
					debug_printf("Tentative de suppression du match %s\n", path);
					res = fatfs_err_to_errno(f_unlink(path));
					if(res)
						debug_printf("Impossible de supprimer %s : %s\n", path, strerror(res));
					else
						debug_printf("%s supprimé\n", path);
				}
			}
		}
		else
			debug_printf("%s supprimé\n", argv[i]);
	}

	return 0;
}
