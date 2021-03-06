#include "borders_scan.h"

#if SCAN_BORDURE

#include "../QS/QS_objects.h"
#include "../QS/QS_maths.h"
#include "../QS/QS_outputlog.h"
#include "../it.h"

#define NB_DATA_POINTS  50
#define BORDER_SCAN_PSEUDO_VARIANCE_SEUIL        10
#define NB_POINTS_MINI  25
#define DISTANCE_MAX    20
#define NB_IT_ELEMENTARY_POINT		3000
#define NB_IT_ZONE_POINT			3000
#define COS_CARRE_SCALAIRE 0.00030458649//pour une erreur d'angle de 1� entre les droites



typedef struct{
	GEOMETRY_point_t tab[NB_DATA_POINTS];
	float a;
	float b;
	Sint16 xmoy;
	Sint16 ymoy;
	Uint8 nb_points;
	Sint32 num;
	Sint32 den;
	Uint16 time;
}debug_struct_s;

#define NB_RUN_DEBUG			24
static volatile debug_struct_s tabDebug[NB_RUN_DEBUG];
static volatile Uint16 indexDebug = 0;

static volatile bool_e scan_try_proposition = FALSE;

static volatile scan_point_time_t blue_north[NB_DATA_POINTS];
static volatile Uint8 blue_north_index;
static volatile bool_e blue_north_enable;

static volatile scan_point_time_t middle_north[NB_DATA_POINTS];
static volatile Uint8 middle_north_index;
static volatile bool_e middle_north_enable;

static volatile scan_point_time_t yellow_north[NB_DATA_POINTS];
static volatile Uint8 yellow_north_index;
static volatile bool_e yellow_north_enable;

static volatile scan_point_time_t blue_start[NB_DATA_POINTS];
static volatile Uint8 blue_start_index;
static volatile bool_e blue_start_enable;

static volatile scan_point_time_t blue_corner[NB_DATA_POINTS];
static volatile Uint8 blue_corner_index;
static volatile bool_e blue_corner_enable;

static volatile scan_point_time_t yellow_start[NB_DATA_POINTS];
static volatile Uint8 yellow_start_index;
static volatile bool_e yellow_start_enable;

static volatile scan_point_time_t yellow_corner[NB_DATA_POINTS];
static volatile Uint8 yellow_corner_index;
static volatile bool_e yellow_corner_enable;

static volatile scan_point_time_t blue_south[NB_DATA_POINTS];
static volatile Uint8 blue_south_index;
static volatile bool_e blue_south_enable;

static volatile scan_point_time_t yellow_south[NB_DATA_POINTS];
static volatile Uint8 yellow_south_index;
static volatile bool_e yellow_south_enable;

volatile bool_e BORDERS_SCAN_zone_blue_north_enable = FALSE;
volatile bool_e BORDERS_SCAN_zone_middle_north_enable = FALSE;
volatile bool_e BORDERS_SCAN_zone_yellow_north_enable = FALSE;
volatile bool_e BORDERS_SCAN_zone_blue_start_enable = FALSE;
volatile bool_e BORDERS_SCAN_zone_blue_corner_enable = FALSE;
volatile bool_e BORDERS_SCAN_zone_yellow_start_enable = FALSE;
volatile bool_e BORDERS_SCAN_zone_yellow_corner_enable = FALSE;
volatile bool_e BORDERS_SCAN_zone_blue_south_enable = FALSE;
volatile bool_e BORDERS_SCAN_zone_yellow_south_enable = FALSE;

volatile Sint32 xmoy_blue_north = 0;
volatile Sint32 ymoy_blue_north = 0;
volatile Uint16 date_blue_north = 0;
volatile Sint32 xmoy_middle_north = 0;
volatile Sint32 ymoy_middle_north = 0;
volatile Uint16 date_middle_north = 0;
volatile Sint32 xmoy_yellow_north = 0;
volatile Sint32 ymoy_yellow_north = 0;
volatile Uint16 date_yellow_north = 0;
volatile Sint32 xmoy_blue_start = 0;
volatile Sint32 ymoy_blue_start = 0;
volatile Uint16 date_blue_start = 0;
volatile Sint32 xmoy_blue_corner = 0;
volatile Sint32 ymoy_blue_corner = 0;
volatile Uint16 date_blue_corner = 0;
volatile Sint32 xmoy_yellow_start = 0;
volatile Sint32 ymoy_yellow_start = 0;
volatile Uint16 date_yellow_start = 0;
volatile Sint32 xmoy_yellow_corner = 0;
volatile Sint32 ymoy_yellow_corner = 0;
volatile Uint16 date_yellow_corner = 0;
volatile Sint32 xmoy_blue_south = 0;
volatile Sint32 ymoy_blue_south = 0;
volatile Uint16 date_blue_south = 0;
volatile Sint32 xmoy_yellow_south = 0;
volatile Sint32 ymoy_yellow_south = 0;
volatile Uint16 date_yellow_south = 0;


typedef enum{
	BORDER_SCAN_MODE_DEN_X = 0,
	BORDER_SCAN_MODE_DEN_Y,
}border_scan_mode_den_e;


static void BORDERS_SCAN_addPointZone(volatile scan_point_time_t zone[], volatile Uint8 *zone_index, volatile bool_e *zone_enable, GEOMETRY_point_t pos_mesure);
bool_e BORDERS_SCAN_calculeZone(volatile scan_point_time_t zone[], volatile Uint8 *zone_index, volatile bool_e zone_enable, volatile Sint32 * xmoy_border, volatile Sint32 * ymoy_border, volatile Uint16 *date_point, border_scan_mode_den_e mode_den);


