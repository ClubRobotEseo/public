#ifndef ACT_AVOIDANCE_H
#define ACT_AVOIDANCE_H

#include "../QS/QS_all.h"
#include "act_functions.h"


typedef struct{
	Uint16 Xleft;
	Uint16 Xright;
	Uint16 Yfront;
	Uint16 Yback;
	bool_e init;
	bool_e active;
	Uint16 act_cmd;
	queue_id_e act_avoid_id;
}offset_avoid_s;


#define ACT_AVOID_NB_MAX_CMD		(30)


void ACT_AVOIDANCE_init();
void ACT_AVOIDANCE_reset_actionneur(queue_id_e act_avoid_id);
void ACT_AVOIDANCE_new_classic_cmd(queue_id_e act_avoid_id, Uint8 act_cmd);
void ACT_AVOIDANCE_get_offset(offset_avoid_s * offset);

#endif // ACT_AVOIDANCE_H
