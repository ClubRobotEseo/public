/*
 *	Club Robot ESEO 2010 - 2011
 *	Check Norris
 *
 *	Fichier : main.c
 *	Package : Actionneur
 *	Description : Ordonnanceur de la carte actionneur
 *  Auteurs : AurÃ©lien
 *  Version 20110225
 */
 
#include "QS/QS_all.h"
#include "QS/QS_lowLayer/QS_sys.h"
#include "QS/QS_buttons.h"
#include "QS/QS_lowLayer/QS_ports.h"
#include "QS/QS_lowLayer/QS_adc.h"
#include "QS/QS_lowLayer/QS_uart.h"
#include "QS/QS_lowLayer/QS_pwm.h"
#include "QS/QS_lowLayer/QS_timer.h"
#include "QS/QS_lowLayer/QS_rcc.h"
#include "QS/QS_outputlog.h"
#include "QS/QS_actuator/QS_ax12.h"
#include "QS/QS_who_am_i.h"
#include "QS/QS_actuator/QS_servo.h"
#include "QS/QS_lowLayer/QS_can.h"
#include "QS/QS_CANmsgList.h"
#include "QS/QS_sensor/QS_CapteurCouleurCW.h"
#include "QS/QS_can_verbose.h"
#include "QS/QS_IHM.h"
#include "QS/QS_systick.h"
#include "QS/QS_actuator/QS_mosfet.h"
#include "QS/QS_actuator/QS_DCMotorSpeed.h"
#include "QS/QS_simulator/QS_simulator.h"
#include "terminal/term_io.h"
#include "queue.h"
#include "ActManager.h"
#include "Can_msg_processing.h"
#include "it.h"


static void MAIN_onButton0();
static void MAIN_onButton1();
static void MAIN_onButton2();
static void MAIN_onButton3();
static void MAIN_onButton4();
static void MAIN_onButton5();
static void MAIN_onButton0LongPush();
static void MAIN_onButton1LongPush();
static void MAIN_onButton2LongPush();
static void MAIN_onButton3LongPush();
static void MAIN_onButton4LongPush();
static void MAIN_onButton5LongPush();

static void MAIN_global_var_init();
static void MAIN_sensor_test();

typedef struct {
	Uint11 sid;
	Sint16	order;
}Button_command_t;

#define NB_BUTTON_COMMANDS(x)		(sizeof(x) / sizeof(Button_command_t))

