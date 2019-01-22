#ifndef ACTION_CHECKER_H
	#define ACTION_CHECKER_H

	#include "../QS/QS_all.h"

	//V�rifie l'�tat d'une microstrat: microstrat en cours, microstrat termin�e correctement ou microstrat termin�e avec une erreur. S'utilise comme try_going pour les �tats.
	Uint8 check_sub_action_result(error_e sub_action, Uint8 in_progress_state, Uint8 success_state, Uint8 failed_state);

#endif // ACTION_CHECKER_H
