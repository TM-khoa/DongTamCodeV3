#include "GUI.h"
#include "../BoardParameter.h"
#include "esp_timer.h"
#include "../OnlineManage/OnlineManage.h"
TaskHandle_t taskHandleGUI;
GUI_Manager gui;
EventGroupHandle_t evgGUI;
extern BoardParameter brdParam;
Page prePage;
void HandleEventResetLCD();
void OnPageRun();
void HandleNextPageEvent();
void HandleChangeValueEvent();
void SaveFlashAction();
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
    gui.ShowPointer();
    for(;;){
        if(xTaskNotifyWait(pdFALSE,pdTRUE,&e,10/portTICK_PERIOD_MS)){
            gui.ClearPointer();
            gui.WaitForButtonEvent(e);
            ParamID id = paramMappingDisplay[gui.GetParamDisplayIndex()]; 
            gui.AllowSpeedUpValueIfPointerNowIsValue(id);
            HandleNextPageEvent();
            SaveFlashAction();
            HandleChangeValueEvent();
            GUI_LoadParamsToBuffer();
            gui.ShowPointer();
        }
        OnPageRun();
        HandleEventResetLCD();
    }
}

void OnPageRun(){
    if(gui.GetCurrentPage() != PAGE_RUN) return;
    EventBits_t e = xEventGroupWaitBits(evgGUI,SHIFT_BIT_LEFT(GUI_EVT_UPDATE_VALUE_FROM_UART), pdTRUE, pdFALSE, 0);
    if(CHECKFLAG(e,GUI_EVT_UPDATE_VALUE_FROM_UART) == false) return;
    static uint8_t valueLengthPre[5] = {0};
    static uint32_t showWifiStatusTick = 0;
    static bool isWiFiStatusAlreadyShown = false;
    static uint8_t sendServerFailCount = 0;
    // Sau 1s sẽ bật cờ hiển thị trạng thái kết nối mạng
    if((uint32_t)(esp_timer_get_time()/1000) - showWifiStatusTick > 2000){
        showWifiStatusTick = (uint32_t)(esp_timer_get_time()/1000);
        EventBits_t e = xEventGroupGetBits(evgGUI);
        if(CHECKFLAG(e,GUI_EVT_PAGE_RUN_SHOW_NETWORK_STATUS) == true){
            xEventGroupClearBits(evgGUI,SHIFT_BIT_LEFT(GUI_EVT_PAGE_RUN_SHOW_NETWORK_STATUS));
            gui.clear();
            vTaskDelay(20/portTICK_PERIOD_MS);
            isWiFiStatusAlreadyShown = false;
        }
        else {
            xEventGroupSetBits(evgGUI,SHIFT_BIT_LEFT(GUI_EVT_PAGE_RUN_SHOW_NETWORK_STATUS));
            isWiFiStatusAlreadyShown = false;
        }
    }

    e = xEventGroupGetBits(evgGUI);
    if(CHECKFLAG(e,GUI_EVT_PAGE_RUN_SHOW_NETWORK_STATUS) == true){
        if(isWiFiStatusAlreadyShown == true) return;
        gui.clear();
        vTaskDelay(20/portTICK_PERIOD_MS); // wait LCD to clear screen
        if(OnlineManage_CheckEvent(ONLEVT_STA_GOT_IP)){
            gui.print("WIFI CONNECTED",0,0);
            gui.print(OnlineManage_GetStationSSID(),0,1);
            if(OnlineManage_GetCodeHTTP() != HTTP_OK){// HTTP_OK
                gui.print("Connect Server fail",0,2);
                char s[20] = {0};
                sprintf(s,"Err:%d",OnlineManage_GetCodeHTTP());
                gui.print(s,0,3);
                sendServerFailCount++;
                ESP_LOGI("GUI","Send fail:%u",sendServerFailCount);
                if(sendServerFailCount >= 50){
                    sendServerFailCount = 0;
                    wifi_manager_disconnect_async();
                }
            } 
            else sendServerFailCount = 0;
        }
        else {
            gui.print("WIFI NOT CONNECT",0,0);
            char s[30] = "AP:";
            strcat(s,DEFAULT_AP_SSID);
            gui.print(s,0,1);
            memset(s,0,strlen(s));
            strcpy(s,"Pass:");
            strcat(s,DEFAULT_AP_PASSWORD);
            gui.print(s,0,2);
            memset(s,0,strlen(s));
            strcpy(s,"IP:");
            strcat(s,DEFAULT_AP_IP);
            gui.print(s,0,3);
        }
        isWiFiStatusAlreadyShown = true;
        return;
    }

    // Hiển thị thông số bình thường trên LCD
    char s[LCD_COLS] = {0};
    float ams5195 = brdParam.GetPressureAMS5915();
    float sp100 = brdParam.GetPressureSP100();
    uint16_t *maxPressure = (uint16_t*)brdParam.GetValueAddress(PARAM_DP_HIGH);
    uint16_t *minPressure = (uint16_t*)brdParam.GetValueAddress(PARAM_DP_LOW);
    uint8_t pressureBarLevel = gui.CalculateLevelFromPressure(*maxPressure,*minPressure,ams5195);
    gui.SetLevel(pressureBarLevel);
    gui.OutputStatusLED(1);
    gui.OutputErrorLED(0);
    uint8_t currentVavleTrigger = brdParam.GetCurrentValveTrigger();
    uint16_t valveStatus = brdParam.GetValveStatus();
    RTC_t t = brdParam.GetRTC();
    uint8_t lcdRow = 0;
    int temp;
    // In giá trị áp suất AMS5915
    strcpy(s,"AMS5915:");
    temp = sprintf(s + strlen(s),"%.2f",ams5195);
    //Nếu số ô cần hiển thị nhỏ hơn trước đó thì xóa bớt các ô hiển thị thừa của giá trị cũ 
    if((uint8_t)temp < valueLengthPre[lcdRow]){
        for(uint8_t i = temp; i < valueLengthPre[lcdRow]; i++){
            strcat(s," ");
        }
        valueLengthPre[lcdRow] = temp;
    }
    else {
        valueLengthPre[lcdRow] = temp;
    }
    gui.print(s,0,lcdRow);
    gui.print("mbar",LCD_COLS - strlen("mbar"),lcdRow);
    memset(s,0,strlen(s));
    // In giá trị áp suất SP100
    lcdRow++;
    strcpy(s,"SP100:");
    temp = sprintf(s + strlen(s),"%.2f",sp100);
    //Nếu số ô cần hiển thị nhỏ hơn trước đó thì xóa bớt các ô hiển thị thừa của giá trị cũ 
    if((uint8_t)temp < valueLengthPre[lcdRow]){
        for(uint8_t i = temp; i < valueLengthPre[lcdRow]; i++){
            strcat(s," ");
        }
        valueLengthPre[lcdRow] = temp;
    }
    else {
        valueLengthPre[lcdRow] = temp;
    }
    gui.print(s,0,lcdRow);
    gui.print("MPa",LCD_COLS - strlen("MPa"),lcdRow);
    memset(s,0,strlen(s));
    // In thời gian thực
    lcdRow++;
    strcpy(s,"Time:");
    temp = sprintf(s + strlen(s),"%u:%u:%u %u/%u",t.hour,t.minute,t.second,t.day,t.month);
    //Nếu số ô cần hiển thị nhỏ hơn trước đó thì xóa bớt các ô hiển thị thừa của giá trị cũ 
    if((uint8_t)temp < valueLengthPre[lcdRow]){
        for(uint8_t i = temp; i < valueLengthPre[lcdRow]; i++){
            strcat(s," ");
        }
        valueLengthPre[lcdRow] = temp;
    }
    else {
        valueLengthPre[lcdRow] = temp;
    }
    gui.print(s,0,lcdRow);
    memset(s,0,strlen(s));
    // In valve đang kích và trạng thái valve
    lcdRow++;
    strcpy(s,"Valve:");
    temp = sprintf(s + strlen(s),"%u ",currentVavleTrigger + 1);
    valveStatus > 0 ? strcat(s,"OK") : strcat(s,"Err");
    if(valveStatus == 0){
        gui.OutputErrorLED(1);
        gui.OutputStatusLED(0);
    }
    if((uint8_t)temp < valueLengthPre[lcdRow]){
        for(uint8_t i = temp; i < valueLengthPre[lcdRow]; i++){
            strcat(s," ");
        }
        valueLengthPre[lcdRow] = temp;
    }
    else {
        valueLengthPre[lcdRow] = temp;
    }
    gui.print(s,0,lcdRow);
    memset(s,0,strlen(s));
}

