#include "BoardParameter.h"

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

const char *TriggerValve[] = {
    "On",
    "Off",
};

const char* paramUnit[] = {
        "Pa",
        "ms",
        "s",
        "h",
};

template<typename T> 
struct Parameter_t params[21];

template<typename T> 
void BrdParam_SetParameter(struct Parameter_t *param,const char* keyName, T value, DataType dataType, uint8_t stepChange, uint16_t minValue, uint16_t maxValue){
    param->keyName = keyName;
    param->value = value;
    param->dataType = dataType
    param->stepChange = stepChange;
    param->minValue = minValue;
    param->maxValue = maxValue;
}

void BrdParam_Init(){
    BrdParam_SetParameter<uint16_t>(params<uint16_t>[0], "TotalValve", 0, TYPE_UINT16, 1, 0, 16);
    BrdParam_SetParameter<uint16_t>(params<uint16_t>[1], "DownTCycle", 6, TYPE_UINT16, 1, 0, 32);
    BrdParam_SetParameter<uint16_t>(params<uint16_t>[2], "CleanMode ", 3, TYPE_UINT16, 1, 1, 5);
    BrdParam_SetParameter<uint16_t>(params<uint16_t>[3], "TestMode  ", 3, TYPE_UINT16, 1, 0, 7);
    BrdParam_SetParameter<uint16_t>(params<uint16_t>[4], "Contrast  ", 50, TYPE_UINT16, 5, 10, 200);
    BrdParam_SetParameter<uint16_t>(params<uint16_t>[5], "DP-Low    ", 700, TYPE_UINT16, 50, 250, 4000);
    BrdParam_SetParameter<uint16_t>(params<uint16_t>[6], "DP-High   ", 1300, TYPE_UINT16, 50, 250, 4000);
    BrdParam_SetParameter<uint16_t>(params<uint16_t>[7], "DP-Alarm  ", 2300, TYPE_UINT16, 50, 300, 5000);
    BrdParam_SetParameter<uint16_t>(params<uint16_t>[8], "ODC High  ", 1000, TYPE_UINT16, 50, 250, 4000);
    BrdParam_SetParameter<uint16_t>(params<uint16_t>[9], "ODC Low   ", 250, TYPE_UINT16, 50, 250, 4000);
    BrdParam_SetParameter<uint16_t>(params<uint16_t>[10],"Pulse Time", 60, TYPE_UINT16, 10, 40, 300);
    BrdParam_SetParameter<uint16_t>(params<uint16_t>[10],"Inter Time", 10, TYPE_UINT16, 2, 40, 500);
    BrdParam_SetParameter<uint16_t>(params<uint16_t>[11],"Cycle Time", 60, TYPE_UINT16, 1, 20, 100);
    BrdParam_SetParameter<uint16_t>(params<uint16_t>[12],"OperHour  ", 0, TYPE_UINT16, 100, 0, 25000);
    BrdParam_SetParameter<uint16_t>(params<uint16_t>[13],"ServRunH  ", 3000, TYPE_UINT16, 100, 0, 25000);
    BrdParam_SetParameter<uint16_t>(params<uint16_t>[14],"ServAlarm ", 0, TYPE_UINT16, 100, 0, 25000);

}


uint8_t FindTotalStringArrayElements(const char **string, size_t sizeOfArray)
{
    for(uint8_t i = 0; i < 255; i++){
        if(*(string + i) == NULL){
            printf("%u\n",i - 1);
            return i - 1;
        }
    }
    return 0;
}