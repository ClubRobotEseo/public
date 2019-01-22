/*
 * actuators.c
 *
 *  Created on: 14 juin 2018
 *      Author: Nirgal
 */

#include <string.h>
#include "actuators.h"
#include "secretary.h"
#include "power.h"

#include "QS/QS_all.h"
#include "QS/QS_CANmsgList.h"
#include "QS/QS_actuator/QS_ax12.h"
#include "QS/QS_actuator/QS_rx24.h"
#include "QS/QS_outputlog.h"
#include "QS/QS_lowLayer/QS_can.h"
#include "QS/QS_stateMachineHelper.h"
#include "QS/QS_who_am_i.h"

#include "actuators/actuator.h"
#include "actuators/actuator_functions.h"
#include "actuators/act_utils.h"
#include "actuators/actuators_stratEasy.h"

#include "actuators/impl/impl_ax12/impl_ax12_pos.h"
#include "actuators/impl/impl_ax12/impl_ax12_speed.h"
#include "actuators/impl/impl_rx24/impl_rx24_pos.h"
#include "actuators/impl/impl_rx24/impl_rx24_speed.h"
#include "actuators/impl/impl_pwm/impl_pwm.h"
#include "actuators/impl/impl_pwm/impl_ppm.h"


typedef struct {
	error_e state;
	Uint32 begin_time;
	void* actuator;
	ACT_functions_t* functions;
}ACTUATOR_actuator_t;


//ID soudé sur la carte act expander.
#define ACT_EXPANDER_ID		ACT_EXPANDER_ID_11
#define TIMEOUT 2000

// Macro permettant de linker vers des fonctions actionneurs
#define ACT_DECLARE_FUNCTIONS(kind, prefix)               		act_functions[kind] = (ACT_functions_t){&prefix##_init, &prefix##_init_pos, &prefix##_stop, NULL,                   &prefix##_run_order, &prefix##_check_order, NULL,                 NULL}
#define ACT_DECLARE_FUNCTIONS_WITH_CONFIG(kind, prefix)   		act_functions[kind] = (ACT_functions_t){&prefix##_init, &prefix##_init_pos, &prefix##_stop, &prefix##_reset_config, &prefix##_run_order, &prefix##_check_order, &prefix##_get_config, &prefix##_set_config}

static ACT_functions_t act_functions[NB_ACTUATOR_KINDS];  // Tableau des fonctions des actionneurs

// Macro permettant de linker vers l'instance d'un actionneur
#define ACT_DECLARE_ACTUATOR(prefix) 	nb_acts++; actuators[prefix.id] = (ACTUATOR_actuator_t) {.state = END_OK, .actuator = &prefix, .functions = &(act_functions[prefix.kind])}

static ACTUATOR_actuator_t actuators[ACTUATOR_NB];  // Tableau des instances des actionneurs

// Temps d'attente avant l'initialisation en position des actionneurs (temps d'établisssment de l'alim)
#define TIME_BEFORE_INIT_POS	(200)

// Délai d'attente entre l'initialisation de deux actionneurs
#define DELAY_INIT_POS			(50)

// Fonctions privees
static void ACTUATOR_declare_actuators();
static void ACTUATOR_declare_functions();
static error_e ACTUATOR_mae_reset_act(bool_e reset);

// Variables privées
static bool_e init_reset_act = FALSE;


/**
 * @brief Fonction permettant de déclarer les actionneurs.
 * Cette fonction permet d'initialiser le tableau actuators[].
 * Ceci ne peut pas être fait statiquement à la compilation car les instances des actionneurs ne sont pas constantes.
 */
static void ACTUATOR_declare_actuators() {
	Uint8 i = 0;
	Uint8 nb_acts = 0; // Utiliser dans la macro ACT_DECLARE_ACTUATOR pour la vérification du nombre d'actionneur

	// On initialise le tableau d'actionneurs
	for(i = 0; i < ACTUATOR_NB; i++) {
		actuators[i] = (ACTUATOR_actuator_t) {.state = END_OK, .actuator = NULL, .functions = NULL};
	}

	// Déclaration des actionneurs de STRAT_EASY
	//ACT_DECLARE_ACTUATOR(exemple);
	ACT_DECLARE_ACTUATOR(arm_left);
	ACT_DECLARE_ACTUATOR(arm_right);

	// Vérification
	assert(nb_acts <= ACTUATOR_NB); // Nombre d'actionneurs déclarés supérieur à ACTUATOR_NB

	const char* robot_name = QS_WHO_AM_I_get_name();
	debug_printf("%s has %d actuators available.\n", robot_name, i);
}