int main (void)
{
	CAN_msg_t msg;
	/*-------------------------------------
		Demarrage
	-------------------------------------*/

	//Initialisation du systeme
	SYS_init();				// Init systeme
	MAIN_global_var_init();	// Init variable globale
	SYSTICK_init((time32_t*)&(global.absolute_time));

	#if MODE_TEST_GPIO
		WATCHDOG_init();
		do{
			TEST_gpioShortCircuit();
			bool_e flag;
			WATCHDOG_create_flag(10000, &flag);
			while(!flag);
		}while(1);
	#endif

	#if DISABLE_SECURE_GPIO_INIT
		PORTS_init();	// Config des ports
	#else
		if(PORTS_secure_init() == FALSE){
			error_printf("Blocage car le code ne démarre pas sur le bon slot !\n");
			while(1);
		}
	#endif

	PWM_init();

	GPIO_SetBits(LED_RUN);
	GPIO_ResetBits(LED_USER);
	GPIO_SetBits(LED_CAN);

	// Initialisation des peripheriques
	CAN_process_init();
	UART_init();
	TIMER_init();
	IT_init();
	QUEUE_init();
	BUTTONS_init();
	IHM_init(&global.flags.match_started);

	//Sur quel robot est-on ?
	QS_WHO_AM_I_find();	//Détermine le robot sur lequel est branchée la carte.
	debug_printf("--- Hello, I'm ACT (%s) ---\n", QS_WHO_AM_I_get_name());

	// Envoie du message de reboot à la strat (Permet la demande du mode de simulation)
	CAN_send_sid(STRAT_ACT_BOARD_REBOOTED);

	//Init actioneurs
	ACTMGR_init();
	TERMINAL_init();

	#ifdef USE_SCAN
		SCAN_init();
	#endif

	IHM_define_act_button(BP_0_IHM, &MAIN_onButton0, &MAIN_onButton0LongPush);
	IHM_define_act_button(BP_1_IHM, &MAIN_onButton1, &MAIN_onButton1LongPush);
	IHM_define_act_button(BP_2_IHM, &MAIN_onButton2, &MAIN_onButton2LongPush);
	IHM_define_act_button(BP_3_IHM, &MAIN_onButton3, &MAIN_onButton3LongPush);
	IHM_define_act_button(BP_4_IHM, &MAIN_onButton4, &MAIN_onButton4LongPush);
	IHM_define_act_button(BP_5_IHM, &MAIN_onButton5, &MAIN_onButton5LongPush);

	debug_printf("---   ACT Ready    ---\n");

	// Demande des états initiaux des switchs
	CAN_send_sid(IHM_GET_SWITCH);

	GPIO_SetBits(I_AM_READY);

	while(1)
	{

		global.pos.updated = FALSE;

		/*-------------------------------------
			Gestion des DELs, boutons, etc
		-------------------------------------*/

		toggle_led(LED_USER);

		QUEUE_run();
		BUTTONS_update();
		MAIN_sensor_test();

		ACTMGR_process_main();

		/*-------------------------------------
			Réception CAN et exécution
		-------------------------------------*/
		while(CAN_data_ready()){
			// Réception et acquittement
			toggle_led(LED_CAN);
			msg = CAN_get_next_msg();
			CAN_process_msg(&msg);		// Traitement du message pour donner les consignes à la machine d'état
			#ifdef CAN_VERBOSE_MODE
				QS_CAN_VERBOSE_can_msg_print(&msg, VERB_INPUT_MSG);
			#endif

		}

		#ifdef USE_UART1
			while(UART1_data_ready()){
				TERMINAL_uart_checker(UART1_get_next_msg());
			}
		#endif


		#if defined(USE_MOSFETS_MODULE) && defined(USE_MOSFET_MULTI)
			MOSFET_do_order_multi(NULL);
		#endif

		OUTPUTLOG_process_main();

	}//Endloop
	return 0;
}


// Cette année, la flemme de dupliquer du code pour les boutons donc on innove...

Button_command_t commandsButton0Front[] = {
		{ACT_BIG_LOCKER_FRONT_RIGHT, ACT_BIG_LOCKER_FRONT_RIGHT_UNLOCK}, // security
		{ACT_BIG_ELEVATOR_FRONT_RIGHT, ACT_BIG_ELEVATOR_FRONT_RIGHT_TOP},
		{ACT_BIG_ELEVATOR_FRONT_RIGHT, ACT_BIG_ELEVATOR_FRONT_RIGHT_ALMOST_BOT},
		{ACT_BIG_ELEVATOR_FRONT_RIGHT, ACT_BIG_ELEVATOR_FRONT_RIGHT_BOT},
};

Button_command_t commandsButton1Front[] = {
		{ACT_BIG_ELEVATOR_FRONT_MIDDLE, ACT_BIG_ELEVATOR_FRONT_MIDDLE_ALMOST_BOT},
		{ACT_BIG_ELEVATOR_FRONT_MIDDLE, ACT_BIG_ELEVATOR_FRONT_MIDDLE_TOP},
		{ACT_BIG_ELEVATOR_FRONT_MIDDLE, ACT_BIG_ELEVATOR_FRONT_MIDDLE_BOT},
};

Button_command_t commandsButton3Front[] = {
		{ACT_BIG_LOCKER_FRONT_LEFT, ACT_BIG_LOCKER_FRONT_LEFT_UNLOCK},	// security
		{ACT_BIG_ELEVATOR_FRONT_LEFT, ACT_BIG_ELEVATOR_FRONT_LEFT_TOP},
		{ACT_BIG_ELEVATOR_FRONT_LEFT, ACT_BIG_ELEVATOR_FRONT_LEFT_ALMOST_BOT},
		{ACT_BIG_ELEVATOR_FRONT_LEFT, ACT_BIG_ELEVATOR_FRONT_LEFT_BOT},
};

