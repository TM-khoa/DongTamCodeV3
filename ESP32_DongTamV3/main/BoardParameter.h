#ifndef __BOARD_PARAMETER_H__
#define __BOARD_PARAMETER_H__
#include "main.h"

#define LENGTH_OF_PARAM     11 //length of paramText

typedef enum DataType{
    TYPE_UINT8,
    TYPE_UINT16,
    TYPE_UINT32,
    TYPE_FLOAT,
    TYPE_STRING,
}DataType;

typedef struct PointerStringValue_t{
    char **s;
    uint8_t index;
    uint8_t totalElement;
}PointerStringValue_t;

template <typename T>
struct Parameter_t{
    const char keyName[LENGTH_OF_PARAM];
    T value;
    DataType dataType;
    uint8_t stepChange;
    uint16_t minValue;
    uint16_t maxValue;
    char* unit;
};

uint8_t FindTotalStringArrayElements(const char **string, size_t sizeOfArray);

#endif /*__BOARD_PARAMETER_H__*/