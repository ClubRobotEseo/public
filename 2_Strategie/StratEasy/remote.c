/*
 * remote.c
 *
 *  Created on: 3 oct. 2018
 *      Author: Admin
 */
#include "../QS/QS_all.h"
#include "../QS/QS_stateMachineHelper.h"
#include "remote.h"
#include "config.h"
#include "stepper_motors.h"
#include "../QS/QS_lowLayer/QS_uart.h"
#include "actuators.h"

volatile static int32_t quantity = 50;
volatile static int32_t translation_mm = 0;
volatile static int32_t rotation_rad4096 = 0;



void REMOTE_read_uart(uint8_t c)
{
	int32_t previous_quantity;
	previous_quantity = quantity;
	int32_t arm;

	switch(c)
	{
		case '8':	//avance un poil
			translation_mm += quantity;
			break;
		case '2':	//recule un poil
			translation_mm -= quantity;
			break;
		case '4': 	//tourne sens trigo
			rotation_rad4096 += 10*quantity;
			break;
		case '6':	//tourne sens horaire
			rotation_rad4096 -= 10*quantity;
			break;
		case '+':	// + vite !
			if(quantity < 100)
				quantity+=10;
			break;
		case '-':	// - vite !
			if(quantity > 19)
				quantity-=10;
			break;
		case '*':
			quantity = 100;
			break;
		case '/':
			quantity = 10;
			break;
		case 'r':	//éteindre la pompe droite
			ACTUATOR_go(ACTUATOR_PUMP_RIGHT, 0, 0);
			ACTUATOR_go(ACTUATOR_SOLENOID_VALVE_RIGHT, 1, 0);
			break;
		case 'R':	//allume la pompe droite
			ACTUATOR_go(ACTUATOR_PUMP_RIGHT, 1, 0);
			break;
		case 'l':	//éteindre la pompe gauche
			ACTUATOR_go(ACTUATOR_PUMP_LEFT, 0, 0);
			ACTUATOR_go(ACTUATOR_SOLENOID_VALVE_LEFT, 1, 0);
			break;
		case 'L':	//allume la pompe gauche
			ACTUATOR_go(ACTUATOR_PUMP_LEFT, 1, 0);
			break;
		case 'a':
			arm = ACTUATOR_get_config(ACTUATOR_ARM_LEFT, POSITION_CONFIG);
			ACTUATOR_go(ACTUATOR_ARM_LEFT, ACT_STRATEASY_CUSTOM_POS_IN_PARAM, arm-10);
			break;
		case 'z':
			arm = ACTUATOR_get_config(ACTUATOR_ARM_LEFT, POSITION_CONFIG);
			ACTUATOR_go(ACTUATOR_ARM_LEFT, ACT_STRATEASY_CUSTOM_POS_IN_PARAM, arm+10);
			break;
		case 'o':
			arm = ACTUATOR_get_config(ACTUATOR_ARM_RIGHT, POSITION_CONFIG);
			ACTUATOR_go(ACTUATOR_ARM_RIGHT, ACT_STRATEASY_CUSTOM_POS_IN_PARAM, arm-10);
			break;
		case 'p':
			arm = ACTUATOR_get_config(ACTUATOR_ARM_RIGHT, POSITION_CONFIG);
			ACTUATOR_go(ACTUATOR_ARM_RIGHT, ACT_STRATEASY_CUSTOM_POS_IN_PARAM, arm+10);
			break;
		default:
			break;
	}
	if(previous_quantity != quantity)
		printf("quantity=%ld\n",quantity);
}

void REMOTE_state_machine(void)
{
	CREATE_MAE(INIT,
			WAIT_ORDER,
			MOVING);

	static order_t order;

	switch(state)
	{
		case INIT:
			state = WAIT_ORDER;

			break;
		case WAIT_ORDER:
			if(translation_mm)
			{
				order.forward = (translation_mm>=0)?TRUE:FALSE;
				order.nb_step = absolute(translation_mm)*NB_MICROSTEP_BY_MM;
				order.trajectory = TRAJECTORY_TRANSLATION;
				order.max_speed = SPEED_MAX/10;		//mm/sec
				state = MOVING;
				translation_mm = 0;
			}
			else if(rotation_rad4096)
			{
				order.forward = (rotation_rad4096>=0)?TRUE:FALSE;
				order.nb_step = (uint32_t)((double)(absolute(rotation_rad4096))*NB_MICROSTEP_BY_RAD4096);
				order.trajectory = TRAJECTORY_ROTATION;
				order.max_speed = SPEED_ROTATION_DEFAULT/4;
				state = MOVING;
				rotation_rad4096 = 0;
			}
			break;

		case MOVING:
			if(STEPPER_MOTOR_state_machine(&order))
			{
				printf("x=%04ld \t y=%04ld \t t=%05ld\n", global.pos.x, global.pos.y, global.pos.teta);
				state = WAIT_ORDER;
			}
			break;
		default:
			break;
	}
}


void REMOTE_process_main(void)
{
	REMOTE_state_machine();
}


