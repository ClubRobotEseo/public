/*
 *	Club Robot ESEO 2012-2015
 *
 *	Fichier : motor.c
 *	Package : Balise �mettrice
 *	Description : Gestion du pilotage du moteur avec asservissement en vitesse.
 *					La vitesse angulaire doit �tre le plus constante possible.
 *	Auteur : Nirgal(PIC)/Arnaud(STM32)
 *	Version 201203
 */

#include "motor.h"
#include "QS/QS_lowLayer/QS_qei.h"
#include "QS/QS_lowLayer/QS_pwm.h"
#include "QS/QS_lowLayer/QS_timer.h"
#include "QS/QS_outputlog.h"
#include "QS/QS_rf.h"
#include "synchro_rf.h"

#ifdef MOTOR_REVERSE
	#define PWM_USE		PWM_REVERSE
	#define PWM_ZERO	PWM_FORWARD
#else
	#define PWM_USE		PWM_FORWARD
	#define PWM_ZERO	PWM_REVERSE
#endif

#define AROUND(x)   ((((x)>0)?((Sint16)(x)+0.5):((Sint16)(x)-0.5)))

#define NB_EMISSION_TOUR	(Sint16)(2)	 // Nombre d'emission par tour
#define KV		 	(Sint32)(8)
#define KP			(Sint32)(32)
#define KD			(Sint32)(4)
#define KI			(Sint32)(2)
#define COMMAND		(Sint32)(AROUND(2048 * (NB_EMISSION_TOUR * 1000. / (PERIODE_FLASH*2 - NO_FLASH_TIME*2))/500.))  // pas par 2ms
								//		 arrondi(2048*	(2 * 1000 / (50*2 - 4*2) )   / 500   )
									  //	 cela donne :  85
										//2048*(nb_tour*1000/temps_emission) pas par secondes
										//(nb_tour*1000/temps_emission) tours par seconde

volatile Uint16 duty_filtered;

#define NB_IT_PER_TURN			(Uint8)(30)	//it = 2ms, 1 tour = 60ms => 30 it par tour.
#define	NB_ENCODER_STEP_PER_IT	(Uint8)(68)

void MOTOR_init(void){
	QEI_init();
	PWM_init();
	duty_filtered = 0;
	PWM_run(0, PWM_MOTOR_TIM, PWM_ZERO);
}

//A appeler toutes les 2 ms.
//ce choix est arbitraire, et tr�s li� au fait que le reste du code utilise �galement avec une IT 2ms
void MOTOR_process_it(Uint8 period_it){
	static Sint16 count_prec = 0;
	static Sint16 error_prec = 0;
	static Sint16 sum_error = 0;
	Sint16 count, error, delta_error;
	Sint16 duty;
	Sint16 speed;

	count = QEI1_get_count();		//Lecture codeur.

	speed = absolute(count - count_prec);	//Calcul vitesse

	error = COMMAND - speed;		//Calcul de l'erreur sur la vitesse

	delta_error = error - error_prec;	//Calcul de la d�riv�e de l'erreur sur la vitesse

	if((Uint32)sum_error + error < 65535)
		sum_error -= (speed < 10)?0:error;	//Calcul de l'int�grale de l'erreur sur la vitesse (avec s�curit� si vitesse faible !)

	//Calcul du rapport cyclique de la PWM � envoyer au moteur.
	duty = (		KV * COMMAND		//85*8 -> 680   -> /16 -> 42 pour 500 -> 8% de PWM
				+ 	KP * error
				+ 	KD * delta_error
				+ 	KI * sum_error
			)/16;

	if(duty <0)	//Ecretage mini
		duty = 1;
	if(duty > 500)
		duty = 500;			//Duty s'exprime de 0 � 500 (0 � 100%)


	PWM_run(duty/5, PWM_MOTOR_TIM, PWM_USE);

	//On sauvegarde pour le prochain tour.
	count_prec = count;
	error_prec = error;

	//duty est en [0 � 500]
	//on veut un nombre de [0 � 100] (c'est le plus simple...et mieux pour la RF) -> on divise par 5.
	//puis, on filtre en prenant 8/128 de la valeur actuelle    +    120/128 de la valeur pr�c�dente.
	duty_filtered = (duty_filtered*120 + duty*8)>>7;

	//V�rification de non d�bordement avec les valeurs max :
					//( 500 * 120    +    ( 500  * 8  )   )		/   128
					//				64000						/	128
					//				c'est bon !
}


Uint8 MOTOR_get_duty_filtered(void){
	return (Uint8)(duty_filtered/5);	//[de 0 � 100]
}