scan_zone_e BORDERS_SCAN_treatment(GEOMETRY_point_t pos_mesure){

	if ((pos_mesure.x<20)&&(pos_mesure.y<1050)&&(pos_mesure.y>50)&&(pos_mesure.x>-1000)){
		//pos_mesure.x = 0;
		//pos_mesure.y = 55 + blue_north_index*15;
		//pos_mesure.x = 0.1745 * pos_mesure.y;
		BORDERS_SCAN_addPointZone(blue_north, &blue_north_index, &blue_north_enable, pos_mesure);
		//calculeZone(middle_north, &middle_north_index, middle_north_enable, &angle_blue_north, &xmoy_blue_north, BORDER_SCAN_MODE_DEN_X);
		return BLUE_NORTH;
	}
	else if ((pos_mesure.x<20)&&(pos_mesure.y<1750)&&(pos_mesure.y>1250)&&(pos_mesure.x>-1000)){
		//pos_mesure.x = 0;
		//pos_mesure.y = 1255 + middle_north_index*5;
		//pos_mesure.x = -0.01745 * pos_mesure.y;

		BORDERS_SCAN_addPointZone(middle_north, &middle_north_index, &middle_north_enable, pos_mesure);
		//calculeZone(middle_north, &middle_north_index, middle_north_enable, &angle_middle_north, &xmoy_middle_north, BORDER_SCAN_MODE_DEN_X);
		return MIDDLE_NORTH;
	}
	else if ((pos_mesure.x<20)&&(pos_mesure.y>1950)&&(pos_mesure.y<2950)&&(pos_mesure.x>-1000)){
		//pos_mesure.x = 0;
		//pos_mesure.y = 1955 + yellow_north_index*15;
		//pos_mesure.x = -0.01745 * pos_mesure.y;

		BORDERS_SCAN_addPointZone(yellow_north, &yellow_north_index, &yellow_north_enable, pos_mesure);
		//calculeZone(yellow_north, &yellow_north_index, yellow_north_enable, &angle_yellow_north, &xmoy_yellow_north, BORDER_SCAN_MODE_DEN_X);
		return YELLOW_NORTH;
	}
	else if ((pos_mesure.y<20)&&(pos_mesure.x<580)&&(pos_mesure.x>50)&&(pos_mesure.y>-1000)){
		//pos_mesure.y = 0;
		//pos_mesure.x = 55 + blue_start_index*5;
		//pos_mesure.y = 0.01745 * pos_mesure.x;

		BORDERS_SCAN_addPointZone(blue_start, &blue_start_index, &blue_start_enable, pos_mesure);
		//calculeZone(blue_start, &blue_start_index, blue_start_enable, &angle_blue_start, &ymoy_blue_start, BORDER_SCAN_MODE_DEN_Y);
		return BLUE_START;
	}
	else if ((pos_mesure.y<20)&&(pos_mesure.x<1950)&&(pos_mesure.x>1450)&&(pos_mesure.y>-1000)){
		//pos_mesure.y = 0;
		//pos_mesure.x = 1455 + blue_corner_index*5;
		//pos_mesure.y = 0.01745 * pos_mesure.x;


		BORDERS_SCAN_addPointZone(blue_corner, &blue_corner_index, &blue_corner_enable, pos_mesure);
		//calculeZone(blue_corner, &blue_corner_index, blue_corner_enable, &angle_blue_corner, &ymoy_blue_corner, BORDER_SCAN_MODE_DEN_Y);
		return BLUE_CORNER;
	}
	else if ((pos_mesure.y>2980)&&(pos_mesure.x<580)&&(pos_mesure.x>50)&&(pos_mesure.y<4000)){
		//pos_mesure.y = 3000;
		//pos_mesure.x = 55 + yellow_start_index*5;
		//pos_mesure.y += 0.01745 * pos_mesure.x;
		//pos_mesure.x -= 50;

		BORDERS_SCAN_addPointZone(yellow_start, &yellow_start_index, &yellow_start_enable, pos_mesure);
		//calculeZone(yellow_start, &yellow_start_index, yellow_start_enable, &angle_yellow_start, &ymoy_yellow_start, BORDER_SCAN_MODE_DEN_Y);
		return YELLOW_START;
	}
	else if ((pos_mesure.y>2980)&&(pos_mesure.x<1950)&&(pos_mesure.x>1450)&&(pos_mesure.y<4000)){
		//pos_mesure.y = 3000;
		//pos_mesure.x = 1455 + yellow_corner_index*5;
		//pos_mesure.y += 0.01745 * pos_mesure.x;
		//pos_mesure.x -= 50;

		BORDERS_SCAN_addPointZone(yellow_corner, &yellow_corner_index, &yellow_corner_enable, pos_mesure);
		//calculeZone(yellow_corner, &yellow_corner_index, yellow_corner_enable, &angle_yellow_corner, &ymoy_yellow_corner, BORDER_SCAN_MODE_DEN_Y);
		return YELLOW_CORNER;
	}
	else if ((pos_mesure.x>1980)&&(pos_mesure.y<1200)&&(pos_mesure.y>50)&&(pos_mesure.x<3000)){
		//pos_mesure.x = 2000;
		//pos_mesure.y = 55 + blue_south_index*15;
		//pos_mesure.x -= 0.01745 * pos_mesure.y;
		//pos_mesure.y += 40;

		BORDERS_SCAN_addPointZone(blue_south, &blue_south_index, &blue_south_enable, pos_mesure);
		//calculeZone(blue_south, &blue_south_index, blue_south_enable, &angle_blue_south, &xmoy_blue_south, BORDER_SCAN_MODE_DEN_X);
		return BLUE_SOUTH;
	}
	else if ((pos_mesure.x>1980)&&(pos_mesure.y<2950)&&(pos_mesure.y>1800)&&(pos_mesure.x<3000)){
		//pos_mesure.x = 2000;
		//pos_mesure.y = 1985 + blue_north_index*15;
		//pos_mesure.x -= 0.01745 * pos_mesure.y;
		//pos_mesure.y += 40;

		BORDERS_SCAN_addPointZone(yellow_south, &yellow_south_index, &yellow_south_enable, pos_mesure);
		//calculeZone(yellow_south, &yellow_south_index, yellow_south_enable, &angle_yellow_south, &xmoy_yellow_south, BORDER_SCAN_MODE_DEN_X);
		return YELLOW_SOUTH;
	}
	else{
		return OTHER_ZONE;
	}
}