/**
 * @brief Fonction permettant de déclarer les fonctions actionneurs.
 * Cette fonction permet d'initialiser le tableau act_functions[].
 * Cette affectation est faite dynamiquement pour être indépendant de l'ordre de déclaration des kinds dans l'enum Actuator_kind_e.
 */
static void ACTUATOR_declare_functions() {
#ifdef USE_AX12_SERVO
	ACT_DECLARE_FUNCTIONS_WITH_CONFIG(KIND_AX12_POS, IMPL_AX12_POS);
	ACT_DECLARE_FUNCTIONS_WITH_CONFIG(KIND_AX12_SPEED, IMPL_AX12_SPEED);
#endif
#ifdef USE_RX24_SERVO
	ACT_DECLARE_FUNCTIONS_WITH_CONFIG(KIND_RX24_POS, IMPL_RX24_POS);
	ACT_DECLARE_FUNCTIONS_WITH_CONFIG(KIND_RX24_SPEED, IMPL_RX24_SPEED);
#endif
	ACT_DECLARE_FUNCTIONS(KIND_PWM, IMPL_PWM);
	ACT_DECLARE_FUNCTIONS(KIND_PPM, IMPL_PPM);
}

/**
 * @brief Fonction d'initialisation de ActManager. Initialise les actionneurs en position.
 */
void ACTUATOR_init() {
	Uint8 i;

	// Declaration des fonctions et des instances des actionneurs
	ACTUATOR_declare_functions();
	ACTUATOR_declare_actuators();

	//Initialisation des actionneurs
	debug_printf("---------------- Initialisation des actionneurs -----------------\n");
	for(i = 0; i < ACTUATOR_NB; i++) {
		if(actuators[i].functions != NULL && actuators[i].functions->init != NULL) {
			actuators[i].functions->init(actuators[i].actuator);
		}
	}
	debug_printf("-----------------------------------------------------------------\n");

	ACTUATOR_reset_act();
}

/**
 * @brief Fonction qui indique une demande de réinitialisation de tous les actionneurs en position.
 */
void ACTUATOR_reset_act() {
	init_reset_act = TRUE;
}

/**
 * @brief Fonction qui réinitialise tous les actionneurs en position.
 */
void ACTUATOR_run_reset_act() {
	static error_e ret_mae = IN_PROGRESS;
	static bool_e reset_act_launch = FALSE;
	static bool_e previous_power = FALSE;
	bool_e powerup;
	bool_e current_power;
	current_power = POWER_get_state();
	powerup = (!previous_power && current_power)?TRUE:FALSE;
	previous_power = current_power;

	//Si le match n'est pas commencé où est terminé...
	if(global.flags.match_started == FALSE || global.flags.match_over == TRUE)
	{
		if(init_reset_act || powerup)
		{
			ACTUATOR_mae_reset_act(TRUE);
			init_reset_act = FALSE;
			reset_act_launch = TRUE;
		}
		else if(reset_act_launch)
		{
			ret_mae = ACTUATOR_mae_reset_act(FALSE);
			if(ret_mae != IN_PROGRESS)
			{
				reset_act_launch = FALSE;
			}
		}
	}
}

/**
 * @brief Machine à étét qui initialise tous les actionneurs en position.
 * On attend TIME_BEFORE_INIT_POS avant de lancé le premier init.
 * On attent DELAY_INIT_POS entre chaque servo pour ne pas trop en demander à l'alim.
 * @param reset permet de réinitialiser la mae.
 * @return IN_PROGRESS si l'action est en cours et END_OK lorsque c'est fini.
 */
