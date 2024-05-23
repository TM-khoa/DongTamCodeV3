#include "BoardParameter.h"
uint8_t FindTotalStringArrayElements(uint8_t sizeOfAllStringPointer);


const char *language[] = {
    "Eng",
    "Jan",
    "Fra",
    "Vie",
    "US", 
    "UK",
};

const char *displayRange[] = {
    "Pa",
    "InWc",
    "mmHg",
    "Bar",
    "MPa"
};

const char *paramCode[] = {
    "F1",
    "F2",
    "F3",
    "F4",
    "F5",
};

const char *techCode[] = {
    "F0",
};

const char *DPmode[] = {
    "F0",
};

const char *triggerValve[] = {
    "On",
    "Off",
};

ParamID preID;
uint8_t indexID = 0;

uint16_t paramInt[]= {
    0,//total
    6,//down cyc
    3,// clean mode
    3,// test mode
    50,// contrast
    700,//dp low
    1300,//dp high
    2300,//warn
    1000,//odc high
    250,//odc low
    60,//pulse
    10,//interval
    6,//cyc
    0,//oper h
    3000,// serv run
    0,// serv alarm
};

Parameter_t params[22];
BoardParameter brdParam;

/**
 * @brief Cho biết với mã id thông số tương ứng với thứ tự phần tử nào của mảng params
 * @param id 
 * @return phần tử nào của mảng params có mã id khớp với mã id đã cho
 */
uint8_t GetIndexFromParamID(ParamID id){
    uint8_t i = 0;
    uint8_t numOfParamElement = sizeof(params) / sizeof(Parameter_t);
    for(i = 0; i < numOfParamElement;i++){
        if(params[i].id == id) return i;
    }
    return 255;
}

void BoardParameter::SetParameter(Parameter_t *param, const char* keyName, void* value, DataType dataType, ParamID id, uint8_t stepChange, uint16_t minValue, uint16_t maxValue,const char* unit){
    param->keyName = keyName;
    param->unit = unit;
    param->value = value;
    param->stepChange = stepChange;
    param->dataType = dataType;
    param->minValue = minValue;
    param->maxValue = maxValue;
    param->id = id;
    // ESP_LOGI("SetParam","id:%d,dataType:%d",id,param->dataType);

}

void BoardParameter::SetParameter(Parameter_t *param, const char* keyName, void* value, DataType dataType, ParamID id, uint8_t index, uint8_t maxElement,const  char* unit)
{
    param->keyName = keyName;
    param->unit = unit;
    param->value = value;
    param->dataType = dataType;
    param->maxValue = maxElement;
    param->id = id;
    // ESP_LOGI("SetParam","id:%d,dataType:%d",id,param->dataType);
}

/**
 * @brief Tăng một đơn vị giá trị của thông số với id đã cho
 * @param id 
 */
void BoardParameter::IncreaseNextValue(ParamID id){
    uint8_t i = 0;
    if(id != preID) {
        i = GetIndexFromParamID(id);
        preID = id;
    }
    DataType dataType = params[i].dataType;
    switch(dataType){
        case TYPE_UINT8:{
            uint8_t *a = (uint8_t*)params[i].value;
            if(*a >= params[i].maxValue) *a = params[i].maxValue;
            else *a += params[i].stepChange;
        }
        break;
        case TYPE_UINT16:{
            uint16_t *a = (uint16_t*)params[i].value;
            if(*a >= params[i].maxValue) *a = params[i].maxValue;
            else *a += params[i].stepChange;
        }
        break;
        case TYPE_UINT32:{
            uint32_t *a = (uint32_t*)params[i].value;
            if(*a >= params[i].maxValue) *a = params[i].maxValue;
            else *a += params[i].stepChange;
        }
        break;
        case TYPE_FLOAT:{
            float *a = (float*)params[i].value;
            if(*a >= params[i].maxValue) *a = params[i].maxValue;
            else *a += params[i].stepChange;
        }
        break;
        case TYPE_STRING:{
            if(params[i].index >= params[i].maxValue) params[i].index = params[i].maxValue;
            else params[i].index++;
        }
        break;
        default:
        break;
    }
    
}

