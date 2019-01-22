/*
 *	Club Robot ESEO 2009 - 2010
 *	CHOMP
 *
 *	Fichier : act_functions.h
 *	Package : Carte Principale
 *	Description : 	Fonctions de gestion de la pile
 *					de l'actionneur
 *	Auteur : Julien et Ronan
 *	Version 20110313
 */

#ifndef ACT_FUNCTIONS_H
#define ACT_FUNCTIONS_H

#include "../QS/QS_all.h"
#include "../QS/QS_CANmsgList.h"
#include "queue.h"
#include "act_can.h"

//Voir aussi act_can.h et state_machine_helper.h

typedef struct{
	ACT_sid_e sid;
	queue_id_e queue_id;
	char * name;
}act_link_SID_Queue_s;

typedef struct{
	bool_e info_received;
	ACT_order_e pos;
	Sint8 torque;
	Uint8 temperature;
	Sint8 load;
}act_config_s;

typedef enum{
	ACT_EXPANDER_PUMP_BIG_BACKWARD_BEGIN,
	ACT_EXPANDER_PUMP_BIG_BACKWARD_NULL_0,
	ACT_EXPANDER_PUMP_BIG_BACKWARD_VERY_RIGHT,
	ACT_EXPANDER_PUMP_BIG_BACKWARD_VERY_LEFT,
	ACT_EXPANDER_PUMP_BIG_BACKWARD_LEFT,
	ACT_EXPANDER_PUMP_BIG_BACKWARD_RIGHT,
	ACT_EXPANDER_PUMP_BIG_BACKWARD_END,

	ACT_EXPANDER_PUMP_BIG_FORWARD_BEGIN,
	ACT_EXPANDER_PUMP_BIG_FORWARD_RIGHT,
	ACT_EXPANDER_PUMP_BIG_FORWARD_MIDDLE,
	ACT_EXPANDER_PUMP_BIG_FORWARD_LEFT,
	ACT_EXPANDER_PUMP_BIG_FORWARD_END,

	ACT_EXPANDER_PUMP_SMALL_BEGIN,
	ACT_EXPANDER_PUMP_SMALL_END,

	ACT_EXPANDER_NB_PUMP,
	ACT_EXPANDER_PUMP_ERROR
}actExpanderPumpId_e;

typedef enum{
	ACT_EXPANDER_SENSOR_BIG_BACKWARD_BEGIN,
	ACT_EXPANDER_SENSOR_BIG_BACWARD_NULL_0,
	ACT_EXPANDER_SENSOR_BIG_BACWARD_NULL_1,
	ACT_EXPANDER_SENSOR_BIG_BACWARD_NULL_2,
	ACT_EXPANDER_SENSOR_BIG_BACWARD_NULL_3,
	ACT_EXPANDER_SENSOR_BIG_BACWARD_NULL_4,
	ACT_EXPANDER_SENSOR_BIG_BACWARD_NULL_5,
	ACT_EXPANDER_SENSOR_BIG_BACWARD_NULL_6,
	ACT_EXPANDER_SENSOR_BIG_BACWARD_NULL_7,
	ACT_EXPANDER_SENSOR_BIG_BACKWARD_END,

	ACT_EXPANDER_SENSOR_BIG_FORWARD_BEGIN,
	ACT_EXPANDER_SENSOR_BIG_FORWARD_NULL_0,
	ACT_EXPANDER_SENSOR_BIG_FORWARD_NULL_1,
	ACT_EXPANDER_SENSOR_BIG_FORWARD_NULL_2,
	ACT_EXPANDER_SENSOR_BIG_FORWARD_NULL_3,
	ACT_EXPANDER_SENSOR_BIG_FORWARD_END,

	ACT_EXPANDER_SENSOR_BIG_ELEVATOR_BEGIN,
	ACT_EXPANDER_SENSOR_BIG_ELEVATOR_END,

	ACT_EXPANDER_SENSOR_SMALL_BEGIN,
	ACT_EXPANDER_SENSOR_SMALL_NULL_0,
	ACT_EXPANDER_SENSOR_SMALL_NULL_1,
	ACT_EXPANDER_SENSOR_SMALL_NULL_2,
	ACT_EXPANDER_SENSOR_SMALL_NULL_3,
	ACT_EXPANDER_SENSOR_SMALL_NULL_4,
	ACT_EXPANDER_SENSOR_SMALL_NULL_5,
	ACT_EXPANDER_SENSOR_SMALL_END,

	ACT_EXPANDER_NB_SENSOR,
	ACT_EXPANDER_SENSOR_ERROR
}actExpanderSensorId_e;

extern const act_link_SID_Queue_s act_link_SID_Queue[];
Uint8 ACT_search_link_SID_Queue(ACT_sid_e sid);

/*
 * Exemple de code pour faire une action et l'attendre:
 * case MON_ETAT:
 *                    // On demande qu'une seule fois de faire l'action. Les actions du même actionneur peuvent
 *      if(entrance)  // être empilée et seront executées les unes après les autres
 *           ACT_fruit_mouth_goto(ACT_FRUIT_Mid);
 *
 *      //Après la demande, on attend la fin de l'action (ou des actions si plusieurs ont été empilées)
 *      //Pour la liste des actionneurs (comme ACT_QUEUE_Fruit), voir l'enum queue_e
 *      state = check_act_status(ACT_QUEUE_Fruit, MON_ETAT, ETAT_OK, ETAT_ERREUR);
 *
 *      //check_act_status est définie dans state_machine_helper.h/c
 *      break;
 */

