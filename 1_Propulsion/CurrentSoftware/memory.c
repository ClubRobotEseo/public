/*
 * memory.c
 *
 *  Created on: 20 sept. 2018
 *      Author: a.guilmet
 */

#include "memory.h"
#include "QS/QS_sensor/QS_m95m02.h"
#include "QS/QS_outputlog.h"
#include "QS/QS_lowLayer/QS_ports.h"
#include "QS/QS_watchdog.h"

#define MEMORY_DATA_SYSTEM_CRC16_POLYNOMIAL 			0x8005
#define MEMORY_DATA_SYSTEM_CRC16_BASE_VALUE 			0xFFFF
#define MEMORY_NB_TRY_READ_CRC							3
#define MEMORY_CRC_SIZE									2
typedef Uint16 memory_crc_t;

typedef enum{
	MEMORY_COEF_ODO_TRANSLATION,
	MEMORY_COEF_ODO_ROTATION,
	MEMORY_COEF_ODO_SYMMETRY,
	MEMORY_COEF_ODO_CENTRIFUGE,
	MEMORY_ROBOT_DISTANCE,
	MEMORY_NB
}MEMORY_dataType_e;

#define IS_MEMORY_DATA_TYPE(x)		(											\
									(x) == MEMORY_COEF_ODO_TRANSLATION			\
									|| (x) == MEMORY_COEF_ODO_ROTATION			\
									|| (x) == MEMORY_COEF_ODO_SYMMETRY			\
									|| (x) == MEMORY_COEF_ODO_CENTRIFUGE		\
									|| (x) == MEMORY_ROBOT_DISTANCE				\
									)

typedef struct{
	M95M02_address_t address;
	M95M02_dataSize_t size;
}MEMORY_table_s;

static volatile M95M02_id_t idMemory;

// Il faut pensé que la taille d'une cellule mémoire est de size + CRC_SIZE octets
static volatile MEMORY_table_s memoryTable[MEMORY_NB] = {
		{0,		4},		// MEMORY_COEF_ODO_TRANSLATION
		{6,		4},		// MEMORY_COEF_ODO_ROTATION
		{12,	4},		// MEMORY_COEF_ODO_SYMMETRY
		{18,	4},		// MEMORY_COEF_ODO_CENTRIFUGE
		{24,	8},		// MEMORY_ROBOT_DISTANCE
};

static bool_e MEMORY_setData(MEMORY_dataType_e type, Uint8 * data);
static bool_e MEMORY_getData(MEMORY_dataType_e type, Uint8 * data);
static Uint16 MEMORY_computeCRC(Uint8 * data, M95M02_dataSize_t size);

bool_e MEMORY_init(void){
	M95M02_error_e error = M95M02_init();
	if(error != M95M02_ERROR_OK){
		error_printf("M95M02_init impossible erreur %d\n", error);
		return FALSE;
	}

	idMemory = M95M02_add(SPI2, GPIOB, GPIO_Pin_12, M95M02_WAIT_MODE__BEFORE_ACCESS, M95M02_OPTIMIZATION__SIZE);

	error = M95M02_configSPI(idMemory);
	if(error != M95M02_ERROR_OK){
		error_printf("M95M02_configSPI impossible erreur %d\n", error);
		return FALSE;
	}

	return TRUE;
}

bool_e MEMORY_getCoefOdometry(Sint32 * value, PROPULSION_coef_e coef){
	if(coef < ODOMETRY_COEF_TRANSLATION && coef > ODOMETRY_COEF_CENTRIFUGAL){
		error_printf("MEMORY_getCoefOdometry : Erreur coefficient incorrect\n");
		return FALSE;
	}

	MEMORY_dataType_e type = coef - ODOMETRY_COEF_TRANSLATION;

	return MEMORY_getData(type, (Uint8 *) value);
}

bool_e MEMORY_setCoefOdometry(Sint32 value, PROPULSION_coef_e coef){
	if(coef < ODOMETRY_COEF_TRANSLATION && coef > ODOMETRY_COEF_CENTRIFUGAL){
		error_printf("MEMORY_setCoefOdometry : Erreur coefficient incorrect\n");
		return FALSE;
	}

	MEMORY_dataType_e type = coef - ODOMETRY_COEF_TRANSLATION;

	return MEMORY_setData(type, (Uint8 *) &value);
}

bool_e MEMORY_getRobotDistance(Sint64 * value){
	return MEMORY_getData(MEMORY_ROBOT_DISTANCE, (Uint8 *) value);
}

bool_e MEMORY_setRobotDistance(Sint64 value){
	return MEMORY_setData(MEMORY_ROBOT_DISTANCE, (Uint8 *) &value);
}

