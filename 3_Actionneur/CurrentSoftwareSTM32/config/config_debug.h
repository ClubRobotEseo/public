/*
 *  Club Robot ESEO 2013 - 2014
 *
 *  $Id: config_debug.h 6160 2018-03-02 18:14:48Z vbesnard $
 *
 *  Package : Carte Actionneur
 *  Description : Configuration des aides au débuggage
 *  Auteur : Jacen, Alexis
 */

#ifndef CONFIG_DEBUG_H
#define CONFIG_DEBUG_H

#define OUTPUTLOG_DEFAULT_MAX_LOG_LEVEL			LOG_LEVEL_Debug		//Pour connaitre les valeurs possibles, voir output_log.h (enum log_level_e)
//#define OUTPUTLOG_PRINT_ALL_COMPONENTS							//Si défini, affiche les messages de tous les composants (OUTPUT_LOG_COMPONENT_* ne sont pas pris en compte, les niveau de débuggage le sont par contre)

//Activation des logs de sections précise			(LOG_PRINT_On / LOG_PRINT_Off)
#define OUTPUT_LOG_COMPONENT_CANPROCESSMSG			LOG_PRINT_On
#define OUTPUT_LOG_COMPONENT_ACTQUEUEUTILS			LOG_PRINT_On
#define OUTPUT_LOG_COMPONENT_QUEUE					LOG_PRINT_Off
#define OUTPUT_LOG_COMPONENT_SELFTEST				LOG_PRINT_On
#define OUTPUT_LOG_COMPONENT_MOSEFT					LOG_PRINT_On

// Actionneurs
#define OUTPUT_LOG_COMPONENT_IMPL_AX12_POS          LOG_PRINT_On
#define OUTPUT_LOG_COMPONENT_IMPL_AX12_POS_DOUBLE   LOG_PRINT_On
#define OUTPUT_LOG_COMPONENT_IMPL_AX12_SPEED        LOG_PRINT_On
#define OUTPUT_LOG_COMPONENT_IMPL_RX24_POS          LOG_PRINT_On
#define OUTPUT_LOG_COMPONENT_IMPL_RX24_POS_DOUBLE   LOG_PRINT_On
#define OUTPUT_LOG_COMPONENT_IMPL_RX24_SPEED        LOG_PRINT_On
#define OUTPUT_LOG_COMPONENT_IMPL_MOTOR_POS			LOG_PRINT_On
#define OUTPUT_LOG_COMPONENT_IMPL_MOTOR_SPEED       LOG_PRINT_On
#define OUTPUT_LOG_COMPONENT_IMPL_PWM               LOG_PRINT_On
#define OUTPUT_LOG_COMPONENT_IMPL_MOSFET			LOG_PRINT_On

//Activation de define pour l'affichage de debug
//#define AX12_DEBUG_PACKETS

#endif /* CONFIG_DEBUG_H */
