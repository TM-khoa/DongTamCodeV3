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

#define TIMER_PERIOD_MS 10
#define MAX_NUM_VAN 16

class ValveControl {
	private:

		uint16_t _timerTick;
		uint16_t _pulseTime; // thời gian kích van
		uint16_t _intervalTime; // thời gian nghỉ giữa 2 lần kích van
		uint16_t _cycleIntervalTime; //thời gian nghỉ giữa 2 chu kỳ kích van (khoảng nghỉ sau khi kích van cuối cùng và bắt đầu chu trình mới)
		uint16_t _currentValveOn;
		uint8_t _totalValve;
		HC595 _hc595;

	public:

		void Begin();
		void AssignControlPin(GPIO_TypeDef *gpio, uint16_t pin, HC595_PinName HC595_PinName);
		void IncreaseTick();
		void SetTick(uint16_t tick);
		uint16_t GetTick();

		HAL_StatusTypeDef SetTotalValve(uint8_t totalValve);
		uint8_t GetTotalValve();

		HAL_StatusTypeDef SetPulseTime(uint16_t pulseTime);
		uint16_t GetPulseTime();

		HAL_StatusTypeDef SetCurrentValveOn(uint16_t currentValveOn);
		uint16_t GetCurrentValveOn();

		HAL_StatusTypeDef SetIntervalTime(uint16_t intervalTime);
		uint16_t GetIntervalTime();

		void SetOutputValve(uint8_t valve, bool on);
		void SetOutputMultiValve(uint16_t valve, bool on);

		void StartValveProcess();
		void StopValveProcess();

};

#endif /* INC_VALVECONTROLPROCESS_H_ */
