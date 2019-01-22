/**
 *	Club Robot ESEO 2017 - 2018
 *
 *	@file : queue.h
 *	@brief : Gestion des files d'actions actionneurs
 *  @author : Valentin
 */

#include "QS/QS_all.h"

#ifndef QUEUE_H
#define QUEUE_H

#include "it.h"

/**
 * Definition de type de la taille d'une queue.
 */
typedef Uint8 queue_size_t;

/**
 * Definition du type de l'id d'une queue.
 */
typedef Uint8 queue_id_t;

/**
 * Definition du type action_t, le pointeur de fonction correspondant � l'action a executer.
 */
typedef void(*action_t)(queue_id_t queue_id, void* actuator, bool_e init);

/**
 * Definition du type OperationFinishedCallback, le pointeur de fonction correspondant � la fonction
 * de callback devant etre executee lorsqu'une action est termin�e.
 */
typedef bool_e (*OperationFinishedCallback)(queue_id_t queue_id, Uint8 result, Uint8 error_code, Uint16 param);

/**
 * Definition de la structure QUEUE_arg_t qui comprend :
 * - la commande � effectuer
 * - un param�tre de l'action � effectuer (position du servo, vitesse etc...)
 * - la fonction de callback (fonction appel�e lorsque l'action est termin�e, peut �tre NULL)
 * - un booleen pour savoir si l'ordre doit s'arreter lors de la reception de l'ordre suivant.
 */
typedef struct {
	ACT_order_e command;
	Sint16 param;
	OperationFinishedCallback callback;
	bool_e flush_on_next_order;
} QUEUE_arg_t;

/**
 * Taille des queues.
 */
#define QUEUE_SIZE	(16)

/**
 * Valeur par d�faut de l'id d'une queue non affect� (par exemple pour un actionneur sans queue).
 */
#define NO_QUEUE_ID	(0xFF)

/**
 * Nombre de queues.
 */
#define NB_QUEUES (MAX(NB_QUEUES_BIG, NB_QUEUES_SMALL))

/**
 * @brief Fonction d'intialisation des queues.
 */
void QUEUE_init();

/**
 * @brief Fonction permettant d'associer l'instance d'un actionneur � une queue.
 * @param queue_id l'id de la queue.
 * @param actuator l'instance de l'actionneur.
 */
void QUEUE_set_actuator(queue_id_t queue_id, void* actuator);

/**
 * @brief Fonction permettant d'obtenir l'instance actionneur d'une queue.
 * @param queue_id l'id de la queue.
 * @param actuator l'instance de l'actionneur.
 */
void * QUEUE_get_actuator(queue_id_t queue_id);

/**
 * @brief Fonction pour obtenir l'argument de l'action en t�te de queue.
 * @param queue_id l'id de la queue.
 * @return l'argument de l'action en t�te de queue.
 */
QUEUE_arg_t* QUEUE_get_arg(queue_id_t queue_id);

/**
 * @brief Fonction pour obtenir l'instant initial de demarrage de l'action courante.
 * @param queue_id l'id de la queue
 * @return l'instant initial de d�marrage de l'action courante (en ms).
 */
time32_t QUEUE_get_initial_time(queue_id_t queue_id);

/**
 * @brief Fonction pour savoir si une queue est vide.
 * @param queue_id l'id de la queue.
 * @return TRUE si la queue est vide et FALSE sinon.
 */
bool_e QUEUE_is_empty(queue_id_t queue_id);

/**
 * @brief Fonction pour savoir si une queue est pleine.
 * @param queue_id l'id de la queue.
 * @return TRUE si la queue est pleine et FALSE sinon.
 */
bool_e QUEUE_is_full(queue_id_t queue_id);

/**
 * @brief Fonction de scrutation des queues qui doit tourner en t�che de fond (i.e. dans le main()).
 */
void QUEUE_run();

/**
 * @brief Fonction permettant d'ajouter une action dans une queue.
 * @param queue_id l'id de la queue.
 * @param action l'action a ajouter dans la queue (pointeur de fonction de la fonction � executer).
 * @param l'argument associ� � l'action (typiquement la position de l'actionneur).
 */
void QUEUE_add(queue_id_t queue_id, action_t action, QUEUE_arg_t arg);

/**
 * @brief Fonction qui retire une action de la queue afin de l'executer.
 * @param queue_id l'id de la queue.
 */
void QUEUE_behead(queue_id_t queue_id);

/**
 * @brief Fonction de qui appelle la callback de l'action courante et qui d�pile la suivante.
 * @param queue_id l'id de la queue
 * @param error_code le code d'erreur de l'action courante.
 * @param param un param�tre quelconque pass� � la fonction de callback.
 */
void QUEUE_next(queue_id_t queue_id, Uint8 result, Uint8 error_code, Uint16 param);

/**
 * @brief Fonction qui indique qu'une erreur est survenue lors de l'execution d'une fonction dans une queue.
 *        Les fonctions suivantes pourront agir en cons�quence.
 * @param queue_id l'id de la queue.
 */
void QUEUE_set_error(queue_id_t queue_id);

/**
 * @brief Fonction qui permet de r�cup�rer l'�tat d'erreur d'une queue.
 * @param queue_id l'id de la queue.
 * @return TRUE si une erreur est survenue lors de l'execution d'une fonction dans la queue indiqu�e et FALSE sinon.
 */
bool_e QUEUE_has_error(queue_id_t queue_id);

/**
 * @brief Fonction qui permet de vider toutes les actions d'une queue.
 * @param queue_id l'id de la queue.
 */
void QUEUE_flush(queue_id_t queue_id);

/**
 * @brief Fonction qui permet de vider toutes les queues.
 */
void QUEUE_flush_all();



#endif /* ndef QUEUE_H */
