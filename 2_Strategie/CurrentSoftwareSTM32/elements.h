#ifndef ELEMENTS_H
#define ELEMENTS_H

#include "QS/QS_all.h"
#include "Supervision/SD/SD.h"
#include "propulsion/movement.h"


	#define NB_ATOMS_IN_SMALL_DISPENSOR		(3)
	#define NB_ATOMS_IN_LARGE_DISPENSOR		(6)
	#define NB_ATOMS_IN_SCALE				(6)
	#define NB_ATOMS_IN_ACCELERATOR			(10)

	typedef enum {
		NO_ELEMENT,
		OUR_ELEMENT,
		ADV_ELEMENT,
		NEUTRAL_ELEMENT
	} ELEMENTS_property_e;

	typedef enum {
		NO_SIDE,
		RIGHT,
		LEFT
	} ELEMENTS_side_e;

	typedef enum {
		OUR_SIDE,
		ADV_SIDE
	} ELEMENTS_side_match_e;

	typedef enum {
		NO_ATOM,
		REDIUM,
		GREENIUM,
		BLUEIUM,
		GOLDENIUM,
		UNKNOWN_ATOM
	} ATOM_kind_e;

	typedef struct  {
		ATOM_kind_e kind;
		bool_e available;
	}ATOM_e;

	typedef struct {
		ATOM_kind_e atoms[NB_ATOMS_IN_ACCELERATOR];
		Uint8 nb;
	} ACCELERATOR_e;

	typedef struct {
		ATOM_kind_e atoms[NB_ATOMS_IN_SCALE];
		Uint8 nb;
	} SCALE_e;

	typedef enum {
		ATOM_POS_ELEVATOR_FRONT_LEFT,
		ATOM_POS_ELEVATOR_FRONT_MIDDLE,
		ATOM_POS_ELEVATOR_FRONT_RIGHT,
		ATOM_POS_SORTING_BACK_VERY_LEFT,
		ATOM_POS_SORTING_BACK_LEFT,
		ATOM_POS_SORTING_BACK_RIGHT,
		ATOM_POS_SORTING_BACK_VERY_RIGHT,
		ATOM_POS_CHAOS_LEFT,
		ATOM_POS_CHAOS_MIDDLE,
		ATOM_POS_CHAOS_RIGHT,
		ATOM_POS_CHAOS_BACK,
		ATOM_POS_SLOPE_TAKER,
		ATOM_POS_LOCKER_BACK,
		ATOM_NB_POS
	} ATOM_POS_id_e;

	typedef enum {
		ATOM_NO_CONFIG,					// pas de config (peut être utilisé en cas d'absence de config ou de config invalide)
		ATOM_CONFIG_ELEVATOR_FRONT,		// on prend avec l'actionneur ELEVATOR_FRONT
		ATOM_CONFIG_SORTING_BACK_RIGHT,	// on prend avec les actionneurs SORTING_BACK_VERY_RIGHT, SORTING_BACK_RIGHT, et SORTING_BACK_VERY_LEFT
		ATOM_CONFIG_SORTING_BACK_LEFT,	// on prend avec les actionneurs SORTING_BACK_VERY_RIGHT, SORTING_BACK_LEFT, et SORTING_BACK_VERY_LEFT
		ATOM_NB_CONFIG
	} ATOM_CONFIG_kind_e;

	typedef enum {
		DISPENSOR_SMALL,
		DISPENSOR_LARGE_NEXT_TO_SCALE,
		DISPENSOR_LARGE_NEXT_TO_START_ZONE
	} DISPENSOR_id_e;

	typedef enum {
		ATOM_RELEASE_AREA_GROUND_UNKNOWN,  // Lorsque qu'on ne sait pas si l'atome a été déposé dans la bonne case.
		ATOM_RELEASE_AREA_GROUND_WELL_PLACED,
		ATOM_RELEASE_AREA_GROUND_NOT_WELL_PLACED,
		ATOM_RELEASE_AREA_SCALE,
		ATOM_RELEASE_AREA_ACCELERATOR
	} ATOM_RELEASE_AREA_kind_e;

	typedef enum {
		PUMP_STATUS_LEFT,
		PUMP_STATUS_MIDDLE,
		PUMP_STATUS_RIGHT
	} PUMP_STATUS_id_e;





