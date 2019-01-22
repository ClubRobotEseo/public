/*
 *	Club Robot ESEO 2017 - 2018
 *
 *	@file : queue.c
 *	@brief : Gestion des files d'actions actionneurs
 *  @author : Valentin
 */

#include "queue.h"

#ifndef OUTPUT_LOG_COMPONENT_QUEUE
#  define OUTPUT_LOG_COMPONENT_QUEUE LOG_PRINT_Off
#  warning "OUTPUT_LOG_COMPONENT_QUEUE is not defined, defaulting to Off"
#endif

#define LOG_PREFIX "queue: "
#define LOG_COMPONENT OUTPUT_LOG_COMPONENT_QUEUE
#include "QS/QS_outputlog.h"
#include "QS/QS_CANmsgList.h"
#define component_printf_global(log_level, format, ...) component_printf(log_level, "[all] " format, ## __VA_ARGS__)
#define component_printf_queue(log_level, queueId, format, ...) component_printf(log_level, "[%d] " format, queueId, ## __VA_ARGS__)

// Incrémente l'index d'une queue
#define QUEUE_INC(index)	((index + 1) % QUEUE_SIZE)

/**
 * Structure queue_t qui comprend :
 * - les actions à effectuer
 * - les arguments pour les actions
 * - l'id de l'actionneur
 * - l'index de la tête
 * - l'index de la queue
 * - l'instant de démarrage de l'action courante
 * - l'état d'erreur : TRUE si une erreur est survenue lors de l'execution des fonctions dans la queue (erreur indiquée par QUEUE_set_error(queue_id))
 * - l'instance de l'actionneur associé à cette queue
 *
 * Info sur l'enum QUEUE_act_e:
 *  Cet enum est définie dans le fichier config_global_vars_types.h du robot compilé.
 *  Elle contient la liste des id des queues.
 *  @see Pour le gros robot, voir config/config_big/config_global_vars_types.h
 *  @see Pour le petit robot, voir config/config_big/config_global_vars_types.h
 */
typedef struct{
	action_t action[QUEUE_SIZE];
	QUEUE_arg_t arg[QUEUE_SIZE];
	QUEUE_act_e id;
	queue_size_t head;
	queue_size_t tail;
	time32_t initial_time_of_current_action;
	bool_e error_occured;
	void* actuator;
} queue_t;

// Déclaration des queues (une queue par actionneur)
static queue_t queues[NB_QUEUES];

/**
 * @brief Fonction d'intialisation des queues.
 */
void QUEUE_init()
{
	component_printf_global(LOG_LEVEL_Debug, "Initialisation\n");

	static bool_e initialized = FALSE;
	Uint8 i;

	if (initialized)
		return;

	//initialisation des files
	for (i = 0; i < NB_QUEUES; i++) {
		queues[i].id = i;
		queues[i].head = 0;
		queues[i].tail = 0;
		queues[i].error_occured = FALSE;
	}

	IT_init();
	component_printf_global(LOG_LEVEL_Debug, "Initialized\n");
	initialized = TRUE;
}

/**
 * @brief Fonction permettant d'associer l'instance d'un actionneur à une queue.
 * @param queue_id l'id de la queue.
 * @param actuator l'instance de l'actionneur.
 */
void QUEUE_set_actuator(queue_id_t queue_id, void* actuator)
{
	if(queue_id != NO_QUEUE_ID) {
		queues[queue_id].actuator = actuator;
	}
}

/**
 * @brief Fonction permettant d'obtenir l'instance actionneur d'une queue.
 * @param queue_id l'id de la queue.
 * @param actuator l'instance de l'actionneur.
 */
void * QUEUE_get_actuator(queue_id_t queue_id)
{
	assert(queue_id < NB_QUEUES);
	if(queue_id != NO_QUEUE_ID) {
		return queues[queue_id].actuator;
	}
	return NULL;
}

/**
 * @brief Fonction pour obtenir l'argument de l'action en tête de queue.
 * @param queue_id l'id de la queue.
 * @return l'argument de l'action en tête de queue.
 */
QUEUE_arg_t* QUEUE_get_arg(queue_id_t queue_id)
{
	assert(queue_id < NB_QUEUES);
	return &(queues[queue_id].arg[queues[queue_id].head]);
}

/**
 * @brief Fonction pour obtenir l'instant initial de demarrage de l'action courante.
 * @param queue_id l'id de la queue
 * @return l'instant initial de démarrage de l'action courante (en ms).
 */
time32_t QUEUE_get_initial_time(queue_id_t queue_id)
{
	assert(queue_id < NB_QUEUES);
	return queues[queue_id].initial_time_of_current_action;
}

/**
 * @brief Fonction pour savoir si une queue est vide.
 * @param queue_id l'id de la queue.
 * @return TRUE si la queue est vide et FALSE sinon.
 */
bool_e QUEUE_is_empty(queue_id_t queue_id)
{
	assert(queue_id < NB_QUEUES);
	return (queues[queue_id].head == queues[queue_id].tail);
}

/**
 * @brief Fonction pour savoir si une queue est pleine.
 * @param queue_id l'id de la queue.
 * @return TRUE si la queue est pleine et FALSE sinon.
 */
bool_e QUEUE_is_full(queue_id_t queue_id)
{
	assert(queue_id < NB_QUEUES);
	return (queues[queue_id].head == QUEUE_INC(queues[queue_id].tail));
}

/**
 * @brief Fonction de scrutation des queues qui doit tourner en tâche de fond (i.e. dans le main()).
 */
