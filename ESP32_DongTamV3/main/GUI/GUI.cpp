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
    GUI_LoadParamsToBuffer();
    gui.ShowPointer();
    for(;;){
        if(xTaskNotifyWait(pdFALSE,pdTRUE,&e,portMAX_DELAY)){
            gui.ClearPointer();
            gui.WaitForEvent(e);
            ParamID id = paramMappingDisplay[gui.GetParamDisplayIndex()]; 
            gui.AllowSpeedUpValueIfPointerNowIsValue(id);
            GUI_LoadParamsToBuffer();
            gui.ShowPointer();
            // ESP_LOGI("GUI_Event","%lu",e);
            // ESP_LOGI("CurrentSelectedParamID","%d",id);
        }
    }
}

/**
 * @brief Load các thông số vào bộ đệm GUI
 */
void GUI_LoadParamsToBuffer(){
    EventBits_t e;
    // Kiểm tra nếu có sự kiện refresh và load thông số mới vào bộ đệm
    e = xEventGroupWaitBits(evgGUI,
    SHIFT_BIT_LEFT(PARAM_EVT_REFRESH_NEXT_PARAMS_DISPLAY) | // load 4 thông số từ hàng đầu xuống cuối cùng tính từ thông số hiện tại
    SHIFT_BIT_LEFT(PARAM_EVT_REFRESH_PREVIOUS_PARAMS_DISPLAY) // load 4 thông số từ hàng cuối cùng lên hàng đầu tiên tính từ thông số hiện tại
    ,pdTRUE,pdFALSE,0);

    uint8_t currentParamDisplayIndex = gui.GetParamDisplayIndex();
    ParamID id;
    Parameter_t *pParam = NULL; //biến này sẽ dùng để nạp vào bộ đệm của GUI
    // Reset bộ đệm mỗi khi nạp 4 thông số mới
    gui.ResetBufferGUI();

    // Lấy ra thứ tự của các phần tử hiện tại và tiếp theo của Mảng ánh xạ thông số
    if(CHECKFLAG(e,PARAM_EVT_REFRESH_NEXT_PARAMS_DISPLAY) == true) {
        // Load lần lượt 4 thông số liên tiếp vào bộ đệm GUI
        for(uint8_t i = 0; i < LCD_ROWS; i++){
            // dùng để kiểm tra các thông số tiếp theo có vượt quá phạm vi của Mảng ánh xạ thông số
            uint8_t offsetIndex = currentParamDisplayIndex + i;
            uint8_t totalParamDisplayElement = sizeof(paramMappingDisplay) / sizeof(ParamID);
            if(offsetIndex > totalParamDisplayElement - 1) break;
            // Lấy ra giá trị ParamID từ phần tử hiện tại của Mảng ánh xạ thông số trở về sau
            id = paramMappingDisplay[offsetIndex];
            /*
                Bắt buộc phải cung cấp địa chỉ biến (dấu &) thì mới lấy được dữ liệu ra ngoài đưa vào pParam
                Giả sử nếu khai báo Parameter_t **pParam để làm đối số cho brdParam.GetParameter(pParam,id)
                thì ý nghĩa là tạo một biến cục bộ trong hàm và nạp dữ liệu vào biến cục bộ đó thay vì địa chỉ của 
                biến ở ngoài, và sẽ mất dữ liệu khi thoát ra khỏi hàm
            */ 
            ESP_ERROR_CHECK(brdParam.GetParameter(&pParam,id));  
            /* 
                Nếu không có lỗi, thực hiện nạp vào bộ đệm GUI, phần tử i = 0 sẽ nằm ở hàng đầu 
                Khi tham chiếu tới địa chỉ để lấy dữ liệu (lấy dữ liệu từ địa chỉ để nạp thông tin vào hàm) thì không cần dùng dấu địa chỉ (&)
            */
            ESP_ERROR_CHECK(gui.LoadParamAddressToBufferGUI(pParam,i));
            brdParam.PrintParameter(*pParam); 
        }
    }
    else if (CHECKFLAG(e,PARAM_EVT_REFRESH_PREVIOUS_PARAMS_DISPLAY) == true){
        // Load lần lượt 4 thông số liên tiếp vào bộ đệm GUI
        for(uint8_t i = 0; i < LCD_ROWS; i++){
            // dùng để kiểm tra các thông số tiếp theo có vượt quá phạm vi của Mảng ánh xạ thông số
            uint8_t offsetIndex = currentParamDisplayIndex - i;
            uint8_t totalParamDisplayElement = sizeof(paramMappingDisplay) / sizeof(ParamID);
            // Nếu nhỏ hơn 0 thì quay về max nên kiểm tra phạm vi cũng tương tự
            if(offsetIndex > totalParamDisplayElement - 1)  break; 
            // Lấy ra giá trị ParamID từ phần tử hiện tại của Mảng ánh xạ thông số trở về trước
            id = paramMappingDisplay[offsetIndex];
            /*
                Bắt buộc phải cung cấp địa chỉ biến (dấu &) thì mới lấy được dữ liệu ra ngoài đưa vào pParam
                Giả sử nếu khai báo Parameter_t **pParam để làm đối số cho brdParam.GetParameter(pParam,id)
                thì ý nghĩa là tạo một biến cục bộ trong hàm và nạp dữ liệu vào biến cục bộ đó thay vì địa chỉ của 
                biến ở ngoài, và sẽ mất dữ liệu khi thoát ra khỏi hàm
            */ 
            ESP_ERROR_CHECK(brdParam.GetParameter(&pParam,id));  
            /* 
                Nếu không có lỗi, thực hiện nạp vào bộ đệm GUI, phần tử i = 0 sẽ nằm ở hàng cuối 
                Khi tham chiếu tới địa chỉ để lấy dữ liệu (lấy dữ liệu từ địa chỉ để nạp thông tin vào hàm) thì không cần dùng dấu địa chỉ (&)
            */
            ESP_ERROR_CHECK(gui.LoadParamAddressToBufferGUI(pParam, (LCD_ROWS - 1) - i));
            brdParam.PrintParameter(*pParam); 
        }
    }
    else return;
    gui.PrintParamsToLCD();

}

/**
 * @brief Xử lý sự kiện tăng giảm giá trị thông số khi nhấn nút trên màn hình GUI
 */
void HandleChangeValueEvent(){
    if(gui.GetPointerNow() != IS_VALUE) return;
    EventBits_t e;
    // Kiểm tra nếu có sự kiện tăng hoặc giảm giá trị
    e = xEventGroupWaitBits(evgGUI,
    SHIFT_BIT_LEFT(PARAM_EVT_INCREASE_VALUE) | 
    SHIFT_BIT_LEFT(PARAM_EVT_DECREASE_VALUE) 
    ,pdTRUE,pdFALSE,0);
    ParamID id = paramMappingDisplay[gui.GetParamDisplayIndex()];

    if(CHECKFLAG(e,PARAM_EVT_INCREASE_VALUE) == true) {
        brdParam.IncreaseNextValue(id);
    }
    else if(CHECKFLAG(e,PARAM_EVT_DECREASE_VALUE) == true) {
        brdParam.DecreasePreviousValue(id);
    }

}

void InitGUI()
{
    /***********************************************Init section************************************************/
    evgGUI = xEventGroupCreate();

    xEventGroupSetBits(evgGUI,SHIFT_BIT_LEFT(PARAM_EVT_REFRESH_NEXT_PARAMS_DISPLAY));
    gui.Begin(taskHandleGUI,evgGUI,sizeof(paramMappingDisplay)/sizeof(ParamID));
    gui.TurnOnBackLight();
    
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