// Fonctions pour la gestion des flags
void ELEMENTS_init();
void ELEMENTS_process_main();
bool_e ELEMENTS_get_flag(elements_flags_e flag_id);
void ELEMENTS_set_flag(elements_flags_e flag_id, bool_e new_state);
void ELEMENTS_set_flag_without_synchro(elements_flags_e flag_id, bool_e new_state);

// Fonctions pour la synchronisation
error_e ELEMENTS_check_communication(CAN_msg_t * msg);
#if USE_SYNC_ELEMENTS
void ELEMENTS_receive_flags(CAN_msg_t* msg);
void ELEMENTS_update_info(elements_flags_e flagId);
#endif

#if USE_HARDFLAGS
	// Fonctions pour la gestion des hardflags
	void ELEMENTS_receive_hardflags(CAN_msg_t * msg);
#endif

//################################## ELEMENTS OF THE YEAR #################################

/**
 * Take atoms into a dispensor.
 * @param dispensor_id the id of the dispensor.
 * @param property the property of the dispensor (OUR_ELEMENT or ADV_ELEMENT).
 * @param config the configuration used to tkae this dispensor.
 * @param status an array of pumps status used to take this dispensor (maybe some atoms have not been taken).
 * 	This array can be filled using ids defined by PUMP_STATUS_id_e.
 * 		PUMP_STATUS_LEFT: the pump at the left of the robot (e.g., ACT_EXPANDER_PUMP_BIG_BACKWARD_VERY_LEFT, ACT_EXPANDER_PUMP_BIG_FORWARD_LEFT)
 * 		PUMP_STATUS_CENTER: the pump at the middle of the robot.
 * 		PUMP_STATUS_RIGHT: the pump at the right of the robot (e.g., ACT_EXPANDER_PUMP_BIG_BACKWARD_VERY_RIGHT, ACT_EXPANDER_PUMP_BIG_FORWARD_RIGHT)
 */
void ATOM_take_dispensor(DISPENSOR_id_e dispensor_id, ELEMENTS_property_e property, ATOM_CONFIG_kind_e config, bool_e status[]);

/**
 * Take an atom in the robot.
 * @param pos_id the position of the atom.
 * @param atom the atom taken.
 */
void ATOM_take(ATOM_POS_id_e pos_id, ATOM_kind_e atom);

/**
 * Release an atom out of the robot.
 * @param pos_id the position of the atom.
 * @param atom the atom taken.
 */
void ATOM_release(ATOM_POS_id_e pos_id, ATOM_RELEASE_AREA_kind_e release_area);

/**
 * Check if a position is available.
 * @param pos_id the position to check.
 * @return TRUE if this position is available and FALSE otherwise.
 */
bool_e ATOM_is_position_available(ATOM_POS_id_e pos_id);

/**
 * Getter of an atom at a position in the robot.
 * @param pos_id the position to check.
 * @return the atome at the given position or NO_ATOM is empty.
 */
ATOM_kind_e ATOM_get_atom_at_position(ATOM_POS_id_e pos_id);

///**
// * Getter of the number of atoms in the robot.
// * @return the number of atoms in the robot.
// */
//Uint8 ATOM_get_nb_atoms_in_robot();

/**
 * Check if all positions of a config are available.
 * @param config the config to check.
 * @return TRUE if this config is available and FALSE otherwise.
 */
bool_e ATOM_is_config_available(ATOM_CONFIG_kind_e config);

/**
 * Check if a dispensor is empty.
 * @param dispensor the dispensor to check.
 * @param property the property of the dispensor.
 * @return TRUE if this dispensor is empty and FALSE otherwise.
 */
bool_e ATOM_is_dispensor_empty(DISPENSOR_id_e dispensor_id, ELEMENTS_property_e property);

/**
 * Getter of number of atoms in accelerator.
 * @return the number of atoms in accelerator.
 */
Uint8 ATOM_get_nb_in_accelerator();

/**
 * Print an atom.
 * @param atom the atom to print.
 */
char * ATOM_print(ATOM_kind_e atom);

/**
 * Print atoms currently in the robot.
 */
void ATOM_print_robot();

/**
 * Print atoms currently in a dispensor.
 * @param property the property of dispensors to print.
 */
void ATOM_print_dispensor(ELEMENTS_property_e property);

#endif // ELEMENTS_H
