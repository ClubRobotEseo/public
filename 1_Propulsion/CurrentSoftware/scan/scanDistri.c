#include "scanDistri.h"
#include "../QS/QS_outputlog.h"
#include "../QS/QS_lowLayer/QS_adc.h"
#include "../secretary.h"

#if USE_SCAN_DISTRI

	#define SCAN_DISTRI_TIME_MEASURE		10
	#define SCAN_DISTRI_DEBUG_MODE			0
	#define SCAN_DISTRI_CONVERT_TO_MM(x)	((Uint16)((x)*7.0111 - 3978.4))

	static volatile bool_e activateScan = FALSE;
	static volatile Uint16 thresholdScan;
	static volatile Uint16 lastSensorValue = 0;
	static volatile Uint16 sensorValue = 0;
	static volatile bool_e newValue = FALSE;
	static volatile Sint16 sensorPosX = 0;
	static volatile Sint16 sensorPosY = 0;
	static volatile bool_e lastThresholdState;

	static void SCAN_DISTRI_sendThreshold(bool_e threshold);

	void SCAN_DISTRI_init(void){
		activateScan = FALSE;
		newValue = FALSE;
		sensorValue = 0;
	}

	void SCAN_DISTRI_processMain(void){
		if(newValue){
			bool_e threshold = sensorValue > thresholdScan;

			if(threshold == TRUE && lastThresholdState == FALSE){
				SCAN_DISTRI_sendThreshold(TRUE);

			}else if(threshold == FALSE && lastThresholdState == TRUE){
				SCAN_DISTRI_sendThreshold(FALSE);
			}

			lastThresholdState = threshold;
			lastSensorValue = sensorValue;

			newValue = FALSE;
		}

	#if SCAN_DISTRI_DEBUG_MODE
		static time32_t lastDisplay = 0;
		if(global.absolute_time - lastDisplay > 1000){
			display(SCAN_DISTRI_CONVERT_TO_MM(ADC_getValue(ADC_SENSOR_SMALL_LASER)));
			lastDisplay = global.absolute_time;
		}
	#endif

	}

	void SCAN_DISTRI_processIt(Uint8 ms){
		static time32_t lastMeasure = 0;

		lastMeasure += ms;

		if(lastMeasure >= SCAN_DISTRI_TIME_MEASURE){
			lastMeasure = 0;
			sensorValue = SCAN_DISTRI_CONVERT_TO_MM(ADC_getValue(ADC_SENSOR_SMALL_LASER));
			sensorPosX = global.position.x + 110;
			sensorPosY = global.position.y + 80;
			newValue = TRUE;
		}
	}

	void SCAN_DISTRI_receivedCanMsg(CAN_msg_t * msg){
		if(msg->sid != PROP_ACTIVATE_SCAN_DISTRI){
			return;
		}

		thresholdScan = msg->data.prop_activate_scan_distri.threshold;
		activateScan = msg->data.prop_activate_scan_distri.activate;
	}

	static void SCAN_DISTRI_sendThreshold(bool_e threshold){
		CAN_msg_t msg;

		msg.sid = STRAT_SCAN_DISTRI_RESULT;
		msg.size = SIZE_STRAT_SCAN_DISTRI_RESULT;
		msg.data.strat_scan_distri_result.threshold = threshold;
		//msg.data.strat_scan_distri_result.x = sensorPosX;
		msg.data.strat_scan_distri_result.y = sensorPosY;
		msg.data.strat_scan_distri_result.lastValue = lastSensorValue;
		msg.data.strat_scan_distri_result.value = sensorValue;

		SECRETARY_send_canmsg(&msg);
	}

#endif