static void BORDERS_SCAN_addPointZone(volatile scan_point_time_t zone[], volatile Uint8 *zone_index, volatile bool_e *zone_enable, GEOMETRY_point_t pos_mesure){
	zone[(*zone_index)].scan_point = pos_mesure;
	zone[(*zone_index)].scan_time = IT_get_counter();

	//it_printf("%d\t%d\n",pos_mesure.x,pos_mesure.y);
	(*zone_index)++;
	if((*zone_index) == NB_DATA_POINTS){
		*zone_index=0;
		*zone_enable=TRUE;
	}
}


bool_e BORDERS_SCAN_calculeZonePublic(scan_zone_e zone){

	if (zone == BLUE_NORTH){
		return BORDERS_SCAN_calculeZone(blue_north, &blue_north_index, blue_north_enable, &xmoy_blue_north, &ymoy_blue_north, &date_blue_north, BORDER_SCAN_MODE_DEN_Y);
	}
	else if (zone == MIDDLE_NORTH){
		return BORDERS_SCAN_calculeZone(middle_north, &middle_north_index, middle_north_enable, &xmoy_middle_north, &ymoy_middle_north, &date_middle_north, BORDER_SCAN_MODE_DEN_Y);
	}
	else if (zone == YELLOW_NORTH){
		return BORDERS_SCAN_calculeZone(yellow_north, &yellow_north_index, yellow_north_enable, &xmoy_yellow_north, &ymoy_yellow_north, &date_yellow_north, BORDER_SCAN_MODE_DEN_Y);
	}
	else if (zone == BLUE_START){
		return BORDERS_SCAN_calculeZone(blue_start, &blue_start_index, blue_start_enable, &xmoy_blue_start, &ymoy_blue_start, &date_blue_start, BORDER_SCAN_MODE_DEN_X);
	}
	else if (zone == BLUE_CORNER){
		return BORDERS_SCAN_calculeZone(blue_corner, &blue_corner_index, blue_corner_enable, &xmoy_blue_corner, &ymoy_blue_corner, &date_blue_corner, BORDER_SCAN_MODE_DEN_X);
	}
	else if (zone == YELLOW_START){
		return BORDERS_SCAN_calculeZone(yellow_start, &yellow_start_index, yellow_start_enable, &xmoy_yellow_start, &ymoy_yellow_start, &date_yellow_start, BORDER_SCAN_MODE_DEN_X);
	}
	else if (zone == YELLOW_CORNER){
		return BORDERS_SCAN_calculeZone(yellow_corner, &yellow_corner_index, yellow_corner_enable, &xmoy_yellow_corner, &ymoy_yellow_corner, &date_yellow_corner, BORDER_SCAN_MODE_DEN_X);
	}
	else if (zone == BLUE_SOUTH){
		return BORDERS_SCAN_calculeZone(blue_south, &blue_south_index, blue_south_enable, &xmoy_blue_south, &ymoy_blue_south, &date_blue_south, BORDER_SCAN_MODE_DEN_Y);
	}
	else if (zone == YELLOW_SOUTH){
		return BORDERS_SCAN_calculeZone(yellow_south, &yellow_south_index, yellow_south_enable, &xmoy_yellow_south, &ymoy_yellow_south, &date_yellow_south, BORDER_SCAN_MODE_DEN_Y);
	}
	else{
		return FALSE;
	}
	return FALSE;
}


