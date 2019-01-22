/**
 * Club Robot ESEO 2017-2018
 *
 * @file actuator_functions.h
 * @brief Definition de l'interface que doit implementer (tout ou en partie) chaque type d'actionneur.
 * @author Valentin
 */

#ifndef ACTUATORS_FUNCTIONS_H_
#define ACTUATORS_FUNCTIONS_H_

/**
 * @brief Définition du pointeur de fonction pour initialiser un actionneur.
 */
typedef void (*ACT_init)(void* actuator);

/**
 * @brief Définition du pointeur de fonction pour initialiser un actionneur en position.
 */
typedef error_e (*ACT_init_pos)(void* actuator);

/**
 * @brief Définition du pointeur de fonction pour stopper un actionneur.
 */
typedef void (*ACT_stop)(void* actuator);

/**
 * @brief Définition du pointeur de fonction pour remettre a zero la configuration d'un actionneur.
 */
typedef void (*ACT_reset_config)(void* actuator);

/**
 * @brief Définition du pointeur de fonction pour traiter un message CAN actionneur.
 */
typedef bool_e (*ACT_process_msg)(void *actuator, CAN_msg_t* msg);

/**
 * @brief Définition du pointeur de fonction pour executer du code en IT (routine d'interruption).
 */
typedef void (*ACT_process_it)(void *actuator);

/**
 * @brief Définition du pointeur de fonction pour executer du code en tâche de fond.
 */
typedef void (*ACT_process_main)(void *actuator);

/**
 * @brief Définition du pointeur de fonction pour obtenir la configuration d'un actionneur.
 */
typedef void (*ACT_get_config)(void *actuator, CAN_msg_t* msg);

/**
 * @brief Définition du pointeur de fonction pour fixer la configuration d'un actionneur.
 */
typedef void (*ACT_set_config)(void *actuator, CAN_msg_t* msg);

/**
 * @brief Définition du pointeur de fonction pour mettre un warner.
 */
typedef void (*ACT_set_warner)(void *actuator, CAN_msg_t* msg);

/**
 * Définition de la structure de fonctions actionneur.
 * C'est en quelque sorte une interface que doit implémenter (tout ou en partie) chaque type d'actionneur.
 * Si certianes fonctions ne sont pas implémentées, les pointeurs de fonction doivent être mis à NULL.
 */
typedef struct {
	ACT_init init;
	ACT_init_pos init_pos;
	ACT_stop stop;
	ACT_reset_config reset_config;
	ACT_process_msg process_msg;
	ACT_process_it process_it;
	ACT_process_main process_main;
	ACT_get_config get_config;
	ACT_set_config set_config;
	ACT_set_warner set_warner;
}ACT_functions_t;

#endif /* ACTUATORS_FUNCTIONS_H_ */
