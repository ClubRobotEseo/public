/*
 *	Club Robot ESEO 2014
 *
 *	Fichier : led.c
 *	Package : Carte IHM
 *	Description : Contr�le les leds
 *	Auteur : Anthony
 *	Version 2012/01/14
 */

#include "led.h"

#include "QS/QS_lowLayer/QS_ports.h"

#define TIME_BLINK_OFF 	0
#define TIME_BLINK_1HZ 	1000
#define TIME_BLINK_4HZ 	250
#define TIME_BLINK_10MS 100

//Le poids faible : �tat de la led GREEN
//Le poids moyen : �tat de la led RED
//Le poids fort : �tat de la led BLUE
#define LED_GREEN	0x01
#define LED_RED		0x02
#define LED_BLUE	0x04


typedef struct{
	blinkLED_e blink; // Temps de clignotement *10 en ms. Si 50, va clignoter toutes les 500ms
	Uint16 counter;
	bool_e stateUp;
}LED_t;

static LED_t leds[LED_NUMBER_IHM];
static led_color_e led_color = LED_COLOR_BLACK;	//Default color.

void set_LED(led_ihm_e led, bool_e stateUP);
Uint16 get_blink_state(blinkLED_e blink);
void write_color_in_color_led(led_color_e led_color);


void LEDS_init(){
	static bool_e initialized = FALSE;

	if(initialized)
		return;

	Uint8 i;
	for(i = 0; i < LED_NUMBER_IHM; i++){
		set_LED(i, FALSE);
		leds[i].stateUp = FALSE;
		leds[i].counter = 0;
		leds[i].blink = OFF;
	}

	LEDS_set_COLOR(LED_COLOR_BLUE);

	initialized = TRUE;
}

void set_LED(led_ihm_e led, bool_e stateUP){
	assert(led >= 0 && led < LED_NUMBER_IHM);

	switch (led) {
#ifdef LED_IHM_OK
		case LED_OK_IHM:
			GPIO_WriteBit(LED_IHM_OK, stateUP);
			break;
#endif
#ifdef LED_IHM_UP
		case LED_UP_IHM:
			GPIO_WriteBit(LED_IHM_UP, stateUP);
			break;
#endif
#ifdef LED_IHM_DOWN
		case LED_DOWN_IHM:
			GPIO_WriteBit(LED_IHM_DOWN, stateUP);
			break;
#endif
#ifdef LED_IHM_SET
		case LED_SET_IHM:
			GPIO_WriteBit(LED_IHM_SET, stateUP);
			break;
#endif
#ifdef LED0_PORT
		case LED_0_IHM:
			GPIO_WriteBit(LED0_PORT, stateUP);
			break;
#endif
#ifdef LED1_PORT
		case LED_1_IHM:
			GPIO_WriteBit(LED1_PORT, stateUP);
			break;
#endif
#ifdef LED2_PORT
		case LED_2_IHM:
			GPIO_WriteBit(LED2_PORT, stateUP);
			break;
#endif
#ifdef LED3_PORT
		case LED_3_IHM:
			GPIO_WriteBit(LED3_PORT, stateUP);
			break;
#endif
#ifdef LED4_PORT
		case LED_4_IHM:
			GPIO_WriteBit(LED4_PORT, stateUP);
			break;
#endif
#ifdef LED5_PORT
		case LED_5_IHM:
			GPIO_WriteBit(LED5_PORT, stateUP);
			break;
#endif
		case LED_COLOR_IHM:
			write_color_in_color_led( (stateUP)?led_color:LED_COLOR_BLACK);
			break;
		default:
			break;
	}
}

void LEDS_process_it(Uint8 ms){
	Uint8 i; // Compteur pour la fr�quence

	for(i=0;i<LED_NUMBER_IHM;i++){
		if(leds[i].blink == OFF) // Si led non active
			continue;

		// Led active
		if(leds[i].blink != ON){ // Different de OFF et different de ON => Forcement l'un des modes de clignotement
			//Ecoulement du temps.
			if(leds[i].counter > ms)
				leds[i].counter-= ms;
			else
				leds[i].counter = 0;

			if(leds[i].counter == 0){
				leds[i].counter = get_blink_state(leds[i].blink);
				leds[i].stateUp = !leds[i].stateUp;
				set_LED(i, leds[i].stateUp);
			}
		}
	}
}

void LEDS_get_msg(CAN_msg_t *msg){
	Uint8 i,id,blink;

	for(i = 0; i < msg->size; i++){
		id = msg->data.ihm_set_led.led[i].id;
		blink = msg->data.ihm_set_led.led[i].blink;

		leds[id].stateUp = (blink == OFF)?FALSE:TRUE;
		leds[id].blink = blink;
		leds[id].counter = get_blink_state(blink);

		set_LED(id, leds[id].stateUp);
	}
}


Uint16 get_blink_state(blinkLED_e blink){
	Uint16 value;

	switch(blink){
		case OFF:
			value = TIME_BLINK_OFF;
			break;
		case BLINK_1HZ:
			value = TIME_BLINK_1HZ;
			break;
		case SPEED_BLINK_4HZ:
			value = TIME_BLINK_4HZ;
			break;
		case FLASH_BLINK_10MS:
			value = TIME_BLINK_10MS;
			break;
		default:
			break;
	}

	return value;
}


void LEDS_set_COLOR(led_color_e color){
	led_color = color;
	set_LED(LED_COLOR_IHM,TRUE);
}

void LEDS_set_error(ihm_error_e ihm_error, bool_e state){
	static ihm_error_e flags = 0;
	CAN_msg_t msg;
	led_ihm_t led;

	if(state)
		flags |= ihm_error;
	else
		flags &= ~ihm_error;

	if(flags){
		led = (led_ihm_t){LED_WARNING, SPEED_BLINK_4HZ};
		msg.sid = IHM_SET_LED;
		msg.size = 1;
		msg.data.ihm_set_led.led[0].id = led.id;
		msg.data.ihm_set_led.led[0].blink = led.blink;
		LEDS_get_msg(&msg);
	}else{
		led = (led_ihm_t){LED_WARNING, OFF};
		msg.sid = IHM_SET_LED;
		msg.size = 1;
		msg.data.ihm_set_led.led[0].id = led.id;
		msg.data.ihm_set_led.led[0].blink = led.blink;
		LEDS_get_msg(&msg);
	}
}



void write_color_in_color_led(led_color_e led_color){

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

	if(led_color & 0b100){
		GPIO_InitStructure.GPIO_Pin = RED_LED_MASK;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_Init(GPIOD, &GPIO_InitStructure);
		GPIO_ResetBits(RED_LED);

	}else{
		GPIO_InitStructure.GPIO_Pin = RED_LED_MASK;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_Init(GPIOD, &GPIO_InitStructure);
	}

	if(led_color & 0b010){
		GPIO_InitStructure.GPIO_Pin = GREEN_LED_MASK;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_Init(GPIOD, &GPIO_InitStructure);
		GPIO_ResetBits(GREEN_LED);

	}else{
		GPIO_InitStructure.GPIO_Pin = GREEN_LED_MASK;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_Init(GPIOD, &GPIO_InitStructure);
	}

	if(led_color & 0b001){
		GPIO_InitStructure.GPIO_Pin = BLUE_LED_MASK;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_Init(GPIOD, &GPIO_InitStructure);
		GPIO_ResetBits(BLUE_LED);

	}else{
		GPIO_InitStructure.GPIO_Pin = BLUE_LED_MASK;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_Init(GPIOD, &GPIO_InitStructure);
	}
}
