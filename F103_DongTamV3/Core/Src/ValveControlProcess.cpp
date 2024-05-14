/*
 * ValveControlProcess.cpp
 *
 *  Created on: May 12, 2024
 *      Author: SpiritBoi
 *
 */

#include <ValveControlProcess.h>

void ValveControl::StartValveProcess() {

}
void ValveControl::StopValveProcess() {

}

void ValveControl::Begin() {
	if (_totalValve > MAX_NUM_VAN) while (1);
	uint8_t n = 0;
	if (_totalValve > 8)
		n = 2;
	else if (_totalValve < 8 && _totalValve > 0) n = 1;
	HC595_SetTarget(&_hc595);
	HC595_ClearByteOutput(0xffffffff);
	HC595_ShiftOut(NULL, n, 1);
}
void ValveControl::AssignControlPin(GPIO_TypeDef *gpio, uint16_t pin, HC595_PinName HC595_PinName) {
	if (HC595_AssignPin(&_hc595, gpio, pin, HC595_PinName) != HC595_OK) while (1);
}

void ValveControl::IncreaseTick() {
	_timerTick++;
}
void ValveControl::SetTick(uint16_t tick) {
	_timerTick = tick;
}
uint16_t ValveControl::GetTick() {
	return _timerTick;
}

HAL_StatusTypeDef ValveControl::SetTotalValve(uint8_t totalValve) {
	if (totalValve > MAX_NUM_VAN) return HAL_ERROR;
	_totalValve = totalValve;
	return HAL_OK;
}
uint8_t ValveControl::GetTotalValve() {
	return _totalValve;
}

HAL_StatusTypeDef ValveControl::SetPulseTime(uint16_t pulseTime) {
	if (pulseTime < 30 || pulseTime > 300) return HAL_ERROR;
	_pulseTime = pulseTime;
	return HAL_OK;
}
uint16_t ValveControl::GetPulseTime() {
	return _pulseTime;
}

HAL_StatusTypeDef ValveControl::SetCurrentValveOn(uint16_t currentValveOn) {
	if (currentValveOn > MAX_NUM_VAN) return HAL_ERROR;
	_currentValveOn = currentValveOn;
	return HAL_OK;
}
uint16_t ValveControl::GetCurrentValveOn() {
	return _currentValveOn;
}

HAL_StatusTypeDef ValveControl::SetIntervalTime(uint16_t intervalTime) {
	if (intervalTime > 0 && intervalTime <= 100) {
		_intervalTime = intervalTime;
	}
	return HAL_ERROR;
}
uint16_t ValveControl::GetIntervalTime() {
	return _intervalTime;
}

void ValveControl::SetOutputValve(uint8_t valve, bool on) {
	if (valve > MAX_NUM_VAN) while (1);
	uint8_t n = 0;
	if (_totalValve > 8)
		n = 2;
	else if (_totalValve < 8 && _totalValve > 0) n = 1;
	if (on) {
		HC595_SetBitOutput(valve);
	}
	else {
		HC595_ClearBitOutput(valve);
	}
	HC595_ShiftOut(NULL, n, 1);
}

