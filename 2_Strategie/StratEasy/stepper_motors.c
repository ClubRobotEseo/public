/*
 * stepper_motors.c
 *
 *  Created on: 21 juin 2018
 *      Author: Nirgal
 */
#include "stepper_motors.h"
#include "stm32f4xx.h"
#include "stm32f4xx_tim.h"
#include "QS/QS_stateMachineHelper.h"
#include "odometry.h"
#include "config.h"




#define TIMx					TIM2
#define TIMx_IRQn				TIM2_IRQn
#define RCC_APB1Periph_TIMx		RCC_APB1Periph_TIM2

#define DISABLE_IRQ()			NVIC_DisableIRQ(TIMx_IRQn)
#define	ENABLE_IRQ()			NVIC_EnableIRQ(TIMx_IRQn)
#define	START_TIMER()			TIM_Cmd(TIMx, ENABLE)
#define	STOP_TIMER()			TIM_Cmd(TIMx, DISABLE)
#define	SET_PERIOD(period)		TIMx->ARR = period

#define PULSE_AT_1()			GPIOC->BSRRL = GPIO_Pin_9 | GPIO_Pin_8
#define PULSE_AT_0()			GPIOC->BSRRH = GPIO_Pin_9 | GPIO_Pin_8


#define	LIGHT_SPEED				1000					//vitesse max [mm/s]
#define NB_STEP_TO_REFRESH_POS	320


static bool_e initialized = FALSE;


static uint16_t speed_to_step_duration[SPEED_MAX+1];

static volatile way_e way;
static volatile trajectory_e trajectory;
static volatile int32_t current_step;					//en nombre de pulses
static volatile int32_t odometry_step;
static volatile uint32_t goal;
static volatile uint32_t current_time;
static volatile double current_speed;	//[mm/s]
static volatile uint16_t max_speed;	//[mm/s]
static volatile uint16_t step_duration;
static volatile uint16_t nb_step_to_brake;
static volatile uint32_t time_to_brake;

typedef enum
{
	IDLE_PHASE = 0,
	ACCELERATION_PHASE,
	CONSTANT_SPEED_PHASE,
	BRAKING_PHASE,
	URGENT_BRAKING_PHASE
}trajetory_state_e;

static volatile trajetory_state_e trajectory_state;


static void STEPPER_MOTOR_init_timer(void);

void STEPPER_MOTOR_init(void)
{
	uint32_t i;
	speed_to_step_duration[0] =  (uint32_t)((1000000/NB_MICROSTEP_BY_MM)/INITIAL_SPEED);
	for(i=1;i<=SPEED_MAX+1; i++)
	{
		speed_to_step_duration[i] = (uint32_t)((1000000/NB_MICROSTEP_BY_MM)/i);		//[us]
	}
	STEPPER_MOTOR_init_timer();
	GPIO_WriteBit(MOTOR_ENABLE, 1);
}



void STEPPER_MOTOR_set_dir(trajectory_e trajectory, way_e way)
{
	bool_e left_way;
	bool_e right_way;
	if(trajectory == TRAJECTORY_ROTATION)
	{
		if(way == BACKWARD)	//= CLOCKWISE
		{
			left_way = TRUE;
			right_way = FALSE;
		}
		else	//=TRIGOWISE
		{
			left_way = FALSE;
			right_way = TRUE;
		}
	}
	else
	{
		left_way = (way == BACKWARD)?FALSE:TRUE;
		right_way = left_way;
	}
	GPIO_WriteBit(MOTOR_LEFT_DIR, left_way);
	GPIO_WriteBit(MOTOR_RIGHT_DIR, !right_way);
}

	//un mouvement s'articule en trois temps.
	//1- on demande le mouvement
	//2- on fournit les pulses aux moteurs selon une rampe de vitesse
	//3- à la fin (ou en cas d'arrêt imprévu initialement), on met à jour la position atteinte



//retourne en us par pas l'inverse d'une vitesse en mm/s
static uint16_t inverse(uint16_t speed)
{
	if(speed > SPEED_MAX)
		return speed_to_step_duration[0];
	return speed_to_step_duration[speed];
}