Button_command_t commandsButton2Front[] = {
		{ACT_BIG_LOCKER_FRONT_RIGHT, ACT_BIG_LOCKER_FRONT_RIGHT_UNLOCK},
		{ACT_BIG_LOCKER_FRONT_RIGHT, ACT_BIG_LOCKER_FRONT_RIGHT_VERY_UNLOCK},
		{ACT_BIG_LOCKER_FRONT_RIGHT, ACT_BIG_LOCKER_FRONT_RIGHT_VERY_LOCK},
		{ACT_BIG_LOCKER_FRONT_RIGHT, ACT_BIG_LOCKER_FRONT_RIGHT_LOCK},
};

Button_command_t commandsButton4Front[] = {
		{ACT_BIG_LOCKER_FRONT_MIDDLE, ACT_BIG_LOCKER_FRONT_MIDDLE_LOCK},
		{ACT_BIG_LOCKER_FRONT_MIDDLE, ACT_BIG_LOCKER_FRONT_MIDDLE_UNLOCK},
};

Button_command_t commandsButton5Front[] = {
		{ACT_BIG_LOCKER_FRONT_LEFT, ACT_BIG_LOCKER_FRONT_LEFT_UNLOCK},
		{ACT_BIG_LOCKER_FRONT_LEFT, ACT_BIG_LOCKER_FRONT_LEFT_VERY_UNLOCK},
		{ACT_BIG_LOCKER_FRONT_LEFT, ACT_BIG_LOCKER_FRONT_LEFT_VERY_LOCK},
		{ACT_BIG_LOCKER_FRONT_LEFT, ACT_BIG_LOCKER_FRONT_LEFT_LOCK},
};

Button_command_t commandsButton3Back[] = {
		{ACT_BIG_SORTING_BACK_LEFT, ACT_BIG_SORTING_BACK_LEFT_ACCELERATOR_BOT},
		{ACT_BIG_SORTING_BACK_LEFT, ACT_BIG_SORTING_BACK_LEFT_DISPENSOR_LEVEL},
		{ACT_BIG_SORTING_BACK_LEFT, ACT_BIG_SORTING_BACK_LEFT_TOP},
};

Button_command_t commandsButton0Back[] = {
		{ACT_BIG_SORTING_BACK_RIGHT, ACT_BIG_SORTING_BACK_RIGHT_ACCELERATOR_BOT},
		{ACT_BIG_SORTING_BACK_RIGHT, ACT_BIG_SORTING_BACK_RIGHT_DISPENSOR_LEVEL},
		{ACT_BIG_SORTING_BACK_RIGHT, ACT_BIG_SORTING_BACK_RIGHT_TOP},
};

Button_command_t commandsButton5Back[] = {
		{ACT_BIG_SLOPE_TAKER_BACK, ACT_BIG_SLOPE_TAKER_BACK_OUT},
		{ACT_BIG_SLOPE_TAKER_BACK, ACT_BIG_SLOPE_TAKER_BACK_IN},
};