bool_e BORDERS_SCAN_calculeZone(volatile scan_point_time_t zone[], volatile Uint8 *zone_index, volatile bool_e zone_enable, volatile Sint32 *xmoy_border, volatile Sint32 *ymoy_border, volatile Uint16 *date_point, border_scan_mode_den_e mode_den){
	Sint32 xmoy=0, ymoy=0, num=0, den=0, xmoyfinal=0, ymoyfinal=0;
	Uint8 i;
	double a=0;
	Sint32 b=0;
	Uint16 pseudovariance=0;
	Uint8 nb_points_in_zone=0;
	Uint16 distance=0;
	bool_e flag_correction=TRUE;
	bool_e vertical=FALSE;

	bool_e firstLoop = FALSE;

	if(indexDebug < NB_RUN_DEBUG){
		firstLoop = TRUE;
	}

	if(firstLoop){
		for(i=0; i<NB_DATA_POINTS; i++){
			tabDebug[indexDebug].tab[i].x = zone[i].scan_point.x;
			tabDebug[indexDebug].tab[i].y = zone[i].scan_point.y;
		}
	}

	do{

		xmoy = 0;
		ymoy = 0;
		xmoyfinal = 0;
		ymoyfinal = 0;
		num = 0;
		den = 0;
		pseudovariance = 0;

		flag_correction = TRUE;
		vertical=FALSE;

		nb_points_in_zone = 0;
		for(i=0; i<NB_DATA_POINTS; i++){
			if(zone[i].scan_time<(IT_get_counter() - NB_IT_ELEMENTARY_POINT)){
				zone[i].scan_point.x = 0;
				zone[i].scan_point.y = 0;
			}
			if((zone[i].scan_point.x != 0) || (zone[i].scan_point.y != 0)){		// Si c'est un point valide
				nb_points_in_zone++;
				xmoy += (Sint32) zone[i].scan_point.x;
				ymoy += (Sint32) zone[i].scan_point.y;
			}
			//it_printf("x=%d\ty=%d\txmoy=%d\tymoy=%d\n",zone[i].scan_point.x,zone[i].scan_point.y,xmoy,ymoy);
		}

		if(nb_points_in_zone < NB_POINTS_MINI)
			return FALSE;

		xmoyfinal = (xmoy<<10) / nb_points_in_zone;
		xmoy = xmoyfinal>>10;
		ymoyfinal = (ymoy<<10) / nb_points_in_zone;
		ymoy = ymoyfinal>>10;

		//it_printf("\nfalbala xmoy=%d\tymoy=%d\n\n",xmoy,ymoy);

		for(i=0; i<NB_DATA_POINTS; i++){
			if((zone[i].scan_point.x != 0) || (zone[i].scan_point.y != 0)){		// Si c'est un point valide
				num += (zone[i].scan_point.x - xmoy)*(zone[i].scan_point.y - ymoy);
				if(mode_den == BORDER_SCAN_MODE_DEN_X)
					den += (zone[i].scan_point.x - xmoy)*(zone[i].scan_point.x - xmoy);
				else
				    den += (zone[i].scan_point.y - ymoy)*(zone[i].scan_point.y - ymoy);
			}
		}
		if(den != 0){
			a = (double)num / (double)den;
			if(mode_den != BORDER_SCAN_MODE_DEN_X){
				if(a!=0){
					a= -1/a;
				}else{
					vertical=TRUE;
				}
			}

			b = ymoy - a * xmoy;

			if(firstLoop){
				tabDebug[indexDebug].a = a;
				tabDebug[indexDebug].b = b;
				tabDebug[indexDebug].den = den;
				tabDebug[indexDebug].nb_points = nb_points_in_zone;
				tabDebug[indexDebug].num = num;
				tabDebug[indexDebug].xmoy = xmoy;
				tabDebug[indexDebug].ymoy = ymoy;
				tabDebug[indexDebug].time = IT_get_counter();
			//	it_printf("numero de run : %d\n",indexDebug);

				indexDebug++;
				firstLoop = FALSE;
			}

			//if(mode_den == BORDER_SCAN_MODE_DEN_X)
			//*moy_border = xmoy;
			//else
			//    *moy_border = ymoy;
			if(mode_den == BORDER_SCAN_MODE_DEN_X){
				//it_printf("a=%ld.%3ld\tb=%d\tnbpoints=%d\n", (Uint32)(a), (((Uint32)(a*1000))%1000), b, nb_points_in_zone);
				for(i=0; i<NB_DATA_POINTS; i++){

					if((zone[i].scan_point.x != 0) || (zone[i].scan_point.y != 0)){		// Si c'est un point valide
			//			it_printf("x=%d\ty=%d\n",zone[i].scan_point.x,zone[i].scan_point.y);

						distance = absolute(zone[i].scan_point.y - (a * zone[i].scan_point.x + b));
						//it_printf("yre:%d\tyth:%d\n",zone[i].scan_point.y,(Sint16)(a * zone[i].scan_point.x + b));
						if(distance > DISTANCE_MAX){
							//it_printf("dist:\t%d\n",distance);
							zone[i].scan_point.x = 0;
							zone[i].scan_point.y = 0;
							//printf("cest ici\n");
							flag_correction = FALSE;
						}
						pseudovariance += distance;
					}
				}

			}else{
				//it_printf("a=%ld.%3ld\tb=%d\tnbpoints=%d\n", (Uint32)(a), (((Uint32)(a*1000))%1000), b, nb_points_in_zone);

                for(i=0;i<NB_DATA_POINTS;i++){
					if((zone[i].scan_point.x != 0) || (zone[i].scan_point.y != 0)){		// Si c'est un point valide
		//				it_printf("x=%d\ty=%d\n",zone[i].scan_point.x,zone[i].scan_point.y);

						if((a!=0)&&(vertical==FALSE)){
	                    	distance=absolute(zone[i].scan_point.x-(zone[i].scan_point.y-b)/a);
							//it_printf("xre:%d\txth:%d\n",zone[i].scan_point.x,(Sint16)((zone[i].scan_point.y-b)/a));
	                    }else{
	                    	distance=absolute(zone[i].scan_point.x-xmoy);
	                    }

	                    if(distance > DISTANCE_MAX){
							//it_printf("dist:\t%d\n",distance);

							zone[i].scan_point.x = 0;
							zone[i].scan_point.y = 0;
							//it_printf("cest la %d\n",distance);
							flag_correction = FALSE;
						}
						pseudovariance += distance;
					}
                }
			}
		}else{
			return FALSE;
		}
		//it_printf("\nflag_correc:%d\n\n",flag_correction);
	}while(flag_correction == FALSE);

	pseudovariance = pseudovariance / nb_points_in_zone;

	//it_printf("pseudov%d\n", pseudovariance);
	//it_printf("a=%ld.%3ld\tb=%d\tnbpoints=%d", (Uint32)(a), (((Uint32)(a*1000))%1000), b, nb_points_in_zone);

	if (pseudovariance < BORDER_SCAN_PSEUDO_VARIANCE_SEUIL){
		*xmoy_border = xmoyfinal;
		//display(xmoy);
		//display(ymoy);
		*ymoy_border = ymoyfinal;
		*date_point=IT_get_counter();
		//it_printf("xmoy=%d\tymoy=%d\n",xmoy,ymoy);
		scan_try_proposition = TRUE;
		return TRUE;
	}else{
		for(i=0; i<NB_DATA_POINTS; i++){
			//it_printf("%d\t%d\n", zone[i].scan_point.x, zone[i].scan_point.y);
		}
		return FALSE;
	}
}