void QUEUE_run()
{
	queue_id_t queue_id;
	queue_t* queue;
	for (queue_id = 0; queue_id < NB_QUEUES; queue_id++) {
		//Pour ne pas planter si la file est vide
		if (!QUEUE_is_empty(queue_id)) {
			queue = &(queues[queue_id]);
			(queue->action[queue->head])(queue_id, queue->actuator, FALSE);
			component_printf_queue(LOG_LEVEL_Trace, queue_id, "Run\n");
		}
	}
}

/**
 * @brief Fonction permettant d'ajouter une action dans une queue.
 * @param queue_id l'id de la queue.
 * @param action l'action a ajouter dans la queue (pointeur de fonction de la fonction à executer).
 * @param l'argument associé à l'action (typiquement la position de l'actionneur).
 */
void QUEUE_add(queue_id_t queue_id, action_t action, QUEUE_arg_t arg)
{
	queue_t* queue = &(queues[queue_id]);
	//la file doit êre affectée
	assert(queue_id < NB_QUEUES);
	// la file ne doit pas etre pleine
	bool_e queue_is_full = QUEUE_is_full(queue_id);
	assert(!queue_is_full);

	component_printf_queue(LOG_LEVEL_Debug, queue_id, "Add\n");

	// on ajoute l'action à la file
	(queue->action[queue->tail]) = action;
	(queue->arg[queue->tail])= arg;

	//On doit le faire avant d'appeler l'action, sinon si l'action appelle une fonction de ce module, il peut y avoir des problèmes. (bug testé avec un appel à QUEUE_behead)
	queue->tail = QUEUE_INC(queue->tail);

	// si l'action est en tête de file
	if (queue->tail == QUEUE_INC(queue->head)) {
		//on l'initialise
		component_printf_queue(LOG_LEVEL_Debug, queue_id, "Init action\n");
		queue->initial_time_of_current_action = global.absolute_time;
		action(queue_id, queue->actuator, TRUE);
	}

}

/**
 * @brief Fonction qui retire une action de la queue afin de l'executer.
 * @param queue_id l'id de la queue.
 */
void QUEUE_behead(queue_id_t queue_id)
{
	assert(queue_id < NB_QUEUES);
	queue_t* queue = &(queues[queue_id]);
	component_printf_queue(LOG_LEVEL_Debug, queue_id, "Next\n");

	queue->head = QUEUE_INC(queue->head);

	if(!QUEUE_is_empty(queue_id)) {
		//on initialise l'action suivante
		component_printf_queue(LOG_LEVEL_Debug, queue_id, "Init action\n");
		queue->initial_time_of_current_action = global.absolute_time;
		(queue->action[queue->head])(queue_id, queue->actuator, TRUE);
		component_printf_queue(LOG_LEVEL_Debug, queue_id, "Queue empty\n");
	}
}

/**
 * @brief Fonction de qui appelle la callback de l'action courante et qui dépile la suivante.
 * @param queue_id l'id de la queue
 * @param error_code le code d'erreur de l'action courante.
 * @param param un paramètre quelconque passé à la fonction de callback.
 */
void QUEUE_next(queue_id_t queue_id, Uint8 result, Uint8 error_code, Uint16 param)
{
	assert(queue_id < NB_QUEUES);

	//Si il n'y a pas de fonction callback ou qu'elle retourne TRUE, on passe a l'action suivante. Sinon on indique une erreur dans la file
	if(QUEUE_get_arg(queue_id)->callback != NULL &&
	  !(QUEUE_get_arg(queue_id)->callback)(queue_id, result, error_code, param))
	{
		QUEUE_set_error(queue_id);
	}

	QUEUE_behead(queue_id);
}

/**
 * @brief Fonction qui indique qu'une erreur est survenue lors de l'execution d'une fonction dans une queue.
 *        Les fonctions suivantes pourront agir en conséquence.
 * @param queue_id l'id de la queue.
 */
void QUEUE_set_error(queue_id_t queue_id)
{
	assert(queue_id < NB_QUEUES);

	GPIO_SetBits(LED_ERROR);
	queues[queue_id].error_occured = TRUE;
	component_printf_queue(LOG_LEVEL_Warning, queue_id, "Error declared\n");
}

/**
 * @brief Fonction qui permet de récupérer l'état d'erreur d'une queue.
 * @param queue_id l'id de la queue.
 * @return TRUE si une erreur est survenue lors de l'execution d'une fonction dans la queue indiquée et FALSE sinon.
 */
bool_e QUEUE_has_error(queue_id_t queue_id)
{
	assert(queue_id < NB_QUEUES);
	return queues[queue_id].error_occured;
}

/**
 * @brief Fonction qui permet de vider toutes les actions d'une queue.
 * @param queue_id l'id de la queue.
 */
void QUEUE_flush(queue_id_t queue_id)
{
	assert(queue_id < NB_QUEUES);

	component_printf_queue(LOG_LEVEL_Debug, queue_id, " Queue reset\n");
	queues[queue_id].head = 0;
	queues[queue_id].tail = 0;
	queues[queue_id].error_occured = FALSE;
}

/**
 * @brief Fonction qui permet de vider toutes les queues.
 */
void QUEUE_flush_all()
{
	Uint8 i;
	component_printf_global(LOG_LEVEL_Info, "Reseting all queues\n");
	for (i=0; i < NB_QUEUES; i++) {
		QUEUE_flush(i);
	}
}