/**
 * @brief Giảm một đơn vị giá trị của thông số với id đã cho
 * @param id 
 */
void BoardParameter::DecreasePreviousValue(ParamID id){
    uint8_t i = 0;
    if(id != preID) {
        i = GetIndexFromParamID(id);
        preID = id;
    }
    DataType dataType = params[i].dataType;
    switch(dataType){
        case TYPE_UINT8:{
            uint8_t *a = (uint8_t*)params[i].value;
            if(*a <= params[i].minValue) *a = params[i].minValue;
            else *a -= params[i].stepChange;
        }
        break;
        case TYPE_UINT16:{
            uint16_t *a = (uint16_t*)params[i].value;
            if(*a <= params[i].minValue) *a = params[i].minValue;
            else *a -= params[i].stepChange;
        }
        break;
        case TYPE_UINT32:{
            uint32_t *a = (uint32_t*)params[i].value;
            if(*a <= params[i].minValue) *a = params[i].minValue;
            else *a -= params[i].stepChange;
        }
        break;
        case TYPE_FLOAT:{
            float *a = (float*)params[i].value;
            if(*a <= params[i].minValue) *a = params[i].minValue;
            else *a -= params[i].stepChange;
        }
        break;
        case TYPE_STRING:{
            // vì index là số nguyên dương nên khi 0 - 1 sẽ ra số dương max, do đó thêm điều kiện lớn hơn max index
            if(params[i].index == 0 || params[i].index > params[i].maxValue - 1) params[i].index = 0;
            else params[i].index--;
        }
        break;
        default:
        break;
    }
    
}

esp_err_t BoardParameter::GetParameter(Parameter_t *param, ParamID id, uint16_t *value){
    if(id != preID) {
        indexID = GetIndexFromParamID(id);
        preID = id;
    }
    ESP_LOGI("GetParam","indexID:%u, id:%d, dataType:%d",indexID, id, params[indexID].dataType);
    if(params[indexID].dataType != TYPE_UINT16) return ESP_ERR_INVALID_ARG;
    if(value != NULL) *value = *(uint16_t*)params[indexID].value;
    *param = params[indexID];
    return ESP_OK;
}

esp_err_t BoardParameter::GetParameter(Parameter_t **pParam, ParamID id){
    if(id != preID) {
        indexID = GetIndexFromParamID(id);
        preID = id;
    }
    ESP_LOGI("GetParam","indexID:%u, id:%d, dataType:%d",indexID, id, params[indexID].dataType);
    *pParam = params + indexID;
    return ESP_OK;
}

esp_err_t BoardParameter::GetParameter(Parameter_t *param, ParamID id, const char **value){
    if(id != preID) {
        indexID = GetIndexFromParamID(id);
        preID = id;
    }
    ESP_LOGI("GetParam","indexID:%u, id:%d, dataType:%d",indexID, id, params[indexID].dataType);
    if(params[indexID].dataType != TYPE_STRING) return ESP_ERR_INVALID_ARG;
    /**
     * Con trỏ void* mà params.value đang trỏ tới là con trỏ cấp hai **s trỏ tới mảng chuỗi (con trỏ cấp một là trỏ tới chuỗi)
     * Do đó cần phải ép kiểu về con trỏ cấp hai trước ((char**)s + i)(với i là phần tử thứ i của mảng chuỗi), 
     * sau đó lấy giá trị *(s + i) là giá trị của phần tử thuộc mảng chuỗi (hay chính là chuỗi - con trỏ cấp một)
    */ 
    *value = *((const char**)params[indexID].value + params[indexID].index);
    *param = params[indexID];
    return ESP_OK;
}

