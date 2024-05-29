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
		ValveFeedback() {
			HC165_AssignPin(&expanderInput, _74HC165_CLK_GPIO_Port, _74HC165_CLK_Pin, HC165_CP);
			HC165_AssignPin(&expanderInput, _74HC165_DATA_GPIO_Port, _74HC165_DATA_Pin, HC165_DATA);
			HC165_AssignPin(&expanderInput, _74HC165_LOAD_GPIO_Port, _74HC165_LOAD_Pin, HC165_PL);
		}
		uint16_t GetInputValue() {
			inputValue = (uint16_t) HC165_ReadState(EXPANDER_INPUT_PORT);
			return inputValue;
		}
	private:
		uint32_t inputValue;
		HC165_t expanderInput;
};

class ValveControl {
	private:

		uint16_t _timerTick;
		uint16_t _pulseTime; // thời gian kích van, tính bằng ms
		uint16_t _intervalTime; // thời gian nghỉ giữa 2 lần kích van, tính bằng s
		uint8_t _valveRemainToTrigger; // số van còn lại trong chu trình cần phải kích
		uint8_t _cycleIntervalTime; //thời gian nghỉ giữa 2 chu kỳ kích van (khoảng nghỉ sau khi kích van cuối cùng và bắt đầu chu trình mới), tính bằng s
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
		ValveControl() {
			AssignControlPin(_74HC595_CLK_GPIO_Port, _74HC595_CLK_Pin, HC595_CLK);
			AssignControlPin(_74HC595_DATA_GPIO_Port, _74HC595_DATA_Pin, HC595_DS);
			AssignControlPin(_74HC595_STORE_GPIO_Port, _74HC595_STORE_Pin, HC595_LATCH);
			_totalValve = 16;
			_pulseTime = 50;
			_intervalTime = 1;
			_cycleIntervalTime = 3;

			if (_totalValve > MAX_NUM_VAN)
				while (1);
			uint8_t n = 0;
			if (_totalValve > 8)
				n = 2;
			else if (_totalValve < 8 && _totalValve > 0)
				n = 1;
			HC595_ClearByteOutput(0xffffffff);
			HC595_ShiftOut(NULL, n, 1);
			_isOnProcess = false;
		}

		void AssignControlPin(GPIO_TypeDef *gpio, uint16_t pin, HC595_PinName HC595_PinName) {
			if (HC595_AssignPin(&_hc595, gpio, pin, HC595_PinName) != HC595_OK)
				while (1);
		}

		void IncreaseTick() {
			_timerTick++;
		}
		void SetTick(uint16_t tick) {
			_timerTick = tick;
		}
		uint16_t GetTick() {
			return _timerTick;
		}

		void SetTotalValve(uint8_t totalValve) {
			if (totalValve > MAX_NUM_VAN)
				_totalValve = MAX_NUM_VAN;
			else
				_totalValve = totalValve;
			if (_processStep != PROCESS_IDLE)
				_processStep = PROCESS_START;
		}
		uint8_t GetTotalValve() {
			return _totalValve;
		}

		void SetPulseTime(uint16_t pulseTime) {
			if (pulseTime < 30)
				pulseTime = 30;
			else if (pulseTime > 300)
				pulseTime = 300;
			else
				_pulseTime = pulseTime;
		}
		uint16_t GetPulseTime() {
			return _pulseTime;
		}

		void SetIntervalTime(uint16_t intervalTime) {
			if (intervalTime < 0)
				intervalTime = 0;
			else if (intervalTime > 100)
				_intervalTime = 100;
			else
				_intervalTime = intervalTime;
		}
		uint16_t GetIntervalTime() {
			return _intervalTime;
		}

		void SetCycleIntervalTime(uint8_t cycleIntervalTime) {
			if (cycleIntervalTime < 2)
				_cycleIntervalTime = 2;
			else if (cycleIntervalTime > 100)
				_cycleIntervalTime = 100;
			else
				_cycleIntervalTime = cycleIntervalTime;

		}
		uint8_t GetCycleIntervalTime() {
			return _cycleIntervalTime;
		}

		void SetOutputValve(uint8_t valve, bool on) {
			if (valve > MAX_NUM_VAN)
				while (1);
			if (on) {
				HC595_SetBitOutput(valve);
			}
			else {
				HC595_ClearBitOutput(valve);
			}
			HC595_ShiftOut(NULL, 2, 1);
		}

		bool IsOnProcess();
		ValveControlProcessStep GetProcessStep();
		void StartValveProcess();
		void StopValveProcess();
		void ValveProcessRun();

};

#endif /* INC_VALVECONTROLPROCESS_H_ */
