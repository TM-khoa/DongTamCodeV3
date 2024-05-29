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

