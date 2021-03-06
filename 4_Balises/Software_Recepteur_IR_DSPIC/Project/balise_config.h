/*
 *	Club Robot ESEO 2009 - 2010
 *	
 *
 *	Fichier : balise_config.h
 *	Package : Balises 2010
 *	Description : configuration du code de la balise
 *	Auteur : Nirgal
 *	Version 200907
 */

#ifndef BALISE_CONFIG_H
	#define BALISE_CONFIG_H
	

/////////////////////////////////COMMUNICATION//////////////////////////////////////
	
	//#define VERBOSE_CAN_OVER_UART1	//Envoyer le contenu des messages can sur l'uart, avec une belle pr�sentation..
	
	//#define CAN_SEND_OVER_UART1		//Envoyer les messages can sur l'uart 1.

	#define VERBOSE_MODE				//Activation des debug_printf()

	//#define DEBUG_TSOP_MATLAB			// Activation de la visualisation des TSOP/Angle/Distance en temps r�el sous matlab veill� � bien enlever les autres debug_printf

	#define USE_CAN
	#define CAN_BUF_SIZE		4	//	Nombre de messages CAN conserv�s pour traitement hors interuption
	#define QS_CAN_RX_IT_PRI CAN_INT_PRI_5	//Priorit� du CAN baiss�e volontairement.

	#define USE_UART1
	#define USE_UART1RXINTERRUPT
//	#define USE_UART2
//	#define USE_UART2RXINTERRUPT
	#define UART_RX_BUF_SIZE	12	//	Taille de la chaine de caracteres memorisant les caracteres recus sur UART
	

//////////////////////////////////MODES_DE_FONCTIONNEMENT////////////////////////////
	//CHOISIR L'UN de ces modes...
	
//	#define MODE_TEST_DIODES
	//les diodes s'allument chacune leur tour selon le timer 2
	//ce mode peut �tre super fun en fin de match !!!
	
	//#define MODE_RECEPTION_SIMPLE	
	//dans ce mode, aucun buffer n'est pris en compte, 
	//les diodes s'allument simplement lorsque les r�cepteurs recoivent
	
	typedef Uint8 error_t;

				


//////////////////////////////////PARAMETRAGES///////////////////////////////////////	
/////////////////////////////////////////////////////////////////////////////////////		
	#define MINIMUM_DURATION_BETWEEN_TWO_DETECTIONS 11// =1,43ms //[130us] si l'on a rien re�u pendant cette dur�e, on inaugure la d�tection suivante.  Unit� : [TMR3]
	

	//P�riode d'�chantillonage des r�ceptions.
	//Attention, si trop petit le PIC est pas assez rapide pour �x�cuter l'IT...
	//la frequence minimum vitale est 6,66kHz = 150�s...(shannon).... (=2*p�riode de l'�metteur)
	//je choisis environ 7,7kHz	 > 6,66... il fauat �tre sur qu'un �ventuel rapport cyclique !=50% ne nous g�ne pas...
	//Ensuite, on compte chaque changement d'�tat...
	//plus d'infos ? >> samuelp5@gmail.com
	#define	CONFIG_TIMER_ECHANTILLONAGE 130//130		//[�s]	
	
	//Dur�e minimale entre deux envois vers la carte strat�gie...
	#define DUREE_MINI_ENTRE_DEUX_ENVOIS 250	//ms (maximum = 255 !)
	
	//pour info, utilis� dans le code, fig� et impos� par la carte.
	//Le code n'est pas cependant suffisamment g�n�rique pour permette que cette seule modifications soit r�percut�s aux endroits concern�s...
	#define NOMBRE_TSOP 				16	


	#define SEUIL_SIGNAL_TACHE			10	
	//TODO regler !!! > si un r�cepteur � re�u une "puissance" d'au moins ceci, il peut appartenir � la tache
	
	
	#define SEUIL_SIGNAL_INSUFFISANT	4	
	//TODO regler !!! > si aucun r�cepteur n'a recu une "puissance" d'au moins ceci, on d�clare l'�tat d'erreur de signal insuffisant !
	
	
	#define SEUIL_TROP_DE_SIGNAL		300	
	// > quel maximum a t'on au plus pr�s pour un r�cepteur
	//Si un r�cepteur � recu plus que ce signal en un cycle, on l�ve l'�tat d'erreur de TROP_DE_SIGNAL !
	
	
	#define TAILLE_TACHE_MAXIMALE		12
	// > combien de r�cepteurs captent au plus pr�s
	//Si ce nombre de r�cepteur ont recu du signal (ils appartiennent tous � la tache) >> �tat d'erreur lev� !
	
	#define PRODUIT_DISTANCE_EVIT_PUISSANCE_EVIT	10800
	//  = "distance d'�vitement souhait�e" * "PuissanceRe�ue mesur�e � cette distance"
	//Ce coefficient permet la mesure de distance... 
	//La distance �tant d�termin�e par une courbe en 1/P, il faut connaitre le produit Puissance*Distance
	//Pour plus de pr�cision, on d�termine ce produit sur la distance que l'on souhaite la plus pr�cise : la distance d'�vitement !
	

	
	
	
#endif /* ndef BALISE_CONFIG_H */