static error_e ACTUATOR_mae_reset_act(bool_e reset) {

	CREATE_MAE(INIT,
			SELECT_ACTUATOR,
			INIT_POS_ACTUATOR,
			WAIT,
			DONE);

	static time32_t local_time;
	static Uint8 act_id;
	static error_e status;
	char * name;

	if(reset){
		RESET_MAE();
		return IN_PROGRESS;
	}

	switch(state) {
		case INIT:
			if(entrance) {
				act_id = 0;
				local_time = global.absolute_time + TIME_BEFORE_INIT_POS;
			}

			if(global.absolute_time > local_time) {
				state = SELECT_ACTUATOR;
				debug_printf("------------------ Initialisation en position -------------------\n");
			}
			break;

		case SELECT_ACTUATOR:
			while(act_id < ACTUATOR_NB &&
					(actuators[act_id].functions == NULL ||
					actuators[act_id].functions->init_pos == NULL ||
					((Actuator_t*) actuators[act_id].actuator)->standard_init == FALSE)
			) {
				act_id++;
			}

			if(act_id >= ACTUATOR_NB)
				state = DONE;
			else
				state = INIT_POS_ACTUATOR;
			break;

		case INIT_POS_ACTUATOR:
			if(entrance) {
				name = ((Actuator_t *) (actuators[act_id].actuator))->name;
				if(name == NULL || strncmp(name, "", 10) == 0) {
					name = "unknown";
				}
				debug_printf("Actuator \"%s\" init position\n", name);
			}
			status = actuators[act_id].functions->init_pos(actuators[act_id].actuator);

			if(status != IN_PROGRESS) {
				act_id++; // Incremente le act_id, celui-ci a été traité
				state = WAIT;
			}
			break;

		case WAIT:
			// Il est préférable d'attendre un peu entre l'init de deux actionneurs pour que l'alim se rétablisse correctement.
			if(entrance) {
				local_time = global.absolute_time + DELAY_INIT_POS;
			}
			if(global.absolute_time > local_time) {
				state = SELECT_ACTUATOR;
			}
			break;

		case DONE:
			debug_printf("-----------------------------------------------------------------\n");
			RESET_MAE();
			return END_OK;
			break;

		default:
			debug_printf("Default case of ACTUATOR_mae_reset_act()\n");
	}
	return IN_PROGRESS;
}

/**
 * @brief Fonction qui stoppe tous les actionneurs (asservissement et puissance).
 */
void ACTUATOR_stop()
{
	Uint8 i;
	for(i = 0; i < ACTUATOR_NB; i++) {
		if(actuators[i].functions != NULL && actuators[i].functions->stop != NULL)
			actuators[i].functions->stop(actuators[i].actuator);
	}
}

/**
 * @brief Fonction permettant de réinitialiser la config de tous les actionneurs.
 */
void ACTUATOR_reset_config() {
	Uint8 i;
	debug_printf("--------------- Re-initialisation des actionneurs ---------------\n");
	for(i = 0; i < ACTUATOR_NB; i++) {
		if(actuators[i].functions != NULL && actuators[i].functions->reset_config != NULL)
			actuators[i].functions->reset_config(actuators[i].actuator);
	}
	debug_printf("-----------------------------------------------------------------\n");
}

/**
 * @brief Fonction qui permet de récupérer la valeur d'une config sur un actionneur.
 * @param id l'id de l'actionneur concerné.
 * @return la valeur de la config demandée.
 */
Uint16 ACTUATOR_get_config(actuator_e id, act_config_e config) {
	Uint16 value = 0;
	if(id < ACTUATOR_NB && actuators[id].functions != NULL && actuators[id].functions->get_config != NULL) {
		value = actuators[id].functions->get_config(actuators[id].actuator, config);
	} else {
		error_printf("Msg ACT_GET_CONFIG not processed for actuator id=0x%x\n",id);
	}
	return value;
}

/**
 * @brief Fonction qui permet de setter une config sur un actionneur.
 * @param id l'id de l'actionneur concerné.
 * @param config la configuration à changer.
 * @param value la nouvelle valeur de la configuration (Pour les servos: Si value est égale à 0, cela reprend la valeur par défaut).
 */
void ACTUATOR_set_config(actuator_e id, act_config_e config, Uint16 value) {
	if(id < ACTUATOR_NB && actuators[id].functions != NULL && actuators[id].functions->set_config != NULL) {
		actuators[id].functions->set_config(actuators[id].actuator, config, value);
	} else {
		error_printf("Msg ACT_SET_CONFIG not processed for actuator id=0x%x\n", id);
	}
}

/**
 * @brief Envoie un ordre à un actionneur.
 * @param id l'id de lactionneur concerné.
 * @param param un paramètre.
 * @param order l'ordre demandé.
 */
