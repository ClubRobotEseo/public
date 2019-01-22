#ifndef ACTION_CHECKER_H
	#define ACTION_CHECKER_H

	#include "../QS/QS_all.h"

	//Vérifie l'état d'une microstrat: microstrat en cours, microstrat terminée correctement ou microstrat terminée avec une erreur. S'utilise comme try_going pour les états.
	Uint8 check_sub_action_result(error_e sub_action, Uint8 in_progress_state, Uint8 success_state, Uint8 failed_state);

#endif // ACTION_CHECKER_H
