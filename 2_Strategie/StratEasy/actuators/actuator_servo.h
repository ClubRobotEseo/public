/**
 *	Club Robot ESEO 2017 - 2018
 *
 * @file actuator_servo.h
 * @brief Definition d'un actionneur servomoteur.
 * @author Valentin
 */

#ifndef ACTUATORS_ACTUATOR_SERVO_H_
#define ACTUATORS_ACTUATOR_SERVO_H_

#include "actuator.h"
#include "../QS/Qs_all.h"
#include "../QS/QS_CANmsgList.h"


// Valeurs par défaut des servos AX12 et RX24
#define DEFAULT_SERVO_TIMEOUT				2000	// Si le servo n'atteint pas la position demandée avant ce temps, on considère qu'il y a un problème. Temps en ms.
#define DEFAULT_SERVO_POS_EPSILON			6		// Lorsque le servo atteint sa position avec +/- POS_EPSILON degré d'écart max, on considère qu'il a atteint sa position. Angle en degré.
#define DEFAULT_SERVO_POS_LARGE_EPSILON		28		// Si le déplacement du servotimeout mais que sa position est à +/- POS_LARGE_EPSILON degré d'écart max, on considère qu'il a atteint sa position. Angle en degré.
#define DEFAULT_SERVO_MAX_TORQUE_PERCENT	80		// Couple maximum en pourcentage du servo. Utilisé pour limiter le courant dans le moteur. A mettre a une valeur correcte pour pincer assez fort sans risquer d'endommager le servo.
#define DEFAULT_SERVO_MAX_TEMPERATURE		60		// Température maximale en degré
#define DEFAULT_SERVO_SPEED					100		// Vitesse par défaut du servo
#define DEFAULT_SERVO_MIN_VALUE				0		// Position minimale par défaut du servo
#define DEFAULT_SERVO_MAX_VALUE				1024	// Position maximale par défaut du servo
#define DEFAULT_SERVO_KP					48		// Valeur par default du Kp pour l'asservissement en couple du servo

// Valeurs utiles pour le mode simulation
#define AX12_THEORETICAL_SPEED				59		// Vitesse théorique d'un AX12 (en rpm)
#define RX24_THEORETICAL_SPEED				126		// Vitesse théorique d'un RX24 (en rpm)


/**
 * Definition de la structure de données pour la simulation d'un servo.
 */
typedef struct {
	time32_t exec_finished_time;
	Sint16 current_pos;
	Uint8 current_speed;
} Actuator_servo_simu_t;

/**
 * Definition de la structure de données d'asservissement en couple d'un servomoteur.
 */
typedef struct {
	Sint32 Kp;
	bool_e activated;
	bool_e ask_to_finish;
} Actuator_servo_torque_asser_t;


/**
 * Definition de la structure de données d'un servomoteur.
 */
typedef struct {
	Uint8 id;
	Uint16 timeout;
	time32_t exec_start_time;
	Uint8 epsilon;
	Uint8 large_epsilon;
	Uint8 max_torque;
	Uint8 max_temperature;
	Uint8 speed;
	Uint16 min_angle;
	Uint16 max_angle;
	Sint16 init_pos;
//	Actuator_servo_torque_asser_t torque_asser;  // Non utilisé sur StratEasy
	Actuator_command_t * commands;
	Uint8 nb_commands;
	Sint16 current_cmd_index;
	Actuator_servo_simu_t simu;
} Actuator_servo_data_t;

/**
 * Definition d'un actionneur servomoteur simple.
 */
typedef struct {
	ABSTRACT_Actuator_t;
	Actuator_servo_data_t servo;
} Actuator_servo_t;

/**
 * Definition d'un actionneur servomoteur double.
 */
typedef struct {
	ABSTRACT_Actuator_t;
	Actuator_servo_data_t servo[2];
} Actuator_servo_double_t;

/**
 * @brief Calcul du temps mis par un servo pour effectuer une commande d'asservissement en position.
 * @param kind le type de l'actionneur.
 * @param servo_speed la vitesse du servo.
 * @param goal_position la nouvelle position demandé.
 * @param current_position la position courante du servo.
 * @return le temps thérotique mis par le servo pour effectuer la commande (en ms).
 */
time32_t ACT_SERVO_compute_cmd_exec_time(Actuator_kind_e kind, Uint8 servo_speed, Sint16 goal_position, Sint16 current_position);

#ifdef USE_AX12_SERVO
/**
 * @brief Fonction de configuration des AX12.
 * @param servo l'instance du servo concerne (ici juste Actuator_servo_data_t suffit).
 * @param config la configuration à changer.
 * @param value la nouvelle valeur de la configuration (Si égale à 0, cela reprend la valeur par défaut).
 */
void ACT_SERVO_config_AX12(Actuator_servo_data_t * servo, act_config_e config, Uint16 value);
#endif /* USE_RX12_SERVO */

#ifdef USE_RX24_SERVO
/**
 * @brief Fonction de configuration des RX24.
 * @param servo l'instance du servo concerne (ici juste Actuator_servo_data_t suffit).
 * @param config la configuration à changer.
 * @param value la nouvelle valeur de la configuration (Si égale à 0, cela reprend la valeur par défaut).
 */
void ACT_SERVO_config_RX24(Actuator_servo_data_t * servo, act_config_e config, Uint16 value);
#endif /* USE_RX24_SERVO */

#endif /* ACTUATORS_ACTUATOR_SERVO_H_ */
