/*
 *	Club Robot ESEO 2008 - 2009
 *	Archi-Tech'
 *
 *	Fichier : main.c
 *	Package : Projet Propulsion
 *	Description : fonction principale d'exemple pour le projet
 *				standard construit par la QS pour exemple, pour
 *				utilisation en Match
 *	Auteur : Jacen
 *	Version 20080924
 */

#include "main.h"
#include "QS/QS_lowLayer/QS_ports.h"
#include "QS/QS_lowLayer/QS_uart.h"
#include "QS/QS_buttons.h"
#include "QS/QS_CANmsgList.h"
#include "QS/QS_who_am_i.h"
#include "QS/QS_outputlog.h"
#include "QS/QS_watchdog.h"
#include "QS/QS_lowLayer/QS_can.h"
#include "QS/QS_lowLayer/QS_sys.h"
#include "QS/QS_systick.h"
#include "QS/QS_lowLayer/QS_adc.h"
#include "QS/QS_IHM.h"
#include "QS/QS_lowLayer/QS_rcc.h"
#include "QS/QS_hokuyo/hokuyo.h"
#include "odometry.h"
#include "copilot.h"
#include "pilot.h"
#include "supervisor.h"
#include "warner.h"
#include "it.h"
#include "roadmap.h"
#include "secretary.h"
#include "sequences.h"
#include "debug.h"
#include "joystick.h"
#include "gyroscope.h"
#include "detection.h"
#include "detection_choc.h"
#include "scan/scan.h"
#include "scan/borders_scan.h"
#include "scan/rotation_scan.h"
#include "scan/scanDistri.h"
#include "kalman/kalman.h"
#include "kalman/beacon.h"


#if MODE_SAVE_STRUCTURE_GLOBAL_A_CHAQUE_IT
	extern volatile global_data_storage_t SAVE;
#endif



#if	 (MODE_PRINTF_TABLEAU) 						||		\
	 (MODE_SAVE_STRUCTURE_GLOBAL_A_CHAQUE_IT) 	||		\
	 (SUPERVISOR_DISABLE_ERROR_DETECTION) 		||		\
	 (MODE_REGLAGE_KV)
	#warning "SOYEZ CONSCIENT QUE VOUS NE COMPILEZ PAS EN MODE MATCH..."
#endif

static void MAIN_global_var_init();
static void MAIN_sensor_test();
volatile Uint8 t_ms = 0;
volatile bool_e flag_selftest_asked = FALSE;


void blue_button_action(void)
{
	flag_selftest_asked = TRUE;
}


void initialisation(void)
{

	#ifdef USE_QSx86
		// Initialisation pour EVE
		EVE_manager_card_init();
	#endif	/* USE_QSx86 */

	// Initialisation du système
	SYS_init();				// Init système
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


	GPIO_SetBits(LED_RUN);

	SECRETARY_init();	//Pour recevoir tout les messages CAN envoyés très tôt...
	GPIO_ResetBits(LED_RUN);
	UART_init();

	RCC_read();

	//Doit se faire AVANT ODOMETRY_init() !!!

	//Sur quel robot est-on ?
	QS_WHO_AM_I_find();	//Détermine le robot sur lequel est branchée la carte.
	debug_printf("--- Hello, I'm PROP (%s) ---\n", QS_WHO_AM_I_get_name());
	TIMER_init();
	ODOMETRY_init();
	WATCHDOG_init();
	SUPERVISOR_init();
	COPILOT_init();
	PILOT_init();
	ROADMAP_init();
	WARNER_init();
	JOYSTICK_init();
	DEBUG_init();
	ADC_init();
	BUTTONS_init();

	IHM_init(&global.flags.match_started);
	IHM_define_act_button(BP_CALIBRATION_IHM,&SEQUENCES_calibrate,NULL);
#if MODE_PRINT_FIRST_TRAJ
	IHM_define_act_button(BP_PRINTMATCH_IHM,&DEBUG_display,NULL);
#endif

	//BUTTONS_define_actions(BUTTON0, &SCAN_onPower, NULL, 1);

	#if USE_KALMAN_FILTER
		KALMAN_init();
	#endif

	DETECTION_init();
	#if USE_HOKUYO
		HOKUYO_init();
	#endif

	#if SCAN
		SCAN_init();
	#endif

	#if SCAN_OBJETS
		OBJECTS_SCAN_init();
	#endif

	#if SCAN_ROTATION
		SCAN_CORNER_init();
	#endif

	#if USE_GYROSCOPE
		GYRO_init();
	#endif

	#if DETECTION_CHOC
		DETECTION_CHOC_init();
	#endif

	#if USE_SCAN_DISTRI
		SCAN_DISTRI_init();
	#endif

	IT_init();

	// Demande des états initiaux des switchs
	CAN_send_sid(IHM_GET_SWITCH);

	CAN_send_sid(STRAT_PROP_BOARD_REBOOTED);

	GPIO_SetBits(I_AM_READY);

	/*
	Récapitulatif des priorités des ITs :
	-> 7 : Codeurs QEI_on_it
	-> 6 : CAN
	-> 5 : timer1 (5ms)
	-> 4 : UART
	-> 3 : timer2 (100ms)
	-> 0 : tâche de fond
	*/
}

