/*
 *	Club Robot ESEO 2018 - 2019
 *
 *	Fichier : act_utils.c
 *	Package : Carte Actionneur
 *	Description : Propose des fonctions génériques pour gérer les actionneurs.
 *	Auteur : Valentin
 */

#include "act_utils.h"

#include "../QS/QS_CANmsgList.h"
#include "../QS/QS_lowLayer/QS_can.h"
#include "../QS/QS_actuator/QS_ax12.h"
#include "../QS/QS_actuator/QS_rx24.h"

#define LOG_PREFIX "ActUtils: "
#define LOG_COMPONENT OUTPUT_LOG_COMPONENT_ACTUTILS
#include "../QS/QS_outputlog.h"

#ifdef USE_AX12_SERVO
bool_e ACTQ_check_status_ax12(Uint8 ax12Id, Sint16 wantedGoal, Sint16 currentGoal, Uint16 epsilon, time32_t start_time_ms, Uint16 timeout_ms, Uint16 large_epsilon, Uint8* result, Uint8* error_code, Uint16* line) {
	AX12_reset_last_error(ax12Id);

	Uint8 error = AX12_get_last_error(ax12Id).error;
	Uint16 dummy;

	//Avec ça, on ne se soucie pas du contenu de result et error_code par la suite, on sait qu'ils ne sont pas NULL
	if(!result) result = (Uint8*)&dummy;
	if(!error_code) error_code = (Uint8*)&dummy;
	if(!line) line = &dummy;

	if(absolute((Sint16)currentGoal - (Sint16)(wantedGoal)) <= epsilon) {
		*result = ACT_RESULT_DONE;
		*error_code = ACT_RESULT_ERROR_OK;
		*line = __LINE__;
	} else if((error & AX12_ERROR_TIMEOUT) && (error & AX12_ERROR_RANGE)) {
		//Si le driver a attendu trop longtemps, c'est a cause d'un deadlock plutot qu'un manque de ressources (il attend suffisament longtemps pour que les commandes soit bien envoyées)
		AX12_set_torque_enabled(ax12Id, FALSE);
		*result = ACT_RESULT_NOT_HANDLED;
		*error_code = ACT_RESULT_ERROR_LOGIC;
		*line = __LINE__;
	} else if(error & AX12_ERROR_OVERLOAD) {
		AX12_set_torque_enabled(ax12Id, FALSE);
		*result = ACT_RESULT_FAILED;
		*error_code = ACT_RESULT_ERROR_OVERLOAD;
		*line = __LINE__;
	}else if(error & AX12_ERROR_OVERHEATING) {
		//autres erreurs fiable, les autres on les teste pas car si elle arrive, c'est plus probablement un problème de transmission ou code ...
		AX12_set_torque_enabled(ax12Id, FALSE);
		*result = ACT_RESULT_FAILED;
		*error_code = ACT_RESULT_ERROR_OVERHEATING;
		*line = __LINE__;
	}else if(error & AX12_ERROR_TIMEOUT) {
		//AX12_set_torque_enabled(ax12Id, FALSE);
		*result = ACT_RESULT_FAILED;
		*error_code = ACT_RESULT_ERROR_NOT_HERE;
		*line = __LINE__;
	} else if(global.absolute_time > (time32_t) (start_time_ms + timeout_ms)) {
		//Timeout, l'ax12 n'a pas bouger à la bonne position a temps
		if(absolute((Sint16)currentGoal - (Sint16)(wantedGoal)) <= large_epsilon) {
			*result = ACT_RESULT_FAILED;
			*error_code = ACT_RESULT_ERROR_TIMEOUT;
			*line = __LINE__;
		} else {
			AX12_set_torque_enabled(ax12Id, FALSE);
			*result = ACT_RESULT_FAILED;
			*error_code = ACT_RESULT_ERROR_TIMEOUT;
			*line = __LINE__;
		}
	} else if(error) {
		component_printf(LOG_LEVEL_Error, "Error AX12 %d\n", error);
		return FALSE;
	} else return FALSE;

	return TRUE;
}
#endif /* USE_AX12_SERVO */


