/*
 * scan.h
 *
 *  Created on: 11 mai 2018
 *      Author: a.guilmet
 */

#include "QS/QS_all.h"

#ifndef SCAN_H_
	#define SCAN_H_

	#include "QS/QS_CANmsgList.h"

	void SCAN_init(void);
	void SCAN_receivedCanMsg(CAN_msg_t *msg);
	void SCAN_setScanDistri(bool_e activate, Uint16 threshold);
	bool_e SCAN_getScanDistri(Sint16 * x, Sint16 * y);
	bool_e SCAN_getStateScanDistri(void);
	void SCAN_HOKUYO_DISTRI(CAN_msg_t *msg);

#endif /* SCAN_H_ */
