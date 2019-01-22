/*
 *	Club Robot ESEO 2012 - 2013
 *	Krusty & Tiny
 *
 *	Fichier : act_queue_utils.c
 *	Package : Carte Actionneur
 *	Description : Propose des fonctions pour gérer les actions avec la pile.
 *	Auteur : Alexis
 *	Version 20130420
 */

#include <string.h>
#include "act_queue_utils.h"
#include "QS/QS_CANmsgList.h"
#include "QS/QS_lowLayer/QS_can.h"
#include "QS/QS_actuator/QS_ax12.h"
#include "QS/QS_actuator/QS_rx24.h"
#include "QS/QS_actuator/QS_DCMotor2.h"

#define LOG_PREFIX "ActUtils: "
#define LOG_COMPONENT OUTPUT_LOG_COMPONENT_ACTQUEUEUTILS
#include "QS/QS_outputlog.h"

typedef enum {
	CAN_TPT_NoParam, //Pas de paramètre
	CAN_TPT_Line,    //Le paramètre est un numéro de ligne de code
	CAN_TPT_Normal   //Le paramètre est un nombre normal sans signification particulière pour nous (ce code)
} CAN_result_param_type_t;

static void ACTQ_internal_printResult(Actuator_t * actuator, Uint8 original_command, Uint8 result, Uint8 error_code, CAN_result_param_type_t param_type, Uint16 param, bool_e sent_can_result);


//Met sur la pile une action qui sera gérée par act_function_ptr avec en paramètre param. L'action est protégée par semaphore avec act_id
//Cette fonction est appelée par les fonctions de traitement des messages CAN de chaque actionneur.
void ACTQ_push_operation_from_msg(CAN_msg_t* msg, QUEUE_act_e queue_id, action_t act_function_ptr, Sint16 param, bool_e send_result) {

	if(msg->data.act_order.run_now) {
		QUEUE_flush(queue_id);
	} else if(!QUEUE_is_empty(queue_id) && QUEUE_get_arg(queue_id)->flush_on_next_order) {
		// Si la queue n'est pas vide et que l'ordre courant doit être flushé sur réception du prochain ordre, on le flush.
		QUEUE_next(queue_id, ACT_RESULT_DONE, ACT_RESULT_ERROR_OK, __LINE__);
	}

	// Si nous voulons, nous envoyer des messages nous-même sans avertir les autres cartes
	if(send_result == TRUE)
		QUEUE_add(queue_id, act_function_ptr, (QUEUE_arg_t){msg->data.act_order.order, param, &ACTQ_finish_SendResult, FALSE});
	else
		QUEUE_add(queue_id, act_function_ptr, (QUEUE_arg_t){msg->data.act_order.order, param, &ACTQ_finish_SendNothing, FALSE});

}

//Envoie le message CAN de retour à la strat (et affiche des infos de debuggage si activé)
void ACTQ_sendResult(Actuator_t * actuator, Uint8 original_command, Uint8 result, Uint8 error_code) {
	CAN_msg_t resultMsg;

	if(global.flags.match_started == TRUE) {
		resultMsg.sid = ACT_RESULT;
		resultMsg.size = SIZE_ACT_RESULT;
		resultMsg.data.act_result.sid = actuator->sid & 0xFF;
		resultMsg.data.act_result.cmd = original_command;
		resultMsg.data.act_result.result = result;
		resultMsg.data.act_result.error_code = error_code;
		CAN_send(&resultMsg);
	}

	ACTQ_internal_printResult(actuator, original_command, result, error_code, CAN_TPT_NoParam, 0, TRUE);
}

//Comme CAN_sendResult mais ajoute un paramètre au message. Peut servir pour debuggage.
void ACTQ_sendResultWithParam(Actuator_t * actuator, Uint8 original_command, Uint8 result, Uint8 error_code, Uint32 param) {
	CAN_msg_t resultMsg;

	resultMsg.sid = ACT_RESULT;
	resultMsg.size = SIZE_ACT_RESULT;
	resultMsg.data.act_result.sid = actuator->sid & 0xFF;
	resultMsg.data.act_result.cmd = original_command;
	resultMsg.data.act_result.result = result;
	resultMsg.data.act_result.error_code = error_code;
	resultMsg.data.act_result.param = param;
	CAN_send(&resultMsg);

	ACTQ_internal_printResult(actuator, original_command, result, error_code, CAN_TPT_Normal, param, TRUE);
}