static void MAIN_onButton0() {
	if(I_AM_BIG()) {
		static Uint8 stateFront = 0;
		static Uint8 stateBack = 0;
		Uint8 nb = 0;
		CAN_msg_t msg;
		msg.size = 8;
		msg.data.act_order.run_now = TRUE;

		if(IHM_switchs_get(SWITCH_BUTTONS_ACT)) {
			msg.sid = commandsButton0Back[stateBack].sid;
			msg.data.act_order.order = commandsButton0Back[stateBack].order;
			nb = NB_BUTTON_COMMANDS(commandsButton0Back);
			stateBack = (stateBack >= nb - 1)? 0 : stateBack + 1;
		} else {
			msg.sid = commandsButton0Front[stateFront].sid;
			msg.data.act_order.order = commandsButton0Front[stateFront].order;
			nb = NB_BUTTON_COMMANDS(commandsButton0Front);
			stateFront = (stateFront >= nb - 1)? 0 : stateFront + 1;
		}

		if(msg.sid != 0)
			CAN_process_msg(&msg);

	} else {
		/* Exemple:
		static Uint8 state = 0;
		CAN_msg_t msg;
		msg.size = 8;
		msg.data.act_order.run_now = TRUE;

		if(state == 0){
			msg.sid = ACT_BIG_EXAMPLE;
			msg.data.act_order.order = ACT_CMD_BIG_EXAMPLE_POS_1;
		}else if(state == 1){
			msg.sid = ACT_BIG_EXAMPLE;
			msg.data.act_order.order = ACT_CMD_BIG_EXAMPLE_POS_2;
		}else if(state == 2){
			msg.sid = ACT_BIG_EXAMPLE;
			msg.data.act_order.order = ACT_CMD_BIG_EXAMPLE_POS_3;
		}

		if(msg.sid != 0)
			CAN_process_msg(&msg);
		state = (state == 2)? 0 : state + 1; */
	}
}

static void MAIN_onButton0LongPush(){
	if(I_AM_BIG()) {

	} else {

	}
}

static void MAIN_onButton1() {
	if(I_AM_BIG()) {
		static Uint8 stateFront = 0;
		static Uint8 stateBack = 0;
		Uint8 nb = 0;
		CAN_msg_t msg;
		msg.size = 8;
		msg.data.act_order.run_now = TRUE;

		if(IHM_switchs_get(SWITCH_BUTTONS_ACT)) {
//			msg.sid = commandsButton1Back[stateBack].sid;
//			msg.data.act_order.order = commandsButton1Back[stateBack].order;
//			nb = NB_BUTTON_COMMANDS(commandsButton1Back);
//			stateBack = (stateBack >= nb - 1)? 0 : stateBack + 1;
		} else {
			msg.sid = commandsButton1Front[stateFront].sid;
			msg.data.act_order.order = commandsButton1Front[stateFront].order;
			nb = NB_BUTTON_COMMANDS(commandsButton1Front);
			stateFront = (stateFront >= nb - 1)? 0 : stateFront + 1;
		}

		if(msg.sid != 0)
			CAN_process_msg(&msg);
	} else {

	}
}

static void MAIN_onButton1LongPush() {
	if(I_AM_BIG()) {

	} else {

	}
}

static void MAIN_onButton2() {
	if(I_AM_BIG()) {
		static Uint8 stateFront = 0;
		static Uint8 stateBack = 0;
		Uint8 nb = 0;
		CAN_msg_t msg;
		msg.size = 8;
		msg.data.act_order.run_now = TRUE;

		if(IHM_switchs_get(SWITCH_BUTTONS_ACT)) {
//			msg.sid = commandsButton2Back[stateBack].sid;
//			msg.data.act_order.order = commandsButton2Back[stateBack].order;
//			nb = NB_BUTTON_COMMANDS(commandsButton2Back);
//			stateBack = (stateBack >= nb - 1)? 0 : stateBack + 1;
		} else {
			msg.sid = commandsButton2Front[stateFront].sid;
			msg.data.act_order.order = commandsButton2Front[stateFront].order;
			nb = NB_BUTTON_COMMANDS(commandsButton2Front);
			stateFront = (stateFront >= nb - 1)? 0 : stateFront + 1;
		}

		if(msg.sid != 0)
			CAN_process_msg(&msg);
	} else {


	}
}


static void MAIN_onButton2LongPush() {
	if(I_AM_BIG()) {

	} else {

	}
}

