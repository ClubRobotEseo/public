#include "interface.h"
#include "middleware.h"
#include "image/image.h"
#include "low layer/lcd_display.h"
#include "../QS/QS_uart.h"
#include "../environment.h"


typedef enum {
	ID_FOE_BALL_PLACED,
	ID_OUR_BALL_PLACED,
	ID_WATER_GETTED,
	ID_BEE_PLACED,
	ID_BURSTED_BALLOON,
	ID_FIRST_STAGE,
	ID_SECOND_STAGE,
	ID_THIRD_STAGE,
	ID_FOURTH_STAGE,
	ID_FIFTH_STAGE,
	ID_WELL_DONE_BUILDING,
	ID_DOMOTIC_PLACED,
	ID_SWITCH_ACTIVATED,
	ID_ESTIMATION,
	NB_ESTIMATION_ID
} estimationId_e;


static volatile INTERFACE_ihm_e actualIhm = INTERFACE_IHM_WAIT;

// Estimation
static objectId_t id_global_estimation, id_match_time;
static objectId_t id_estimation[2][NB_ESTIMATION_ID];
static char * estimation_mapping[NB_ESTIMATION_ID] = {
		"FOE_BALL_PLACED",
		"OUR_BALL_PLACED",
		"WATER_GETTED",
		"BEE_PLACED",
		"BURSTED_BALLOON",
		"FIRST_STAGE",
		"SECOND_STAGE",
		"THIRD_STAGE",
		"FOURTH_STAGE",
		"FIFTH_STAGE",
		"GOOD_BUILDING",
		"DOMOTIC_PLACED",
		"SWITCH_ACTIVATED",
		"TOTAL"
};


// Combinaison de couleurs
static objectId_t id_color_code[3];
static Uint32 color_mapping[NB_CUBE_COLOR] = {
		LCD_DISPLAY_COLOR_YELLOW,	// CUBE_COLOR_YELLOW
		LCD_DISPLAY_COLOR_GREEN,	// CUBE_COLOR_GREEN
		LCD_DISPLAY_COLOR_BLACK,	// CUBE_COLOR_BLACK
		LCD_DISPLAY_COLOR_BLUE,		// CUBE_COLOR_BLUE
		LCD_DISPLAY_COLOR_ORANGE	// CUBE_COLOR_ORANGE
};

// Autres variables privées
static bool_e unused_touch = FALSE;
static time32_t last_match_time;
static time32_t local_time;


static void INTERFACE_IHM_wait(bool_e entrance);
static void INTERFACE_IHM_match(bool_e entrance);
static void INTERFACE_IHM_custom(bool_e entrance);
static void INTERFACE_IHM_user_button(bool_e entrance);

void INTERFACE_init(void){
	last_match_time = 0;
	local_time = global.absolute_time;
	MIDDLEWARE_init();
}

void INTERFACE_processMain(void){
	static INTERFACE_ihm_e lastIhm = -10;
	bool_e entrance = FALSE;


	static bool_e in_user_interface = FALSE;
	if(HAL_GPIO_ReadPin(user_button_GPIO_Port, user_button_Pin) && global.absolute_time > local_time + 500) {
		local_time = global.absolute_time;
		if(in_user_interface) {
			INTERFACE_setInterface(INTERFACE_IHM_MATCH);
			in_user_interface = FALSE;
		} else {
			INTERFACE_setInterface(INTERFACE_IHM_USER_BUTTON);
			in_user_interface = TRUE;
		}
	}

	if(lastIhm != actualIhm)
		entrance = TRUE;

	lastIhm = actualIhm;

	switch(actualIhm){
		case INTERFACE_IHM_WAIT:
			INTERFACE_IHM_wait(entrance);
			break;

		case INTERFACE_IHM_MATCH:
			INTERFACE_IHM_match(entrance);
			break;

		case INTERFACE_IHM_CUSTOM:
			INTERFACE_IHM_custom(entrance);
			break;

		case INTERFACE_IHM_USER_BUTTON:
			INTERFACE_IHM_user_button(entrance);
			break;
	}

	MIDDLEWARE_processMain();
}

void INTERFACE_setInterface(INTERFACE_ihm_e ihm){
	debug_printf("setInterface %d\n", ihm);
	actualIhm = ihm;
	MIDDLEWARE_resetScreen();
}

INTERFACE_ihm_e INTERFACE_getInterface(){
	return actualIhm;
}

static void INTERFACE_IHM_wait(bool_e entrance){
	if(entrance){
		MIDDLEWARE_setBackground(LCD_DISPLAY_Layer_0, LCD_DISPLAY_COLOR_WHITE);
		MIDDLEWARE_addImage(LCD_DISPLAY_Layer_0,
							F0_OFFSET_WINDOW_X + F0_WIDTH/2 - logoESEO.width/2,
							50,
							&logoESEO);
		MIDDLEWARE_addAnimatedImage(LCD_DISPLAY_Layer_0,
							F0_OFFSET_WINDOW_X + F0_WIDTH/2 - waiting.frame[0].width/2,
							350,
							&waiting);
	}

	if(global.flags.communication_available){
		INTERFACE_setInterface(INTERFACE_IHM_MATCH);
	}
}

