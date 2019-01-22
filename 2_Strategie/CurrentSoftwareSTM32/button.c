/*
 *	Club Robot ESEO 2008 - 2010
 *	Archi'Tech, PACMAN
 *
 *	Fichier : button.c
 *	Package : Carte Principale
 *	Description : 	Fonctions de gestion du bouton et de la biroute
 *	Auteur : Jacen & Ronan
 *	Version 20100408
 */

#define BUTTON_C

#include "button.h"
#include "QS/QS_buttons.h"
#include "QS/QS_outputlog.h"
#include "QS/QS_IHM.h"
#include "QS/QS_who_am_i.h"
#include "QS/QS_lowLayer/QS_timer.h"
#include "QS/QS_lowLayer/QS_adc.h"
#include "actuator/act_functions.h"
#include "elements.h"
#include "main.h"
#include "Supervision/Selftest.h"
#include "Supervision/LCD_interface.h"
#include "Supervision/SD/SD.h"
#include "propulsion/movement.h"
#include "propulsion/prop_functions.h"

void BUTTON_verbose(void);
static void BUTTON_suspend_match();
static void BUTTON_0_long_push(void);
static void BUTTON_1_long_push(void);
static void BUTTON_2_long_push(void);
static void BUTTON_3_long_push(void);
static void BUTTON_4_long_push(void);
static void BUTTON_5_long_push(void);

void BUTTON_init()
{
	ADC_init();
	BUTTONS_init();
	BUTTONS_define_actions(BUTTON0,BUTTON_start, NULL, 1);
	IHM_define_act_button(BP_0_IHM,NULL, BUTTON_0_long_push);
	IHM_define_act_button(BP_1_IHM,NULL, BUTTON_1_long_push);
	IHM_define_act_button(BP_2_IHM,NULL, BUTTON_2_long_push);
	IHM_define_act_button(BP_3_IHM,NULL, BUTTON_3_long_push);
	IHM_define_act_button(BP_4_IHM,NULL, BUTTON_4_long_push);
	IHM_define_act_button(BP_5_IHM,NULL, BUTTON_5_long_push);
	IHM_define_act_button(BP_SELFTEST,SELFTEST_ask_launch, NULL);
	IHM_define_act_button(BP_PRINTMATCH,SD_print_previous_match, NULL);
	IHM_define_act_button(BP_SUSPEND_RESUME_MATCH, BUTTON_suspend_match, NULL);
}


void BUTTON_update()
{
	BUTTONS_update();
	BUTTON_verbose();
}

void BUTTON_start()
{
	debug_printf("START\r\n");
	global.flags.ask_start = TRUE;
}



static void BUTTON_suspend_match(){
	if(global.flags.match_started){
		if(!global.flags.match_suspended)
			global.flags.ask_suspend_match = TRUE;
	}
}


/*
void BUTTON_go_to_home(void){
	debug_printf("Flag Boutton go to home activé\n");
	global.flags.go_to_home = TRUE;
}
*/



static void BUTTON_0_long_push(void){
	if(I_AM_BIG()){
		static Uint8 state_front = 0;
		static Uint8 state_back = 0;

		if(IHM_switchs_get(SWITCH_BUTTONS_ACT)) {
			if(state_front == 0) {
				ACT_EXPANDER_setPump(ACT_EXPANDER_PUMP_BIG_BACKWARD_RIGHT, TRUE);
				ACT_EXPANDER_setSolenoidValve(ACT_EXPANDER_PUMP_BIG_BACKWARD_RIGHT, FALSE);
			} else if(state_front == 1) {
				ACT_EXPANDER_setPump(ACT_EXPANDER_PUMP_BIG_BACKWARD_RIGHT, FALSE);
				ACT_EXPANDER_setSolenoidValve(ACT_EXPANDER_PUMP_BIG_BACKWARD_RIGHT, TRUE);
			}
			state_front = (state_front == 1) ? 0 : state_front + 1;
		} else {
			if(state_back == 0) {
				ACT_EXPANDER_setPump(ACT_EXPANDER_PUMP_BIG_FORWARD_RIGHT, TRUE);
				ACT_EXPANDER_setSolenoidValve(ACT_EXPANDER_PUMP_BIG_FORWARD_RIGHT, FALSE);
			} else if(state_back == 1) {
				ACT_EXPANDER_setPump(ACT_EXPANDER_PUMP_BIG_FORWARD_RIGHT, FALSE);
				ACT_EXPANDER_setSolenoidValve(ACT_EXPANDER_PUMP_BIG_FORWARD_RIGHT, TRUE);
			}
			state_back = (state_back == 1)? 0 : state_back + 1;
		}
	} else {

	}
}