bool_e STEPPER_MOTOR_state_machine(order_t * order)
{
	CREATE_MAE(
			INIT,
			WAIT_ORDER,
			DOING_ORDER,
			COMPUTE);
	bool_e ret;
	ret = FALSE;
	switch(state)
	{
		case INIT:
			STEPPER_MOTOR_init();
			state = WAIT_ORDER;
			break;
		case WAIT_ORDER:
			if(order != NULL && order->nb_step > 0)
			{
				//min_step_duration = 1000000 / order->max_speed;	//la durée mini d'un pas correspond à la vitesse max...
				max_speed = order->max_speed;
				max_speed = MIN(max_speed, SPEED_MAX);
				way = (order->forward)?FORWARD:BACKWARD;
				trajectory = order->trajectory;
				STEPPER_MOTOR_set_dir(trajectory, way);
				goal = order->nb_step;
				state = DOING_ORDER;
			}
			else
				ret = TRUE;	//on a plus rien à faire !
			break;
		case DOING_ORDER:
			if(entrance)
			{
				trajectory_state = ACCELERATION_PHASE;
				current_speed = 0.0;	//on démarre à vitesse nulle
				nb_step_to_brake = 0;
				time_to_brake = 0;
				current_step = 0;
				current_time = 0;
				current_step = 0;
				step_duration = inverse((uint16_t)current_speed);		//[us]
				SET_PERIOD(step_duration);
				START_TIMER();
				odometry_step = 0;
			}

			static int32_t local_odometry_step = 0;
			NVIC_DisableIRQ(TIMx_IRQn);
				//section critique.
				if(trajectory_state == IDLE_PHASE)
				{
					STOP_TIMER();
					TIMx->CNT = 0;
					state = COMPUTE;
				}
				local_odometry_step += odometry_step;
				odometry_step = 0;
			NVIC_EnableIRQ(TIMx_IRQn);

			if(local_odometry_step >= NB_STEP_TO_REFRESH_POS || state == COMPUTE)
			{
				if(trajectory == TRAJECTORY_TRANSLATION)
					ODOMETRY_new_movement(((way==BACKWARD)?(-1):1)*((int64_t)(local_odometry_step)), 0);
				else
					ODOMETRY_new_movement(0,((way==BACKWARD)?(-1):1)*((int64_t)(local_odometry_step)));
				local_odometry_step = 0;
			}

			break;
		case COMPUTE:

			state = WAIT_ORDER;
			ret = TRUE;
			break;
		default:
			break;
	}

	return ret;
}

#define TIMER_US_PRESCALER  (PCLK1_FREQUENCY_HZ / (1000000))

/* Configuation de l'ensemble du bloc timer */
static void STEPPER_MOTOR_init_timer(void)
{
	if (initialized)
		return;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	STOP_TIMER();
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = 0xFFFF;	//Le moins rapide par défaut
	TIM_TimeBaseStructure.TIM_Prescaler = 0xFFFF;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;

	//On s'arrange pour que la clock sur TIM11 soit la même que sur TIM12-14, voir manuel de réference STM32F4xx page 114
#if (PCLK2_FREQUENCY_HZ/PCLK1_FREQUENCY_HZ) != 1 && (PCLK2_FREQUENCY_HZ/PCLK1_FREQUENCY_HZ) != 2 && (PCLK2_FREQUENCY_HZ/PCLK1_FREQUENCY_HZ) != 4
#error "Incorrect HCLK/PCLK1 ratio, must be 1, 2 or 4"
#endif

	TIM_TimeBaseInit(TIMx, &TIM_TimeBaseStructure);
	TIM_UpdateRequestConfig(TIMx, TIM_UpdateSource_Regular);
	TIM_ITConfig(TIMx,  TIM_IT_Update, ENABLE);
	TIM_ClearITPendingBit(TIMx,  TIM_IT_Update);
	TIMx->SR = 0;
	/* NVIC: initialisation des IT timer avec les niveaux de priorité adéquats*/
	NVIC_InitTypeDef NVICInit;
	NVICInit.NVIC_IRQChannelCmd = ENABLE;
	NVICInit.NVIC_IRQChannelSubPriority = 0;
	NVICInit.NVIC_IRQChannelPreemptionPriority = 0;			//très forte priorité !!!
	NVICInit.NVIC_IRQChannel = TIMx_IRQn;
	NVIC_Init(&NVICInit);

	TIM_Cmd(TIMx, DISABLE);
	RCC_ClocksTypeDef clocksSpeed;
	Uint32 prescaler_mul = 1;
	RCC_GetClocksFreq(&clocksSpeed);

	if(clocksSpeed.SYSCLK_Frequency / clocksSpeed.PCLK1_Frequency > 1)
		prescaler_mul = 2;

	TIM_PrescalerConfig(TIMx, (prescaler_mul*TIMER_US_PRESCALER) - 1, TIM_PSCReloadMode_Immediate);
	SET_PERIOD(speed_to_step_duration[0]);

	TIM_SetCounter(TIMx, 0);

	/* Fin de la configuration */
	initialized = TRUE;
}


