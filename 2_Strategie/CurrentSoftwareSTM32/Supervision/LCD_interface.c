/*
 *	Club Robot ESEO 2013
 *
 *	Fichier : LCD_interface.c
 *	Package : Supervision
 *	Description : implémentation de l'interface de l'écran LCD MCCOG42005A6W-BNMLWI
 *	Licence : CeCILL-C
 *	Auteur : HERZAEONE
 *	Version 201308
 */

#include "LCD_interface.h"

#if USE_LCD

	#include "../QS/QS_outputlog.h"
	#include "../QS/QS_stateMachineHelper.h"
	#include "../QS/QS_lcd_over_uart.h"
	#include "../QS/QS_watchdog.h"
	#include "Selftest.h"
	#include "RTC.h"
	#include "../brain.h"
	#include "SD/SD.h"
	#include "string.h"
	#include "../detection.h"
	#include "../strats_2019/score.h"
	#include "../strats_2019/actions_prop.h"

	#define SELFTEST_NB_DISPLAYED_LINE	13
	#define LCD_MIN_TIME_REFRESH		400
	#define LCD_DEBUG_STRING_COUNT		22
	#define LCD_DEBUG_STRING_LENGTH		50
	#define LCD_STRAT_NAME_LENGTH		20
	#define LCD_NB_STRATS_PER_PAGE		6

	typedef enum{
		MENU_WAIT_SWITCH = -1,
		MENU_WAIT_IHM = 0,
		MENU_WAIT_OHTER_BOARD,
		MENU_MAIN,
		MENU_SELFTEST,
		MENU_STRAT_INUTILE,
		MENU_IHM_TEST,
		MENU_CHECK_LIST,
		MENU_STRAT,
		MENU_ODOMETRY_CALIBRATION,
		MENU_DEBUG
	}LCD_state_e;

	static LCD_state_e LCD_state = MENU_WAIT_IHM;
	static LCD_state_e LCD_previous_state = MENU_WAIT_IHM;
	static bool_e LCD_is_under_IHM_control = FALSE;

	static bool_e LCD_batteryCritical = FALSE;

	typedef struct{
		char string[LCD_DEBUG_STRING_LENGTH];
		bool_e stringChanged;
		LCD_objectId_t id;
	}LCD_debugString_s;

	static LCD_debugString_s debugString[LCD_DEBUG_STRING_COUNT] = {0};

	typedef enum{
		LCD_ODOMETRY_TRANSLATION,
		LCD_ODOMETRY_ROTATION,
		LCD_ODOMETRY_SYMETRY
	}LCD_odometryType_e;

	static LCD_odometryType_e odometryType;
	static Sint32 odometryCoef;
	static Sint32 odometryError;
	static Uint8 odometryRound;
	static bool_e odometrySuccess;
	static bool_e odometryWriteStateAnswer;
	static bool_e odometryWriteState;

	static void LCD_processState(void);
	static LCD_state_e LCD_MENU_waitIHM(bool_e init);
	static LCD_state_e LCD_MENU_waitOtherBoard(bool_e init);
	static LCD_state_e LCD_MENU_mainMenu(bool_e init);
	static LCD_state_e LCD_MENU_selftest(bool_e init);
	static LCD_state_e LCD_MENU_ihmTest(bool_e init);
	static LCD_state_e LCD_MENU_strat_inutile(bool_e init);
	static LCD_state_e LCD_MENU_checkList(bool_e init);
	static LCD_state_e LCD_MENU_strat(bool_e init);
	static LCD_state_e LCD_MENU_odometryCalibration(bool_e init);
	static LCD_state_e LCD_MENU_debug(bool_e init);

	void LCD_init(void){
		LCD_OVER_UART_init();
	}

	void LCD_processMain(void){
		LCD_processState();
		LCD_OVER_UART_processMain();
	}

	void LCD_IHM_control(bool_e control){
		LCD_is_under_IHM_control = control;
	}

	void LCD_setDebugMenu(void){
		LCD_state = MENU_DEBUG;
	}

	static void LCD_processState(void){
		bool_e entrance = LCD_state != LCD_previous_state;
		LCD_previous_state = LCD_state;

		if(entrance)
			LCD_OVER_UART_resetScreen();

		if(LCD_is_under_IHM_control == TRUE){
			LCD_state = MENU_WAIT_SWITCH;
		}

		switch(LCD_state){
			case MENU_WAIT_SWITCH:
				if(entrance){
					LCD_OVER_UART_deleteAllObject();
				}

				if(LCD_is_under_IHM_control == FALSE){
					LCD_state = MENU_MAIN;
				}
				break;

			case MENU_WAIT_IHM:
				LCD_state = LCD_MENU_waitIHM(entrance);
				break;

			case MENU_WAIT_OHTER_BOARD:
				LCD_state = LCD_MENU_waitOtherBoard(entrance);
				break;

			case MENU_MAIN:
				LCD_state = LCD_MENU_mainMenu(entrance);
				break;

			case MENU_SELFTEST:
				LCD_state = LCD_MENU_selftest(entrance);
				break;

			case MENU_STRAT_INUTILE:
				LCD_state = LCD_MENU_strat_inutile(entrance);
				break;

			case MENU_IHM_TEST:
				LCD_state = LCD_MENU_ihmTest(entrance);
				break;

			case MENU_CHECK_LIST:
				LCD_state = LCD_MENU_checkList(entrance);
				break;

			case MENU_STRAT:
				LCD_state = LCD_MENU_strat(entrance);
				break;

			case MENU_ODOMETRY_CALIBRATION:
				LCD_state = LCD_MENU_odometryCalibration(entrance);
				break;

			case MENU_DEBUG:
				LCD_state = LCD_MENU_debug(entrance);
				break;
		}
	}

	static LCD_state_e LCD_MENU_waitIHM(bool_e init){
		static time32_t time;
		if(init)
			time = global.absolute_time;

		if(global.absolute_time - time > 1000){
			LCD_OVER_UART_setMenu(LCD_MENU_CUSTOM);
			return MENU_WAIT_OHTER_BOARD;
		}

		return MENU_WAIT_IHM;
	}

	static LCD_state_e LCD_MENU_ihmTest(bool_e init){
		if(init)
			LCD_OVER_UART_setMenu(LCD_MENU_DEBUG);

		return MENU_IHM_TEST;
	}

	static LCD_state_e LCD_MENU_waitOtherBoard(bool_e init){
		static LCD_objectId_t actReady, propReady;
		static bool_e actDisplay = FALSE, propDisplay = FALSE;
		static bool_e skip = FALSE;
		static time32_t beginTimeMenu;

		if(init){
			actDisplay = FALSE;
			propDisplay = FALSE;
			skip = FALSE;

			LCD_OVER_UART_addImage(0, 0, LCD_IMAGE_LOGO);
			LCD_OVER_UART_addAnimatedImage(270, 55, LCD_ANIMATION_WAIT_CIRCLE);
			LCD_OVER_UART_addButton(250, 10, 0, 0, FALSE, &skip, LCD_COLOR_BLACK, LCD_COLOR_GREEN, LCD_COLOR_RED, LCD_COLOR_BLACK, LCD_TEXT_FONTS_11x18, "Skip");
			LCD_OVER_UART_addText(20, 225, LCD_COLOR_BLACK, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_7x10, "En attente de l'initialisation des cartes");
			actReady = LCD_OVER_UART_addText(10, 10, LCD_COLOR_BLACK, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_11x18, "Attente actionneur");
			propReady = LCD_OVER_UART_addText(10, 35, LCD_COLOR_BLACK, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_11x18, "Attente propulsion");
			beginTimeMenu = global.absolute_time;
		}

		if(ACT_IS_READY && actDisplay == FALSE){
			LCD_OVER_UART_setText(actReady, "Actionneur ready");
			actDisplay = TRUE;
		}

		if(PROP_IS_READY && propDisplay == FALSE){
			LCD_OVER_UART_setText(propReady, "Propulsion ready");
			propDisplay = TRUE;
		}

		if((ACT_IS_READY && PROP_IS_READY && (global.absolute_time - beginTimeMenu) > 2000) || skip)
			return MENU_MAIN;

		return MENU_WAIT_OHTER_BOARD;
	}


	void LCD_setBatteryCritical(void){
		LCD_batteryCritical = TRUE;
	}

	static LCD_state_e LCD_MENU_mainMenu(bool_e init){
		static bool_e changeMenuSelftest = FALSE, lastChangeMenuSelftest = FALSE;
		static bool_e changeMenuCheckList = FALSE, lastChangeMenuCheckList = FALSE;
		static bool_e changeMenuStrat = FALSE, lastChangeMenuStrat = FALSE;
		static LCD_objectId_t idPos, idVoltage, idTime, idStrat, idMatch, idMatchTime, idScore, idMemoryState;
		static LCD_objectId_t idAdv[4];
		static time32_t lastRefresh;

		static Sint16 lastX, lastY, lastTeta, lastVoltate, lastScore;
		static bool_e batteryCriticalDisplayed = FALSE;
		static date_t lastDate;
		static char lastStratName[LCD_STRAT_NAME_LENGTH];
		static Uint16 lastIdSD;
		static bool_e lastStateSD;
		static time32_t lastMatchTime;
		static foe_t lastFoe[4];
		static prop_memory_startup_check_e lastMemoryState;

		if(init){
			changeMenuSelftest = FALSE;
			lastChangeMenuSelftest = FALSE;
			changeMenuCheckList = FALSE;
			lastChangeMenuCheckList = FALSE;
			changeMenuStrat = FALSE;
			lastChangeMenuStrat = FALSE;

			LCD_OVER_UART_addButton(10, 210, 0, 0, FALSE, &changeMenuSelftest, LCD_COLOR_BLACK, LCD_COLOR_GREEN, LCD_COLOR_RED, LCD_COLOR_BLACK, LCD_TEXT_FONTS_11x18, "Selftest");
			LCD_OVER_UART_addButton(130, 210, 0, 0, FALSE, &changeMenuCheckList, LCD_COLOR_BLACK, LCD_COLOR_GREEN, LCD_COLOR_RED, LCD_COLOR_BLACK, LCD_TEXT_FONTS_11x18, "Check");
			LCD_OVER_UART_addButton(230, 210, 0, 0, FALSE, &changeMenuStrat, LCD_COLOR_BLACK, LCD_COLOR_GREEN, LCD_COLOR_RED, LCD_COLOR_BLACK, LCD_TEXT_FONTS_11x18, "Strat");

			idPos = LCD_OVER_UART_addText(5, 5, LCD_COLOR_BLACK, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_16x26, "%4d | %4d | %3d.%1d", global.pos.x, global.pos.y, global.pos.angle*180/PI4096, absolute((global.pos.angle*1800/PI4096)%10));
			lastX = global.pos.x;
			lastY = global.pos.y;
			lastTeta = global.pos.angle;

			idVoltage = LCD_OVER_UART_addText(130, 180, LCD_COLOR_BLACK, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_11x18, "%d mV", SELFTEST_measure24_mV());
			lastVoltate = SELFTEST_measure24_mV();

			idScore = LCD_OVER_UART_addText(70, 75, LCD_COLOR_BLUE, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_16x26, "Score: %3d", SCORE_get_score());

			date_t date;
			RTC_get_local_time(&date);

			lastDate = date;

			idTime = LCD_OVER_UART_addText(15, 180, LCD_COLOR_BLACK, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_11x18, "%02d:%02d:%02d", date.hours, date.minutes, date.seconds);

			idStrat = LCD_OVER_UART_addText(15, 47, LCD_COLOR_BLACK, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_11x18, "%s", BRAIN_get_current_strat_name());

			strncpy(lastStratName, BRAIN_get_current_strat_name(), LCD_STRAT_NAME_LENGTH);

			idMatchTime = LCD_OVER_UART_addText(250, 45, LCD_COLOR_BLACK, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_16x26, "%02d", (Uint16)(global.match_time/1000));
			lastMatchTime = global.match_time/1000;

			if(SD_isOK())
				idMatch = LCD_OVER_UART_addText(230, 180, LCD_COLOR_BLACK, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_11x18, "SD: %d", SD_get_match_id());
			else
				idMatch = LCD_OVER_UART_addText(230, 180, LCD_COLOR_RED, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_11x18, "SD: error");

			lastStateSD = SD_isOK();
			lastIdSD = SD_get_match_id();

			if(global.prop.prop_memory_startup_check_state == PROP_MEMORY_STARTUP_CHECK__GOOD)
				idMemoryState = LCD_OVER_UART_addText(220, 155, LCD_COLOR_BLACK, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_11x18, "EEPROM:OK");
			else
				idMemoryState = LCD_OVER_UART_addText(220, 155, LCD_COLOR_RED, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_11x18, "EEPROM:KO");

			lastMemoryState = global.prop.prop_memory_startup_check_state;

			LCD_OVER_UART_addLine(0, 35, 319, 35, LCD_COLOR_BLACK);

			Uint8 i;
			for(i=0; i<4; i++){
				if(i < 2){

					if(global.foe[MAX_HOKUYO_FOES+i].enable)
						idAdv[i] = LCD_OVER_UART_addText(10, 110 + 15*i, LCD_COLOR_BLACK, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_7x10, "advH%d dist : %4d  |  angle : %3ld", i, global.foe[i].dist, ((Sint32)global.foe[i].angle)*180/PI4096);
					else
						idAdv[i] = LCD_OVER_UART_addText(10, 110 + 15*i, LCD_COLOR_BLACK, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_7x10, "advH%d obsolate", i);

					lastFoe[i].dist = global.foe[i].dist;
					lastFoe[i].angle = global.foe[i].angle;
					lastFoe[i].enable = global.foe[i].enable;
				}else{

					if(global.foe[MAX_HOKUYO_FOES+i-2].enable)
						idAdv[i] = LCD_OVER_UART_addText(10, 110 + 15*i, LCD_COLOR_BLACK, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_7x10, "advIR%d dist : %4d  |  angle : %3ld", i-2, global.foe[MAX_HOKUYO_FOES+i-2].dist, ((Sint32)global.foe[MAX_HOKUYO_FOES+i-2].angle)*180/PI4096);
					else if(global.foe[MAX_HOKUYO_FOES+i-2].fiability_error)
						idAdv[i] = LCD_OVER_UART_addText(10, 110 + 15*i, LCD_COLOR_BLACK, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_7x10, "advIR%d erreur %d", i-2, global.foe[MAX_HOKUYO_FOES+i-2].fiability_error);
					else
						idAdv[i] = LCD_OVER_UART_addText(10, 110 + 15*i, LCD_COLOR_BLACK, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_7x10, "advIR%d obsolate", i-2);

					lastFoe[i].dist = global.foe[MAX_HOKUYO_FOES+i-2].dist;
					lastFoe[i].angle = global.foe[MAX_HOKUYO_FOES+i-2].angle;
					lastFoe[i].fiability_error = global.foe[MAX_HOKUYO_FOES+i-2].fiability_error;
					lastFoe[i].enable = global.foe[MAX_HOKUYO_FOES+i-2].enable;
				}
			}
		}

		if(global.absolute_time - lastRefresh > LCD_MIN_TIME_REFRESH){
			lastRefresh = global.absolute_time;

			date_t date;
			RTC_get_local_time(&date);

			if(lastX != global.pos.x || lastY != global.pos.y || lastTeta != global.pos.angle){
				LCD_OVER_UART_setText(idPos, "%4d | %4d | %3d.%1d", global.pos.x, global.pos.y, global.pos.angle*180/PI4096, absolute((global.pos.angle*1800/PI4096)%10));
				lastX = global.pos.x;
				lastY = global.pos.y;
				lastTeta = global.pos.angle;
			}


			if(lastVoltate != SELFTEST_measure24_mV()){
				LCD_OVER_UART_setText(idVoltage, "%d mV", SELFTEST_measure24_mV());
				lastVoltate = SELFTEST_measure24_mV();
			}

			if(LCD_batteryCritical && batteryCriticalDisplayed == FALSE){
				batteryCriticalDisplayed = TRUE;
				LCD_OVER_UART_setTextColor(idVoltage, LCD_COLOR_RED);
			}

			if(lastDate.seconds != date.seconds){
				LCD_OVER_UART_setText(idTime, "%02d:%02d:%02d", date.hours, date.minutes, date.seconds);
				lastDate = date;
			}

			Uint16 currentScore = SCORE_get_score();
			if(lastScore != currentScore){
				LCD_OVER_UART_setText(idScore, "Score: %3d", currentScore);
				lastScore = currentScore;
			}

			if(strncmp(lastStratName, BRAIN_get_current_strat_name(), LCD_STRAT_NAME_LENGTH) != 0){
				LCD_OVER_UART_setText(idStrat, "%s", BRAIN_get_current_strat_name());
				strncpy(lastStratName, BRAIN_get_current_strat_name(), LCD_STRAT_NAME_LENGTH);
			}

			if(lastMatchTime != global.match_time/1000){
				LCD_OVER_UART_setText(idMatchTime, "%02d", (Uint16)(global.match_time/1000));
				lastMatchTime = global.match_time/1000;
			}

			if(lastStateSD != SD_isOK() || lastIdSD != SD_get_match_id()){
				if(SD_isOK())
					LCD_OVER_UART_setText(idMatch, "SD: %d", SD_get_match_id());
				else
					LCD_OVER_UART_setText(idMatch, "SD: error");

				lastStateSD = SD_isOK();
				lastIdSD = SD_get_match_id();
			}

			if(lastMemoryState != global.prop.prop_memory_startup_check_state){
				if(global.prop.prop_memory_startup_check_state){
					LCD_OVER_UART_setText(idMemoryState, "EEPROM:OK");
					LCD_OVER_UART_setTextColor(idMemoryState, LCD_COLOR_BLACK);
				}else{
					LCD_OVER_UART_setText(idMemoryState, "EEPROM:KO");
					LCD_OVER_UART_setTextColor(idMemoryState, LCD_COLOR_RED);
				}

				lastMemoryState = global.prop.prop_memory_startup_check_state;
			}

			Uint8 i;
			for(i=0; i<4; i++){
				if(i < 2){
					if(lastFoe[i].dist != global.foe[i].dist || lastFoe[i].angle != global.foe[i].angle || lastFoe[i].enable != global.foe[i].enable){

						if(global.foe[MAX_HOKUYO_FOES+i].enable)
							LCD_OVER_UART_setText(idAdv[i], "advH%d dist : %4d  |  angle : %3ld", i, global.foe[i].dist, ((Sint32)global.foe[i].angle)*180/PI4096);
						else
							LCD_OVER_UART_setText(idAdv[i], "advH%d obsolate", i);

						lastFoe[i].dist = global.foe[i].dist;
						lastFoe[i].angle = global.foe[i].angle;
						lastFoe[i].enable = global.foe[i].enable;
					}
				}else{
					if(lastFoe[i].dist != global.foe[MAX_HOKUYO_FOES+i-2].dist || lastFoe[i].angle != global.foe[MAX_HOKUYO_FOES+i-2].angle || lastFoe[i].fiability_error != global.foe[MAX_HOKUYO_FOES+i-2].fiability_error || lastFoe[i].enable != global.foe[MAX_HOKUYO_FOES+i-2].enable){

						if(global.foe[MAX_HOKUYO_FOES+i-2].enable)
							LCD_OVER_UART_setText(idAdv[i], "advIR%d dist : %4d  |  angle : %3ld", i-2, global.foe[MAX_HOKUYO_FOES+i-2].dist, ((Sint32)global.foe[MAX_HOKUYO_FOES+i-2].angle)*180/PI4096);
						else if(global.foe[MAX_HOKUYO_FOES+i-2].fiability_error)
							LCD_OVER_UART_setText(idAdv[i], "advIR%d erreur %d", i-2, global.foe[MAX_HOKUYO_FOES+i-2].fiability_error);
						else
							LCD_OVER_UART_setText(idAdv[i], "advIR%d obsolate", i-2);

						lastFoe[i].dist = global.foe[MAX_HOKUYO_FOES+i-2].dist;
						lastFoe[i].angle = global.foe[MAX_HOKUYO_FOES+i-2].angle;
						lastFoe[i].fiability_error = global.foe[MAX_HOKUYO_FOES+i-2].fiability_error;
						lastFoe[i].enable = global.foe[MAX_HOKUYO_FOES+i-2].enable;
					}
				}
			}
		}

		if(!changeMenuSelftest && lastChangeMenuSelftest){
			return MENU_SELFTEST;
		}
		lastChangeMenuSelftest = changeMenuSelftest;

		if(!changeMenuCheckList && lastChangeMenuCheckList){
			return MENU_CHECK_LIST;
		}
		lastChangeMenuCheckList = changeMenuCheckList;

		if(!changeMenuStrat && lastChangeMenuStrat){
			return MENU_STRAT;
		}
		lastChangeMenuStrat = changeMenuStrat;

		return MENU_MAIN;
	}

	static LCD_state_e LCD_MENU_selftest(bool_e init){

		CREATE_MAE(
				INIT,
				WAIT_LAUNCH_SELFTEST,
				WAIT_RESULT,
				DISPLAY_RESULT);

		static bool_e exit = FALSE;
		static bool_e launchSelftest = FALSE;
		static bool_e up = FALSE, lastUp = FALSE;
		static bool_e down = FALSE, lastDown = FALSE;

		static LCD_objectId_t idText[SELFTEST_NB_DISPLAYED_LINE], idSelftestProgress;

		static Uint8 ptrError = 0;

		static Uint8 selftestProgressBar = 0;
		static Uint8 lastSelftestProgressState = 0;

		bool_e refreshDisplay = FALSE;
		Uint8 i;

		if(init){
			exit = FALSE;
			launchSelftest = FALSE;
			up = FALSE;
			lastUp = FALSE;
			down = FALSE;
			lastDown = FALSE;

			RESET_MAE();
		}

		if(entrance)
			LCD_OVER_UART_resetScreen();

		switch(state){
			case INIT:
				for(i=0; i<SELFTEST_NB_DISPLAYED_LINE; i++){
					idText[i] = LCD_OBJECT_ID_ERROR_FULL;
				}
				state = WAIT_LAUNCH_SELFTEST;
				break;

			case WAIT_LAUNCH_SELFTEST:
				if(entrance){
					LCD_OVER_UART_addText(10, 10, LCD_COLOR_BLACK, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_11x18, "Selftest");
					LCD_OVER_UART_addButton(100, 100, 0, 0, FALSE, &launchSelftest, LCD_COLOR_BLACK, LCD_COLOR_GREEN, LCD_COLOR_RED, LCD_COLOR_BLACK, LCD_TEXT_FONTS_11x18, "Lancer");
					LCD_OVER_UART_addButton(250, 10, 0, 0, FALSE, &exit, LCD_COLOR_BLACK, LCD_COLOR_GREEN, LCD_COLOR_RED, LCD_COLOR_BLACK, LCD_TEXT_FONTS_11x18, "Exit");
				}

				if(launchSelftest){
					SELFTEST_ask_launch();
					state = WAIT_RESULT;
				}
				break;

			case WAIT_RESULT:
				if(entrance){
					selftestProgressBar = 0;
					LCD_OVER_UART_addText(10, 10, LCD_COLOR_BLACK, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_11x18, "Selftest");
					LCD_OVER_UART_addButton(250, 10, 0, 0, FALSE, &exit, LCD_COLOR_BLACK, LCD_COLOR_GREEN, LCD_COLOR_RED, LCD_COLOR_BLACK, LCD_TEXT_FONTS_11x18, "Exit");
					LCD_OVER_UART_addAnimatedImage(140, 80, LCD_ANIMATION_WAIT_CIRCLE);
					LCD_OVER_UART_addProgressBar(60, 130, 200, 20, LCD_OBJECT_HORIZONTAL_L_2_R, &selftestProgressBar);
					idSelftestProgress = LCD_OVER_UART_addText(90, 165, LCD_COLOR_BLACK, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_11x18, "%s", SELFTEST_get_progress_state_char());
				}

				if(SELFTEST_get_progress_state() != lastSelftestProgressState){
					lastSelftestProgressState = SELFTEST_get_progress_state();
					selftestProgressBar = (lastSelftestProgressState * 100) / SELFTEST_PROGRESS_NUMBER;
					LCD_OVER_UART_setText(idSelftestProgress, "%s", SELFTEST_get_progress_state_char());
				}

				if(SELFTEST_is_over()){
					state = DISPLAY_RESULT;
				}
				break;

			case DISPLAY_RESULT:
				if(entrance){
					LCD_OVER_UART_addText(10, 10, LCD_COLOR_BLACK, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_11x18, "Selftest %d erreurs", SELFTEST_get_errors_number());
					LCD_OVER_UART_addButton(250, 10, 0, 0, FALSE, &exit, LCD_COLOR_BLACK, LCD_COLOR_GREEN, LCD_COLOR_RED, LCD_COLOR_BLACK, LCD_TEXT_FONTS_11x18, "Exit");

					if(SELFTEST_get_errors_number() > SELFTEST_NB_DISPLAYED_LINE){
						LCD_OVER_UART_addButton(250, 60, 60, 60, FALSE, &up, LCD_COLOR_BLACK, LCD_COLOR_GREEN, LCD_COLOR_RED, LCD_COLOR_BLACK, LCD_TEXT_FONTS_11x18, "up");
						LCD_OVER_UART_addButton(250, 140, 60, 60, FALSE, &down, LCD_COLOR_BLACK, LCD_COLOR_GREEN, LCD_COLOR_RED, LCD_COLOR_BLACK, LCD_TEXT_FONTS_11x18, "down");
					}

					for(i = 0; i < SELFTEST_NB_DISPLAYED_LINE; i++){
						if(i < SELFTEST_get_errors_number())
							idText[i] = LCD_OVER_UART_addText(20, 40 + 15*i, LCD_COLOR_BLACK, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_7x10, "%s", SELFTEST_getError_string(SELFTEST_getError(i)));
					}
				}

				if(down && !lastDown){
					if(ptrError < SELFTEST_get_errors_number() - SELFTEST_NB_DISPLAYED_LINE)
						ptrError++;
					refreshDisplay = TRUE;
				}

				if(up && !lastUp){
					if(ptrError != 0)
						ptrError--;

					refreshDisplay = TRUE;
				}


				if(refreshDisplay){
					for(i = ptrError; i < ptrError + SELFTEST_NB_DISPLAYED_LINE; i++){
						LCD_OVER_UART_setText(idText[i - ptrError], "%s", SELFTEST_getError_string(SELFTEST_getError(i)));
					}
					refreshDisplay = FALSE;
				}

				lastDown = down;
				lastUp = up;
				break;

			default:
				RESET_MAE();
				break;
		}

		if(exit){
			RESET_MAE();
			return MENU_MAIN;
		}

		return MENU_SELFTEST;
	}



	static LCD_state_e LCD_MENU_checkList(bool_e init){
		CREATE_MAE(
			INIT,
			DISPLAY_CHECK);

		static bool_e exit = FALSE;
		static bool_e prec = FALSE, lastPrec = FALSE, next = FALSE, lastNext = FALSE;
		static LCD_objectId_t checkListText = LCD_OBJECT_ID_ERROR_FULL;

		const char * checkListString[] = {
				"Check packix ordre actionneur",
				"Check conflit cable",
				"Check bonne position des actionnneurs",
				"Check balle/cube dans les robots",
				"Check high level strategie",
				"Nettoyer les capteurs couleurs",
				"Check les balises IR",
				"Check l'hokuyo",
				"Lancer le selftest",
				"Check la tension batterie > 24",
				"Check switch (actionneur et strategie)",
				"Check propreté des roues",
				"Check asservissement du robot",
				"Placer les biroutes"
		};

		const Uint8 checkListStringSize = sizeof(checkListString) / sizeof(const char *);

		static Uint8 index = 0, lastIndex = 0;

		if(init){
			exit = FALSE;
			checkListText = LCD_OBJECT_ID_ERROR_FULL;

			RESET_MAE();
		}

		switch(state){
			case INIT:
				state = DISPLAY_CHECK;
				break;

			case DISPLAY_CHECK:
				if(entrance){
					LCD_OVER_UART_addText(10, 10, LCD_COLOR_BLACK, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_11x18, "Check list");
					LCD_OVER_UART_addButton(250, 10, 0, 0, FALSE, &exit, LCD_COLOR_BLACK, LCD_COLOR_GREEN, LCD_COLOR_RED, LCD_COLOR_BLACK, LCD_TEXT_FONTS_11x18, "Exit");
					LCD_OVER_UART_addButton(10, 150, 0, 0, FALSE, &prec, LCD_COLOR_BLACK, LCD_COLOR_GREEN, LCD_COLOR_RED, LCD_COLOR_BLACK, LCD_TEXT_FONTS_11x18, "Precedent");
					LCD_OVER_UART_addButton(150, 150, 0, 0, FALSE, &next, LCD_COLOR_BLACK, LCD_COLOR_GREEN, LCD_COLOR_RED, LCD_COLOR_BLACK, LCD_TEXT_FONTS_11x18, "Suivant");
					checkListText = LCD_OVER_UART_addText(10, 80, LCD_COLOR_BLACK, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_7x10, "n°%d : %s", index, checkListString[index]);
				}

				if(lastNext == TRUE && next == FALSE){
					if(index < checkListStringSize-1)
						index++;
					else
						index = 0;
				}

				if(lastPrec == TRUE && prec == FALSE){
					if(index > 0)
						index--;
					else
						index = checkListStringSize-1;
				}

				if(lastIndex != index){
					LCD_OVER_UART_setText(checkListText, "n°%d : %s", index, checkListString[index]);
				}

				lastPrec = prec;
				lastNext = next;
				lastIndex = index;
				break;

			default:
				RESET_MAE();
				break;
		}

		if(exit){
			RESET_MAE();
			return MENU_MAIN;
		}

		return MENU_CHECK_LIST;
	}


	void LCD_setOdometryCoef(Sint32 coef){
		odometryCoef = coef;
	}

	void LCD_setOdometryError(Sint32 error){
		odometryError = error;
	}

	void LCD_setOdometryRound(Uint8 round){
		odometryRound = round;
	}

	void LCD_setOdometrySuccess(bool_e success){
		odometrySuccess = success;
	}

	void LCD_setOdometryWriteState(bool_e state){
		odometryWriteStateAnswer = TRUE;
		odometryWriteState = state;
	}

	static LCD_state_e LCD_MENU_strat(bool_e init){
		CREATE_MAE(
			INIT,
			DISPLAY_CHECK
			);


		static bool_e buttonTranslation, buttonRotation, buttonSymetry, buttonExit,buttonStratInutile;

		if(init){
			buttonTranslation = FALSE;
			buttonRotation = FALSE;
			buttonSymetry = FALSE;
			buttonExit = FALSE;
			buttonStratInutile=FALSE;
			RESET_MAE();
		}

		switch(state){
			case INIT:
				state = DISPLAY_CHECK;
				break;

			case DISPLAY_CHECK:
				if(entrance){
					LCD_OVER_UART_addText(10, 10, LCD_COLOR_BLACK, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_11x18, "Menu strat");
					LCD_OVER_UART_addButton(250, 10, 0, 0, FALSE, &buttonExit, LCD_COLOR_BLACK, LCD_COLOR_GREEN, LCD_COLOR_RED, LCD_COLOR_BLACK, LCD_TEXT_FONTS_11x18, "Exit");
					LCD_OVER_UART_addButton(75, 60, 0, 0, FALSE, &buttonStratInutile, LCD_COLOR_BLACK, LCD_COLOR_GREEN, LCD_COLOR_GREEN, LCD_COLOR_BLACK, LCD_TEXT_FONTS_11x18, "Strat inutile");
					LCD_OVER_UART_addButton(80, 100, 0, 0, FALSE, &buttonTranslation, LCD_COLOR_BLACK, LCD_COLOR_GREEN, LCD_COLOR_GREEN, LCD_COLOR_BLACK, LCD_TEXT_FONTS_11x18, "Translation");
					LCD_OVER_UART_addButton(95, 140, 0, 0, FALSE, &buttonRotation, LCD_COLOR_BLACK, LCD_COLOR_GREEN, LCD_COLOR_GREEN, LCD_COLOR_BLACK, LCD_TEXT_FONTS_11x18, "Rotation");
					LCD_OVER_UART_addButton(95, 180, 0, 0, FALSE, &buttonSymetry, LCD_COLOR_BLACK, LCD_COLOR_GREEN, LCD_COLOR_GREEN, LCD_COLOR_BLACK, LCD_TEXT_FONTS_11x18, "Symetrie");
				}
				break;

			default:
				RESET_MAE();
				break;
		}
		if (buttonStratInutile){
			RESET_MAE();
			return MENU_STRAT_INUTILE;
		}
		if(buttonExit){
			RESET_MAE();
			return MENU_MAIN;
		}

		if(buttonTranslation){
			RESET_MAE();
			odometryType = LCD_ODOMETRY_TRANSLATION;
			return MENU_ODOMETRY_CALIBRATION;
		}

		if(buttonRotation){
			RESET_MAE();
			odometryType = LCD_ODOMETRY_ROTATION;
			return MENU_ODOMETRY_CALIBRATION;
		}

		if(buttonSymetry){
			RESET_MAE();
			odometryType = LCD_ODOMETRY_SYMETRY;
			return MENU_ODOMETRY_CALIBRATION;
		}
		if (buttonStratInutile){
			RESET_MAE();
			return MENU_STRAT_INUTILE;
		}

		return MENU_STRAT;
	}

	static LCD_state_e LCD_MENU_strat_inutile(bool_e init){
		CREATE_MAE(INIT,
					DISPLAY_CHECK,
					DOWN,
					UP,
					DONE);

		static bool_e strats_buttons[LCD_NB_STRATS_PER_PAGE];
		static bool_e buttonDown = FALSE, buttonUp = FALSE, buttonExit = FALSE;
		static uint16_t nb_strats, i;
		static Uint8 page = 0;
		static LCD_objectId_t buttonsId[LCD_NB_STRATS_PER_PAGE];
		Uint16 strat_index = 0;

		switch(state){
			case INIT:
				nb_strats = BRAIN_get_number_of_displayed_strategy();
				LCD_OVER_UART_addText(10, 10, LCD_COLOR_BLACK, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_11x18, "Menu strat Inutile");
				for (i = 0; i < LCD_NB_STRATS_PER_PAGE; i++){
					strat_index = page * LCD_NB_STRATS_PER_PAGE + i;
					if(strat_index < nb_strats) {
						buttonsId[i] = LCD_OVER_UART_addButton(10, 50 + i * 30, 0, 0, FALSE, &strats_buttons[i], LCD_COLOR_BLACK, LCD_COLOR_GREEN, LCD_COLOR_RED, LCD_COLOR_BLACK, LCD_TEXT_FONTS_11x18, BRAIN_get_displayed_strat_name(strat_index));
					}
				}
				LCD_OVER_UART_addButton(250, 10, 0, 0, FALSE, &buttonExit, LCD_COLOR_BLACK, LCD_COLOR_GREEN, LCD_COLOR_RED, LCD_COLOR_BLACK, LCD_TEXT_FONTS_11x18, "Exit");
				LCD_OVER_UART_addButton(255, 90, 55, 50, FALSE, &buttonUp, LCD_COLOR_BLACK, LCD_COLOR_GREEN, LCD_COLOR_RED, LCD_COLOR_BLACK, LCD_TEXT_FONTS_11x18, "up");
				LCD_OVER_UART_addButton(255, 160, 55, 50, FALSE, &buttonDown, LCD_COLOR_BLACK, LCD_COLOR_GREEN, LCD_COLOR_RED, LCD_COLOR_BLACK, LCD_TEXT_FONTS_11x18, "down");
				state = DISPLAY_CHECK;
				break;

			case DISPLAY_CHECK:
				if(entrance){
					for (i = 0; i < LCD_NB_STRATS_PER_PAGE; i++){
						strat_index = page * LCD_NB_STRATS_PER_PAGE + i;
						if(strat_index < nb_strats) {
							LCD_OVER_UART_setText(buttonsId[i], BRAIN_get_displayed_strat_name(strat_index));
						} else {
							LCD_OVER_UART_setText(buttonsId[i], " ");
						}
					}
				}

				if (buttonDown && page < (nb_strats / LCD_NB_STRATS_PER_PAGE)) {
					state = DOWN;
				} else if (buttonUp && page > 0) {
					state = UP;
				} else if (buttonExit) {
					state = DONE;
				}

				for (i = 0; i < LCD_NB_STRATS_PER_PAGE; i++) {
					if(strats_buttons[i]) {
						strats_buttons[i]= FALSE;
						strat_index = page * LCD_NB_STRATS_PER_PAGE + i;
						if(strat_index < nb_strats) {
							BRAIN_set_strategy_index(strat_index);
						}
						state = DONE;
					}
				}
				break;

			case DOWN:
				if(page < (nb_strats / LCD_NB_STRATS_PER_PAGE)) {
					page++;
				}
				buttonDown = FALSE;
				state = DISPLAY_CHECK;
				break;

			case UP:
				if(page > 0) {
					page--;
				}
				buttonUp = FALSE;
				state = DISPLAY_CHECK;
				break;

			case DONE:
				RESET_MAE();
				return MENU_MAIN;
				break;

			default:
				RESET_MAE();
				break;
		}

		return MENU_STRAT_INUTILE;
	}

	static LCD_state_e LCD_MENU_odometryCalibration(bool_e init){
		CREATE_MAE(
			INIT,
			DISPLAY_CHECK
			);

		static Sint32 lastOdometryCoef;
		static Sint32 lastOdometryError;
		static Uint8 lastOdometryRound;

		static bool_e buttonExit;
		static bool_e buttonWriteOnMemory;
		static bool_e successDisplayed;
		static bool_e stateWriteDisplayed;
		static bool_e coefWrited;
		static LCD_objectId_t odometryCoefTextId, odometryErrorTextId, odometryRoundTextId;

		if(init){
			buttonWriteOnMemory = FALSE;
			buttonExit = FALSE;
			successDisplayed = FALSE;
			stateWriteDisplayed = FALSE;
			coefWrited = FALSE;
			RESET_MAE();
		}

		switch(state){
			case INIT:
				if(odometryType == LCD_ODOMETRY_ROTATION){
					BRAIN_set_strategy_index(1);
				}else if(odometryType == LCD_ODOMETRY_TRANSLATION){
					BRAIN_set_strategy_index(2);
				}else{
					BRAIN_set_strategy_index(3);
				}

				BRAIN_start();
				state = DISPLAY_CHECK;
				break;

			case DISPLAY_CHECK:
				if(entrance){
					LCD_OVER_UART_addButton(250, 200, 0, 0, FALSE, &buttonExit, LCD_COLOR_BLACK, LCD_COLOR_GREEN, LCD_COLOR_RED, LCD_COLOR_BLACK, LCD_TEXT_FONTS_11x18, "Exit");

					if(odometryType == LCD_ODOMETRY_TRANSLATION){
						LCD_OVER_UART_addText(10, 10, LCD_COLOR_BLACK, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_11x18, "Odometrie translation");
						odometryCoefTextId = LCD_OVER_UART_addText(10, 60, LCD_COLOR_BLACK, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_11x18, "Coefficient actuel : %lx", odometryCoef);
						odometryErrorTextId = LCD_OVER_UART_addText(10, 100, LCD_COLOR_BLACK, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_11x18, "Erreur actuel : %ld [mm]", odometryError);

					}else if(odometryType == LCD_ODOMETRY_ROTATION){
						LCD_OVER_UART_addText(10, 10, LCD_COLOR_BLACK, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_11x18, "Odometrie rotation");
						odometryCoefTextId = LCD_OVER_UART_addText(10, 60, LCD_COLOR_BLACK, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_11x18, "Coefficient actuel : %lx", odometryCoef);
						odometryErrorTextId = LCD_OVER_UART_addText(10, 100, LCD_COLOR_BLACK, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_11x18, "Erreur actuel : %ld [PI4096]", odometryError);
						odometryRoundTextId = LCD_OVER_UART_addText(10, 140, LCD_COLOR_BLACK, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_11x18, "Niveau de calibration : %d", odometryRound);

					}else{
						LCD_OVER_UART_addText(10, 10, LCD_COLOR_BLACK, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_11x18, "Odometrie symetrie");
						odometryCoefTextId = LCD_OVER_UART_addText(10, 60, LCD_COLOR_BLACK, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_11x18, "Coefficient actuel : %lx", odometryCoef);
						odometryErrorTextId = LCD_OVER_UART_addText(10, 100, LCD_COLOR_BLACK, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_11x18, "Erreur actuel : %ld [PI4096]", odometryError);

					}

					lastOdometryCoef = odometryCoef;
					lastOdometryError = odometryError;
					lastOdometryRound = odometryRound;
				}

				if(lastOdometryCoef != odometryCoef){
					LCD_OVER_UART_setText(odometryCoefTextId, "Coefficient actuel : %lx", odometryCoef);
					lastOdometryCoef = odometryCoef;
				}

				if(lastOdometryError != odometryError){
					if(odometryType == LCD_ODOMETRY_TRANSLATION){
						LCD_OVER_UART_setText(odometryErrorTextId, "Erreur actuel : %ld [mm]", odometryError);
					}else if(odometryType == LCD_ODOMETRY_ROTATION){
						LCD_OVER_UART_setText(odometryErrorTextId, "Erreur actuel : %ld [PI4096]", odometryError);
					}else{
						LCD_OVER_UART_setText(odometryErrorTextId, "Erreur actuel : %ld [PI4096]", odometryError);
					}
					lastOdometryError = odometryError;
				}

				if(lastOdometryRound != odometryRound){
					LCD_OVER_UART_setText(odometryRoundTextId, "Niveau de calibration : %d", odometryRound);
					lastOdometryRound = odometryRound;
				}

				if(successDisplayed == FALSE && odometrySuccess){
					successDisplayed = TRUE;
					if(odometryType == LCD_ODOMETRY_ROTATION){
						LCD_OVER_UART_deleteObject(odometryRoundTextId);
					}
					LCD_OVER_UART_addText(10, 175, LCD_COLOR_RED, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_16x26, "Succes !");
					LCD_OVER_UART_addButton(70, 130, 0, 0, FALSE, &buttonWriteOnMemory, LCD_COLOR_BLACK, LCD_COLOR_GREEN, LCD_COLOR_RED, LCD_COLOR_BLACK, LCD_TEXT_FONTS_16x26, "Sauvegarder");
				}

				if(coefWrited == FALSE && buttonWriteOnMemory){
					coefWrited = TRUE;
					stratAction_memoryWriteCoef();
				}

				if(stateWriteDisplayed == FALSE && odometryWriteStateAnswer){
					if(odometryWriteState){
						LCD_OVER_UART_addText(10, 210, LCD_COLOR_RED, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_11x18, "Reussite sauvegarde");
					}else{
						LCD_OVER_UART_addText(10, 210, LCD_COLOR_RED, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_11x18, "Echec sauvegarde");
					}
				}


				break;

			default:
				RESET_MAE();
				break;
		}

		if(buttonExit){
			RESET_MAE();
			return MENU_STRAT;
		}

		return MENU_ODOMETRY_CALIBRATION;
	}

	static LCD_state_e LCD_MENU_debug(bool_e init){
		CREATE_MAE(
			INIT,
			DISPLAY_CHECK
			);

		static bool_e buttonExit;

		if(init){
			buttonExit = FALSE;
			RESET_MAE();
		}

		switch(state){
			case INIT:
				state = DISPLAY_CHECK;
				break;

			case DISPLAY_CHECK:
				if(entrance){
					LCD_OVER_UART_addButton(250, 10, 0, 0, FALSE, &buttonExit, LCD_COLOR_BLACK, LCD_COLOR_GREEN, LCD_COLOR_RED, LCD_COLOR_BLACK, LCD_TEXT_FONTS_11x18, "Exit");
					Uint8 i;
					for(i=0; i<LCD_DEBUG_STRING_COUNT; i++){
						debugString[i].id = LCD_OVER_UART_addText(10, 10 + 10 * i, LCD_COLOR_BLACK, LCD_COLOR_TRANSPARENT, LCD_TEXT_FONTS_7x10, debugString[i].string);
					}
				}

				Uint8 i;
				for(i=0; i<LCD_DEBUG_STRING_COUNT; i++){
					if(debugString[i].stringChanged){
						LCD_OVER_UART_setText(debugString[i].id, debugString[i].string);
					}
				}

				break;

			default:
				RESET_MAE();
				break;
		}

		if(buttonExit){
			RESET_MAE();
			return MENU_MAIN;
		}

		return MENU_DEBUG;
	}

	void LCD_printf(Uint8 line, bool_e switch_on_menu, bool_e log_on_sd, char * chaine, ...){
		if(line >= LCD_DEBUG_STRING_COUNT)
			return;

		if(switch_on_menu)
			LCD_setDebugMenu();

		va_list args_list;
		va_start(args_list, chaine);
		vsnprintf(debugString[line].string, LCD_DEBUG_STRING_LENGTH, chaine, args_list);
		va_end(args_list);

		debugString[line].stringChanged = TRUE;

		debug_printf("LCD_printf(%d) : %s\n", line, debugString[line].string);

		if(log_on_sd){
			SD_printf("LCD_printf(%d) : %s\n", line, debugString[line].string);
		}
	}
#endif