static void BUTTON_1_long_push(void){
	if(I_AM_BIG()){
		static Uint8 state_front = 0;
		static Uint8 state_back = 0;

		if(IHM_switchs_get(SWITCH_BUTTONS_ACT)) {
			if(state_front == 0) {
				ACT_EXPANDER_setPump(ACT_EXPANDER_PUMP_BIG_BACKWARD_VERY_RIGHT, TRUE);
				ACT_EXPANDER_setSolenoidValve(ACT_EXPANDER_PUMP_BIG_BACKWARD_VERY_RIGHT, FALSE);
			} else if(state_front == 1) {
				ACT_EXPANDER_setPump(ACT_EXPANDER_PUMP_BIG_BACKWARD_VERY_RIGHT, FALSE);
				ACT_EXPANDER_setSolenoidValve(ACT_EXPANDER_PUMP_BIG_BACKWARD_VERY_RIGHT, TRUE);
			}
			state_front = (state_front == 1) ? 0 : state_front + 1;
		} else {
			if(state_back == 0) {
				ACT_EXPANDER_setPump(ACT_EXPANDER_PUMP_BIG_FORWARD_MIDDLE, TRUE);
				ACT_EXPANDER_setSolenoidValve(ACT_EXPANDER_PUMP_BIG_FORWARD_MIDDLE, FALSE);
			} else if(state_back == 1) {
				ACT_EXPANDER_setPump(ACT_EXPANDER_PUMP_BIG_FORWARD_MIDDLE, FALSE);
				ACT_EXPANDER_setSolenoidValve(ACT_EXPANDER_PUMP_BIG_FORWARD_MIDDLE, TRUE);
			}
			state_back = (state_back == 1) ? 0 : state_back + 1;
		}
	} else {

	}
}

static void BUTTON_2_long_push(void) {
	if(I_AM_BIG()){

	} else {

	}
}


static void BUTTON_3_long_push(void){
	if(I_AM_BIG()){
		static Uint8 state_front = 0;
		static Uint8 state_back = 0;

		if(IHM_switchs_get(SWITCH_BUTTONS_ACT)) {
			if(state_front == 0) {
				ACT_EXPANDER_setPump(ACT_EXPANDER_PUMP_BIG_BACKWARD_LEFT, TRUE);
				ACT_EXPANDER_setSolenoidValve(ACT_EXPANDER_PUMP_BIG_BACKWARD_LEFT, FALSE);
			} else if(state_front == 1) {
				ACT_EXPANDER_setPump(ACT_EXPANDER_PUMP_BIG_BACKWARD_LEFT, FALSE);
				ACT_EXPANDER_setSolenoidValve(ACT_EXPANDER_PUMP_BIG_BACKWARD_LEFT, TRUE);
			}
			state_front = (state_front == 1) ? 0 : state_front + 1;
		} else {
			if(state_back == 0) {
				ACT_EXPANDER_setPump(ACT_EXPANDER_PUMP_BIG_FORWARD_LEFT, TRUE);
				ACT_EXPANDER_setSolenoidValve(ACT_EXPANDER_PUMP_BIG_FORWARD_LEFT, FALSE);
			} else if(state_back == 1) {
				ACT_EXPANDER_setPump(ACT_EXPANDER_PUMP_BIG_FORWARD_LEFT, FALSE);
				ACT_EXPANDER_setSolenoidValve(ACT_EXPANDER_PUMP_BIG_FORWARD_LEFT, TRUE);
			}
			state_back = (state_back == 1)? 0 : state_back + 1;
		}
	} else {

	}
}