void ACTUATOR_go(actuator_e id,  ACT_order_e order, Sint32 param)
{
	CAN_msg_t msg;
	bool_e order_processed = FALSE;

	// Dispatch du message aux actionneurs implémentés dans le répertoire "actuators" (servos)
	act_result_state_e result = ACT_RESULT_NOT_HANDLED;
	if(id < ACTUATOR_NB && actuators[id].functions != NULL && actuators[id].functions->run_order != NULL) {
		order_processed = TRUE;
		result = actuators[id].functions->run_order(actuators[id].actuator, order, param);
		if(result != ACT_RESULT_DONE) {
			error_printf("Execution of order=%d for actuator id=%d  has failed\n", order, id);
		} else {
			actuators[id].state = IN_PROGRESS;
			actuators[id].begin_time = global.absolute_time;
		}
	}

	if(!order_processed) {
		switch(id)
		{
			case ACTUATOR_PUMP_LEFT:
			//no break;
			case ACTUATOR_PUMP_RIGHT:
				msg.sid = ACT_EXPANDER_SET_PUMP;
				msg.data.actExpander_setPump.id = ACT_EXPANDER_ID;
				msg.data.actExpander_setPump.idPump = (id == ACTUATOR_PUMP_LEFT)?0:1;
				msg.data.actExpander_setPump.state = (order)?TRUE:FALSE;
				msg.size = SIZE_ACT_EXPANDER_SET_PUMP;
				SECRETARY_send(&msg);
				if(global.flags.virtual_mode)
					actuators[id].state = END_OK;
				else
					actuators[id].state = IN_PROGRESS;
				actuators[id].begin_time = global.absolute_time;
				break;
			case ACTUATOR_SOLENOID_VALVE_LEFT:
					//no break;
			case ACTUATOR_SOLENOID_VALVE_RIGHT:
				msg.sid = ACT_EXPANDER_SET_SOLENOID_VALVE;
				msg.data.actExpander_setSolenoideValve.id = ACT_EXPANDER_ID;
				msg.data.actExpander_setSolenoideValve.idSolenoidValve = (id == ACTUATOR_SOLENOID_VALVE_LEFT)?0:1;
				msg.data.actExpander_setSolenoideValve.state = (order)?TRUE:FALSE;
				msg.data.actExpander_setSolenoideValve.duration = 3000;
				msg.size = SIZE_ACT_EXPANDER_SET_SOLENOID_VALVE;
				SECRETARY_send(&msg);
				if(global.flags.virtual_mode)
					actuators[id].state = END_OK;
				else
					actuators[id].state = IN_PROGRESS;
				actuators[id].begin_time = global.absolute_time;
				break;
			default:
				error_printf("Order=%d for actuator id=%d cannot be executed\n", order, id);
				break;

		}
	}
}

/**
 * @brief Permet de vérifier si un actionneur est arrivé.
 * @param id l'id de l'actionneur concerné.
 * @return TRUE si l'actionneur est arrivé et FALSE sinon.
 */
bool_e ACTUATOR_is_arrived(actuator_e id)
{
	return actuators[id].state == END_OK;
}

/**
 * @brief Permet de vérifier si un actionneur est en timeout.
 * @param id l'id de l'actionneur concerné.
 * @return TRUE si l'actionneur est en timeout et FALSE sinon.
 */
bool_e ACTUATOR_is_in_timeout(actuator_e id)
{
	return actuators[id].state == END_WITH_TIMEOUT;
}

/**
 * @brief Fonction de traitement des messages CAN.
 * @param msg le message CAN à traiter.
 */
void ACTUATOR_process_msg(CAN_msg_t * msg)
{
	actuator_e id;
	switch(msg->sid)
	{
		case ACT_EXPANDER_RESULT_SET_PUMP:
			if(msg->data.actExpander_resultSetPump.idPump <= 1)
			{
				id = msg->data.actExpander_resultSetPump.idPump + ACTUATOR_PUMP_LEFT;
				if(actuators[id].state == IN_PROGRESS)
				{
					actuators[id].state = END_OK;
				}
			}
			break;
		case ACT_EXPANDER_RESULT_SET_SOLENOID_VALVE:
			if(msg->data.actExpander_resultSetSolenoideValve.idSolenoidValve <= 1)
			{
				id = msg->data.actExpander_resultSetSolenoideValve.idSolenoidValve + ACTUATOR_SOLENOID_VALVE_LEFT;
				if(actuators[id].state == IN_PROGRESS)
				{
					actuators[id].state = END_OK;
				}
			}
			break;
		default:
			break;
	}

}

/**
 * @brief Fonction exécutée en tâche de fond.
 */
void ACTUATOR_process_main(void)
{
	uint8_t id;
	act_result_state_e result;

	for(id=0; id<ACTUATOR_NB; id++)
	{
		// Vérification des ordres actionneurs
		if(actuators[id].functions != NULL && actuators[id].functions->check_order != NULL) {
			result = actuators[id].functions->check_order(actuators[id].actuator);
			if(result == ACT_RESULT_DONE) {
				actuators[id].state = END_OK;
			} else if(result == ACT_RESULT_FAILED) {
				actuators[id].state = NOT_HANDLED;
			}
		}

		// Traitement des timeouts
		if(actuators[id].state == IN_PROGRESS)
		{
			if(actuators[id].begin_time + TIMEOUT <= global.absolute_time)
				actuators[id].state = END_WITH_TIMEOUT;
		}
	}

	// Rénitialisation des actionneurs
	ACTUATOR_run_reset_act();
}
