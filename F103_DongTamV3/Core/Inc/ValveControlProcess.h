/*
 * ValveControlProcess.h
 *
 *  Created on: May 12, 2024
 *      Author: SpiritBoi
 */

#ifndef INC_VALVECONTROLPROCESS_H_
#define INC_VALVECONTROLPROCESS_H_

#include "main.h"
#include "74HC595.h"
#include "74HC165.h"

#define TIMER_PERIOD_MS 10
#define MAX_NUM_VAN 16
#define EXPANDER_INPUT_PORT 2

typedef enum ValveControlProcessStep {
	PROCESS_IDLE,
	PROCESS_START,
	PROCESS_VALVE_ON,
	PROCESS_PULSE_TIME,
	PROCESS_VALVE_OFF,
	PROCESS_INTERVAL_TIME,
	PROCESS_CYCLE_INTERVAL_TIME,
	PROCESS_END,
} ValveControlProcessStep;

//typedef enum ResumeValveControlProcessBehavior {
//	RESUME_RESET_TO_FIRST_VALVE,
//	RESUME_CONTINUE_TO_NEXT_VALVE,
//} ResumeValveControlProcessBehavior;

class ValveFeedback {
	public:
		ValveFeedback();
		~ValveFeedback();
		void SetInputValue(uint16_t value);
		uint16_t GetInputValue();
	private:
		uint32_t inputValue;
		HC165_t expanderInput;
};

class ValveControl {
	private:

		uint16_t _timerTick;
		uint16_t _pulseTime; // thời gian kích van
		uint16_t _intervalTime; // thời gian nghỉ giữa 2 lần kích van
		uint16_t _valveRemainToTrigger; // số van còn lại trong chu trình cần phải kích
		uint8_t _cycleIntervalTime; //thời gian nghỉ giữa 2 chu kỳ kích van (khoảng nghỉ sau khi kích van cuối cùng và bắt đầu chu trình mới)
		uint8_t _totalValve;
		bool _isOnProcess;
		ValveControlProcessStep _processStep;
		HC595 _hc595;
		void InValveOnProcess();
		void InPulseTimeProcess();
		void InValveOffProcess();
		void InIntervalTimeProcess();
		void InCycleIntervalTimeProcess();
		void NextValve();

	public:
		ValveControl();
		~ValveControl();
		void AssignControlPin(GPIO_TypeDef *gpio, uint16_t pin, HC595_PinName HC595_PinName);
		void IncreaseTick();
		void SetTick(uint16_t tick);
		uint16_t GetTick();

		void SetTotalValve(uint8_t totalValve);
		uint8_t GetTotalValve();

		void SetPulseTime(uint16_t pulseTime);
		uint16_t GetPulseTime();

		void SetIntervalTime(uint16_t intervalTime);
		uint16_t GetIntervalTime();

		void SetCycleIntervalTime(uint8_t cycleIntervalTime);
		uint8_t GetCycleIntervalTime();

		void SetOutputValve(uint8_t valve, bool on);
		void SetOutputMultiValve(uint16_t valve, bool on);

		bool IsOnProcess();
		ValveControlProcessStep GetProcessStep();
		void StartValveProcess();
		void StopValveProcess();
		void ValveProcessRun();

};

#endif /* INC_VALVECONTROLPROCESS_H_ */