static void MAIN_onButton3() {
	if(I_AM_BIG()) {
		static Uint8 stateFront = 0;
		static Uint8 stateBack = 0;
		Uint8 nb = 0;
		CAN_msg_t msg;
		msg.size = 8;
		msg.data.act_order.run_now = TRUE;

		if(IHM_switchs_get(SWITCH_BUTTONS_ACT)) {
			msg.sid = commandsButton3Back[stateBack].sid;
			msg.data.act_order.order = commandsButton3Back[stateBack].order;
			nb = NB_BUTTON_COMMANDS(commandsButton3Back);
			stateBack = (stateBack >= nb - 1)? 0 : stateBack + 1;
		} else {
			msg.sid = commandsButton3Front[stateFront].sid;
			msg.data.act_order.order = commandsButton3Front[stateFront].order;
			nb = NB_BUTTON_COMMANDS(commandsButton3Front);
			stateFront = (stateFront >= nb - 1)? 0 : stateFront + 1;
		}

		if(msg.sid != 0)
			CAN_process_msg(&msg);

	} else {


	}
}

static void MAIN_onButton3LongPush() {
	if(I_AM_BIG()) {

	} else {



	}
}

static void MAIN_onButton4(){
	if(I_AM_BIG()) {
		static Uint8 stateFront = 0;
		static Uint8 stateBack = 0;
		Uint8 nb = 0;
		CAN_msg_t msg;
		msg.size = 8;
		msg.data.act_order.run_now = TRUE;

		if(IHM_switchs_get(SWITCH_BUTTONS_ACT)) {
//			msg.sid = commandsButton4Back[stateBack].sid;
//			msg.data.act_order.order = commandsButton4Back[stateBack].order;
//			nb = NB_BUTTON_COMMANDS(commandsButton4Back);
//			stateBack = (stateBack >= nb - 1)? 0 : stateBack + 1;
		} else {
			msg.sid = commandsButton4Front[stateFront].sid;
			msg.data.act_order.order = commandsButton4Front[stateFront].order;
			nb = NB_BUTTON_COMMANDS(commandsButton4Front);
			stateFront = (stateFront >= nb - 1)? 0 : stateFront + 1;
		}

		if(msg.sid != 0)
			CAN_process_msg(&msg);
	} else {



	}
}

static void MAIN_onButton4LongPush(){
	if(I_AM_BIG()) {


	} else {


	}
}

static void MAIN_onButton5() {
	if(I_AM_BIG()) {
		static Uint8 stateFront = 0;
		static Uint8 stateBack = 0;
		Uint8 nb = 0;
		CAN_msg_t msg;
		msg.size = 8;
		msg.data.act_order.run_now = TRUE;

		if(IHM_switchs_get(SWITCH_BUTTONS_ACT)) {
			msg.sid = commandsButton5Back[stateBack].sid;
			msg.data.act_order.order = commandsButton5Back[stateBack].order;
			nb = NB_BUTTON_COMMANDS(commandsButton5Back);
			stateBack = (stateBack >= nb - 1)? 0 : stateBack + 1;
		} else {
			msg.sid = commandsButton5Front[stateFront].sid;
			msg.data.act_order.order = commandsButton5Front[stateFront].order;
			nb = NB_BUTTON_COMMANDS(commandsButton5Front);
			stateFront = (stateFront >= nb - 1)? 0 : stateFront + 1;
		}

		if(msg.sid != 0)
			CAN_process_msg(&msg);
	} else {


	}
}

static void MAIN_onButton5LongPush() {
	if(I_AM_BIG()) {

	} else {


	}
}

static void MAIN_global_var_init(){
	// Initialisation de la variable global
	global.flags.match_started = FALSE;
	global.flags.match_over = FALSE;
	global.flags.power = FALSE;
	global.flags.virtual_mode = FALSE;
	global.alim_value = 0;
	global.absolute_time = 0;
}

static void MAIN_sensor_test(){
	static bool_e led_on = FALSE;

	UNUSED_VAR(led_on);

	if(I_AM_BIG()) {
		// Test des capteurs de BIG_ROBOT
		if(ELEVATOR_FDC){
			if(led_on == FALSE){
				IHM_leds_send_msg(1, (led_ihm_t){LED_SENSOR_TEST, ON});
				led_on = TRUE;
			}
		}else if(led_on == TRUE){
			IHM_leds_send_msg(1, (led_ihm_t){LED_SENSOR_TEST, OFF});
			led_on = FALSE;
		}
	} else {
		// Test des capteurs de SMALL_ROBOT
	}
}
