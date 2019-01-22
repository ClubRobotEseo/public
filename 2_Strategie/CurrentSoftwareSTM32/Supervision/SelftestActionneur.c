#include "SelftestActionneur.h"
#include "Selftest.h"
#include "../QS/QS_stateMachineHelper.h"
#include "../QS/QS_outputlog.h"
#include "../QS/QS_types.h"
#include "../actuator/act_functions.h"
#include "../actuator/queue.h"
#include "../utils/generic_functions.h"
#include "../utils/actionChecker.h"

error_e SELFTESTACT_run(){
	CREATE_MAE_WITH_VERBOSE(SM_ID_SELFTEST_ACT,
			INIT,
			MOVE_ACTIONNEUR,
			CHECK_STATUS,
			COMPUTE_NEXT_ETAPE,
			DECLARE_ERROR,
			ERROR,
			DONE
	);

	static const struct_selftest_t tableau_selftest_big[]  = {

//numero etape, nom de l'actionneur,				nom de l'action a effectuer,		param,		id queue de l'actionneur,				id selftest de l'actionneur,

			// Front
			{1, ACT_BIG_LOCKER_FRONT_LEFT,	 		ACT_BIG_LOCKER_FRONT_LEFT_UNLOCK,		 	0,		ACT_QUEUE_Big_locker_front_left, 		SELFTEST_ACT_BIG_LOCKER_FRONT_LEFT },
			{2, ACT_BIG_LOCKER_FRONT_RIGHT,	 		ACT_BIG_LOCKER_FRONT_RIGHT_UNLOCK,		 	0,		ACT_QUEUE_Big_locker_front_right, 		SELFTEST_ACT_BIG_LOCKER_FRONT_RIGHT },

			{3, ACT_BIG_ELEVATOR_FRONT_LEFT,	 	ACT_BIG_ELEVATOR_FRONT_LEFT_TOP,		 	0,		ACT_QUEUE_Big_elevator_front_left, 		SELFTEST_ACT_BIG_ELEVATOR_FRONT_LEFT },
			{4, ACT_BIG_ELEVATOR_FRONT_MIDDLE,	 	ACT_BIG_ELEVATOR_FRONT_MIDDLE_TOP,		 	0,		ACT_QUEUE_Big_elevator_front_middle, 	SELFTEST_ACT_BIG_ELEVATOR_FRONT_MIDDLE },
			{5, ACT_BIG_ELEVATOR_FRONT_RIGHT,	 	ACT_BIG_ELEVATOR_FRONT_RIGHT_TOP,		 	0,		ACT_QUEUE_Big_elevator_front_right, 	SELFTEST_ACT_BIG_ELEVATOR_FRONT_RIGHT },
			{6, ACT_BIG_ELEVATOR_FRONT_LEFT,	 	ACT_BIG_ELEVATOR_FRONT_LEFT_ALMOST_BOT,		0,		ACT_QUEUE_Big_elevator_front_left, 		SELFTEST_ACT_BIG_ELEVATOR_FRONT_LEFT },
			{7, ACT_BIG_ELEVATOR_FRONT_MIDDLE,	 	ACT_BIG_ELEVATOR_FRONT_MIDDLE_ALMOST_BOT,	0,		ACT_QUEUE_Big_elevator_front_middle, 	SELFTEST_ACT_BIG_ELEVATOR_FRONT_MIDDLE },
			{8, ACT_BIG_ELEVATOR_FRONT_RIGHT,	 	ACT_BIG_ELEVATOR_FRONT_RIGHT_ALMOST_BOT,	0,		ACT_QUEUE_Big_elevator_front_right, 	SELFTEST_ACT_BIG_ELEVATOR_FRONT_RIGHT },
			{9, ACT_BIG_ELEVATOR_FRONT_LEFT,	 	ACT_BIG_ELEVATOR_FRONT_LEFT_BOT,			0,		ACT_QUEUE_Big_elevator_front_left, 		SELFTEST_ACT_BIG_ELEVATOR_FRONT_LEFT },
			{10, ACT_BIG_ELEVATOR_FRONT_MIDDLE,	 	ACT_BIG_ELEVATOR_FRONT_MIDDLE_BOT,			0,		ACT_QUEUE_Big_elevator_front_middle, 	SELFTEST_ACT_BIG_ELEVATOR_FRONT_MIDDLE },
			{11, ACT_BIG_ELEVATOR_FRONT_RIGHT,	 	ACT_BIG_ELEVATOR_FRONT_RIGHT_BOT,			0,		ACT_QUEUE_Big_elevator_front_right, 	SELFTEST_ACT_BIG_ELEVATOR_FRONT_RIGHT },

			{12, ACT_BIG_LOCKER_FRONT_LEFT,	 		ACT_BIG_LOCKER_FRONT_LEFT_LOCK,		 		0,		ACT_QUEUE_Big_locker_front_left, 		SELFTEST_ACT_BIG_LOCKER_FRONT_LEFT },
			{13, ACT_BIG_LOCKER_FRONT_RIGHT,	 	ACT_BIG_LOCKER_FRONT_RIGHT_LOCK,		 	0,		ACT_QUEUE_Big_locker_front_right, 		SELFTEST_ACT_BIG_LOCKER_FRONT_RIGHT },
			{14, ACT_BIG_LOCKER_FRONT_MIDDLE,	 	ACT_BIG_LOCKER_FRONT_MIDDLE_LOCK,		 	0,		ACT_QUEUE_Big_locker_front_middle, 		SELFTEST_ACT_BIG_LOCKER_FRONT_MIDDLE },
			{15, ACT_BIG_LOCKER_FRONT_MIDDLE,	 	ACT_BIG_LOCKER_FRONT_MIDDLE_UNLOCK,		 	0,		ACT_QUEUE_Big_locker_front_middle, 		SELFTEST_ACT_BIG_LOCKER_FRONT_MIDDLE },


			// Back
//			{, ACT_BIG_SORTING_BACK_VERY_LEFT,	 	ACT_BIG_SORTING_BACK_VERY_LEFT_TOP,		 	0,		ACT_QUEUE_Big_sorting_back_very_left, 	SELFTEST_ACT_BIG_SORTING_BACK_VERY_LEFT },
//			{, ACT_BIG_SORTING_BACK_VERY_LEFT,	 	ACT_BIG_SORTING_BACK_VERY_LEFT_BOT,		 	0,		ACT_QUEUE_Big_sorting_back_very_left, 	SELFTEST_ACT_BIG_SORTING_BACK_VERY_LEFT },
			{16, ACT_BIG_SORTING_BACK_LEFT,	 		ACT_BIG_SORTING_BACK_LEFT_ACCELERATOR_BOT,	0,		ACT_QUEUE_Big_sorting_back_left, 		SELFTEST_ACT_BIG_SORTING_BACK_LEFT },
			{17, ACT_BIG_SORTING_BACK_LEFT,	 		ACT_BIG_SORTING_BACK_LEFT_TOP,		 		0,		ACT_QUEUE_Big_sorting_back_left, 		SELFTEST_ACT_BIG_SORTING_BACK_LEFT },
			{18, ACT_BIG_SORTING_BACK_RIGHT,	 	ACT_BIG_SORTING_BACK_RIGHT_ACCELERATOR_BOT, 0,		ACT_QUEUE_Big_sorting_back_right, 		SELFTEST_ACT_BIG_SORTING_BACK_RIGHT },
			{19, ACT_BIG_SORTING_BACK_RIGHT,	 	ACT_BIG_SORTING_BACK_RIGHT_TOP,		 		0,		ACT_QUEUE_Big_sorting_back_right, 		SELFTEST_ACT_BIG_SORTING_BACK_RIGHT },
//			{, ACT_BIG_SORTING_BACK_VERY_RIGHT,		ACT_BIG_SORTING_BACK_VERY_RIGHT_TOP,		0,		ACT_QUEUE_Big_sorting_back_very_right, 	SELFTEST_ACT_BIG_SORTING_BACK_VERY_RIGHT },
//			{, ACT_BIG_SORTING_BACK_VERY_RIGHT,		ACT_BIG_SORTING_BACK_VERY_RIGHT_BOT,		0,		ACT_QUEUE_Big_sorting_back_very_right, 	SELFTEST_ACT_BIG_SORTING_BACK_VERY_RIGHT },

			{20, ACT_BIG_SLOPE_TAKER_BACK,			ACT_BIG_SLOPE_TAKER_BACK_OUT,				0,		ACT_QUEUE_Big_slope_taker_back, 			SELFTEST_ACT_BIG_SLOP_TAKER_BACK },
			{21, ACT_BIG_SLOPE_TAKER_BACK,			ACT_BIG_SLOPE_TAKER_BACK_IN,				0,		ACT_QUEUE_Big_slope_taker_back, 			SELFTEST_ACT_BIG_SLOP_TAKER_BACK },
	};

	static const struct_selftest_t tableau_selftest_small[]  = {

			//Ex: {1, ACT_SMALL_BALL_TAKER,	 			ACT_CMD_SMALL_BALL_TAKER_RIGHT,		 			0,		ACT_QUEUE_Small_ball_taker, 			SELFTEST_ACT_SMALL_BALL_TAKER },


	};

	#define NB_MAX_ACTIONS	 (50)

	static const struct_selftest_t *tableau_selftest;
	static error_e liste_etat_actionneur[NB_MAX_ACTIONS];
	static ACT_sid_e liste_error_actionneur[NB_MAX_ACTIONS];
	static Uint8 nb_actions;
	static Uint8 nb_etapes;
	static Uint8 indice;
	static Uint8 ind_start_etape, ind_end_etape;
	static Uint8 etape_en_cours;
	static bool_e check_finish;
	static bool_e new_error;
	static Uint8 ind_nb_errors;
	Uint8 i;
	Uint8 j;


	switch(state){
		case INIT:
			// Choix des infos BIG ou SMALL robot
			if(I_AM_BIG()){
				tableau_selftest = tableau_selftest_big;
				nb_actions = (sizeof(tableau_selftest_big) / sizeof(struct_selftest_t));
				nb_etapes = tableau_selftest_big[nb_actions - 1].numero_etape;
			}else{
				tableau_selftest = tableau_selftest_small;
				nb_actions = (sizeof(tableau_selftest_small) / sizeof(struct_selftest_t));
				nb_etapes = tableau_selftest_small[nb_actions - 1].numero_etape;
			}
			debug_printf("SELFTEST ACT nb_actions = %d  nb_etapes = %d\n", nb_actions, nb_etapes);
			assert(nb_actions < NB_MAX_ACTIONS);

			// Initialisation de la liste des états actionneurs
			for(int i=0; i<nb_actions; i++){
				liste_etat_actionneur[i] = IN_PROGRESS;
			}

			etape_en_cours = 1;
			indice = 0;
			ind_nb_errors = 0;
			state = MOVE_ACTIONNEUR;
			break;

		case MOVE_ACTIONNEUR:
			ind_start_etape = indice;
			while(indice < nb_actions && tableau_selftest[indice].numero_etape == etape_en_cours ){
				ACT_push_order_with_param(tableau_selftest[indice].actionneur, tableau_selftest[indice].position, tableau_selftest[indice].param);
				indice++;
			}
			ind_end_etape = indice - 1;

			state = CHECK_STATUS;
			break;

		case CHECK_STATUS:
			check_finish = TRUE;
			for(i = ind_start_etape; i <= ind_end_etape; i++){
				if(liste_etat_actionneur[i] == IN_PROGRESS){
					liste_etat_actionneur[i] = ACT_check_status(tableau_selftest[i].actionneur, IN_PROGRESS, END_OK, NOT_HANDLED);
					if(liste_etat_actionneur[i] == IN_PROGRESS){
						check_finish = FALSE;
					}
				}
			}

			if(check_finish){
				state = DECLARE_ERROR;
			}
			break;

		case DECLARE_ERROR :
			for(i = ind_start_etape; i <= ind_end_etape; i++){
				if (liste_etat_actionneur[i] == NOT_HANDLED){
					new_error = TRUE;

					// On recherche si l'erreur a déjà été déclarée
					for(j=0; j<ind_nb_errors; j++){
						if(liste_error_actionneur[j]==tableau_selftest[i].actionneur){
							new_error = FALSE;
						}
					}

					// Si l'erreur n'a pas été déclarée, on la déclare
					if (new_error == TRUE){
						SELFTEST_declare_errors(NULL, tableau_selftest[i].error_code);
						debug_printf("ERROR %d\n", tableau_selftest[i].error_code);
						liste_error_actionneur[ind_nb_errors] = tableau_selftest[i].actionneur;
						ind_nb_errors++;
					}
				}
			}
			state = COMPUTE_NEXT_ETAPE;
			break;

		case COMPUTE_NEXT_ETAPE:
			etape_en_cours += 1;
			if (etape_en_cours <= nb_etapes){
				state = MOVE_ACTIONNEUR;
			}else{
				state = DONE;
			}
			break;

		case ERROR:
			RESET_MAE();
			on_turning_point();
			return NOT_HANDLED;
			break;

		case DONE:
			RESET_MAE();
			on_turning_point();
			return END_OK;
			break;

		default:
			if(entrance)
				debug_printf("default case in SELFTESTACT_run\n");
			break;
	}

	return IN_PROGRESS;
}


