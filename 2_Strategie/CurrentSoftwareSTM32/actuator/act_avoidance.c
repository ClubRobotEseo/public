#include "act_avoidance.h"


#define LOG_PREFIX "act_avoid: "
#define LOG_COMPONENT OUTPUT_LOG_COMPONENT_ACTFUNCTION
#include "../QS/QS_outputlog.h"
#include "../QS/QS_who_am_i.h"
#include "../QS/QS_lowLayer/QS_can.h"

static offset_avoid_s offset_avoid[ACT_AVOID_NB_MAX_CMD] = {0};
static offset_avoid_s total_offset_avoid = {0};
static Uint8 nbCommands = 0;

static void init_new_offset(queue_id_e act_avoid_id, Uint8 act_cmd, Uint16 Xleft, Uint16 Xright, Uint16 Yfront, Uint16 Yback);
static void send_total_offset_avoid();
static void refresh_total_offset_avoid();

void ACT_AVOIDANCE_init(){
	Uint8 i;

	// Initialisation de tout les offsets à FALSE pour être sûr de ne pas imposer un offset non controlé
	for(i = 0; i < ACT_AVOID_NB_MAX_CMD; i++){
		offset_avoid[i].init = FALSE;
	}


	// Listing de tout les offsets d'évitement de tout les actionneurs

	if(QS_WHO_AM_I_get() == BIG_ROBOT){ // Seulement sur le gros robot
		//----------------------------------------------------------------------------------------------------------------left-----right----front---back----

		//init_new_offset(ACT_QUEUE_Exemple,						ACT_EXEMPLE_OUT,			  				    		0,	    0,      0,	   0);




	}else{ //Seulement sur le petit robot

		//init_new_offset(ACT_QUEUE_Exemple,						ACT_EXEMPLE_OUT,			  				    		0,	    0,      0,	   0);


	}
}


static void init_new_offset(queue_id_e act_avoid_id, Uint8 act_cmd, Uint16 Xleft, Uint16 Xright, Uint16 Yfront, Uint16 Yback){
	if(nbCommands >= ACT_AVOID_NB_MAX_CMD){
		debug_printf("Error : tentative d'initialisation d'évitement actionneur -> augmentez ACT_AVOID_NB_MAX_CMD\n");
		return;
	}else if(act_avoid_id >= NB_QUEUE){
		debug_printf("Error : tentative d'initialisation d'évitement actionneur -> act_avoid_id %d inconnue\n", act_avoid_id);
		return;
	}

	offset_avoid[nbCommands].Xleft = Xleft;
	offset_avoid[nbCommands].Xright = Xright;
	offset_avoid[nbCommands].Yfront = Yfront;
	offset_avoid[nbCommands].Yback = Yback;
	offset_avoid[nbCommands].init = TRUE;
	offset_avoid[nbCommands].active = FALSE;
	offset_avoid[nbCommands].act_cmd = act_cmd;
	offset_avoid[nbCommands].act_avoid_id = act_avoid_id;
	nbCommands++;
}

void ACT_AVOIDANCE_reset_actionneur(queue_id_e act_avoid_id){
	if(act_avoid_id >= NB_QUEUE){
		debug_printf("Error : tentative de reset d'évitement actionneur -> act_avoid_id %d inconnue\n", act_avoid_id);
		return;
	}

	Uint8 i = 0;
	for(i=0; i<nbCommands; i++) {
		if(offset_avoid[i].act_avoid_id == act_avoid_id) {
			offset_avoid[i].active = FALSE;
		}
	}
}

static void refresh_total_offset_avoid(){
	Uint8 i;

	offset_avoid_s past_offset;

	past_offset.Xleft = total_offset_avoid.Xleft;
	past_offset.Xright = total_offset_avoid.Xright;
	past_offset.Yback = total_offset_avoid.Yback;
	past_offset.Yfront = total_offset_avoid.Yfront;

	total_offset_avoid.Xleft = 0;
	total_offset_avoid.Xright = 0;
	total_offset_avoid.Yfront = 0;
	total_offset_avoid.Yback = 0;

	for(i=0; i<nbCommands; i++){
		if(offset_avoid[i].active){

			if(offset_avoid[i].Xleft > total_offset_avoid.Xleft)
				total_offset_avoid.Xleft = offset_avoid[i].Xleft;

			if(offset_avoid[i].Xright > total_offset_avoid.Xright)
				total_offset_avoid.Xright = offset_avoid[i].Xright;

			if(offset_avoid[i].Yfront > total_offset_avoid.Yfront)
				total_offset_avoid.Yfront = offset_avoid[i].Yfront;

			if(offset_avoid[i].Yback > total_offset_avoid.Yback)
				total_offset_avoid.Yback = offset_avoid[i].Yback;
		}
	}

	if(total_offset_avoid.Xleft != past_offset.Xleft
			|| total_offset_avoid.Xright != past_offset.Xright
			|| total_offset_avoid.Yback != past_offset.Yback
			|| total_offset_avoid.Yfront != past_offset.Yfront){
		debug_printf("New total avoid offset : L:%d R:%d F:%d B:%d\n", total_offset_avoid.Xleft, total_offset_avoid.Xright, total_offset_avoid.Yfront, total_offset_avoid.Yback);
		send_total_offset_avoid();
	}
}

static void send_total_offset_avoid(){
	CAN_msg_t msg;
	msg.sid = PROP_OFFSET_AVOID;
	msg.size = SIZE_PROP_OFFSET_AVOID;
	msg.data.prop_offset_avoid.x_left = total_offset_avoid.Xleft;
	msg.data.prop_offset_avoid.x_right = total_offset_avoid.Xright;
	msg.data.prop_offset_avoid.y_front = total_offset_avoid.Yfront;
	msg.data.prop_offset_avoid.y_back = total_offset_avoid.Yback;
	CAN_send(&msg);
}

void ACT_AVOIDANCE_new_classic_cmd(queue_id_e act_avoid_id, Uint8 act_cmd){
	if(act_avoid_id >= NB_QUEUE){
		debug_printf("Error : tentative d'activation d'évitement actionneur -> act_avoid_id %d inconnue\n", act_avoid_id);
		return;
	}

	ACT_AVOIDANCE_reset_actionneur(act_avoid_id);

	Uint8 i = 0;
	bool_e found = FALSE;
	while(i<nbCommands && !found){
		offset_avoid_s this_avoid = offset_avoid[i];
		if(this_avoid.act_avoid_id == act_avoid_id && this_avoid.act_cmd == act_cmd && this_avoid.init) {
			offset_avoid[i].active = TRUE;
			found = TRUE;
		}
		i++;
	}
	refresh_total_offset_avoid();
	debug_printf("Act : %d reset avoidance\n", act_avoid_id);
}

void ACT_AVOIDANCE_get_offset(offset_avoid_s * offset){
	offset->Xleft = total_offset_avoid.Xleft;
	offset->Xright = total_offset_avoid.Xright;
	offset->Yback = total_offset_avoid.Yback;
	offset->Yfront = total_offset_avoid.Yfront;
}
