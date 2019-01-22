/*
 * power.c
 *
 *  Created on: 15 nov. 2018
 *      Author: Nirgal
 */
#include "power.h"
#include "../QS/QS_lowLayer/QS_adc.h"

static volatile uint32_t Vbat_mv = 0;
static volatile bool_e power = FALSE;
static volatile uint32_t t = 0;

void POWER_process_ms(void)
{
	if(t)
		t--;
}



void POWER_process_main(void)
{
	static uint32_t low_bat = 0;
	uint16_t v;
	if(!t)
	{
		t = 10;	//Lecture de la tension batterie toutes les 10ms.
		v = ADC_getValue(ADC_14);
		Vbat_mv = (((uint32_t)(v))*24000)/1024;
		//Pont diviseur : 4,7k et 33k
		//tension mesurée = Vbat * 4.7/(33+4.7) = 0.124*Vbat.
		//Si Vbat = 25.2V -> 3.14V      -->1023
		//Si Vbat = 24V   -> 2.99V		--> 1023
		//Si Vbat = 18V   -> 2.24V      --> 765

		if(Vbat_mv<17000)	//seuil 17V
		{
			if(power)
				printf("Puissance : OFF\n");
			power = FALSE;
		}
		else
		{
			if(!power)
				printf("Puissance : ON\n");
			power = TRUE;
		}

		if(Vbat_mv>=17000)
		{
			if(Vbat_mv<19000)//seuil de batterie faible
			{
				if(low_bat < 20)
					low_bat++;	//la batterie à l'air faible
				if(low_bat == 20)	//ca fait 20*10ms (2s) qu'on voit que la batterie est faible !
				{
					printf("!!!!!!!!!!!!!!!! Batterie Faible !\n");
					//TODO envoyer l'info à l'IHM.
				}
			}
			else
				low_bat = 0;	//everything is all right.
		}
	}
}


bool_e POWER_get_state(void)
{
	return power;
}

uint32_t POWER_get_vbat_mv(void)
{
	return Vbat_mv;
}

