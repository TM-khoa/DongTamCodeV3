#include "BoardParameter.h"
#include "nvs_flash.h"
#include "esp_err.h"

static uint8_t FindTotalStringArrayElements(uint8_t sizeOfAllStringPointer);
static const char* TAG = "BoardParameter";

static const char *language[] = {
    "Eng",
    "Jan",
    "Fra",
    "Vie",
    "US", 
    "UK",
};

static const char *displayRange[] = {
    "Pa",
    "InWc",
    "mmHg",
    "Bar",
    "MPa"
};

static const char *paramCode[] = {
    "F1",
    "F2",
    "F3",
    "F4",
    "F5",
};

static const char *techCode[] = {
    "F0",
};

static const char *DPmode[] = {
    "F0",
};

static const char *triggerValve[] = {
    "On",
    "Off",
};

static ParamID preID;
static uint8_t idxParam = 0;

static uint16_t paramUInt16[]= {
    0,//total
    6,//down cyc
    3,// clean mode
    3,// test mode
    50,// contrast
    300,//dp low
    1000,//dp high
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

// Dùng để lưu và truy xuất index hiện tại của thông số kiểu string vào flash
static uint8_t indexOfStringParam[PARAM_END_PARAM - PARAM_STRING_PARAM_OFFSET - 1] = {0};
static Parameter_t params[PARAM_END_PARAM - 2]; // not include PARAM_END and PARAM_START
BoardParameter brdParam;




/**
 * @brief - Cho biết với mã id thông số tương ứng với thứ tự phần tử nào của mảng params
 * @brief - Ví dụ thông số PARAM_TOTAL_VALVE do phần tử thứ 0 trong mảng params (params[0]) đang nắm giữ thông tin
 * @test - uint8_t i = GetParamIndexFromID(PARAM_TOTAL_VALVE); // i = 0, PARAM_TOTAL_VALVE = 1
 * @test - //It means params[0].id = PARAM_TOTAL_VALVE
 * @param id 
 * @return uint8_t, 255 nếu không tìm thấy phần tử tương ứng trong mảng params
 */
uint8_t GetParamIndexFromID(ParamID id){
    uint8_t numOfParamElement = sizeof(params) / sizeof(Parameter_t);
    for(uint8_t i = 0; i < numOfParamElement;i++){
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
esp_err_t BoardParameter::IncreaseNextValue(ParamID id){
    if(id != preID) {
        idxParam = GetParamIndexFromID(id);
        preID = id;
    } 
    DataType dataType = params[idxParam].dataType;
    // not found param index match
    if(idxParam == 255) return ESP_ERR_INVALID_ARG;
    switch(dataType){
        case TYPE_UINT8:{
            uint8_t *a = (uint8_t*)params[idxParam].value;
            if(*a >= params[idxParam].maxValue) *a = params[idxParam].maxValue;
            else *a += params[idxParam].stepChange;
        }
        break;
        case TYPE_UINT16:{
            uint16_t *a = (uint16_t*)params[idxParam].value;
            if(*a >= params[idxParam].maxValue) *a = params[idxParam].maxValue;
            else *a += params[idxParam].stepChange;
        }
        break;
        case TYPE_UINT32:{
            uint32_t *a = (uint32_t*)params[idxParam].value;
            if(*a >= params[idxParam].maxValue) *a = params[idxParam].maxValue;
            else *a += params[idxParam].stepChange;
        }
        break;
        case TYPE_FLOAT:{
            float *a = (float*)params[idxParam].value;
            if(*a >= params[idxParam].maxValue) *a = params[idxParam].maxValue;
            else *a += params[idxParam].stepChange;
        }
        break;
        case TYPE_STRING:{
            if(params[idxParam].index >= params[idxParam].maxValue - 1) params[idxParam].index = params[idxParam].maxValue - 1;
            else params[idxParam].index++;
        }
        break;
        default:
        break;
    }
    return ESP_OK;
}

/**
 * @brief Giảm một đơn vị giá trị của thông số với id đã cho
 * @param id 
 */
esp_err_t BoardParameter::DecreasePreviousValue(ParamID id){
    if(id != preID) {
        idxParam = GetParamIndexFromID(id);
        preID = id;
    }
    if(idxParam == 255 || idxParam == PARAM_START_PARAM || idxParam == PARAM_END_PARAM) return ESP_ERR_INVALID_ARG;
    DataType dataType = params[idxParam].dataType;
    switch(dataType){
        case TYPE_UINT8:{
            uint8_t *a = (uint8_t*)params[idxParam].value;
            if(*a <= params[idxParam].minValue) *a = params[idxParam].minValue;
            else *a -= params[idxParam].stepChange;
        }
        break;
        case TYPE_UINT16:{
            uint16_t *a = (uint16_t*)params[idxParam].value;
            if(*a <= params[idxParam].minValue) *a = params[idxParam].minValue;
            else *a -= params[idxParam].stepChange;
        }
        break;
        case TYPE_UINT32:{
            uint32_t *a = (uint32_t*)params[idxParam].value;
            if(*a <= params[idxParam].minValue) *a = params[idxParam].minValue;
            else *a -= params[idxParam].stepChange;
        }
        break;
        case TYPE_FLOAT:{
            float *a = (float*)params[idxParam].value;
            if(*a <= params[idxParam].minValue) *a = params[idxParam].minValue;
            else *a -= params[idxParam].stepChange;
        }
        break;
        case TYPE_STRING:{
            // vì index là số nguyên dương nên khi 0 - 1 sẽ ra số dương max, do đó thêm điều kiện lớn hơn max index
            if(params[idxParam].index == 0 || params[idxParam].index > params[idxParam].maxValue - 1) params[idxParam].index = 0;
            else params[idxParam].index--;
        }
        break;
        default:
        break;
    }
    return ESP_OK;
}

/**
 * @brief Lấy thông số có mã ID từ bảng thông số
 * @note Bắt buộc phải đưa vào địa chỉ của con trỏ cấp 1 thì mới lấy được thông tin trả về. Ví dụ Parameter_t *param. 
 * BoardParameter::GetParameter(&param, ParamID id)
 * 
 * @param pParam Địa chỉ của con trỏ cấp 1 chứa thông tin thông số
 * @param id mã định danh thông số
 * @return 
 */
esp_err_t BoardParameter::GetParameter(Parameter_t **pParam, ParamID id){
    if(id != preID) {
        idxParam = GetParamIndexFromID(id);
        preID = id;
    }
    if(idxParam == 255) return ESP_ERR_NOT_FOUND;
    ESP_LOGI("GetParam","idxParam:%u, id:%d, dataType:%d",idxParam, id, params[idxParam].dataType);
    *pParam = params + idxParam;
    return ESP_OK;
}


/**
 * @brief - Lấy giá trị chứa trong Parameter_t của phần tử đang giữ thông tin thông số id thuộc mảng params
 * @brief - Giá trị có thể là số hoặc con trỏ mảng chuỗi 
 * @test - uint16_t *a = (uint16_t*)BoardParameter::GetValueAddress(PARAM_TOTAL_VALVE);
 * @test - indexString = 1;
 * @test - const char *s = *((const char**)BoardParameter::GetValueAddress(PARAM_TRIG_VALVE) + indexString); // *s = "Off"
 * @param id Thông số cần lấy giá trị
 * @return void*
 */
void* BoardParameter::GetValueAddress(ParamID id){return params[GetParamIndexFromID(id)].value;}

void BoardParameter::PrintParameter(ParamID id){
    Parameter_t *param; 
    if(id > PARAM_STRING_PARAM_OFFSET && id < PARAM_END_PARAM){
        ESP_ERROR_CHECK(GetParameter(&param,id));
        const char *s = *((const char**)param->value + param->index);
        if(param->unit != NULL)
            ESP_LOGI("PrintParam","KeyName:%s, value:%s, index:%u, maxElement:%d, unit:%s", param->keyName, s, param->index, param->maxValue, param->unit);
        else 
            ESP_LOGI("PrintParam","KeyName:%s, value:%s, index:%u, maxElement:%d", param->keyName, s, param->index, param->maxValue);
    } else {
        ESP_ERROR_CHECK(GetParameter(&param,id));
        uint16_t *valueNum = (uint16_t*)param->value;
        if(param->unit != NULL)
            ESP_LOGI("PrintParam","KeyName:%s, value:%d, step:%d, min:%d, max:%d, unit:%s", param->keyName, *valueNum, param->stepChange, param->minValue, param->maxValue, param->unit);
        else
            ESP_LOGI("PrintParam","KeyName:%s, value:%d, step:%d, min:%d, max:%d", param->keyName, *valueNum, param->stepChange, param->minValue, param->maxValue);
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

esp_err_t BoardParameter::SaveParamsValueToFlash(){
    esp_err_t err;
    err = nvs_open("Board",NVS_READWRITE,&_brdHandleNVS);
    // ghi dữ liệu từ mảng uint16 nạp vào flash  
    if (err != ESP_OK) {
        ESP_LOGE(TAG,"NVS Write Error (%s) opening NVS handle!", esp_err_to_name(err));
        return err;
    }
    err = nvs_set_blob(_brdHandleNVS,"ParamUI16",(void*)&paramUInt16,sizeof(paramUInt16));
    if(err != ESP_OK) {
        ESP_LOGE(TAG,"NVS Write UI16 Error (%s)!", esp_err_to_name(err));
        return err;
    }
    // Lấy index từ mảng Parameter_t có kiểu dữ liệu là chuỗi nạp vào flash 
    uint8_t j = 0;
    for(uint8_t i = PARAM_LANGUAGE; i <= PARAM_TRIG_VALVE; i++){
        uint8_t idxParam = GetParamIndexFromID((ParamID)i);
        if(params[idxParam].dataType != TYPE_STRING) {
            ESP_LOGE(TAG,"NVS Param %u isn't type string",idxParam);
            return ESP_ERR_INVALID_ARG;
        }
        indexOfStringParam[j] = params[idxParam].index;
        j++;
    }
    err = nvs_set_blob(_brdHandleNVS,"ParamString",(void*)&indexOfStringParam,sizeof(indexOfStringParam));
    if(err != ESP_OK) {
        ESP_LOGE(TAG,"NVS Write str Error (%s)!", esp_err_to_name(err));
        return err;
    }
    err = nvs_commit(_brdHandleNVS);
    if(err != ESP_OK) ESP_LOGE(TAG,"NVS cannot commit");
    nvs_close(_brdHandleNVS);
    return err;
}

esp_err_t BoardParameter::ReadParamsValueFromFlash(){
    esp_err_t err;
    err = nvs_open("Board",NVS_READONLY,&_brdHandleNVS);
    if (err != ESP_OK) {
        ESP_LOGE(TAG,"NVS Read error (%s) opening NVS handle!", esp_err_to_name(err));
        return err;
    }
    size_t sz;
    // Đọc kích thước và lấy dữ liệu từ flash nạp vào mảng uint16
    err = nvs_get_blob(_brdHandleNVS,"ParamUI16",NULL,&sz);
    if(sz != sizeof(paramUInt16)){
        ESP_LOGE(TAG,"NVS Read UI16 size different: %d, %u",sz,sizeof(paramUInt16));
        return ESP_ERR_INVALID_SIZE;
    }
    err = nvs_get_blob(_brdHandleNVS,"ParamUI16",&paramUInt16,&sz);
    if(err != ESP_OK){
        ESP_LOGE(TAG,"NVS Read UI16 Error (%s) get data from NVS!", esp_err_to_name(err));
        return err;
    }
    err = nvs_get_blob(_brdHandleNVS,"ParamString",NULL,&sz);
    if(sz != sizeof(indexOfStringParam)){
        ESP_LOGE(TAG,"NVS Read size different: %d, %u",sz,sizeof(indexOfStringParam));
        return ESP_ERR_INVALID_SIZE;
    }
    err = nvs_get_blob(_brdHandleNVS,"ParamString",&indexOfStringParam,&sz);
    if(err != ESP_OK){
        ESP_LOGE(TAG,"NVS Read error (%s) get str!", esp_err_to_name(err));
        return err;
    }
    uint8_t j = 0;
    for(uint8_t i = PARAM_LANGUAGE; i <= PARAM_TRIG_VALVE; i++){
        uint8_t idxParam = GetParamIndexFromID((ParamID)i);
        if(params[idxParam].dataType != TYPE_STRING) {
            ESP_LOGE(TAG,"NVS Param %u isn't type string",idxParam);
            return ESP_ERR_INVALID_ARG;
        }
        params[idxParam].index = indexOfStringParam[j];
        j++;
    }
    return ESP_OK;
}


void BoardParameter::Begin()
{
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    


    uint8_t i = 0;
    brdParam.SetParameter(&params[i], "TotalValve", (void*)&paramUInt16[i], TYPE_UINT16, PARAM_TOTAL_VALVE, 1, 0, 16, NULL); i++;
    brdParam.SetParameter(&params[i], "DownTCycle", (void*)&paramUInt16[i], TYPE_UINT16, PARAM_DOWN_TIME_CYCLE, 1, 0, 32, NULL); i++;
    brdParam.SetParameter(&params[i], "CleanMode ", (void*)&paramUInt16[i], TYPE_UINT16, PARAM_ODC_CLEAN_MODE, 1, 1, 5, NULL); i++;
    brdParam.SetParameter(&params[i], "TestMode  ", (void*)&paramUInt16[i], TYPE_UINT16, PARAM_TEST_MODE, 1, 0, 7, NULL); i++;
    brdParam.SetParameter(&params[i], "Contrast  ", (void*)&paramUInt16[i], TYPE_UINT16, PARAM_DISPLAY_CONTRAST, 5, 10, 200, NULL); i++;
    brdParam.SetParameter(&params[i], "DP-Low    ", (void*)&paramUInt16[i], TYPE_UINT16, PARAM_DP_LOW, 50, 250, 4000, "Pa"); i++;
    brdParam.SetParameter(&params[i], "DP-High   ", (void*)&paramUInt16[i], TYPE_UINT16, PARAM_DP_HIGH, 50, 250, 4000, "Pa"); i++;
    brdParam.SetParameter(&params[i], "DP-Alarm  ", (void*)&paramUInt16[i], TYPE_UINT16, PARAM_DP_WARN, 50, 300, 5000, "Pa"); i++;
    brdParam.SetParameter(&params[i], "ODC High  ", (void*)&paramUInt16[i], TYPE_UINT16, PARAM_ODC_HIGH, 50, 250, 4000, "Pa"); i++;
    brdParam.SetParameter(&params[i], "ODC Low   ", (void*)&paramUInt16[i], TYPE_UINT16, PARAM_ODC_LOW, 50, 250, 4000, "Pa"); i++;
    brdParam.SetParameter(&params[i], "Pulse Time", (void*)&paramUInt16[i], TYPE_UINT16, PARAM_PULSE_TIME, 10, 40, 300, "ms"); i++;
    brdParam.SetParameter(&params[i], "Inter Time", (void*)&paramUInt16[i], TYPE_UINT16, PARAM_INTERVAL_TIME, 2, 40, 500, "s"); i++;
    brdParam.SetParameter(&params[i], "Cycle Time", (void*)&paramUInt16[i], TYPE_UINT16, PARAM_CYCLE_INTERVAL_TIME, 1, 20, 100, "s"); i++;
    brdParam.SetParameter(&params[i], "OperHour  ", (void*)&paramUInt16[i], TYPE_UINT16, PARAM_OPERATE_HOURS, 100, 0, 25000, "h"); i++;
    brdParam.SetParameter(&params[i], "ServRunH  ", (void*)&paramUInt16[i], TYPE_UINT16, PARAM_SERV_RUN_HOURS, 100, 0, 25000, "h"); i++;
    brdParam.SetParameter(&params[i], "ServAlarm ", (void*)&paramUInt16[i], TYPE_UINT16, PARAM_SERV_RUN_HOURS_ALARM, 100, 0, 25000, "h"); i++;
    brdParam.SetParameter(&params[i], "Language  ", (void*)&language,    TYPE_STRING, PARAM_LANGUAGE, 0, FindTotalStringArrayElements(sizeof(language)), NULL); i++;
    brdParam.SetParameter(&params[i], "DispRange ", (void*)&displayRange,TYPE_STRING, PARAM_DISPLAY_RANGE, 0, FindTotalStringArrayElements(sizeof(displayRange)), NULL); i++;
    brdParam.SetParameter(&params[i], "ParamCode ", (void*)&paramCode,   TYPE_STRING, PARAM_PARAM_CODE, 0, FindTotalStringArrayElements(sizeof(paramCode)), NULL); i++;
    brdParam.SetParameter(&params[i], "TechCode  ", (void*)&techCode,    TYPE_STRING, PARAM_TECH_CODE, 0, FindTotalStringArrayElements(sizeof(techCode)), NULL); i++;
    brdParam.SetParameter(&params[i], "DP mode   ", (void*)&DPmode,      TYPE_STRING, PARAM_DP_MODE, 0, FindTotalStringArrayElements(sizeof(DPmode)), NULL); i++;
    brdParam.SetParameter(&params[i], "TrigValve ", (void*)&triggerValve,TYPE_STRING, PARAM_TRIG_VALVE, 0, FindTotalStringArrayElements(sizeof(triggerValve)), NULL); i++;
    ReadParamsValueFromFlash();
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