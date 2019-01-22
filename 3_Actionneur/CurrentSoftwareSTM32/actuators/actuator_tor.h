/**
 *	Club Robot ESEO 2017 - 2018
 *
 * @file actuator_tor.h
 * @brief Definition d'un actionneur TOR (Tout Ou Rien).
 * @author Valentin
 */
#ifndef ACTUATORS_ACTUATOR_TOR_H_
#define ACTUATORS_ACTUATOR_TOR_H_

#include "actuator.h"

/**
 * Definition de la structure de données TOR.
 */
typedef struct {
	GPIO_TypeDef* gpio;
	Uint16 pin;
} Actuator_tor_data_t;

/**
 * Definition d'un actionneur de type TOR.
 */
typedef struct {
	ABSTRACT_Actuator_t;
	Actuator_tor_data_t tor;
}Actuator_tor_t;


#endif /* ACTUATORS_ACTUATOR_TOR_H_ */
