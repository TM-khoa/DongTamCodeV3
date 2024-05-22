#include "GUI.h"
#include "../BoardParameter.h"
TaskHandle_t taskHandleGUI;
GUI_Manager gui;
EventGroupHandle_t evgGUI;
extern BoardParameter brdParam;
/**
 * @brief Thứ tự hiển thị các thông số trên màn hình khớp với giao diện gốc của Đồng Tâm
 * Đây không phải thứ tự thực sự của thông số trong BoardParameter.h mà là thứ tự đã được mapping lại
 * 
 */
ParamID paramMappingDisplay[] = {
    PARAM_PARAM_CODE,
    PARAM_TRIG_VALVE,
    PARAM_DP_LOW,
    PARAM_DP_HIGH,
    PARAM_DP_WARN,
    PARAM_PULSE_TIME,
    PARAM_INTERVAL_TIME,
    PARAM_CYCLE_INTERVAL_TIME,
    PARAM_TOTAL_VALVE,
    PARAM_DOWN_TIME_CYCLE,
    PARAM_ODC_HIGH,
    PARAM_ODC_LOW,
    PARAM_OPERATE_HOURS,
    PARAM_LANGUAGE,
    PARAM_DISPLAY_RANGE,
    PARAM_ODC_CLEAN_MODE,
    PARAM_TEST_MODE,
    PARAM_DISPLAY_CONTRAST,
    PARAM_SERV_RUN_HOURS,
    PARAM_SERV_RUN_HOURS_ALARM,
    PARAM_TECH_CODE,
    PARAM_DP_MODE,
};



void TaskManageGUI(void *pvParameter)
{
    EventBits_t e;
    InitGUI();
    for(;;){
        if(xTaskNotifyWait(pdFALSE,pdTRUE,&e,portMAX_DELAY)){
            ESP_LOGI("GUI_Event","%lu",e);
            gui.ClearPointer();
            gui.WaitForEvent(e);
            gui.ShowPointer();
            ParamID id = paramMappingDisplay[gui.GetParamDisplayIndex()]; 
            gui.AllowSpeedUpValueIfPointerNowIsValue(id);
            ESP_LOGI("CurrentSelectedParamID","%d",id);
        }
    }
}



/**
 * @brief Load các thông số ban đầu vào màn hình
 */
void GUI_LoadParamsToBuffer(){
    // Lấy ra thứ tự của phần tử hiện tại của Mảng ánh xạ thông số
    uint8_t currentParamDisplayIndex = gui.GetParamDisplayIndex();
    ParamID id;
    Parameter_t param; //biến này sẽ dùng để nạp vào bộ đệm của GUI
    // Load lần lượt 4 thông số liên tiếp vào bộ đệm GUI
    for(uint8_t i = 0; i < LCD_ROWS; i++){
        // Lấy ra giá trị ParamID từ phần tử hiện tại của Mảng ánh xạ thông số
        id = paramMappingDisplay[currentParamDisplayIndex + 0];
        // Nếu id nhận được từ Mảng ánh xạ thông số thuộc về giá trị chuỗi
        if(id > PARAM_STRING_PARAM_OFFSET){
            // Thực hiện lấy thông số từ BoardParameter
            char s[10] = {0}; //biến này có thể dùng để debug
            ESP_ERROR_CHECK(brdParam.GetParameter(&param,id,s,sizeof(s)));   
        }
        // Nếu id nhận được từ Mảng ánh xạ thông số thuộc về giá trị số
        else {
            uint16_t value = 0; //biến này có thể dùng để debug
            // Thực hiện lấy thông số từ BoardParameter
            ESP_ERROR_CHECK(brdParam.GetParameter(&param,id,&value));   
        }
        // Nếu không có lỗi, thực hiện nạp vào bộ đệm GUI
        ESP_ERROR_CHECK(gui.LoadToBufferGUI(param,i));
    }
}



void InitGUI()
{
    /***********************************************Init section************************************************/
    evgGUI = xEventGroupCreate();
    gui.Begin(taskHandleGUI,evgGUI,sizeof(paramMappingDisplay)/sizeof(ParamID));
    gui.TurnOnBackLight();
    gui.ShowPointer();
    /***********************************************Test section************************************************/

    // Chạy vòng lặp không thoát trong phần test nút nhấn GUI, chỉ dùng để test
    // btnGUI.TestScanBtn();
}


/**
 * @brief Dùng để nhận thông tin từ các task khác thông qua phương pháp TaskNotify
 * Đây là đối số truyền vào khi khởi tạo GUITask dùng hàm xTaskCreate
 * 
 * @return TaskHandle_t* 
 */
TaskHandle_t* GUI_GetTaskHandle(){ return &taskHandleGUI;}