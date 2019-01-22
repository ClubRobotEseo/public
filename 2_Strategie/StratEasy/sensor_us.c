/*
 * sensor_us.c
 *
 *  Created on: 19 juil. 2018
 *      Author: Nirgal
 */
#include "QS/QS_all.h"
#include "QS/QS_lowLayer/QS_timer.h"

#define TIM_SENSOR_US	TIM12
#define TIMEOUT_SENSOR	60000

typedef enum
{
	SENSOR_US_FRONT_LEFT = 0,
	SENSOR_US_FRONT_RIGHT,
	SENSOR_US_BACK_LEFT,
	SENSOR_US_BACK_RIGHT,
	SENSOR_US_NB
}sensor_us_id_e;

typedef enum
{
	SENSOR_US_EVENT_BEGIN_ECHO,
	SENSOR_US_EVENT_END_ECHO,
	SENSOR_US_EVENT_TIMEOUT_ECHO
}sensor_us_event_e;

typedef enum
{
	SENSOR_US_INIT = 0,
	SENSOR_US_TRIG,
	SENSOR_US_WAIT_BEGIN_ECHO,
	SENSOR_US_WAIT_END_ECHO,
	SENSOR_US_COMPUTE
}sensor_us_state_e;

volatile static sensor_us_state_e state[SENSOR_US_NB] = {SENSOR_US_INIT};
volatile static uint32_t t_trig[SENSOR_US_NB];
volatile static uint32_t t_begin[SENSOR_US_NB];
volatile static uint32_t t_end[SENSOR_US_NB];
volatile static uint32_t distances[SENSOR_US_NB];
volatile static bool_e flag_updated[SENSOR_US_NB] = {FALSE};

volatile static uint32_t t_overflow;

void SENSOR_US_init(void)
{
	//TIMERx_run_us(TIM_SENSOR_US, 0);
	//TIMERx_enableInt(TIM_SENSOR_US);
}

void _T2Interrupt(void)
{
	t_overflow++;
}

uint32_t get_current_us_time(void)
{
	return t_overflow * 65536 + TIM_GetCounter(TIM_SENSOR_US);
}


void sensor_us_irq_event(sensor_us_id_e id, sensor_us_event_e event)
{

	switch(state[id])
	{
		case SENSOR_US_WAIT_BEGIN_ECHO:
			if(event == SENSOR_US_EVENT_BEGIN_ECHO)
			{
				t_begin[id] = get_current_us_time();
				state[id] = SENSOR_US_EVENT_END_ECHO;
			}
			break;
		case SENSOR_US_WAIT_END_ECHO:
			if(event == SENSOR_US_EVENT_END_ECHO)
			{
				t_end[id] = get_current_us_time();
				state[id] = SENSOR_US_COMPUTE;
			}
			break;
		default:
			break;
	}

}




void sensor_us_process_main(void)
{
	sensor_us_id_e s;
	for(s=0;s<SENSOR_US_NB; s++)
	{
		switch(state[s])
		{
			case SENSOR_US_INIT:
				break;
			case SENSOR_US_TRIG:
				t_trig[s] = get_current_us_time();
				break;
			case SENSOR_US_WAIT_BEGIN_ECHO:
				break;	//Nothing (gestion en it)
			case SENSOR_US_WAIT_END_ECHO:
				if(get_current_us_time() - t_trig[s] > TIMEOUT_SENSOR)
					state[s] = SENSOR_US_TRIG;
				break;	//Nothing (gestion en it)
			case SENSOR_US_COMPUTE:
				if(t_end[s] > t_begin[s])
					distances[s] = t_end[s] - t_begin[s];
				else
					distances[s] = 65536 - t_begin[s] + t_end[s];

				//On a une distance en "us d'ultrason"
				//Si l'on considère que le son va à 343,75 m/s (vrai à environ 20°C)
				distances[s] = (11*distances[s]) >> 5;	//[mm]    //  11/32 = 343,75/1000

				flag_updated[s] = TRUE;
				break;
			default:
				break;
		}
	}
}
