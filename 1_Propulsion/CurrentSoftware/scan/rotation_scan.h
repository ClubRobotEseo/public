
#ifndef SCAN_BLOC_H
	#define SCAN_BLOC_H

	#include "../QS/QS_all.h"

	#if SCAN_ROTATION

		#include "scan.h"
		#include "../QS/QS_can.h"

		void SCAN_CORNER_init(void);
		void SCAN_CORNER_process_it();
		void SCAN_CORNER_canMsg(CAN_msg_t *msg);
		void SCAN_CORNER_calculate();
		void SCAN_CORNER_process_main();

	#endif

#endif /* ROTATION_SCAN_H */
