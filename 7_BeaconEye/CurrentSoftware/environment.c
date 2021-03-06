#include "environment.h"
#include "IHM/led.h"
#include "IHM/switch.h"
#include "IHM/button.h"
#include "LCD/low layer/ssd2119.h"
#include "IHM/view.h"
#include "IHM/terminal.h"
#include "IHM/buzzer.h"
#include "zone.h"

void ENVIRONMENT_init() {

	// Initialisation des variables globales
	global.flags.match_started = FALSE;
	global.flags.match_over = FALSE;
	global.flags.match_suspended = FALSE;
	global.absolute_time = 0;
	global.match_time = 0;
	global.current_color = BOT_COLOR;

	// Initialisation de l'IHM
	LED_init();
	BUTTON_init();
	SWITCH_init();
}

void ENVIRONMENT_processIt(Uint8 ms) {

	// On met � jour le temps du match si ce dernier est commenc�
	if(global.flags.match_started && !global.flags.match_over && !global.flags.match_suspended) {
		global.match_time++;
	}

	// On regarde si le match est termin�
	if (!global.flags.match_over && !global.flags.match_suspended) {
		if (MATCH_DURATION != 0 && (global.match_time >= (MATCH_DURATION))) {
			global.flags.match_over = TRUE;
		}
	}
}

void ENVIRONMENT_processMain() {
#if defined(USE_BEACON_EYE)
	SWITCH_processMain();
	BUTTON_processMain();
	BUZZER_processMain();
#endif
}

void ENVIRONMENT_setColor(color_e color) {

	// Changement de la couleur
	global.current_color = color;

	// Mise � jour de la led indiquand la couleur
	LED_setColor((color == TOP_COLOR) ? LED_COLOR_YELLOW : LED_COLOR_BLUE);
	TERMINAL_printf("New Color : %s", ((color == TOP_COLOR) ? TOP_COLOR_NAME : BOT_COLOR_NAME));

	// Changement de l'emplacement de la balise sur l'�cran
	VIEW_drawBeaconPosition(color);

	// Mise � jour des zones
	ZONE_cleanAllEvents();
	ZONE_colorChange();
}

color_e ENVIRONMENT_getColor() {
	return global.current_color;
}