void BORDERS_SCAN_afficheDebug(){
	Uint16 i, j;

	for(i=0; i<NB_RUN_DEBUG; i++){
		debug_printf("Run %d\n", i);
		for(j=0; j<NB_DATA_POINTS; j++){

			printf("x=\t%d\ty=\t%d\n",tabDebug[i].tab[j].x,tabDebug[i].tab[j].y);
			//display(tabDebug[i].tab[j].x);
			//display(tabDebug[i].tab[j].y);
		}
		display_float(tabDebug[i].a);
		display(tabDebug[i].b);
		display(tabDebug[i].num);
		display(tabDebug[i].den);
		display(tabDebug[i].nb_points);
		display(tabDebug[i].xmoy);
		display(tabDebug[i].ymoy);
		display(tabDebug[i].time);

	}
}

void BORDERS_SCAN_process_main(){
	Uint8 counter_north_border = 0; //compte le nombre de bordure dispo sur les 3 du nord
	Uint16 date_north_border = 0;	//date du second plus vieux point de zone de la bordure au nord (dsl si c'est incompr�hensible)
	Uint16 date_blue_border = 0;	//date du plus vieux point de zone de la bordure bleue
	Uint16 date_yellow_border = 0;	//date du plus vieux point de zone de la bordure jaune
	Uint16 date_south_border = 0;	//date du plus vieux point de zone de la bordure au sud
	double a_north = 0;				//coefficient directeur de la droite du nord
	double a_south = 0;				//coefficient directeur de la droite du sud
	double a_blue = 0;				//coefficient directeur de la droite du bleu
	double a_yellow = 0;			//coefficient directeur de la droite du jaune
	Sint32 b_north = 0;				//ordonn�e � l'origine de la droite du nord
	Sint32 b_south = 0;				//ordonn�e � l'origine de la droite du sud
	Sint32 b_blue = 0;				//ordonn�e � l'origine de la droite du bleu
	Sint32 b_yellow = 0;			//ordonn�e � l'origine de la droite du jaune
	bool_e yellow_north_zone_enable = FALSE;	//validit� de la zone jaune au nord
	bool_e middle_north_zone_enable = FALSE;	//validit� de la zone entre les deux fus�es au nord
	bool_e blue_north_zone_enable = FALSE;		//validit� de la zone bleue au nord
	bool_e north_zone_enable = FALSE;			//validit� de la bordure au nord
	bool_e south_zone_enable = FALSE;			//validit� de la bordure au sud
	bool_e blue_zone_enable = FALSE;			//validit� de la bordure bleue
	bool_e yellow_zone_enable = FALSE;			//validit� de la bordure jaune
	double cos_produit_scalaire = 0;			//valeur interm�diaire pour un calcul de produit scalaire
	double new_intersection_x = 0;				//d�calage du rep�re sur l'axe des abscisse oui c'est super mal nomm� mais derri�re cela il y a un historique et une grosse flemme
	double new_intersection_y = 0;				//idem mais en ordonn�e
	Sint16 new_correction_angle = 0;			//erreur d'angle mesur�e
	Sint32 den_division = 0;					//valeur de denominateur de certaine division (�a �vite les divisions par 0)
	bool_e a_vertical = FALSE;					//quand une droite est parrall�le � l'axe des ordonn�e c'est super chiant mais je dois quand m�me m'en occuper


	Sint32 new_point_x = 0;						//proposition d'abscisse pour le robot
	Sint32 new_point_y = 0;						//proposition d'ordonn�e pour le robot
	Sint16 new_angle = 0;						//proposition d'angle pour le robot

	Sint32 num_div_de_merde = 0;	//num�rateur d'une division qui m'a �nerv�
	Sint32 den_div_de_merde = 0;	//d�nominateur d'une division qui m'a �nerv�


	if(scan_try_proposition){	//si l'it indique qu'il est pertinent de tenter une proposition, on essaye, sinon ... on essaye pas
		scan_try_proposition = FALSE;
		//printf("a\n");
		if((date_blue_north != 0)&&(date_blue_north > (IT_get_counter()-NB_IT_ZONE_POINT))){	//je compte le nombre de bordure valide sur la bordure au nord
			counter_north_border++;
		}
		if((date_middle_north != 0)&&(date_middle_north > (IT_get_counter()-NB_IT_ZONE_POINT))){
			counter_north_border++;
		}
		if((date_yellow_north != 0)&&(date_yellow_north > (IT_get_counter()-NB_IT_ZONE_POINT))){
			counter_north_border++;
		}
		display(date_blue_north);
		display(date_middle_north);
		display(date_yellow_north);
		display(counter_north_border);

		if(counter_north_border >= 2){	//si il y au moins deux zones valides sur la bordure du nord on peut en faire quelques chose, sinon on oublie

			north_zone_enable = TRUE;
			if((date_yellow_north < date_middle_north) && (date_yellow_north < date_blue_north)){	//je cherche les deux plus r�centes zones de la bordure du nord ainsi que la date de la bordure du nord
				middle_north_zone_enable = TRUE;
				blue_north_zone_enable = TRUE;
				printf("b\n");

				if(date_middle_north < date_blue_border){
					date_north_border = date_middle_north;
				}else{
					date_north_border = date_blue_north;
				}
			}else if((date_yellow_north > date_middle_north) && (date_yellow_north > date_blue_north)){
				printf("c\n");

				if(date_middle_north > date_blue_border){
					middle_north_zone_enable = TRUE;
					yellow_north_zone_enable = TRUE;
					date_north_border = date_middle_north;
				}else{
					blue_north_zone_enable = TRUE;
					yellow_north_zone_enable = TRUE;
					date_north_border = date_blue_north;
				}
			}else{
				printf("d\n");

				date_north_border = date_yellow_north;
				if(date_middle_north < date_blue_border){
					blue_north_zone_enable = TRUE;
					yellow_north_zone_enable = TRUE;
				}else{
					middle_north_zone_enable = TRUE;
					yellow_north_zone_enable = TRUE;
				}
			}
		}
		//si les deux zones de la bordures bleues sont r�centes et valides alors on s'amuse avec la bordure bleue
		if((date_blue_start != 0) && (date_blue_corner != 0) && ((date_blue_start > (IT_get_counter()-NB_IT_ZONE_POINT)) || (date_blue_corner > (IT_get_counter()-NB_IT_ZONE_POINT)))){
			display(date_blue_corner);
			blue_zone_enable = TRUE;
			printf("e\n");

			if(date_blue_start > date_blue_corner){//on cherche la date de la bordure bleue
				date_blue_border = date_blue_corner;

			}else{
				date_blue_border = date_blue_start;
			}
		}
		//pareil pour la bordure jaune
		if((date_yellow_start != 0) && (date_yellow_corner != 0) && ((date_yellow_start > (IT_get_counter()-NB_IT_ZONE_POINT)) || (date_yellow_corner > (IT_get_counter()-NB_IT_ZONE_POINT)))){
			yellow_zone_enable = TRUE;
			printf("f\n");

			if(date_yellow_start > date_yellow_corner){
				date_yellow_border = date_yellow_corner;
			}else{
				date_yellow_border = date_yellow_start;
			}
		}
		//ainsi que pour la bordure au sud
		if((date_blue_south != 0) && (date_yellow_south != 0) && ((date_blue_south > (IT_get_counter()-3000)) && (date_yellow_south > (IT_get_counter()-3000)))){
			south_zone_enable = TRUE;
			printf("g\n");

			if(date_blue_south > date_yellow_south){
				date_south_border = date_yellow_south;
			}else{
				date_south_border = date_blue_south;
			}
		}
//si la bordure au nord est plus r�cente que celle au sud et qu'elle est valide alors on prend la bordure du nord
		if((date_south_border < date_north_border) && (north_zone_enable)){ //c'est parti on se fait chier pour le nord (c'�tait un mauvaise id�e cette troisi�me bordure
			printf("h\n");
//on cherche l'�quation de la bordure du nord en fonction des zones valides de la bordure du nord
			if(yellow_north_zone_enable && blue_north_zone_enable){
				den_division = xmoy_yellow_north - xmoy_blue_north;
				if (den_division == 0){
					a_vertical = TRUE;
					b_north = xmoy_yellow_north>>10; //ici je l'utilise pour l'offset c'est degueulasse mais je vois pas en quoi ce serait plus lisible de rajouter une eni�me variable
				}else{
					a_north = (double)(ymoy_yellow_north - ymoy_blue_north)/(double)(den_division); // on calcule le coefficient directeur
					b_north = (ymoy_yellow_north>>10) - ((Sint32)(a_north * xmoy_yellow_north)>>10);	//...l'ordonn�e � l'origine
				}
				//on r�p�te cela pour les diff�rentes combinaisons de zones valides
			}else if(yellow_north_zone_enable && middle_north_zone_enable){
				den_division = xmoy_yellow_north - xmoy_middle_north;
				if (den_division == 0){
					a_vertical = TRUE;
					b_north = xmoy_yellow_north>>10;
				}else{
					a_north = (double)(ymoy_yellow_north - ymoy_middle_north)/(double)(den_division);
					b_north = (ymoy_yellow_north>>10) - ((Sint32)(a_north * xmoy_yellow_north)>>10);
				}
			}else{
				den_division = xmoy_middle_north - xmoy_blue_north;
				if (den_division == 0){
					a_vertical = TRUE;
					b_north = xmoy_blue_north>>10;
				}else{
					a_north = (double)(ymoy_middle_north - ymoy_blue_north)/(double)(den_division);
					b_north = (ymoy_middle_north>>10) - ((Sint32)(a_north * xmoy_middle_north)>>10);
				}
			}
			//on connait l'�quation de la droite du nord !!!
			//si la jaune est plus r�cente que la bleue et qu'elle est valide et que ses donn�e ne risque pas de nous foutre une division par z�ro ... on s'amuse avec
			if((date_blue_border < date_yellow_border) && (yellow_zone_enable) && ((xmoy_yellow_start - xmoy_yellow_corner) != 0)){
				printf("k\n");
				display(ymoy_blue_start);
				display(ymoy_blue_corner);
				display(xmoy_blue_start);
				display(xmoy_blue_corner);
				//calcul du coefficient directeur de la jaune
				a_yellow = (double)(ymoy_yellow_start - ymoy_yellow_corner)/(double)(xmoy_yellow_start - xmoy_yellow_corner);
				//calcul de l'ordonn�e � l'origine de la jaune
				b_yellow = (ymoy_yellow_start>>10) - ((Sint32)(a_yellow * xmoy_yellow_start)>>10);
				//on accepte une erreur d'angle entre les droites de 1� cos�(89�)=   en PI4096
				if(a_vertical){//si une droite est verticale on s'emmerde avec l'autre droite
					cos_produit_scalaire=(double)(a_yellow * a_yellow)/(double)(a_yellow * a_yellow + 1);
				}else{//sinon on essaye d'avoir l'angle entre les deux droites (par l'int�rm�diaire d'un cos au carr� issue d'un produit scalaire ... une horreur
					cos_produit_scalaire=(double)((a_yellow * a_north + 1) * (a_yellow * a_north + 1))/(double)((a_yellow * a_yellow + 1) * (a_north * a_north + 1));
				}
//si les droites ne forment pas � peu pr�s un angle droit, on refuse sinon on accepte
				if((COS_CARRE_SCALAIRE<cos_produit_scalaire)||(-COS_CARRE_SCALAIRE>cos_produit_scalaire)){
					return;// l'angle les droites n'est pas droit
				}
				printf("byel:%ld\tbnor:%ld\n", b_yellow, b_north);
				display_float(a_north);
				display_float(a_yellow*1000);
//on calcule l'erreur d'angle en fonction de la pente
				new_correction_angle = atan4096(a_yellow);
//on cherche le d�calage qu'a le terrain par rapport � ce qu'on pensait
				if(a_vertical){
					new_intersection_x = b_north;
				}else{
					num_div_de_merde = b_yellow - b_north;
					den_div_de_merde = a_north - a_yellow;
					new_intersection_x = ((double)(num_div_de_merde))/((double)(den_div_de_merde));
				}
				new_intersection_y = (a_yellow * new_intersection_x + b_yellow);
				new_intersection_x += 3000*sin4096(new_correction_angle);
				new_intersection_y -= 3000*cos4096(new_correction_angle);

			}else if ((blue_zone_enable) && (xmoy_blue_start - xmoy_blue_corner != 0)){//sinon si la zone bleu est valide et que ses points ne forcent pas une division par z�ro
				printf("l\n");
//on recommence la m�me chose pour le bleu
				//printf("ybs:%d\tybc:%d\txbs:%d\txbc:%d\t\n", ymoy_blue_start, ymoy_blue_corner, xmoy_blue_start, xmoy_blue_corner);
				display(ymoy_blue_start);
				display(ymoy_blue_corner);
				display(xmoy_blue_start);
				display(xmoy_blue_corner);

				a_blue = (double)(ymoy_blue_start - ymoy_blue_corner)/(double)(xmoy_blue_start - xmoy_blue_corner);
				b_blue = (ymoy_blue_start>>10) - ((Sint32)(a_blue * xmoy_blue_start)>>10);
				if(a_vertical){
					printf("vertical\n");
					cos_produit_scalaire=(a_blue * a_blue)/(a_blue * a_blue + 1);
				}else{
					cos_produit_scalaire=((a_blue * a_north + 1) * (a_blue * a_north + 1))/((a_blue * a_blue + 1) * (a_north * a_north + 1));
				}
				if((COS_CARRE_SCALAIRE<cos_produit_scalaire)||(-COS_CARRE_SCALAIRE>cos_produit_scalaire)){
					return; // l'angle les droites n'est pas droit
				}
				printf("bblu:%ld\tbnor:%ld\n", b_blue, b_north);

				display_float(a_north);
				display_float(a_blue*1000);

				if(a_vertical){
					new_intersection_x = b_north;
				}else{
					num_div_de_merde = b_blue - b_north;
					den_div_de_merde = a_north - a_blue;
					new_intersection_x = ((double)(num_div_de_merde))/((double)(den_div_de_merde));
				}

				new_intersection_y = (Sint32)(a_blue * new_intersection_x + b_blue);

				//printf("\n%llx\n", a_blue);
				new_correction_angle = atan4096(a_blue);

			}
		}else if (south_zone_enable){//sinon si la bordure sud est valide, on essaye
			//et on recommence la m�me chose que pour le nord sauf que ce coup-ci on se fait pas chier avec 3 zones
			printf("i\n");
			//printf("yb:%d\tyy:%d\txb:%d\txy:%d\t\n", ymoy_blue_south, ymoy_yellow_south, xmoy_blue_south, xmoy_yellow_south);

			den_division = xmoy_blue_south - xmoy_yellow_south;
			if (den_division == 0){
				a_vertical = TRUE;
				b_south = xmoy_blue_south>>10;
			}else{
				a_south = (double)(ymoy_blue_south - ymoy_yellow_south)/(double)(den_division);
				b_south = (ymoy_yellow_south>>10) - ((Sint32)(a_yellow * xmoy_yellow_south)>>10);
			}
			if((date_blue_border < date_yellow_border) && (yellow_zone_enable) && (xmoy_yellow_start - xmoy_yellow_corner != 0)){

				a_yellow = (double)(ymoy_yellow_start - ymoy_yellow_corner)/(double)(xmoy_yellow_start - xmoy_yellow_corner);
				b_yellow = (ymoy_yellow_start>>10) - ((Sint32)(a_yellow * xmoy_yellow_start)>>10);

				if(a_vertical){
					cos_produit_scalaire=(a_yellow * a_yellow)/(a_yellow * a_yellow + 1);
				}else{
					cos_produit_scalaire=((a_yellow * a_south + 1) * (a_yellow * a_south + 1))/((a_yellow * a_yellow + 1) * (a_south * a_south + 1));
				}
				if((COS_CARRE_SCALAIRE<cos_produit_scalaire)||(-COS_CARRE_SCALAIRE>cos_produit_scalaire)){
					return; // l'angle les droites n'est pas droit
				}
				//printf("m\n");
				printf("byel:%ld\tbsou:%ld\n", b_yellow, b_south);
				display_float(a_south);
				display_float(a_yellow);


				if(a_vertical){
					new_intersection_x = b_south - 2000;
					new_intersection_y = (Sint32)(a_yellow * new_intersection_x + b_yellow);
					new_intersection_y -= 3000;
				}else{
					num_div_de_merde = b_yellow - b_south;
					den_div_de_merde = a_south - a_yellow;
					new_intersection_x = ((double)(num_div_de_merde))/((double)(den_div_de_merde));
					new_correction_angle = atan4096(a_yellow);
					new_intersection_y = (Sint32)(a_yellow * new_intersection_x + b_yellow);
					new_intersection_x += 3000*sin4096(new_correction_angle)-2000*cos4096(new_correction_angle);
					new_intersection_y += -3000*cos4096(new_correction_angle)-2000*sin4096(new_correction_angle);
#warning 'A TESTER'
				}



				//printf("%ld\n",new_intersection_x);

			}else if((blue_zone_enable) && (xmoy_blue_start-xmoy_blue_corner != 0)){
				printf("j\n");

				a_blue = (double)(ymoy_blue_start-ymoy_blue_corner)/(double)(xmoy_blue_start-xmoy_blue_corner);
				b_blue = (ymoy_blue_start>>10) - ((Sint32)(a_blue * xmoy_blue_start)>>10);
				if(a_vertical){
					cos_produit_scalaire=(a_blue * a_blue)/(a_blue * a_blue + 1);
				}else{
					cos_produit_scalaire=((a_blue * a_south + 1) * (a_blue * a_south + 1))/((a_blue * a_blue + 1) * (a_south * a_south + 1));
				}
				if((COS_CARRE_SCALAIRE<cos_produit_scalaire)||(-COS_CARRE_SCALAIRE>cos_produit_scalaire)){
					return; // l'angle les droites n'est pas droit
				}
				printf("n\n");

				if(a_vertical){
					new_intersection_x = b_south - 2000;
					new_intersection_y = (Sint32)(a_south * new_intersection_x + b_south);
				}else{
					num_div_de_merde = b_blue - b_south;
					den_div_de_merde = a_south - a_blue;
					new_intersection_x = ((double)(num_div_de_merde))/((double)(den_div_de_merde));
					new_intersection_y = (Sint32)(a_south * new_intersection_x + b_south);
					new_correction_angle = atan4096(a_blue);
					new_intersection_x -= 2000*cos4096(new_correction_angle);
					new_intersection_y -= 2000*sin4096(new_correction_angle);
#warning 'A TESTER'
				}
			}
		}//eh ben je peux rien faire pour toi
		//tout est fini il reste plus qu'a officialis� la chose avec la strat�gie
		//si on a trouv� une erreur, alors on calcul notre nouvelle position
		if((new_correction_angle != 0)||(new_intersection_x != 0)||(new_intersection_y != 0)){
			Sint16 cosinus, sinus;
			COS_SIN_4096_get(new_correction_angle, &cosinus, &sinus);
			//notre nouvelle abscisse
			new_point_x = (cosinus * (global.position.x-new_intersection_x) - sinus * (global.position.y-new_intersection_x))/4096;
			//la nouvelle ordonn�e
			new_point_y = (sinus * (global.position.x-new_intersection_y) + cosinus * (global.position.y-new_intersection_y))/4096;
			//et enfin le petit dernier : l'angle
			new_angle = global.position.teta - new_correction_angle;
			printf("newx:%ld\tnewy:%ld\tnewang:%d\n", new_point_x, new_point_y, new_angle);
			display_float(new_intersection_x);
			display_float(new_intersection_y);
			display_float(new_correction_angle);
		}

	}
}//FINI !!!

#endif