void HandleNextPageEvent(){
    EventBits_t e = xEventGroupWaitBits(evgGUI,
    SHIFT_BIT_LEFT(GUI_EVT_NEXT_PAGE)
    ,pdTRUE,pdFALSE,0);
    if(CHECKFLAG(e,GUI_EVT_NEXT_PAGE) == true){
        gui.clear(); // clear LCD
        vTaskDelay(20/portTICK_PERIOD_MS);
    } else return;
    if(gui.GetCurrentPage() == PAGE_SETTING){
        // Cần phải có sự kiện này thì màn hình mới refresh, không thì sẽ không hiển thị
        xEventGroupSetBits(evgGUI,SHIFT_BIT_LEFT(GUI_EVT_REFRESH_NEXT_PARAMS_DISPLAY));
        gui.ResetPointer();
        gui.ResetBufferGUI();
        gui.ResetParamDisplayIndex();
    }
}

void SaveFlashAction()
{
    if(gui.GetCurrentPage() != PAGE_SETTING) return;
    EventBits_t e;
    e = xEventGroupWaitBits(evgGUI,
    SHIFT_BIT_LEFT(GUI_EVT_WRITE_PARAMS_TO_FLASH), pdTRUE, pdFALSE, 0);
    if(CHECKFLAG(e,GUI_EVT_WRITE_PARAMS_TO_FLASH) == false) return;
    ESP_ERROR_CHECK(brdParam.SaveParamsValueToFlash());
    gui.clear();
    vTaskDelay(20/portTICK_PERIOD_MS);
    gui.print("Save data",0,0);
    vTaskDelay(2000/portTICK_PERIOD_MS);
    gui.PrintParamsToLCD();
}