/*
 * Initialisation des modules necessitant de connaitre la couleur.
 * Cette fonction est appelée dans secretary.c lors de la réception de la couleur.
 */
void MAIN_init_with_color(void) {
	BEACON_init();
}


int main (void)
{
	initialisation();
	GPIO_SetBits(LED_RUN);

	//Routines de tests UART et CAN
/*		ROADMAP_add_order(TRAJECTORY_AUTOMATIC_CURVE, 	1793,894	, 0, NOT_RELATIVE, NOT_NOW, ANY_WAY, NOT_BORDER_MODE, NO_MULTIPOINT, FAST, ACKNOWLEDGE_ASKED, CORRECTOR_ENABLE);
		ROADMAP_add_order(TRAJECTORY_AUTOMATIC_CURVE, 	1734,918	, 0, NOT_RELATIVE, NOT_NOW, ANY_WAY, NOT_BORDER_MODE, NO_MULTIPOINT, FAST, ACKNOWLEDGE_ASKED, CORRECTOR_ENABLE);
		ROADMAP_add_order(TRAJECTORY_AUTOMATIC_CURVE, 	1678,947	, 0, NOT_RELATIVE, NOT_NOW, ANY_WAY, NOT_BORDER_MODE, NO_MULTIPOINT, FAST, ACKNOWLEDGE_ASKED, CORRECTOR_ENABLE);
		ROADMAP_add_order(TRAJECTORY_AUTOMATIC_CURVE, 	1624,982	, 0, NOT_RELATIVE, NOT_NOW, ANY_WAY, NOT_BORDER_MODE, NO_MULTIPOINT, FAST, ACKNOWLEDGE_ASKED, CORRECTOR_ENABLE);
		ROADMAP_add_order(TRAJECTORY_AUTOMATIC_CURVE, 	1575,1022	, 0, NOT_RELATIVE, NOT_NOW, ANY_WAY, NOT_BORDER_MODE, NO_MULTIPOINT, FAST, ACKNOWLEDGE_ASKED, CORRECTOR_ENABLE);
		ROADMAP_add_order(TRAJECTORY_AUTOMATIC_CURVE, 	1529,1066	, 0, NOT_RELATIVE, NOT_NOW, ANY_WAY, NOT_BORDER_MODE, NO_MULTIPOINT, FAST, ACKNOWLEDGE_ASKED, CORRECTOR_ENABLE);
		ROADMAP_add_order(TRAJECTORY_AUTOMATIC_CURVE, 	1489,1115	, 0, NOT_RELATIVE, NOT_NOW, ANY_WAY, NOT_BORDER_MODE, NO_MULTIPOINT, FAST, ACKNOWLEDGE_ASKED, CORRECTOR_ENABLE);
		ROADMAP_add_order(TRAJECTORY_AUTOMATIC_CURVE, 	1453,1168	, 0, NOT_RELATIVE, NOT_NOW, ANY_WAY, NOT_BORDER_MODE, NO_MULTIPOINT, FAST, ACKNOWLEDGE_ASKED, CORRECTOR_ENABLE);
		ROADMAP_add_order(TRAJECTORY_AUTOMATIC_CURVE, 	1423,1223	, 0, NOT_RELATIVE, NOT_NOW, ANY_WAY, NOT_BORDER_MODE, NO_MULTIPOINT, FAST, ACKNOWLEDGE_ASKED, CORRECTOR_ENABLE);
		ROADMAP_add_order(TRAJECTORY_AUTOMATIC_CURVE, 	1398,1282	, 0, NOT_RELATIVE, NOT_NOW, ANY_WAY, NOT_BORDER_MODE, NO_MULTIPOINT, FAST, ACKNOWLEDGE_ASKED, CORRECTOR_ENABLE);
		ROADMAP_add_order(TRAJECTORY_AUTOMATIC_CURVE, 	1380,1343	, 0, NOT_RELATIVE, NOT_NOW, ANY_WAY, NOT_BORDER_MODE, NO_MULTIPOINT, FAST, ACKNOWLEDGE_ASKED, CORRECTOR_ENABLE);
		ROADMAP_add_order(TRAJECTORY_AUTOMATIC_CURVE, 	1367,1405	, 0, NOT_RELATIVE, NOT_NOW, ANY_WAY, NOT_BORDER_MODE, NO_MULTIPOINT, FAST, ACKNOWLEDGE_ASKED, CORRECTOR_ENABLE);
		ROADMAP_add_order(TRAJECTORY_AUTOMATIC_CURVE, 	1361,1468	, 0, NOT_RELATIVE, NOT_NOW, ANY_WAY, NOT_BORDER_MODE, NO_MULTIPOINT, FAST, ACKNOWLEDGE_ASKED, CORRECTOR_ENABLE);
		ROADMAP_add_order(TRAJECTORY_AUTOMATIC_CURVE, 	1361,1532	, 0, NOT_RELATIVE, NOT_NOW, ANY_WAY, NOT_BORDER_MODE, NO_MULTIPOINT, FAST, ACKNOWLEDGE_ASKED, CORRECTOR_ENABLE);
		ROADMAP_add_order(TRAJECTORY_AUTOMATIC_CURVE, 	1367,1595	, 0, NOT_RELATIVE, NOT_NOW, ANY_WAY, NOT_BORDER_MODE, NO_MULTIPOINT, FAST, ACKNOWLEDGE_ASKED, CORRECTOR_ENABLE);
		ROADMAP_add_order(TRAJECTORY_AUTOMATIC_CURVE, 	1380,1657	, 0, NOT_RELATIVE, NOT_NOW, ANY_WAY, NOT_BORDER_MODE, NO_MULTIPOINT, FAST, ACKNOWLEDGE_ASKED, CORRECTOR_ENABLE);
		ROADMAP_add_order(TRAJECTORY_AUTOMATIC_CURVE, 	1398,1718	, 0, NOT_RELATIVE, NOT_NOW, ANY_WAY, NOT_BORDER_MODE, NO_MULTIPOINT, FAST, ACKNOWLEDGE_ASKED, CORRECTOR_ENABLE);
		ROADMAP_add_order(TRAJECTORY_AUTOMATIC_CURVE, 	1423,1777	, 0, NOT_RELATIVE, NOT_NOW, ANY_WAY, NOT_BORDER_MODE, NO_MULTIPOINT, FAST, ACKNOWLEDGE_ASKED, CORRECTOR_ENABLE);
		ROADMAP_add_order(TRAJECTORY_AUTOMATIC_CURVE, 	1453,1832	, 0, NOT_RELATIVE, NOT_NOW, ANY_WAY, NOT_BORDER_MODE, NO_MULTIPOINT, FAST, ACKNOWLEDGE_ASKED, CORRECTOR_ENABLE);
		ROADMAP_add_order(TRAJECTORY_AUTOMATIC_CURVE, 	1489,1885	, 0, NOT_RELATIVE, NOT_NOW, ANY_WAY, NOT_BORDER_MODE, NO_MULTIPOINT, FAST, ACKNOWLEDGE_ASKED, CORRECTOR_ENABLE);
		ROADMAP_add_order(TRAJECTORY_AUTOMATIC_CURVE, 	1529,1934	, 0, NOT_RELATIVE, NOT_NOW, ANY_WAY, NOT_BORDER_MODE, NO_MULTIPOINT, FAST, ACKNOWLEDGE_ASKED, CORRECTOR_ENABLE);
		ROADMAP_add_order(TRAJECTORY_AUTOMATIC_CURVE, 	1575,1978	, 0, NOT_RELATIVE, NOT_NOW, ANY_WAY, NOT_BORDER_MODE, NO_MULTIPOINT, FAST, ACKNOWLEDGE_ASKED, CORRECTOR_ENABLE);
		ROADMAP_add_order(TRAJECTORY_AUTOMATIC_CURVE, 	1624,2018	, 0, NOT_RELATIVE, NOT_NOW, ANY_WAY, NOT_BORDER_MODE, NO_MULTIPOINT, FAST, ACKNOWLEDGE_ASKED, CORRECTOR_ENABLE);
		ROADMAP_add_order(TRAJECTORY_AUTOMATIC_CURVE, 	1678,2053	, 0, NOT_RELATIVE, NOT_NOW, ANY_WAY, NOT_BORDER_MODE, NO_MULTIPOINT, FAST, ACKNOWLEDGE_ASKED, CORRECTOR_ENABLE);
		ROADMAP_add_order(TRAJECTORY_AUTOMATIC_CURVE, 	1734,2082	, 0, NOT_RELATIVE, NOT_NOW, ANY_WAY, NOT_BORDER_MODE, NO_MULTIPOINT, FAST, ACKNOWLEDGE_ASKED, CORRECTOR_ENABLE);
		ROADMAP_add_order(TRAJECTORY_AUTOMATIC_CURVE, 	1793,2106	, 0, NOT_RELATIVE, NOT_NOW, ANY_WAY, NOT_BORDER_MODE, NO_MULTIPOINT, FAST, ACKNOWLEDGE_ASKED, CORRECTOR_ENABLE);
		ROADMAP_add_order(TRAJECTORY_AUTOMATIC_CURVE, 	1830,2115	, 0, NOT_RELATIVE, NOT_NOW, ANY_WAY, NOT_BORDER_MODE, NO_MULTIPOINT, FAST, ACKNOWLEDGE_ASKED, CORRECTOR_ENABLE);


*/

	#if USE_GYROSCOPE
//WATCHDOG_create(1000, &GYRO_test, TRUE);
	#endif


	while(1)
	{

		#ifdef USE_QSx86
			// Update pour EVE
			EVE_manager_card();
		#endif	/* USE_QSx86 */

		DEBUG_process_main();

		if(t_ms > 20)	//Pour éviter les rebonds
		{
			t_ms = 0;
			BUTTONS_update();			//Gestion des boutons
			//debug_printf("value = %d\n", ADC_getValue(ADC_SENSOR_FISHS));
		}

		SECRETARY_process_main();	//Communication avec l'extérieur. (Réception des messages)

		WARNER_process_main();		//Communication avec l'extérieur. (Envois des messages)

		#if USE_HOKUYO
			HOKUYO_processMain();
		#endif

		DETECTION_process_main();

		MAIN_sensor_test();

		#if DETECTION_CHOC
			DETECTION_CHOC_process_main();
		#endif

		OUTPUTLOG_process_main();

		#if SCAN
			SCAN_process_main();
		#endif

		#if SCAN_BORDURE
			BORDERS_SCAN_process_main();
		#endif

		#if SCAN_ROTATION
			SCAN_CORNER_process_main();
		#endif

		#if USE_SCAN_DISTRI
			SCAN_DISTRI_processMain();
		#endif


		if(flag_selftest_asked)
		{
			flag_selftest_asked = FALSE;
			debug_printf("BP Selftest pressed\n");
			SEQUENCES_selftest();
		}

	}
	return 0;
}


void MAIN_process_it(Uint8 ms)
{
	t_ms += ms;
}

static void MAIN_global_var_init(){
	// Initialisation de la variable global
	global.debug.mode_best_effort_enable = FALSE;
	global.absolute_time = 0;
	global.flags.match_started = FALSE;
	global.flags.match_over = FALSE;
	global.flags.alim = FALSE;
}

static void MAIN_sensor_test(){
	static bool_e led_on = FALSE;
	if(QS_WHO_AM_I_get() == SMALL_ROBOT){
		if(ADC_getValue(ADC_SENSOR_LASER_LEFT) > 5 || ADC_getValue(ADC_SENSOR_SMALL_LASER) < 50){
			if(led_on == FALSE){
				IHM_leds_send_msg(1, (led_ihm_t){LED_SENSOR_TEST, ON});
				led_on = TRUE;
			}
		}else if(led_on == TRUE){
			IHM_leds_send_msg(1, (led_ihm_t){LED_SENSOR_TEST, OFF});
			led_on = FALSE;
		}
	}
}