#define TRACETABSIZE	20000
static uint32_t traceindex = 0;
static uint32_t tracetab[TRACETABSIZE];
volatile static uint32_t nb_delays = 0;
volatile static uint32_t nb_overlaps = 0;
//Interrupts management and redirection
//Cette IT n'est activée que si l'on veut provoquer des pulses.
void TIM2_IRQHandler()
{
	do{
		TIMx->SR = 0;	//Acquittement timer.
		if(trajectory_state == IDLE_PHASE)
		{
			STOP_TIMER();
			TIMx->CNT = 0;
		}
		else
		{
			PULSE_AT_1();	//Pulse à 1

			if(traceindex < TRACETABSIZE)
				tracetab[traceindex++] = current_speed;
			current_time += step_duration;				//[us]
			switch(trajectory_state)
			{
				case ACCELERATION_PHASE:
					current_speed = (double)(ACCELERATION)*(double)(current_time)/1000000.0;		//[mm/sec]
					nb_step_to_brake++;
					if((uint16_t)current_speed >= max_speed)
					{
						current_speed = (double)max_speed;		//ecretage vitesse
						time_to_brake = current_time;
						trajectory_state = CONSTANT_SPEED_PHASE;
					}
					if(current_step+1 >= goal - nb_step_to_brake)//on a atteint la moitié de la traj, on freine !
					{
						time_to_brake = current_time;
						trajectory_state = BRAKING_PHASE;
					}
					break;
				case CONSTANT_SPEED_PHASE:
					if(current_step+1 >= goal - nb_step_to_brake)
						trajectory_state = BRAKING_PHASE;
					break;
				case BRAKING_PHASE:{
					double deltaV;
					deltaV = (double)(ACCELERATION)*(double)(step_duration)/1000000.0;		//[mm/sec]
					if(time_to_brake > step_duration)
						time_to_brake -=  step_duration;
					else
						time_to_brake = 0;
					//current_speed = (double)(ACCELERATION)*(double)(time_to_brake)/1000000.0;		//[mm/sec]
					current_speed -= deltaV;
					if(current_step >= goal)
						trajectory_state = IDLE_PHASE;
					break;}
				case URGENT_BRAKING_PHASE:{
					double deltaV;
					deltaV = (double)(ACCELERATION)*(double)(step_duration)/1000000.0;		//[mm/sec]
					if(deltaV == 0)
						deltaV = 1;
					//on se fiche du nombre de step restant, on freine maintenant !
					current_speed -= deltaV;
					if(current_speed <= (double)(INITIAL_SPEED))
						trajectory_state = IDLE_PHASE;
					break;}
				case IDLE_PHASE:

					break;
				default:
					break;
			}

			if(current_speed <= (double)(INITIAL_SPEED))
				current_speed = 0;
			step_duration = inverse((int16_t)(current_speed+0.5));		//[us]
			/* Calcul en local (on gagne en RAM si on dégage le tableau, mais on passe de 5us à 10us dans l'IT...)
			 if(current_speed>0.5)
				step_duration = (uint32_t)((1000000/NB_MICROSTEP_BY_MM)/(current_speed+0.5));
			else
				step_duration = (uint32_t)((1000000/NB_MICROSTEP_BY_MM)/INITIAL_SPEED);
			 */
			SET_PERIOD(step_duration);


			current_step++;
			odometry_step++;

			//delay (le pulse doit être au moins de 2,2us)
			volatile static uint32_t d;

			if(TIMx->CNT < 3)
			{
				nb_delays++;
				for(d=0;d<20;d++);
			}

			PULSE_AT_0();	//Pulse à 0
		}
		if(TIMx->CNT > step_duration)
		{
			nb_overlaps++;
		}
	}while(TIMx->CNT > step_duration);
}