void HandleEventResetLCD(){
    EventBits_t e;
    // Kiểm tra nếu có sự kiện refresh và load thông số mới vào bộ đệm
    e = xEventGroupWaitBits(evgGUI,
    SHIFT_BIT_LEFT(GUI_EVT_RESET_LCD), pdTRUE, pdFALSE, 0);  // Nếu có sự kiện reset LCD
    if(!CHECKFLAG(e,GUI_EVT_RESET_LCD)) return;
    gui.reinit_hd44780();
}

/**
 * @brief Load các thông số vào bộ đệm GUI
 */
void GUI_LoadParamsToBuffer(){
    if(gui.GetCurrentPage() != PAGE_SETTING) return;
    EventBits_t e;
    // Kiểm tra nếu có sự kiện refresh và load thông số mới vào bộ đệm
    e = xEventGroupWaitBits(evgGUI,
    SHIFT_BIT_LEFT(GUI_EVT_REFRESH_NEXT_PARAMS_DISPLAY) | // load 4 thông số từ hàng đầu xuống cuối cùng tính từ thông số hiện tại
    SHIFT_BIT_LEFT(GUI_EVT_REFRESH_PREVIOUS_PARAMS_DISPLAY) // load 4 thông số từ hàng cuối cùng lên hàng đầu tiên tính từ thông số hiện tại
    ,pdTRUE,pdFALSE,0);

    uint8_t currentParamDisplayIndex = gui.GetParamDisplayIndex();
    ParamID id;
    Parameter_t *pParam = NULL; //biến này sẽ dùng để nạp vào bộ đệm của GUI
    
    

    // Lấy ra thứ tự của các phần tử hiện tại và tiếp theo của Mảng ánh xạ thông số
    if(CHECKFLAG(e,GUI_EVT_REFRESH_NEXT_PARAMS_DISPLAY) == true) {
        // Reset bộ đệm mỗi khi nạp 4 thông số mới
        gui.ResetBufferGUI();
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
    else if (CHECKFLAG(e,GUI_EVT_REFRESH_PREVIOUS_PARAMS_DISPLAY) == true){
        // Reset bộ đệm mỗi khi nạp 4 thông số mới
        gui.ResetBufferGUI();
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
    if(gui.GetCurrentPage() != PAGE_SETTING || gui.GetPointerNow() != IS_VALUE) return;
    EventBits_t e;
    // Kiểm tra nếu có sự kiện tăng hoặc giảm giá trị
    e = xEventGroupWaitBits(evgGUI,
    SHIFT_BIT_LEFT(GUI_EVT_INCREASE_VALUE) | 
    SHIFT_BIT_LEFT(GUI_EVT_DECREASE_VALUE) 
    ,pdTRUE,pdFALSE,0);
    ParamID id = paramMappingDisplay[gui.GetParamDisplayIndex()];
    if(CHECKFLAG(e,GUI_EVT_INCREASE_VALUE) == true) {
        brdParam.IncreaseNextValue(id);
    }
    else if(CHECKFLAG(e,GUI_EVT_DECREASE_VALUE) == true) {
        brdParam.DecreasePreviousValue(id);
    }
    else return;
    
    //Thực hiện in ra màn hình chỉ duy nhất giá trị thay đổi ở hàng thứ _py mà con trỏ màn hình đang trỏ tới

    // Khởi tạo bộ đệm chuỗi chứa giá trị cần thay đổi trên màn hình
    char strBuffer[10] = {0};
    // Lấy thông số từ BoardParameter của phần tử thứ id
    Parameter_t *pParam = NULL; //biến này sẽ dùng để nạp vào bộ đệm của GUI
    int nowOccupyScreenSlot = 0; // biến này dùng để chứa số ô màn hình chiếm dụng để in ra giá trị của thông số
    // Nếu không reset board thì giá trị hợp lệ
    ESP_ERROR_CHECK(brdParam.GetParameter(&pParam,id));
    // Lúc này thay vì in theo index i thì sẽ in theo id
    if(pParam->dataType == TYPE_STRING){
        // Tạo con trỏ chuỗi lấy giá trị từ con trỏ cấp 2 ép kiểu từ void* cộng với thứ tự phần tử index để trỏ tới chuỗi cần lấy giá trị
        const char *strVal = *((const char**)pParam->value + pParam->index);
        strcpy(strBuffer,strVal);
        nowOccupyScreenSlot = (uint8_t)strlen(strVal);
    } 
    else {
        // ép kiểu con trỏ void* của value về uint16_t* để lấy ra giá trị của thông số
        uint16_t *value = (uint16_t*)pParam->value;
        nowOccupyScreenSlot = (uint8_t)sprintf(strBuffer,"%d",*value);
    }
    // Nếu có đơn vị đo thì ghép vào giá trị thông số
    if(pParam->unit != NULL){
        const char *unit = NULL;
        unit = pParam->unit;
        strcat(strBuffer,unit);
    }
    // Kiểm tra số ô màn hình đang chiếm dụng hiện tại so với lúc refresh màn hình thông số có nhỏ hơn không
    uint8_t preSlot = gui.GetPreValueOccupyScreenSlot(gui.GetPy());
    if(nowOccupyScreenSlot != preSlot){
        if(nowOccupyScreenSlot < preSlot){
            // Xóa bớt các ô thừa
            for(uint8_t j = nowOccupyScreenSlot; j < preSlot; j++){
                // Chỉ thêm slot rỗng vào sau đơn vị đo (nếu có), không ảnh hưởng tới việc hiển thị đơn vị đo
                strcat(strBuffer," ");
            }
            // Cập nhật ô chiếm dụng màn hình mới 
        }
        gui.SetPreValueOccupyScreenSlot(nowOccupyScreenSlot,gui.GetPy());
    }

    gui.print(strBuffer,POINTER_SLOT + LENGTH_OF_PARAM + POINTER_SLOT,gui.GetPy());


}



void InitGUI()
{
    /***********************************************Init section************************************************/
    evgGUI = xEventGroupCreate();

    xEventGroupSetBits(evgGUI,SHIFT_BIT_LEFT(GUI_EVT_REFRESH_NEXT_PARAMS_DISPLAY));
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

void GUI_SetEvent(EventGUI evtGUI){xEventGroupSetBits(evgGUI,SHIFT_BIT_LEFT(evtGUI));}
bool GUI_CheckEvent(EventGUI evtGUI){
    EventBits_t e = xEventGroupGetBits(evgGUI);
    return CHECKFLAG(e,evtGUI);
}