/**
 * @fn static bool_e MEMORY_getData(MEMORY_dataType_e type, Uint8 * data)
 * @brief Fonction de lecture de données
 * @param type : type de la donnée à lire
 * @param data : Les données à lire
 * @return TRUE si la lecture a réussi, FALSE sinon
 */
static bool_e MEMORY_getData(MEMORY_dataType_e type, Uint8 * data){

	M95M02_address_t address;
	M95M02_dataSize_t size;
	memory_crc_t memoryCRC, computedCRC;
	M95M02_error_e error;

	if(IS_MEMORY_DATA_TYPE(type) == FALSE){
		error_printf("MEMORY_getData : Erreur type de données incorrect\n");
		return FALSE;
	}

	if(data == NULL){
		error_printf("MEMORY_getData : Erreur data null\n");
		return FALSE;
	}

	address = memoryTable[type].address;
	size = memoryTable[type].size;

	// Lecture avec vérification du CRC des données
	Uint8 nbTry = 0;
	bool_e successRead = FALSE;
	do{

		error = M95M02_read(idMemory, address, data, size);

		if(error != M95M02_ERROR_OK){
			error_printf("MEMORY_getData : Erreur (%d) impossible de lire les données\n", error);
			return FALSE;
		}

		error = M95M02_read(idMemory, address + size, (Uint8 *) &memoryCRC, MEMORY_CRC_SIZE);

		if(error != M95M02_ERROR_OK){
			error_printf("MEMORY_getData : Erreur (%d) impossible de lire le CRC des données\n", error);
			return FALSE;
		}

		computedCRC = MEMORY_computeCRC(data, size);

		if(computedCRC == memoryCRC){
			successRead = TRUE;
		}

		nbTry++;

	}while(nbTry < MEMORY_NB_TRY_READ_CRC && successRead == FALSE);

	if(successRead == FALSE){
		error_printf("MEMORY_getData : Erreur données corrompue\n");
		return FALSE;
	}

	return TRUE;
}

/**
 * @fn static bool_e MEMORY_setData(MEMORY_dataType_e type, Uint8 * data)
 * @brief Fonction de l'écriture de données
 * @param type : type de la donnée à écrire
 * @param data : Les données à écrire
 * @return TRUE si l'écriture a réussi, FALSE sinon
 */
static bool_e MEMORY_setData(MEMORY_dataType_e type, Uint8 * data){

	M95M02_address_t address;
	M95M02_dataSize_t size;
	memory_crc_t computedCRC;
	M95M02_error_e error;

	if(IS_MEMORY_DATA_TYPE(type) == FALSE){
		error_printf("MEMORY_setData : Erreur type de données incorrect\n");
		return FALSE;
	}

	if(data == NULL){
		error_printf("MEMORY_setData : Erreur data null\n");
		return FALSE;
	}

	address = memoryTable[type].address;
	size = memoryTable[type].size;

	error = M95M02_write(idMemory, address, data, size);

	if(error != M95M02_ERROR_OK){
		error_printf("MEMORY_setData : Erreur (%d) impossible d'écrire les données\n", error);
		return FALSE;
	}

	computedCRC = MEMORY_computeCRC(data, size);

	error = M95M02_write(idMemory, address + size, (Uint8 *) &computedCRC, MEMORY_CRC_SIZE);

	if(error != M95M02_ERROR_OK){
		error_printf("MEMORY_setData : Erreur (%d) impossible d'écrire le CRC des données\n", error);
		return FALSE;
	}

	return TRUE;
}

/**
 * @fn static memory_crc_t MEMORY_computeCRC(Uint8 * data, M95M02_dataSize_t size)
 * @brief Fonction de calcul du CRC16 pour un tableau de données fourni
 * @param data : Tableau de données
 * @param size : Taille du tableau
 * @return La valeur du CRC
 */
static memory_crc_t MEMORY_computeCRC(Uint8 * data, M95M02_dataSize_t size){
	memory_crc_t out = MEMORY_DATA_SYSTEM_CRC16_BASE_VALUE;
	int bits_read = 0, bit_flag;

	/* Sanity check: */
	if(data == NULL)
		return 0;

	while(size > 0)
	{
		bit_flag = out >> 15;

		/* Get next bit: */
		out <<= 1;
		out |= (*data >> (7 - bits_read)) & 1;

		/* Increment bit counter: */
		bits_read++;
		if(bits_read > 7)
		{
			bits_read = 0;
			data++;
			size--;
		}

		/* Cycle check: */
		if(bit_flag)
			out ^= MEMORY_DATA_SYSTEM_CRC16_POLYNOMIAL;

	}
	return out;
}
