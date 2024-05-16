/*
 * ValveControlProcess.cpp
 *
 *  Created on: May 12, 2024
 *      Author: SpiritBoi
 *
 */

#include <ValveControlProcess.h>

#define VALVE_TO_TRIGGER (_totalValve - _valveRemainToTrigger)
void ValveControl::InValveOnProcess() {
	/*
	 * nếu totalValve thay đổi (nhỏ hơn giá trị trước đó)
	 * thì khi trừ sẽ bị âm với int8_t và ra số rất lớn(uint8_t) - lớn hơn nhiều
	 * so với tổng số valve hiện có, do đó phải quay về ban đầu cập nhật lại
	 * _valveRemainToTrigger mới
	 */
	if (VALVE_TO_TRIGGER < 0) {
		_processStep = PROCESS_START;
		return;
	}
	SetOutputValve(VALVE_TO_TRIGGER, 1);
	_processStep = PROCESS_PULSE_TIME;
	_timerTick = 0;
}
void ValveControl::InPulseTimeProcess() {
	if (_timerTick * TIMER_PERIOD_MS > _pulseTime) {
		_timerTick = 0;
		_processStep = PROCESS_VALVE_OFF;
	}
}
void ValveControl::InValveOffProcess() {
	/*
	 * nếu totalValve thay đổi (nhỏ hơn giá trị trước đó)
	 * thì khi trừ sẽ bị âm với int8_t và ra số rất lớn(uint8_t) - lớn hơn nhiều
	 * so với tổng số valve hiện có, do đó phải quay về ban đầu cập nhật lại
	 * _valveRemainToTrigger mới
	 */

	if (VALVE_TO_TRIGGER < 0) {
		_processStep = PROCESS_START;
		return;
	}
	SetOutputValve(VALVE_TO_TRIGGER, 0);
	_timerTick = 0;
	_processStep = PROCESS_INTERVAL_TIME;
}
void ValveControl::InIntervalTimeProcess() {
	if (_timerTick * TIMER_PERIOD_MS >= _intervalTime * 1000) {
		_valveRemainToTrigger--;
		_processStep = PROCESS_VALVE_ON;
		_timerTick = 0;
	}
	else if (_valveRemainToTrigger - 1 == 0) {
		_processStep = PROCESS_CYCLE_INTERVAL_TIME;
		_timerTick = 0;
	}
}
void ValveControl::InCycleIntervalTimeProcess() {
	if (_timerTick * TIMER_PERIOD_MS >= _cycleIntervalTime * 1000) {
		_processStep = PROCESS_START;
		_timerTick = 0;
	}
}

void ValveControl::ValveProcessRun() {
	if (_isOnProcess == false)
		return;
	switch (_processStep) {
	case PROCESS_IDLE:
		break;
	case PROCESS_START:
		_valveRemainToTrigger = _totalValve;
		_processStep = PROCESS_VALVE_ON;
		break;
	case PROCESS_VALVE_ON:
		InValveOnProcess();
		break;
	case PROCESS_PULSE_TIME:
		InPulseTimeProcess();
		break;
	case PROCESS_VALVE_OFF:
		InValveOffProcess();
		break;
	case PROCESS_INTERVAL_TIME:
		InIntervalTimeProcess();
		break;
	case PROCESS_CYCLE_INTERVAL_TIME:
		InCycleIntervalTimeProcess();
		break;
	case PROCESS_END:
		break;

	}
}
void ValveControl::StartValveProcess() {
	_isOnProcess = true;
	_processStep = PROCESS_START;

}
void ValveControl::StopValveProcess() {
	_isOnProcess = false;
	_timerTick = 0;
}

bool ValveControl::IsOnProcess() {
	return _isOnProcess;
}

ValveControlProcessStep ValveControl::GetProcessStep() {
	return _processStep;
}

ValveControl::ValveControl() {
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

ValveControl::~ValveControl() {

}

void ValveControl::AssignControlPin(GPIO_TypeDef *gpio, uint16_t pin, HC595_PinName HC595_PinName) {
	if (HC595_AssignPin(&_hc595, gpio, pin, HC595_PinName) != HC595_OK)
		while (1);
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

void ValveControl::SetTotalValve(uint8_t totalValve) {
	if (totalValve > MAX_NUM_VAN)
		_totalValve = MAX_NUM_VAN;
	else
		_totalValve = totalValve;
	if (_processStep != PROCESS_IDLE)
		_processStep = PROCESS_START;
}
uint8_t ValveControl::GetTotalValve() {
	return _totalValve;
}

void ValveControl::SetPulseTime(uint16_t pulseTime) {
	if (pulseTime < 30)
		pulseTime = 30;
	else if (pulseTime > 300)
		pulseTime = 300;
	else
		_pulseTime = pulseTime;
}
uint16_t ValveControl::GetPulseTime() {
	return _pulseTime;
}

void ValveControl::SetIntervalTime(uint16_t intervalTime) {
	if (intervalTime < 0)
		intervalTime = 0;
	else if (intervalTime > 100)
		_intervalTime = 100;
	else
		_intervalTime = intervalTime;
}
uint16_t ValveControl::GetIntervalTime() {
	return _intervalTime;
}

void ValveControl::SetCycleIntervalTime(uint8_t cycleIntervalTime) {
	if (cycleIntervalTime < 2)
		_cycleIntervalTime = 2;
	else if (cycleIntervalTime > 100)
		_cycleIntervalTime = 100;
	else
		_cycleIntervalTime = cycleIntervalTime;

}
uint8_t ValveControl::GetCycleIntervalTime() {
	return _cycleIntervalTime;
}

void ValveControl::SetOutputValve(uint8_t valve, bool on) {
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

ValveFeedback::ValveFeedback() {
	HC165_AssignPin(&expanderInput, _74HC165_CLK_GPIO_Port, _74HC165_CLK_Pin, HC165_CP);
	HC165_AssignPin(&expanderInput, _74HC165_DATA_GPIO_Port, _74HC165_DATA_Pin, HC165_DATA);
	HC165_AssignPin(&expanderInput, _74HC165_LOAD_GPIO_Port, _74HC165_LOAD_Pin, HC165_PL);
}

ValveFeedback::~ValveFeedback() {

}

uint16_t ValveFeedback::GetInputValue() {
	inputValue = (uint16_t) HC165_ReadState(EXPANDER_INPUT_PORT);
	return inputValue;
}

