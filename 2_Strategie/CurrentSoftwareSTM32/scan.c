/*
 * scan.c
 *
 *  Created on: 11 mai 2018
 *      Author: a.guilmet
 */

#include "scan.h"
#include "QS/QS_lowLayer/QS_can.h"
#include "QS/QS_outputlog.h"

static volatile bool_e scanThresholdState;
static volatile Sint16 scanX = 0;
static volatile Sint16 scanY = 0;
static volatile Uint16 lastValue;
static volatile Uint16 value;

void SCAN_init(void){
	scanThresholdState = FALSE;
}

void SCAN_receivedCanMsg(CAN_msg_t *msg){
	if(msg->sid != STRAT_SCAN_DISTRI_RESULT){
		return;
	}

	//scanX = msg->data.strat_scan_distri_result.x;
	scanY = msg->data.strat_scan_distri_result.y;
	lastValue = msg->data.strat_scan_distri_result.lastValue;
	value = msg->data.strat_scan_distri_result.value;
	scanThresholdState = msg->data.strat_scan_distri_result.threshold;
}

void SCAN_setScanDistri(bool_e activate, Uint16 threshold){
	CAN_msg_t msg;

	msg.sid = PROP_ACTIVATE_SCAN_DISTRI;
	msg.size = SIZE_PROP_ACTIVATE_SCAN_DISTRI;
	msg.data.prop_activate_scan_distri.activate = activate;
	msg.data.prop_activate_scan_distri.threshold = threshold;

	CAN_send(&msg);

	scanThresholdState = FALSE;
}

bool_e SCAN_getStateScanDistri(void){
	return scanThresholdState;
}

bool_e SCAN_getScanDistri(Sint16 * x, Sint16 * y){
	if(scanThresholdState){
		if(x){
			*x = scanX;
		}

		if(y){
			*y = scanY;
		}

		return TRUE;
	}

	return FALSE;
}


void SCAN_HOKUYO_DISTRI(CAN_msg_t *msg)
{
	if(msg->sid != STRAT_SCAN_DISTRI_HOKUYO_RESULT){
			return;
		}
	scanY = msg->data.strat_scan_distri_result.y;
	scanX = msg->data.strat_scan_distri_result.lastValue;
	scanThresholdState = msg->data.strat_scan_distri_result.threshold;
}