void BoardParameter::PrintParameter(ParamID id){
    Parameter_t param; 
    if(id > PARAM_STRING_PARAM_OFFSET && id < PARAM_END_PARAM){
        const char *s = NULL;
        ESP_ERROR_CHECK(GetParameter(&param,id,&s));
        if(param.unit != NULL)
            ESP_LOGI("PrintParam","KeyName:%s, value:%s, index:%u, maxElement:%d, unit:%s", param.keyName, s, param.index, param.maxValue, param.unit);
        else 
            ESP_LOGI("PrintParam","KeyName:%s, value:%s, index:%u, maxElement:%d", param.keyName, s, param.index, param.maxValue);
    } else {
        uint16_t valueNum = 0;
        ESP_ERROR_CHECK(GetParameter(&param,id,&valueNum));
        if(param.unit != NULL)
            ESP_LOGI("PrintParam","KeyName:%s, value:%d, step:%d, min:%d, max:%d, unit:%s", param.keyName, valueNum, param.stepChange, param.minValue, param.maxValue, param.unit);
        else
            ESP_LOGI("PrintParam","KeyName:%s, value:%d, step:%d, min:%d, max:%d", param.keyName, valueNum, param.stepChange, param.minValue, param.maxValue);
    }
    
}


void BoardParameter::PrintParameter(Parameter_t param)
{
    if(param.dataType == TYPE_STRING){
        const char *s = *((const char**)param.value + param.index);
        if(param.unit != NULL)
            ESP_LOGI("PrintParam","KeyName:%s, value:%s, index:%u, maxElement:%d, unit:%s", param.keyName, s, param.index, param.maxValue, param.unit);
        else 
            ESP_LOGI("PrintParam","KeyName:%s, value:%s, index:%u, maxElement:%d", param.keyName, s, param.index, param.maxValue);
    } else {
        uint16_t *valueNum = (uint16_t*)param.value;
        if(param.unit != NULL)
            ESP_LOGI("PrintParam","KeyName:%s, value:%d, step:%d, min:%d, max:%d, unit:%s", param.keyName, *valueNum, param.stepChange, param.minValue, param.maxValue, param.unit);
        else
            ESP_LOGI("PrintParam","KeyName:%s, value:%d, step:%d, min:%d, max:%d", param.keyName, *valueNum, param.stepChange, param.minValue, param.maxValue);
    }
    
}

