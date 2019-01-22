/**
 * Club Robot ESEO 2017 - 2018
 *
 * @file actuator.h
 * @brief Data structure of an abstract actuator.
 * @author Valentin
 */

#ifndef ACTUATORS_ACTUATOR_H_
#define ACTUATORS_ACTUATOR_H_

#include "../QS/QS_all.h"

/**
 * Enumeration des types d'actionneurs.
 */
typedef enum {
	KIND_AX12_POS,
	KIND_AX12_SPEED,
//	KIND_AX12_POS_DOUBLE,
	KIND_RX24_POS,
	KIND_RX24_SPEED,
//	KIND_RX24_POS_DOUBLE,
	KIND_PWM,
	KIND_PPM,
//	KIND_MOSFET,
//	KIND_MOSFET_ON_MOSFET_BOARD,
	NB_ACTUATOR_KINDS
} Actuator_kind_e;

/**
 * Enumeration des types de commandes. (Utile uniquemant quant un actionneur supporte différents types de commandes).
 */
typedef enum {
	CMD_ASSER_POSITION = 0, // Default value for servo
	CMD_ASSER_TORQUE
} Command_kind_e;

typedef enum {
	WAY_ASSER_TORQUE_INC, // On asservit en couple en forcant vers 1024 (en incrémentant)
	WAY_ASSER_TORQUE_DEC // On asservit en couple en forcant vers    0 (en décrémentant)
}Way_asser_torque_e;

/**
 * Etats pour la machine à états permettant d'exécuter une commande.
 */
typedef enum {
	CMD_ACT_INIT,
	CMD_ACT_RUN,
}Command_state_e;

/**
 * Definition d'un type de commande spécifique.
 */
typedef union {
	struct Command_asser_torque_t{
		Uint8 threshold;
		Way_asser_torque_e way;
	} torque;
} Command_param_t;

/**
 * Definition of an actuator command composed of :
 * 		- an order used to designate the command on the strategy level.
 * 		- a value which is the real value of the command to send to the actuator.
 */
typedef const struct {
	 ACT_order_e order;
	 Sint16 value;
	 Command_kind_e kind;
	 Command_param_t param;
} Actuator_command_t;

/**
 * Compte le nombre de commandes actionneurs dans un tableau de commandes.
 */
#define NB_ACT_COMMANDS(x)		(sizeof(x) / sizeof(Actuator_command_t))

/**
 * Valeur par défaut en cas d'erreur
 */
#define ERROR_ACT_VALUE		(0xFFFF)

/**
 * Definition of the data structure of an abstract actuator.
 */
#define ABSTRACT_Actuator_t \
			char * name; \
			Actuator_kind_e kind; \
			Uint16 id; \
			ACT_order_e current_order; \
			Sint16 current_param; \
			bool_e cmd_launched; \
			bool_e is_initialized; \
			bool_e standard_init; \

/**
 * Definition of an actuator type.
 * (Useful to access field of concrete but unkown actuator type.)
 */
typedef struct {
	ABSTRACT_Actuator_t;
} Actuator_t;

/**
 * @brief Fonction permettant de récupérer la valeur d'une commande à partir d'un ordre.
 * @param value le pointeur dans lequel on doit stocker la valeur de la commande.
 * @param order l'ordre ont on veut la valeur associée.
 * @param commands la liste des commandes
 * @param nb_commands le nombre de commandes
 * @return TRUE si l'ordre est valide et FALSE sinon.
 * Un ordre est considéré comme valide s'il est présent dans sa lite de commandes.
 * @pre Il faut vérifier que l'ordre est valide avec ACT_order_is_valid().
 */
bool_e ACT_get_command_value(Sint16 *value, ACT_order_e order, Actuator_command_t commands[], Uint8 nb_commands);

/**
 * @brief Fonction permettant de récupérer l'index d'une commande à partir d'un ordre.
 * @param index le pointeur dans lequel on doit stocker l'index de la commande.
 * @param order l'ordre ont on veut la valeur associée.
 * @param commands la liste des commandes
 * @param nb_commands le nombre de commandes
 * @return TRUE si l'ordre est valide et FALSE sinon.
 * Un ordre est considéré comme valide s'il est présent dans sa lite de commandes.
 * @pre Il faut vérifier que l'ordre est valide avec ACT_order_is_valid().
 */
bool_e ACT_get_command_index(Sint16 *index, ACT_order_e order, Actuator_command_t commands[], Uint8 nb_commands);

/**
 * @brief Calcule le modulo d'une valeur. result = value % modulo.
 * @param value la valeur dont il faut calculer le modulo.
 * @param modulo la valeur du modulo.
 * @return la valeur calculee.
 */
Sint32 ACT_compute_modulo(Sint32 value, Sint32 modulo);

#endif /* ACTUATORS_ACTUATOR_H_ */