static void BUTTON_4_long_push(void) {
	if(I_AM_BIG()){
		static Uint8 state_front = 0;
		//static Uint8 state_back = 0;

		if(IHM_switchs_get(SWITCH_BUTTONS_ACT)) {
			if(state_front == 0) {
				ACT_EXPANDER_setPump(ACT_EXPANDER_PUMP_BIG_BACKWARD_VERY_LEFT, TRUE);
				ACT_EXPANDER_setSolenoidValve(ACT_EXPANDER_PUMP_BIG_BACKWARD_LEFT, FALSE);
			} else if(state_front == 1) {
				ACT_EXPANDER_setPump(ACT_EXPANDER_PUMP_BIG_BACKWARD_LEFT, FALSE);
				ACT_EXPANDER_setSolenoidValve(ACT_EXPANDER_PUMP_BIG_BACKWARD_LEFT, TRUE);
			}
			state_front = (state_front == 1) ? 0 : state_front + 1;
		}
	} else {

	}
}


static void BUTTON_5_long_push(void) {
	if(I_AM_BIG()){

	} else {

	}
}

void BUTTON_verbose(void)
{
	#define SW_BP_NUMBER 20
	static Uint32 previous_state = 0;
	Uint32 current_state = 0x00, up, down;
	bool_e change;

	current_state = 	(BUTTON0_PORT						<< 0) 	|	//Run match
						(IHM_switchs_get(SWITCH_XBEE)		<< 4) 	|
						(IHM_switchs_get(SWITCH_COLOR)		<< 9) 	|
						(IHM_switchs_get(BIROUTE)			<< 10) 	|
						(IHM_switchs_get(SWITCH_LCD)		<< 11);

	up = ~previous_state & current_state;
	down = previous_state & ~current_state;
	change = previous_state != current_state;
	previous_state = current_state;

	if(change){

		if(up & ((Uint32)(1) << 0	))	debug_printf("BP run match pressed\n");
		if(up & ((Uint32)(1) << 3	))	debug_printf("BP print match pressed\n");
		if(up & ((Uint32)(1) << 4	))	debug_printf("SW XBEE enabled\n");

		if(up & ((Uint32)(1) << 8	))	debug_printf("BP Set pressed\n");
		if(up & ((Uint32)(1) << 9	))	debug_printf("SW color changed\n");
		if(up & ((Uint32)(1) << 10	))	debug_printf("Biroute inserted\n");
		if(up & ((Uint32)(1) << 11	))	debug_printf("SW LCD : coord/config\n");
		if(up & ((Uint32)(1) << 12	))	debug_printf("SW Evit enabled\n");
		if(up & ((Uint32)(1) << 13	))	debug_printf("SW Strat1 enabled\n");
		if(up & ((Uint32)(1) << 14	))	debug_printf("SW Strat2 enabled\n");
		if(up & ((Uint32)(1) << 15	))	debug_printf("SW Strat3 enabled\n");
		if(up & ((Uint32)(1) << 16	))	debug_printf("BP Selftest pressed\n");
		if(up & ((Uint32)(1) << 17	))	debug_printf("BP OK pressed\n");
		if(up & ((Uint32)(1) << 18	))	debug_printf("BP MenuUp pressed\n");
		if(up & ((Uint32)(1) << 19	))	debug_printf("BP MenuDown pressed\n");

		if(down & ((Uint32)(1) << 4	))	debug_printf("SW XBEE disabled\n");

		if(down & ((Uint32)(1) << 9	))	debug_printf("SW color changed\n");
		if(down & ((Uint32)(1) << 10	))	debug_printf("Biroute removed\n");
		if(down & ((Uint32)(1) << 11	))	debug_printf("SW LCD : CAN msgs\n");
		if(down & ((Uint32)(1) << 12	))	debug_printf("SW Evit disabled\n");
		if(down & ((Uint32)(1) << 13	))	debug_printf("SW Strat1 disabled\n");
		if(down & ((Uint32)(1) << 14	))	debug_printf("SW Strat2 disabled\n");
		if(down & ((Uint32)(1) << 15	))	debug_printf("SW Strat3 disabled\n");
	}

}