void BoardParameter::Begin()
{
    uint8_t i = 0;
    brdParam.SetParameter(&params[i], "TotalValve", (void*)&paramInt[i], TYPE_UINT16, PARAM_TOTAL_VALVE, 1, 0, 16, NULL); i++;
    brdParam.SetParameter(&params[i], "DownTCycle", (void*)&paramInt[i], TYPE_UINT16, PARAM_DOWN_TIME_CYCLE, 1, 0, 32, NULL); i++;
    brdParam.SetParameter(&params[i], "CleanMode ", (void*)&paramInt[i], TYPE_UINT16, PARAM_ODC_CLEAN_MODE, 1, 1, 5, NULL); i++;
    brdParam.SetParameter(&params[i], "TestMode  ", (void*)&paramInt[i], TYPE_UINT16, PARAM_TEST_MODE, 1, 0, 7, NULL); i++;
    brdParam.SetParameter(&params[i], "Contrast  ", (void*)&paramInt[i], TYPE_UINT16, PARAM_DISPLAY_CONTRAST, 5, 10, 200, NULL); i++;
    brdParam.SetParameter(&params[i], "DP-Low    ", (void*)&paramInt[i], TYPE_UINT16, PARAM_DP_LOW, 50, 250, 4000, "Pa"); i++;
    brdParam.SetParameter(&params[i], "DP-High   ", (void*)&paramInt[i], TYPE_UINT16, PARAM_DP_HIGH, 50, 250, 4000, "Pa"); i++;
    brdParam.SetParameter(&params[i], "DP-Alarm  ", (void*)&paramInt[i], TYPE_UINT16, PARAM_DP_WARN, 50, 300, 5000, "Pa"); i++;
    brdParam.SetParameter(&params[i], "ODC High  ", (void*)&paramInt[i], TYPE_UINT16, PARAM_ODC_HIGH, 50, 250, 4000, "Pa"); i++;
    brdParam.SetParameter(&params[i], "ODC Low   ", (void*)&paramInt[i], TYPE_UINT16, PARAM_ODC_LOW, 50, 250, 4000, "Pa"); i++;
    brdParam.SetParameter(&params[i], "Pulse Time", (void*)&paramInt[i], TYPE_UINT16, PARAM_PULSE_TIME, 10, 40, 300, "ms"); i++;
    brdParam.SetParameter(&params[i], "Inter Time", (void*)&paramInt[i], TYPE_UINT16, PARAM_INTERVAL_TIME, 2, 40, 500, "s"); i++;
    brdParam.SetParameter(&params[i], "Cycle Time", (void*)&paramInt[i], TYPE_UINT16, PARAM_CYCLE_INTERVAL_TIME, 1, 20, 100, "s"); i++;
    brdParam.SetParameter(&params[i], "OperHour  ", (void*)&paramInt[i], TYPE_UINT16, PARAM_OPERATE_HOURS, 100, 0, 25000, "h"); i++;
    brdParam.SetParameter(&params[i], "ServRunH  ", (void*)&paramInt[i], TYPE_UINT16, PARAM_SERV_RUN_HOURS, 100, 0, 25000, "h"); i++;
    brdParam.SetParameter(&params[i], "ServAlarm ", (void*)&paramInt[i], TYPE_UINT16, PARAM_SERV_RUN_HOURS_ALARM, 100, 0, 25000, "h"); i++;
    brdParam.SetParameter(&params[i], "Language  ", (void*)&language,    TYPE_STRING, PARAM_LANGUAGE, 0, FindTotalStringArrayElements(sizeof(language)), NULL); i++;
    brdParam.SetParameter(&params[i], "DispRange ", (void*)&displayRange,TYPE_STRING, PARAM_DISPLAY_RANGE, 0, FindTotalStringArrayElements(sizeof(displayRange)), NULL); i++;
    brdParam.SetParameter(&params[i], "ParamCode ", (void*)&paramCode,   TYPE_STRING, PARAM_PARAM_CODE, 0, FindTotalStringArrayElements(sizeof(paramCode)), NULL); i++;
    brdParam.SetParameter(&params[i], "TechCode  ", (void*)&techCode,    TYPE_STRING, PARAM_TECH_CODE, 0, FindTotalStringArrayElements(sizeof(techCode)), NULL); i++;
    brdParam.SetParameter(&params[i], "DP mode   ", (void*)&DPmode,      TYPE_STRING, PARAM_DP_MODE, 0, FindTotalStringArrayElements(sizeof(DPmode)), NULL); i++;
    brdParam.SetParameter(&params[i], "TrigValve ", (void*)&triggerValve,TYPE_STRING, PARAM_TRIG_VALVE, 0, FindTotalStringArrayElements(sizeof(triggerValve)), NULL); i++;
}

uint8_t FindTotalStringArrayElements(uint8_t sizeOfAllStringPointer)
{
    return sizeOfAllStringPointer/sizeof(char*);
}

void InitBoardParameter(){
    brdParam.Begin();
    brdParam.IncreaseNextValue(PARAM_TOTAL_VALVE);
    brdParam.IncreaseNextValue(PARAM_DISPLAY_RANGE);
    brdParam.IncreaseNextValue(PARAM_DP_LOW);
    brdParam.DecreasePreviousValue(PARAM_LANGUAGE);
    brdParam.PrintAllParameter();
}

void BoardParameter::PrintAllParameter(){
    for(uint8_t i = PARAM_TOTAL_VALVE; i < PARAM_END_PARAM; i++){
        if(i == 17) continue;
        brdParam.PrintParameter((ParamID)i);
    }
}