#include <stdarg.h>
#include "term_io.h"

#include "../it.h"
#include "../QS/QS_lowLayer/QS_uart.h"
#include "../QS/QS_actuator/QS_ax12.h"
#include "../QS/QS_actuator/QS_rx24.h"
#include "../QS/QS_actuator/QS_DCMotor2.h"
#include "../QS/QS_outputlog.h"
#include "../QS/QS_lowLayer/QS_pwm.h"
#include "../QS/QS_lowLayer/QS_ports.h"
#include "../QS/QS_who_am_i.h"
#include "../actuators/actuators_big.h"
#include "../actuators/actuators_small.h"

typedef Sint16(*sensor_position_fun_t)(void);

typedef enum{
	term_AX12,
	term_RX24,
	term_MOTOR,
	term_PWM
}terminal_type;

typedef struct{
	terminal_type type;
	Uint8 inc_quantum;
	char char_selection;
	char name[35];
	Uint8 id;
	Sint16 min_value;
	Sint16 max_value;
	Uint8 sens;
	sensor_position_fun_t fun;
	GPIO_TypeDef* GPIOx_sens;
	uint16_t GPIO_Pin_sens;
}terminal_motor_s;

/*
 * inc      -> nombre de degrés par incrémentation dans le terminal
 * char		-> le caractère que l'on doit envoyer pour sélectionner l'actionneur
 * prefix   -> le nom de l'actionneur
 * fun      -> fonction qui retourne la position du bras
 */
#define DECLARE_RX24(inc, char, prefix) 			terminal_motor[i++] = (terminal_motor_s){term_RX24, inc, char, #prefix, prefix##_RX24_ID, 0, 1024, 0, NULL, NULL, 0}
#define DECLARE_AX12(inc, char, prefix) 			terminal_motor[i++] = (terminal_motor_s){term_AX12, inc, char, #prefix, prefix##_AX12_ID, 0, 1024, 0, NULL, NULL, 0}
#define DECLARE_MOTOR(inc, char, prefix, fun) 		terminal_motor[i++] = (terminal_motor_s){term_MOTOR, inc, char, #prefix, prefix##_ID, prefix##_MIN_VALUE, prefix##_MAX_VALUE, 0, fun, NULL, 0}
#define DECLARE_PWM(inc, char, prefix) 				terminal_motor[i++] = (terminal_motor_s){term_PWM, inc, char, #prefix, prefix##_PWM_NUM, 0, 100, 0, NULL, prefix##_SENS}

terminal_motor_s terminal_motor[NB_ACTUATORS];

Uint8 terminal_motor_size = sizeof(terminal_motor)/sizeof(terminal_motor_s);

#define EGAL_CHAR_INC(c)			(c == 'p' || c == 'P' || c == '+')
#define EGAL_CHAR_DEC(c)			(c == 'm' || c == 'M'|| c == '-')
#define EGAL_CHAR_ACTIVE(c)			(c == 'r')
#define EGAL_CHAR_PRINT(c)			(c == ' ')
#define EGAL_CHAR_HELP(c)			(c == 'h' || c == 'H')

void TERMINAL_init(){
	Uint16 i = 0;
	UNUSED_VAR(i);
	PWM_init();
	AX12_init();
	RX24_init();

	if(I_AM_BIG()) {

		// Initialisation des moteurs de BIG_ROBOT
        //DECLARE_RX24(2, 'A', EXEMPLE);
		DECLARE_RX24(2, 'A', ACT_BIG_ELEVATOR_FRONT_RIGHT);
		DECLARE_RX24(2, 'B', ACT_BIG_ELEVATOR_FRONT_MIDDLE);
		DECLARE_RX24(2, 'C', ACT_BIG_ELEVATOR_FRONT_LEFT);
		DECLARE_RX24(2, 'D', ACT_BIG_LOCKER_FRONT_RIGHT);
		DECLARE_RX24(2, 'E', ACT_BIG_LOCKER_FRONT_MIDDLE);
		DECLARE_RX24(2, 'F', ACT_BIG_LOCKER_FRONT_LEFT);
		DECLARE_RX24(2, 'G', ACT_BIG_LOCKER_BACK);
		// Could not use H
		DECLARE_RX24(2, 'I', ACT_BIG_SLOPE_TAKER_BACK);
		DECLARE_RX24(2, 'J', ACT_BIG_SORTING_BACK_VERY_RIGHT);
		DECLARE_RX24(2, 'K', ACT_BIG_SORTING_BACK_RIGHT);
		DECLARE_RX24(2, 'L', ACT_BIG_SORTING_BACK_LEFT);
		// Could not use M
		DECLARE_RX24(2, 'N', ACT_BIG_SORTING_BACK_VERY_LEFT);
		// Could not use P

	} else {

		// Initialisation des moteurs de SMALL_ROBOT
		//DECLARE_RX24(2, 'a', EXEMPLE);
		DECLARE_RX24(2, 'a', ACT_SMALL_LOCKER_BACK);
		DECLARE_RX24(2, 'b', ACT_SMALL_LOCKER_FRONT_LEFT);
		DECLARE_RX24(2, 'c', ACT_SMALL_LOCKER_FRONT_RIGHT);
		DECLARE_RX24(2, 'd', ACT_SMALL_SORTING_BACK_LEFT);
		DECLARE_RX24(2, 'e', ACT_SMALL_SORTING_BACK_RIGHT);
		DECLARE_RX24(2, 'f', ACT_SMALL_ELEVATOR_FRONT_LEFT);
		DECLARE_RX24(2, 'g', ACT_SMALL_ELEVATOR_FRONT_RIGHT);
		// Could not use h

		// Could not use m
		// Could not use p

	}
}