bool_e ACT_push_order_with_all_params(ACT_sid_e sid,  ACT_order_e order, Uint16 param, Uint16 timeout, bool_e run_now);
bool_e ACT_push_order(ACT_sid_e sid,  ACT_order_e order);
bool_e ACT_push_order_with_timeout(ACT_sid_e sid,  ACT_order_e order, Uint16 timeout);
bool_e ACT_push_order_with_param(ACT_sid_e sid,  ACT_order_e order, Uint16 param);
bool_e ACT_push_order_now(ACT_sid_e sid,  ACT_order_e order);
bool_e ACT_push_order_queued(ACT_sid_e sid,  ACT_order_e order);

Uint8 ACT_check_status(ACT_sid_e sid, Uint8 in_progress_state, Uint8 success_state, Uint8 failed_state);

// -------------------------------- Fonctions de pilotage haut niveau des actionneurs (avec machine à état intégré)

/**
 * @brief ACT_get_sensor
 * @arg act_sensor_id : le capteur voulu
 * @return :	END_OK				-> Le capteur détecte une présence
 *				END_WITH_TIMEOUT	-> pas de réponse actionneur
 *				NOT_HANDLED			-> Le capteur ne détecte aucune présence
 *				IN_PROGRESS			-> demande en cours
 */
error_e ACT_get_sensor(act_sensor_id_e act_sensor_id);

void ACT_sensor_answer(CAN_msg_t* msg);

// -------------------------------- Fonctions de configuration des actionneurs

bool_e ACT_set_config(Uint16 sid, act_config_e cmd, Uint16 value);

// -------------------------------- Fonctions permettant de récupérer l'état courant de l'actionneur (uniquement pour AX12 et RX24)

/**
 * @brief ACT_get_current_state
 * @arg sid : le sid de l'actionneur
 * @arg order : l'ordre que l'on veut vérifier
 * @return : END_OK  				-> l'actionneur est dans la position demandé
 * 			 NOT_HANDLED   			-> l'actionneur n'est pas dans la position demandé
 * 			 END_WITH_TIMEOUT		-> pas de réponse actionneur
 * 			 IN_PROGRESS			-> demande en cours
 *
 * Exemple : ACT_get_current_state(MY_ACT, MY_ACT_OPEN_POS)
 * La valeur de retour sera END_OK si l'actionneur MY_ACT est bien dans la position MY_ACT_OPEN_POS.
 * La valeur de retour sera END_OK si l'actionneur MY_ACT est dans une position différente de MY_ACT_OPEN_POS.
 */
error_e ACT_check_position_config(Uint16 sid, ACT_order_e order);

void ACT_get_config_answer(CAN_msg_t* msg);
bool_e ACT_get_config_request(Uint16 sid, act_config_e config);


bool_e ACT_set_warner(Uint16 sid, ACT_order_e pos);
void ACT_warner_answer(CAN_msg_t* msg);
bool_e ACT_get_warner(Uint16 sid);
void ACT_reset_all_warner();

// ACT Expander

void ACT_EXPANDER_setPump(actExpanderPumpId_e id, bool_e state);
void ACT_EXPANDER_setPumpAll(actExpanderId_e id, bool_e state);

void ACT_EXPANDER_setSolenoidValve(actExpanderPumpId_e id, bool_e state);
void ACT_EXPANDER_setSolenoideValveAll(actExpanderId_e id, bool_e state);

void ACT_EXPANDER_setSolenoidValve(actExpanderPumpId_e id, bool_e state);
Uint8 ACT_EXPANDER_waitStateVacuostat(actExpanderPumpId_e id, ACT_EXPANDER_pumpStatus_e stateToWait, Uint8 inProgress, Uint8 success, Uint8 fail);
ACT_EXPANDER_pumpStatus_e ACT_EXPANDER_getStateVacuostat(actExpanderPumpId_e idVacuostat);
void ACT_EXPANDER_receiveVacuostat_msg(CAN_msg_t * msg);
void ACT_EXPANDER_vacuostaProcessMain();

Uint8 ACT_EXPANDER_waitGetColorSensor(actExpanderSensorId_e id, ACT_EXPANDER_colorSensor_e * color, time32_t timeout, Uint8 inProgress, Uint8 success, Uint8 fail);
Uint8 ACT_EXPANDER_waitStateColorSensor(actExpanderSensorId_e id, ACT_EXPANDER_colorSensor_e color, time32_t timeout, Uint8 inProgress, Uint8 success, Uint8 fail);
void ACT_EXPANDER_receiveColorSensorMsg(CAN_msg_t * msg);

bool_e ACT_EXPANDER_setDistanceSensorThreshold(actExpanderSensorId_e id, Uint16 thresholdDistance, ACT_EXPANDER_DistanceThresholdWay_e way);
bool_e ACT_EXPANDER_getThreshold(actExpanderSensorId_e id);
Uint8 ACT_EXPANDER_getDistanceSensorMeasure(actExpanderSensorId_e id, bool_e * resultPresence, Sint16 * resultDistance, Uint8 inProgress, Uint8 success, Uint8 fail);
void ACT_EXPANDER_receiveDistanceSensorMsg(CAN_msg_t * msg);

void ACT_EXPANDER_receiveErrorShortCircuitMsg(CAN_msg_t * msg);


#endif /* ndef ACT_FUNCTIONS_H */