#ifdef USE_RX24_SERVO
bool_e ACTQ_check_status_rx24(Uint8 rx24Id, Sint16 wantedGoal, Sint16 currentGoal, Uint16 epsilon, time32_t start_time_ms, Uint16 timeout_ms, Uint16 large_epsilon, Uint8* result, Uint8* error_code, Uint16* line) {
	RX24_reset_last_error(rx24Id);

	Uint8 error = RX24_get_last_error(rx24Id).error;
	Uint16 dummy;

	//Avec ça, on ne se soucie pas du contenu de result et error_code par la suite, on sait qu'ils ne sont pas NULL
	if(!result) result = (Uint8*)&dummy;
	if(!error_code) error_code = (Uint8*)&dummy;
	if(!line) line = &dummy;

	if(absolute((Sint16)currentGoal - (Sint16)(wantedGoal)) <= epsilon) {
		*result = ACT_RESULT_DONE;
		*error_code = ACT_RESULT_ERROR_OK;
		*line = __LINE__;
	} else if((error & RX24_ERROR_TIMEOUT) && (error & RX24_ERROR_RANGE)) {
		//Si le driver a attendu trop longtemps, c'est a cause d'un deadlock plutot qu'un manque de ressources (il attend suffisament longtemps pour que les commandes soit bien envoyées)
		RX24_set_torque_enabled(rx24Id, FALSE);
		*result = ACT_RESULT_NOT_HANDLED;
		*error_code = ACT_RESULT_ERROR_LOGIC;
		*line = __LINE__;
	} else if(error & RX24_ERROR_TIMEOUT) {
		//RX24_set_torque_enabled(rx24Id, FALSE);
		*result = ACT_RESULT_FAILED;
		*error_code = ACT_RESULT_ERROR_NOT_HERE;
		*line = __LINE__;
	} else if(error & RX24_ERROR_OVERHEATING) {
		//autres erreurs fiable, les autres on les teste pas car si elle arrive, c'est plus probablement un problème de transmission ou code ...
		RX24_set_torque_enabled(rx24Id, FALSE);
		*result = ACT_RESULT_FAILED;
		*error_code = ACT_RESULT_ERROR_UNKNOWN;
		*line = __LINE__;
	} else if(global.absolute_time > (time32_t) (start_time_ms + timeout_ms)) {
		//Timeout, le RX24 n'a pas bouger à la bonne position a temps
		if(absolute((Sint16)currentGoal - (Sint16)(wantedGoal)) <= large_epsilon) {
			*result = ACT_RESULT_FAILED;
			*error_code = ACT_RESULT_ERROR_TIMEOUT;
			*line = __LINE__;
		} else {
			RX24_set_torque_enabled(rx24Id, FALSE);
			*result = ACT_RESULT_FAILED;
			*error_code = ACT_RESULT_ERROR_TIMEOUT;
			*line = __LINE__;
		}
	} else if(error) {
		component_printf(LOG_LEVEL_Error, "Error RX24 %d\n", error);
		return FALSE;
	} else return FALSE;

	return TRUE;
}
#endif /* USE_RX24_SERVO */

/**
 * @brief Affichage du résultat d'un ordre actionneur.
 * @param actuator l'actionneur concerné.
 * @param order l'ordre effectué.
 * @param result le résultat de l'exécution.
 * @param errorCode le code d'erreur associé au résultat de l'exécution.
 * @param param le paramètre associé à l'ordre.
 */
void ACT_printResult(Actuator_t* actuator, Uint8 order, Uint8 result, Uint8 errorCode, Uint16 param) {
#ifdef VERBOSE_MODE
	const char* originalSidStr = actuator->name;
	const char* resultStr = "Unknown";
	const char* errorCodeStr = "Unknown error";

	switch(result) {
		case ACT_RESULT_DONE:        resultStr = "Done";       break;
		case ACT_RESULT_FAILED:      resultStr = "Failed";     break;
		case ACT_RESULT_NOT_HANDLED: resultStr = "NotHandled"; break;
		case ACT_RESULT_IN_PROGRESS: resultStr = "InProgress"; break;
		default:                     resultStr = "Unknown";    break;
	}

	switch(errorCode) {
		case ACT_RESULT_ERROR_LOGIC:        errorCodeStr = "Logic";				break;
		case ACT_RESULT_ERROR_NOT_HERE:     errorCodeStr = "NotHere";			break;
		case ACT_RESULT_ERROR_NO_RESOURCES: errorCodeStr = "NoResources";		break;
		case ACT_RESULT_ERROR_OK:           errorCodeStr = "Ok";				break;
		case ACT_RESULT_ERROR_OTHER:        errorCodeStr = "Other";				break;
		case ACT_RESULT_ERROR_TIMEOUT:      errorCodeStr = "Timeout";			break;
		case ACT_RESULT_ERROR_INVALID_ARG:  errorCodeStr = "Invalid argument";	break;
		case ACT_RESULT_ERROR_UNKNOWN:      errorCodeStr = "Unknown";			break;
		case ACT_RESULT_ERROR_CANCELED:		errorCodeStr = "Cancelled";			break;
		default:                            errorCodeStr = "Unknown error";		break;
	}

	debug_printf("Result msg: Act: %s, cmd: 0x%x(%u), result: %s(%u), error: %s(%u), param: 0x%x(%u)\n\n",
			originalSidStr,
			order, order,
			resultStr, result,
			errorCodeStr, errorCode,
			param, param);
#endif
}

