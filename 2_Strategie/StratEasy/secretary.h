/*
 * secretary.h
 *
 *  Created on: 28 oct. 2018
 *      Author: Admin
 */

#ifndef SECRETARY_H_
#define SECRETARY_H_

	#include "QS/QS_lowLayer/QS_can.h"


	void SECRETARY_init(void);

	void SECRETARY_process_main(void);

	void SECRETARY_send(CAN_msg_t * msg);


#endif /* SECRETARY_H_ */