//Comme CAN_sendResultWithParam mais le paramètre est considéré comme étant un numéro de ligne.
void ACTQ_sendResultWitExplicitLine(Actuator_t * actuator, Uint8 original_command, Uint8 result, Uint8 error_code, Uint16 line) {
	CAN_msg_t resultMsg;

	resultMsg.sid = ACT_RESULT;
	resultMsg.size = SIZE_ACT_RESULT;
	resultMsg.data.act_result.sid = actuator->sid & 0xFF;
	resultMsg.data.act_result.cmd = original_command;
	resultMsg.data.act_result.result = result;
	resultMsg.data.act_result.error_code = error_code;
	resultMsg.data.act_result.param = line;
	CAN_send(&resultMsg);

	ACTQ_internal_printResult(actuator, original_command, result, error_code, CAN_TPT_Line, line, TRUE);
}


bool_e ACTQ_check_status_ax12(queue_id_t queue_id, Uint8 ax12_id, Sint16 wanted_goal, Sint16 current_goal, Uint16 epsilon, Uint16 timeout_ms, Uint16 large_epsilon, Uint8* result, Uint8* error_code, Uint16* line) {
	AX12_reset_last_error(ax12_id);

	Uint8 error = AX12_get_last_error(ax12_id).error;
	Uint16 dummy;

	//Avec ça, on ne se soucie pas du contenu de result et error_code par la suite, on sait qu'ils ne sont pas NULL
	if(!result) result = (Uint8*)&dummy;
	if(!error_code) error_code = (Uint8*)&dummy;
	if(!line) line = &dummy;

	if(absolute((Sint16)current_goal - (Sint16)(wanted_goal)) <= epsilon) {
		*result = ACT_RESULT_DONE;
		*error_code = ACT_RESULT_ERROR_OK;
		*line = __LINE__;
	} else if((error & AX12_ERROR_TIMEOUT) && (error & AX12_ERROR_RANGE)) {
		//Si le driver a attendu trop longtemps, c'est a cause d'un deadlock plutot qu'un manque de ressources (il attend suffisament longtemps pour que les commandes soit bien envoyées)
		AX12_set_torque_enabled(ax12_id, FALSE);
		*result = ACT_RESULT_NOT_HANDLED;
		*error_code = ACT_RESULT_ERROR_LOGIC;
		*line = __LINE__;
	} else if(error & AX12_ERROR_OVERLOAD) {
		AX12_set_torque_enabled(ax12_id, FALSE);
		*result = ACT_RESULT_FAILED;
		*error_code = ACT_RESULT_ERROR_OVERLOAD;
		*line = __LINE__;
	}else if(error & AX12_ERROR_OVERHEATING) {
		//autres erreurs fiable, les autres on les teste pas car si elle arrive, c'est plus probablement un problème de transmission ou code ...
		AX12_set_torque_enabled(ax12_id, FALSE);
		*result = ACT_RESULT_FAILED;
		*error_code = ACT_RESULT_ERROR_OVERHEATING;
		*line = __LINE__;
	}else if(error & AX12_ERROR_TIMEOUT) {
		//AX12_set_torque_enabled(ax12_id, FALSE);
		*result = ACT_RESULT_FAILED;
		*error_code = ACT_RESULT_ERROR_NOT_HERE;
		*line = __LINE__;
	} else if(ACTQ_check_timeout(queue_id, timeout_ms)) {
		//Timeout, l'ax12 n'a pas bouger à la bonne position a temps
		if(absolute((Sint16)current_goal - (Sint16)(wanted_goal)) <= large_epsilon) {
			*result = ACT_RESULT_FAILED;
			*error_code = ACT_RESULT_ERROR_TIMEOUT;
			*line = __LINE__;
		} else {
			AX12_set_torque_enabled(ax12_id, FALSE);
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

bool_e ACTQ_check_status_rx24(queue_id_t queue_id, Uint8 rx24_id, Sint16 wanted_goal, Sint16 current_goal, Uint16 epsilon, Uint16 timeout_ms, Uint16 large_epsilon, Uint8* result, Uint8* error_code, Uint16* line) {
	RX24_reset_last_error(rx24_id);

	Uint8 error = RX24_get_last_error(rx24_id).error;
	Uint16 dummy;

	//Avec ça, on ne se soucie pas du contenu de result et error_code par la suite, on sait qu'ils ne sont pas NULL
	if(!result) result = (Uint8*)&dummy;
	if(!error_code) error_code = (Uint8*)&dummy;
	if(!line) line = &dummy;

	if(absolute((Sint16)current_goal - (Sint16)(wanted_goal)) <= epsilon) {
		*result = ACT_RESULT_DONE;
		*error_code = ACT_RESULT_ERROR_OK;
		*line = __LINE__;
	} else if((error & RX24_ERROR_TIMEOUT) && (error & RX24_ERROR_RANGE)) {
		//Si le driver a attendu trop longtemps, c'est a cause d'un deadlock plutot qu'un manque de ressources (il attend suffisament longtemps pour que les commandes soit bien envoyées)
		RX24_set_torque_enabled(rx24_id, FALSE);
		*result = ACT_RESULT_NOT_HANDLED;
		*error_code = ACT_RESULT_ERROR_LOGIC;
		*line = __LINE__;
	} else if(error & RX24_ERROR_TIMEOUT) {
		//RX24_set_torque_enabled(rx24_id, FALSE);
		*result = ACT_RESULT_FAILED;
		*error_code = ACT_RESULT_ERROR_NOT_HERE;
		*line = __LINE__;
	} else if(error & RX24_ERROR_OVERHEATING) {
		//autres erreurs fiable, les autres on les teste pas car si elle arrive, c'est plus probablement un problème de transmission ou code ...
		RX24_set_torque_enabled(rx24_id, FALSE);
		*result = ACT_RESULT_FAILED;
		*error_code = ACT_RESULT_ERROR_UNKNOWN;
		*line = __LINE__;
	} else if(ACTQ_check_timeout(queue_id, timeout_ms)) {
		//Timeout, le RX24 n'a pas bouger à la bonne position a temps
		if(absolute((Sint16)current_goal - (Sint16)(wanted_goal)) <= large_epsilon) {
			*result = ACT_RESULT_FAILED;
			*error_code = ACT_RESULT_ERROR_TIMEOUT;
			*line = __LINE__;
		} else {
			RX24_set_torque_enabled(rx24_id, FALSE);
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

#ifdef USE_DCMOTOR2
bool_e ACTQ_check_status_dcmotor(Uint8 dcmotor_id, bool_e timeout_is_ok, Uint8* result, Uint8* error_code, Uint16* line) {
	DCM_working_state_e asserState = DCM_get_state(dcmotor_id);
	Uint16 dummy;

	if(!result) result = (Uint8*)&dummy;
	if(!error_code) error_code = (Uint8*)&dummy;
	if(!line) line = &dummy;

	if(asserState == DCM_IDLE) {
		*result =    ACT_RESULT_DONE;
		*error_code = ACT_RESULT_ERROR_OK;
		*line = __LINE__;
	} else if(asserState == DCM_TIMEOUT) {
		if(timeout_is_ok) {
			*result =    ACT_RESULT_DONE;
			*error_code = ACT_RESULT_ERROR_OK;
			*line = __LINE__;
		} else {
			*result =    ACT_RESULT_FAILED;
			*error_code = ACT_RESULT_ERROR_TIMEOUT;
			*line = __LINE__;
		}
	} else return FALSE;

	return TRUE;
}
#endif

#ifdef USE_DC_MOTOR_SPEED
	bool_e ACTQ_check_status_dcMotorSpeed(DC_MOTOR_SPEED_id id, Uint8* result, Uint8* error_code, Uint16* line){
		DC_MOTOR_SPEED_state_e motorState = DC_MOTOR_SPEED_get_state(id);
		Uint16 dummy;

		if(!result) result = (Uint8*)&dummy;
		if(!error_code) error_code = (Uint8*)&dummy;
		if(!line) line = &dummy;

		switch(motorState){
			case DC_MOTOR_SPEED_IDLE:
				*result =    ACT_RESULT_NOT_HANDLED;
				*error_code = ACT_RESULT_ERROR_CANCELED;
				*line = __LINE__;
				return FALSE;

			case DC_MOTOR_SPEED_RUN:
				*result =    ACT_RESULT_DONE;
				*error_code = ACT_RESULT_ERROR_OK;
				*line = __LINE__;
				return TRUE;

			case DC_MOTOR_SPEED_ERROR:
				*result =    ACT_RESULT_FAILED;
				*error_code = ACT_RESULT_ERROR_TIMEOUT;
				*line = __LINE__;
				return FALSE;

			default:
			case DC_MOTOR_SPEED_INIT_RECOVERY:
			case DC_MOTOR_SPEED_INIT_LAUNCH:
			case DC_MOTOR_SPEED_LAUNCH_RECOVERY:
			case DC_MOTOR_SPEED_LAUNCH:
			case DC_MOTOR_SPEED_RUN_RECOVERY:
				// Waiting
				return FALSE;
		}
	}
#endif

bool_e ACTQ_check_timeout(queue_id_t queue_id, time32_t timeout_ms) {
	if(global.absolute_time >= QUEUE_get_initial_time(queue_id) + timeout_ms)
		return TRUE;
	return FALSE;
}

//Renvoie un retour à la strat dans tous les cas
bool_e ACTQ_finish_SendResult(queue_id_t queue_id, Uint8 result, Uint8 error_code, Uint16 param) {
	ACTQ_sendResultWithParam(QUEUE_get_actuator(queue_id), QUEUE_get_arg(queue_id)->command, result, error_code, param);

	if(result != ACT_RESULT_DONE)
		return FALSE;

	return TRUE;
}

//Retour à la strat seulement si l'opération à fail
bool_e ACTQ_finish_SendResultIfFail(queue_id_t queue_id, Uint8 result, Uint8 error_code, Uint16 param) {
	if(result != ACT_RESULT_DONE && error_code != ACT_RESULT_ERROR_OTHER) {
		ACTQ_sendResultWithParam(QUEUE_get_actuator(queue_id), QUEUE_get_arg(queue_id)->command, result, error_code, param);
		return FALSE;
	} else {
		ACTQ_internal_printResult(QUEUE_get_actuator(queue_id), QUEUE_get_arg(queue_id)->command, result, error_code, CAN_TPT_Line, param, FALSE);
	}

	return TRUE;
}

//Retour à la strat seulement si l'opération à reussi
bool_e ACTQ_finish_SendResultIfSuccess(queue_id_t queue_id, Uint8 result, Uint8 error_code, Uint16 param) {
	if(result == ACT_RESULT_DONE) {
		ACTQ_sendResultWithParam(QUEUE_get_actuator(queue_id), QUEUE_get_arg(queue_id)->command, result, error_code, param);
		return TRUE;
	} else {
		ACTQ_internal_printResult(QUEUE_get_actuator(queue_id), QUEUE_get_arg(queue_id)->command, result, error_code, CAN_TPT_Line, param, FALSE);
	}

	return FALSE;
}

//Ne fait aucun retour
bool_e ACTQ_finish_SendNothing(queue_id_t queue_id, Uint8 result, Uint8 error_code, Uint16 param) {
	ACTQ_internal_printResult(QUEUE_get_actuator(queue_id), QUEUE_get_arg(queue_id)->command, result, error_code, CAN_TPT_Line, param, FALSE);
	if(result == ACT_RESULT_DONE)
		return TRUE;

	return FALSE;
}

void ACTQ_printResult(Actuator_t * actuator, Uint8 original_command, Uint8 result, Uint8 error_code, Uint16 param) {
	ACTQ_internal_printResult(actuator, original_command, result, error_code, CAN_TPT_Normal, param, FALSE);
}

static void ACTQ_internal_printResult(Actuator_t * actuator, Uint8 original_command, Uint8 result, Uint8 error_code, CAN_result_param_type_t param_type, Uint16 param, bool_e sent_can_result) {
#ifdef VERBOSE_MODE
	const char* original_sid_str = "Unknown";
	const char* result_str = "Unknown";
	const char* error_codeStr = "Unknown error";
	const char* sent_str = (sent_can_result == TRUE)? "sent" : "not sent";
	Uint11 original_sid = 0;

	if(actuator != NULL) {
		original_sid = actuator->sid & 0xFF;
		if(actuator->name != NULL || strncmp(actuator->name, "", 1) != 0) {
			original_sid_str = actuator->name;
		}
	}

	switch(result) {
		case ACT_RESULT_DONE:        result_str = "Done";       break;
		case ACT_RESULT_FAILED:      result_str = "Failed";     break;
		case ACT_RESULT_NOT_HANDLED: result_str = "NotHandled"; break;
		default:                     result_str = "Unknown";    break;
	}
	switch(error_code) {
		case ACT_RESULT_ERROR_LOGIC:        error_codeStr = "Logic";				break;
		case ACT_RESULT_ERROR_NOT_HERE:     error_codeStr = "NotHere";			break;
		case ACT_RESULT_ERROR_NO_RESOURCES: error_codeStr = "NoResources";		break;
		case ACT_RESULT_ERROR_OK:           error_codeStr = "Ok";				break;
		case ACT_RESULT_ERROR_OTHER:        error_codeStr = "Other";				break;
		case ACT_RESULT_ERROR_TIMEOUT:      error_codeStr = "Timeout";			break;
		case ACT_RESULT_ERROR_INVALID_ARG:  error_codeStr = "Invalid argument";	break;
		case ACT_RESULT_ERROR_UNKNOWN:      error_codeStr = "Unknown";			break;
		case ACT_RESULT_ERROR_CANCELED:		error_codeStr = "Cancelled";			break;
		case ACT_RESULT_ERROR_OVERHEATING:  error_codeStr = "Overheating";		break;
		case ACT_RESULT_ERROR_OVERLOAD:     error_codeStr = "Overload";			break;
		case ACT_RESULT_ERROR_OVERVOLTAGE_OR_UNDERVOLTAGE:  error_codeStr = "Overvoltage or Undervoltage"; break;
		case ACT_RESULT_ERROR_NOT_INITIALIZED:     			error_codeStr = "Not initialized";			  break;
		default:                            error_codeStr = "Unknown error";		break;
	}
	log_level_e level = LOG_LEVEL_Debug;
	if(result != ACT_RESULT_DONE)
		level = LOG_LEVEL_Error;
	if(param_type == CAN_TPT_Normal) {
		component_printf(level, "Result msg: Act: %s(0x%x), cmd: 0x%x(%u), result: %s(%u), error: %s(%u), param: 0x%x(%u) (%s)\n\n",
			original_sid_str, original_sid & 0xFF,
			original_command, original_command,
			result_str, result,
			error_codeStr, error_code,
			param, param,
			sent_str);
	} else if(param_type == CAN_TPT_Line) {
		component_printf(level, "Result msg: Act: %s(0x%x), cmd: 0x%x(%u), result: %s(%u), error: %s(%u), line: %u (%s)\n\n",
			original_sid_str, original_sid & 0xFF,
			original_command, original_command,
			result_str, result,
			error_codeStr, error_code,
			param,
			sent_str);
	} else {
		component_printf(level, "Result msg: Act: %s(0x%x), cmd: 0x%x(%u), result: %s(%u), error: %s(%u) (%s)\n\n",
			original_sid_str, original_sid & 0xFF,
			original_command, original_command,
			result_str, result,
			error_codeStr, error_code,
			sent_str);
	}
#endif
}
