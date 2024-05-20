#ifndef __BOARD_PARAMETER_H__
#define __BOARD_PARAMETER_H__
#include "main.h"
#include "string.h"
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
    uint16_t a;
public:
    void Begin();
    void SetParameter(Parameter_t *param, const char* keyName, void* value, DataType dataType, ParamID id, uint8_t stepChange, uint16_t minValue, uint16_t maxValue,const char* unit);
    void SetParameter(Parameter_t *param, const char* keyName, void* value, DataType dataType, ParamID id, uint8_t index, uint8_t maxElement,const char* unit);
    void PrintParameter(ParamID id);
    void PrintAllParameter();
    void IncreaseNextValue(ParamID id);
    void DecreasePreviousValue(ParamID id);
    esp_err_t GetParameter(Parameter_t *param, ParamID id, uint16_t *value);
    esp_err_t GetParameter(Parameter_t *param, ParamID id, char *value, uint16_t sizeOfOutputString);
};


uint8_t FindTotalStringArrayElements(uint8_t sizeOfAllStringPointer);

void InitBoardParameter();

#endif /*__BOARD_PARAMETER_H__*/