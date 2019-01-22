/*
 * scanDistri.h
 *
 *  Created on: 11 mai 2018
 *      Author: a.guilmet
 */

#include "../QS/QS_all.h"

#ifndef SCAN_SCANDISTRI_H_
	#define SCAN_SCANDISTRI_H_

	#if USE_SCAN_DISTRI

		#include "../QS/QS_CANmsgList.h"

		void SCAN_DISTRI_init(void);
		void SCAN_DISTRI_processMain(void);
		void SCAN_DISTRI_processIt(Uint8 ms);
		void SCAN_DISTRI_receivedCanMsg(CAN_msg_t * msg);

	#endif

#endif /* SCAN_SCANDISTRI_H_ */
