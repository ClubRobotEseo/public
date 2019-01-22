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
 * @brief D�finition du pointeur de fonction pour initialiser un actionneur.
 */
typedef void (*ACT_init)(void* actuator);

/**
 * @brief D�finition du pointeur de fonction pour initialiser un actionneur en position.
 */
typedef error_e (*ACT_init_pos)(void* actuator);

/**
 * @brief D�finition du pointeur de fonction pour stopper un actionneur.
 */
typedef void (*ACT_stop)(void* actuator);

/**
 * @brief D�finition du pointeur de fonction pour remettre a zero la configuration d'un actionneur.
 */
typedef void (*ACT_reset_config)(void* actuator);

/**
 * @brief D�finition du pointeur de fonction pour lancer l'ex�cution d'un ordre actionneur.
 */
typedef act_result_state_e (*ACT_run_order)(void* actuator, ACT_order_e order, Sint32 param);

/**
 * @brief D�finition du pointeur de fonction pour v�rifier l'ex�cution du dernier ordre actionneur.
 */
typedef act_result_state_e (*ACT_check_order)(void* actuator);

/**
 * @brief D�finition du pointeur de fonction pour obtenir la configuration d'un actionneur.
 */
typedef Uint16 (*ACT_get_config)(void *actuator, act_config_e config);

/**
 * @brief D�finition du pointeur de fonction pour fixer la configuration d'un actionneur.
 */
typedef void (*ACT_set_config)(void *actuator, act_config_e config, Uint16 value);

/**
 * D�finition de la structure de fonctions actionneur.
 * C'est en quelque sorte une interface que doit impl�menter (tout ou en partie) chaque type d'actionneur.
 * Si certianes fonctions ne sont pas impl�ment�es, les pointeurs de fonction doivent �tre mis � NULL.
 */
typedef struct {
	ACT_init init;
	ACT_init_pos init_pos;
	ACT_stop stop;
	ACT_reset_config reset_config;
	ACT_run_order run_order;
	ACT_check_order check_order;
	ACT_get_config get_config;
	ACT_set_config set_config;
}ACT_functions_t;

#endif /* ACTUATORS_FUNCTIONS_H_ */
