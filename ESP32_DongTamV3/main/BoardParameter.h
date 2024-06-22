#ifndef __BOARD_PARAMETER_H__
#define __BOARD_PARAMETER_H__
#include "main.h"
#include "string.h"
#include "RTC_Format.h"
#include "nvs_flash.h"
#define LENGTH_OF_PARAM     11 //length of paramText

typedef enum DataType{
    TYPE_UINT8 = 1,
    TYPE_UINT16,
    TYPE_UINT32,
    TYPE_FLOAT,
    TYPE_STRING,
}DataType;



typedef enum ParamID{
    PARAM_START_PARAM,
    // unit is none
    PARAM_TOTAL_VALVE,
    PARAM_DOWN_TIME_CYCLE,
    PARAM_ODC_CLEAN_MODE,
    PARAM_TEST_MODE,
    PARAM_DISPLAY_CONTRAST,
    // unit is Pa
    PARAM_DP_LOW,
    PARAM_DP_HIGH,
    PARAM_DP_WARN,
    PARAM_ODC_HIGH,
    PARAM_ODC_LOW,
    // unit is ms
    PARAM_PULSE_TIME,
    // unit is s
    PARAM_INTERVAL_TIME,
    PARAM_CYCLE_INTERVAL_TIME,
    // unit is h
    PARAM_OPERATE_HOURS,
    PARAM_SERV_RUN_HOURS,
    PARAM_SERV_RUN_HOURS_ALARM,
    // special param to handle with string value
    PARAM_STRING_PARAM_OFFSET,
    PARAM_LANGUAGE,
    PARAM_DISPLAY_RANGE,
    PARAM_PARAM_CODE,
    PARAM_TECH_CODE,
    PARAM_DP_MODE,
    PARAM_TRIG_VALVE,
    PARAM_END_PARAM,
}ParamID;

typedef struct PointerStringValue_t{
    const char **s;
    uint8_t index;
    uint8_t totalElement;
}PointerStringValue_t;

typedef struct Parameter_t{
    const char *keyName;
    const char* unit;
    void* value;
    uint8_t index;
    uint8_t stepChange;
    uint16_t minValue;
    uint16_t maxValue;
    ParamID id;
    DataType dataType;
}Parameter_t;


class BoardParameter
{
private:
    /* data */
    RTC_t _t;
    float _pressureSP100;
    float _pressureAMS5915;
    uint8_t _currentValveTrigger;
    uint16_t _valveStatus;
    nvs_handle_t _brdHandleNVS;
public:
    void Begin();
    void SetParameter(Parameter_t *param, const char* keyName, void* value, DataType dataType, ParamID id, uint8_t stepChange, uint16_t minValue, uint16_t maxValue,const char* unit);
    void SetParameter(Parameter_t *param, const char* keyName, void* value, DataType dataType, ParamID id, uint8_t index, uint8_t maxElement,const char* unit);
    void PrintParameter(ParamID id);
    void PrintParameter(Parameter_t param);
    void PrintAllParameter();
    esp_err_t IncreaseNextValue(ParamID id);
    esp_err_t DecreasePreviousValue(ParamID id);
    esp_err_t GetParameter(Parameter_t **pParam, ParamID id);
    void* GetValueAddress(ParamID id);
    esp_err_t SaveParamsValueToFlash();
    esp_err_t ReadParamsValueFromFlash();


    void SetRTC(RTC_t t){ _t = t;}
    void SetPressureSP100(float p){_pressureSP100 = p;}
    void SetPressureAMS5915(float p){_pressureAMS5915 = p;}
    void SetValveStatus(uint16_t valveStatus){_valveStatus = valveStatus;}
    esp_err_t SetCurrentValveTrigger(uint8_t currentValveTrigger){
        if(currentValveTrigger > 16 - 1) return ESP_ERR_INVALID_ARG;
        _currentValveTrigger = currentValveTrigger;
        return ESP_OK;
    }

    RTC_t GetRTC(){ return _t;}
    float GetPressureSP100(){return _pressureSP100;}
    float GetPressureAMS5915(){return _pressureAMS5915;}
    uint16_t GetValveStatus(){return _valveStatus;}
    uint8_t GetCurrentValveTrigger(){return _currentValveTrigger;}

    RTC_t* GetAddrRTC(){ return &_t;}

};
void InitBoardParameter();

#endif /*__BOARD_PARAMETER_H__*/