void TERMINAL_uart_checker(unsigned char c){

	static Uint8 state = -1;
	static Uint16 position = 512;
	Uint8 i;

	for(i=0;i<terminal_motor_size;i++){
		if(terminal_motor[i].char_selection == c && state != i){
			debug_printf("%s selected\n", terminal_motor[i].name);
			state = i;
			if(terminal_motor[i].type == term_AX12)
				position = AX12_get_position(terminal_motor[i].id);
			else if(terminal_motor[i].type == term_RX24)
				position = RX24_get_position(terminal_motor[i].id);
			else if(terminal_motor[i].type == term_MOTOR)
				position = terminal_motor[i].fun();
			else if(terminal_motor[i].type == term_PWM){
				position = PWM_get_duty(PWM_TIM_8, terminal_motor[i].id);
				terminal_motor[i].sens = GPIO_ReadOutputDataBit(terminal_motor[i].GPIOx_sens, terminal_motor[i].GPIO_Pin_sens);
			}else
				debug_printf("Erreur ce type de terminal n'existe pas\n");
		}
	}

	if(EGAL_CHAR_PRINT(c)){
		debug_printf("---------------------- Position ----------------------\n");
		for(i=0;i<terminal_motor_size;i++){
			if(terminal_motor[i].type == term_AX12)
				debug_printf("%s : position : %3d    couple : %3d   temperature : %3d\n", terminal_motor[i].name, AX12_get_position(terminal_motor[i].id), AX12_get_load_percentage(terminal_motor[i].id), AX12_get_temperature(terminal_motor[i].id));
			else if(terminal_motor[i].type == term_RX24)
				debug_printf("%s : position : %3d    couple : %3d   temperature : %3d\n", terminal_motor[i].name, RX24_get_position(terminal_motor[i].id), RX24_get_load_percentage(terminal_motor[i].id), RX24_get_temperature(terminal_motor[i].id));
			else if(terminal_motor[i].type == term_MOTOR)
				debug_printf("%s : %d /  real : %d\n" , terminal_motor[i].name, terminal_motor[i].fun(), 0 /*conv_potar_updown_to_dist(terminal_motor[i].fun())*/);
			else if(terminal_motor[i].type == term_PWM)
				debug_printf("%s : %d %s\n" , terminal_motor[i].name, PWM_get_duty(PWM_TIM_8, terminal_motor[i].id), (terminal_motor[i].sens)?"NORMAL":"REVERSE");
			else
				debug_printf("Erreur ce type de terminal n'existe pas\n");
		}
	}

	if(EGAL_CHAR_HELP(c)){
		debug_printf("---------------------- Terminal ----------------------\n");
		debug_printf("Actionneur : \n");
		for(i=0;i<terminal_motor_size;i++){
			if(terminal_motor[i].type == term_AX12)
				debug_printf("(%s) cmd : %c   ID : %d   NAME : %s\n", AX12_is_ready(terminal_motor[i].id) ? "  Connecté" : "Déconnecté", terminal_motor[i].char_selection, terminal_motor[i].id, terminal_motor[i].name);
			else if(terminal_motor[i].type == term_RX24)
				debug_printf("(%s) cmd : %c   ID : %d   NAME : %s\n", RX24_is_ready(terminal_motor[i].id) ? "  Connecté" : "Déconnecté", terminal_motor[i].char_selection, terminal_motor[i].id, terminal_motor[i].name);
			else if(terminal_motor[i].type == term_MOTOR)
				debug_printf("(%s) cmd : %c   ID : %d   NAME : %s\n", "----------", terminal_motor[i].char_selection, terminal_motor[i].id, terminal_motor[i].name);
			else if(terminal_motor[i].type == term_PWM)
				debug_printf("(%s) cmd : %c   PWM : %d   NAME : %s\n", "----------", terminal_motor[i].char_selection, terminal_motor[i].id, terminal_motor[i].name);
			else
				debug_printf("Erreur ce type de terminal n'existe pas\n");
		}
		debug_printf("\nCommande : \n");
		debug_printf("p/+ incrémenter\nm/- décrémenter\nr inverser sens PWM\nESPACE affichage position\nh affichage aide\n");
	}

	if(state == -1)
		return;

	if(EGAL_CHAR_ACTIVE(c)){
		toggle_led_2(terminal_motor[state].GPIOx_sens, terminal_motor[state].GPIO_Pin_sens);
		terminal_motor[state].sens = !terminal_motor[state].sens;
	}

	if(!EGAL_CHAR_INC(c) && !EGAL_CHAR_DEC(c))
		return;

	if(EGAL_CHAR_INC(c)){
		position = ((Sint16)position+terminal_motor[state].inc_quantum >= terminal_motor[state].max_value) ? position : position+terminal_motor[state].inc_quantum;
	}else if(EGAL_CHAR_DEC(c)){
		position = ((Sint16)position-terminal_motor[state].inc_quantum <= terminal_motor[state].min_value) ? position : position-terminal_motor[state].inc_quantum;
	}

	if(terminal_motor[state].type == term_AX12)
		AX12_set_position(terminal_motor[state].id, position);
	else if(terminal_motor[state].type == term_RX24)
		RX24_set_position(terminal_motor[state].id, position);
	else if(terminal_motor[state].type == term_PWM)
		PWM_run(position, PWM_TIM_8, terminal_motor[state].id);
#ifdef USE_DCMOTOR2
	else if(terminal_motor[state].type == term_MOTOR){
		DCM_setPosValue(terminal_motor[state].id, 0, position);
		DCM_goToPos(terminal_motor[state].id, 0);
		DCM_restart(terminal_motor[state].id);
	}
#endif
	else
		debug_printf("Erreur ce type de terminal n'existe pas\n");
}
