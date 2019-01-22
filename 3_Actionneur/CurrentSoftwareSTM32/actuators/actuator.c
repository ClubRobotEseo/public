/**
 * Club Robot ESEO 2017 - 2018
 *
 * @file actuator.c
 * @brief Data structure of an abstract actuator.
 * @author Valentin
 */
#include "actuator.h"

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
bool_e ACT_get_command_value(Sint16 *value, ACT_order_e order, Actuator_command_t commands[], Uint8 nb_commands)
{
	bool_e found = FALSE;
	Sint16 cmd_value = ERROR_ACT_VALUE;
	Uint8 i = 0;

	// If no commands, we cannot find it
	if(commands == NULL || nb_commands == 0) {
		return FALSE;
	}

	// On essaie avec un accès en O(1) pour aller plus vite.
	Sint16 one_index = order - commands[0].order;
	if(one_index >= 0 && one_index < nb_commands && commands[one_index].order == order){
		cmd_value = commands[one_index].value;
		found = TRUE;
	}else{
		while(i < nb_commands && !found){
			if(commands[i].order == order) {
				found = TRUE;
				cmd_value = commands[i].value;
			} else {
				i++;
			}
		}
	}

	if(value != NULL) {
		*value = cmd_value;
	}

	return found;
}

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
bool_e ACT_get_command_index(Sint16 *index, ACT_order_e order, Actuator_command_t commands[], Uint8 nb_commands)
{
	bool_e found = FALSE;
	Sint16 cmd_index = ERROR_ACT_VALUE;
	Uint8 i = 0;

	// If no commands, we cannot find it
	if(commands == NULL || nb_commands == 0) {
		return FALSE;
	}

	// On essaie avec un accès en O(1) pour aller plus vite.
	Sint16 one_index = order - commands[0].order;
	if(one_index >= 0 && one_index < nb_commands && commands[one_index].order == order){
		found = TRUE;
		cmd_index = one_index;
		found = TRUE;
	}else{
		while(i < nb_commands && !found){
			if(commands[i].order == order) {
				found = TRUE;
				cmd_index = i;
			} else {
				i++;
			}
		}
	}

	if(index != NULL) {
		*index = cmd_index;
	}

	return found;
}

/**
 * @brief Calcule le modulo d'une valeur. result = value % modulo.
 * @param value la valeur dont il faut calculer le modulo.
 * @param modulo la valeur du modulo.
 * @return la valeur calculee.
 */
Sint32 ACT_compute_modulo(Sint32 value, Sint32 modulo) {
	Sint32 result = value;

	while(result < 0) {
		result += 1024;
	}

	while(result > 1024) {
		result -= 1024;
	}

	return result;
}