#define SPACE_SCORE_ITEMS	(25)

static void INTERFACE_IHM_match(bool_e entrance){
	CUBE_color_e * color_combination = NULL;
	bool_e color_combination_available = ENV_get_color_combination(&color_combination);
	ScoreEventCounter event_counter_big = ENV_get_event_counter(BIG_ROBOT);
	Uint8 * event_ptr_big = (Uint8 *) &event_counter_big;
	ScoreEventCounter event_counter_small = ENV_get_event_counter(SMALL_ROBOT);
	Uint8 * event_ptr_small = (Uint8 *) &event_counter_small;
	Uint16 estimation_big = ENV_get_score_estimation(BIG_ROBOT);
	Uint16 estimation_small = ENV_get_score_estimation(SMALL_ROBOT);
	Uint16 estimation_total = ENV_get_total_score_estimation();

	// Création des objets
	if(entrance){

		// Match time
		last_match_time = global.match_time;
		id_match_time = MIDDLEWARE_addText(LCD_DISPLAY_Layer_0, 40, 50, LCD_DISPLAY_COLOR_BLUE, LCD_DISPLAY_TRANSPARENT, TEXT_FONTS_16x26,  "MATCH TIME = %3d", (int) global.match_time);

		// Estimation globale
		id_global_estimation = MIDDLEWARE_addText(LCD_DISPLAY_Layer_0, 40, 140, LCD_DISPLAY_COLOR_RED, LCD_DISPLAY_TRANSPARENT, TEXT_FONTS_16x26,  "ESTIMATION = %3d", estimation_total);
		MIDDLEWARE_addText(LCD_DISPLAY_Layer_0, 20, 180, LCD_DISPLAY_COLOR_BLACK, LCD_DISPLAY_TRANSPARENT, TEXT_FONTS_7x10,  "This estimation does not count for referees.");

		// Combinaison de couleurs
		MIDDLEWARE_addText(LCD_DISPLAY_Layer_0, 80, 250, LCD_DISPLAY_COLOR_BLACK, LCD_DISPLAY_TRANSPARENT, TEXT_FONTS_16x26,  "COMBINATION");
		if(color_combination_available) {
			for(Uint8 i = 0; i < 3; i++) {
				id_color_code[i] = MIDDLEWARE_addRectangle(LCD_DISPLAY_Layer_0, 10 + i * 110, 300, 100, 100, LCD_DISPLAY_COLOR_BLACK, color_mapping[color_combination[i]]);
			}
		} else {
			for(Uint8 i = 0; i < 3; i++) {
				id_color_code[i] = MIDDLEWARE_addRectangle(LCD_DISPLAY_Layer_0, 10 + i * 110, 300, 100, 100, LCD_DISPLAY_COLOR_BLACK, LCD_DISPLAY_COLOR_LIGHT_GRAY);
			}
		}

		// Label des items pour l'estimation
		for(Uint8 i = 0; i < NB_ESTIMATION_ID; i++) {
			MIDDLEWARE_addButton(LCD_DISPLAY_Layer_0, 340, 50 + i * SPACE_SCORE_ITEMS, 120, 20, FALSE, &unused_touch, LCD_DISPLAY_COLOR_BLACK, LCD_DISPLAY_COLOR_GRAY, LCD_DISPLAY_COLOR_GRAY, LCD_DISPLAY_COLOR_BLACK, TEXT_FONTS_7x10, estimation_mapping[i]);
		}

		MIDDLEWARE_addButton(LCD_DISPLAY_Layer_0, 470, 20, 55, 20, FALSE, &unused_touch, LCD_DISPLAY_COLOR_BLACK, LCD_DISPLAY_COLOR_BLUE, LCD_DISPLAY_COLOR_BLUE2, LCD_DISPLAY_COLOR_BLACK, TEXT_FONTS_7x10, "BIG");
		MIDDLEWARE_addButton(LCD_DISPLAY_Layer_0, 535, 20, 55, 20, FALSE, &unused_touch, LCD_DISPLAY_COLOR_BLACK, LCD_DISPLAY_COLOR_RED, LCD_DISPLAY_COLOR_RED, LCD_DISPLAY_COLOR_BLACK, TEXT_FONTS_7x10, "SMALL");

		// Estimation Big
		for(Uint8 i = 0; i < NB_ESTIMATION_ID - 1; i++) {
			id_estimation[BIG_ROBOT][i] = MIDDLEWARE_addButton(LCD_DISPLAY_Layer_0, 470, 50 + i * SPACE_SCORE_ITEMS, 55, 20, FALSE, &unused_touch, LCD_DISPLAY_COLOR_BLACK, LCD_DISPLAY_COLOR_BLUE, LCD_DISPLAY_COLOR_BLUE2, LCD_DISPLAY_COLOR_BLACK, TEXT_FONTS_7x10, "%d", event_ptr_big[i]);
		}
		id_estimation[BIG_ROBOT][ID_ESTIMATION] = MIDDLEWARE_addButton(LCD_DISPLAY_Layer_0, 470, 50 + ID_ESTIMATION * SPACE_SCORE_ITEMS, 55, 20, FALSE, &unused_touch, LCD_DISPLAY_COLOR_BLACK, LCD_DISPLAY_COLOR_BLUE, LCD_DISPLAY_COLOR_BLUE2, LCD_DISPLAY_COLOR_BLACK, TEXT_FONTS_7x10, "%d", estimation_big);

		// Estimation Small
		for(Uint8 i = 0; i < NB_ESTIMATION_ID - 1; i++) {
			id_estimation[SMALL_ROBOT][i] = MIDDLEWARE_addButton(LCD_DISPLAY_Layer_0, 535, 50 + i * SPACE_SCORE_ITEMS, 55, 20, FALSE, &unused_touch, LCD_DISPLAY_COLOR_BLACK, LCD_DISPLAY_COLOR_RED, LCD_DISPLAY_COLOR_RED, LCD_DISPLAY_COLOR_BLACK, TEXT_FONTS_7x10, "%d", event_ptr_small[i]);
		}
		id_estimation[SMALL_ROBOT][ID_ESTIMATION] = MIDDLEWARE_addButton(LCD_DISPLAY_Layer_0, 535, 50 + ID_ESTIMATION * SPACE_SCORE_ITEMS, 55, 20, FALSE, &unused_touch, LCD_DISPLAY_COLOR_BLACK, LCD_DISPLAY_COLOR_RED, LCD_DISPLAY_COLOR_RED, LCD_DISPLAY_COLOR_BLACK, TEXT_FONTS_7x10, "%d", estimation_small);

	}

	// Mise à jour de la combinaison de couleur
	if(color_combination_available && global.flags.color_combination_updated) {
		for(Uint8 i = 0; i < 3; i++) {
			MIDDLEWARE_deleteObject(LCD_DISPLAY_Layer_0, id_color_code[i]);
			id_color_code[i] = MIDDLEWARE_addRectangle(LCD_DISPLAY_Layer_0, 10 + i * 110, 300, 100, 100, LCD_DISPLAY_COLOR_BLACK, color_mapping[color_combination[i]]);
		}
	}

	// Mise à jour du temps de match
	if(last_match_time != global.match_time || global.flags.score_updated){
		last_match_time = global.match_time;
		MIDDLEWARE_setText(LCD_DISPLAY_Layer_0, id_match_time, "MATCH TIME = %3d", (int) global.match_time);
	}

	// Mise à jour du score
	if(global.flags.score_updated) {

		// Mise à jour de l'estimation globale
		MIDDLEWARE_setText(LCD_DISPLAY_Layer_0, id_global_estimation, "ESTIMATION = %3d", estimation_total);

		// Mise à jour du score de big et small
		for(Uint8 i = 0; i < NB_ESTIMATION_ID - 1; i++) {
			MIDDLEWARE_setText(LCD_DISPLAY_Layer_0, id_estimation[BIG_ROBOT][i], "%d", event_ptr_big[i]);
			MIDDLEWARE_setText(LCD_DISPLAY_Layer_0, id_estimation[SMALL_ROBOT][i], "%d", event_ptr_small[i]);
		}
		MIDDLEWARE_setText(LCD_DISPLAY_Layer_0, id_estimation[BIG_ROBOT][ID_ESTIMATION], "%d", estimation_big);
		MIDDLEWARE_setText(LCD_DISPLAY_Layer_0, id_estimation[SMALL_ROBOT][ID_ESTIMATION], "%d", estimation_small);
	}

}

static void INTERFACE_IHM_custom(bool_e entrance){

}

#define SPACE_TEXT_ITEMS  (42)
static void INTERFACE_IHM_user_button(bool_e entrance){
	char * text[] = {
			"    Chere Laura,",
			"Tu es si belle et si charmante.",
			"Tu as si bien arbitre en Belgique.",
			"Tu as les yeux revolver,",
			"tu as le regard qui tue.",
			"Tu as tire la premiere,",
			"m'as touche, c'est foutu.",
			"Appelle-moi au 06 06 46 19 77.",
			"                 Arnaud"
	};
	if(entrance){
		Uint8 size = sizeof(text) / sizeof(char*);
		for(Uint8 i = 0; i < size; i++) {
			MIDDLEWARE_addText(LCD_DISPLAY_Layer_0, 20, 20 + i * SPACE_TEXT_ITEMS , LCD_DISPLAY_COLOR_BLUE, LCD_DISPLAY_TRANSPARENT, TEXT_FONTS_16x26,  text[i]);
		}
	}
}

