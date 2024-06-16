#ifndef _GUI_NAVIGATOR_H_
#define _GUI_NAVIGATOR_H_

/**
 * @author: SpiritBoi
 * @brief: Module này chỉ có tác dụng điều hướng thông tin dữ liệu trên màn hình LCD, không thực hiện việc tác động
 * đến chức năng hiển thị của màn hình như module GUI hoặc thay đổi thông số như module BoardParamter
 * Đầu vào nhận sự kiện từ module nút nhấn EventButton và gửi sự kiện yêu cầu module GUI, BoardParameter hoặc module khác(nếu có) 
 * thực hiện hành động dựa trên sự kiện đầu vào EventButton
*/

#include "esp_err.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "ButtonGUI.h"
#include "../BoardParameter.h"
#include "LCD_I2C.hpp"

/**
 * @note: Để tiện trong việc gọi tên thì mục này dùng để quy ước và chú thích tên gọi một số từ 
 * để tránh ghi nhiều lần:
 * Mảng ánh xạ thông số: chỉ tên gọi của mảng mà mỗi phần tử là kiểu ParamID được sắp xếp theo thứ tự hiển thị
 * trên màn hình LCD (theo mong muốn người dùng) chứ không sắp xếp như BoardParameter (tiện cho việc sắp xếp cấu trúc dữ liệu)
 * Tên hiện tại: paramMappingDisplay
*/


/**
 * @brief POINTER_SLOT Ô dùng để hiển thị con trỏ trên màn hình, sẽ chiếm dụng một cột 
 * trước tên thông số và 1 cột trước giá trị của các thông số trên màn hình
 */
#define POINTER_SLOT 1 

/**
 * @brief Cho biết hiện tại con trỏ đang trỏ tới tên thông số hay giá trị của nó
 * 
 */
typedef enum PointerNow{
    IS_KEYWORD = 1,
    IS_VALUE,
}PointerNow; 

typedef enum Page{
    PAGE_START,
    PAGE_RUN,
    PAGE_SETTING,
    // PAGE_CONTROL,
    PAGE_END,
}Page; 


typedef enum EventGUI{
    GUI_EVT_WRITE_PARAMS_TO_FLASH,
    GUI_EVT_GET_PARAMS_FROM_FLASH,
    GUI_EVT_VALUE_REACH_LIMIT,
    GUI_EVT_REFRESH_NEXT_PARAMS_DISPLAY,
    GUI_EVT_REFRESH_PREVIOUS_PARAMS_DISPLAY,
    GUI_EVT_INCREASE_VALUE,
    GUI_EVT_DECREASE_VALUE,
    GUI_EVT_PAGE_RUN_SHOW_NETWORK_STATUS,
    GUI_EVT_RESET_LCD,
    GUI_EVT_NEXT_PAGE,
    GUI_EVT_UPDATE_VALUE_FROM_UART,
}EventGUI;
class GUI_Navigator
{
private:
    uint8_t _px; // Cột mà con trỏ đang chiếm dụng
    uint8_t _py; // Hàng mà con trỏ đang chiếm dụng
    PointerNow _pNow; // Con trỏ cho biết đang lựa chọn thông số hay thay đổi giá trị thông số
    Page _page;
    uint8_t _paramDisplayIndex;// Phần tử hiện tại của Mảng ánh xạ thông số
    uint8_t _maxDisplayParamIndex;//Tổng kích thước của Mảng ánh xạ thông số
    EventGroupHandle_t _evgGUI;// Dùng để trao đổi sự kiện từ module nút nhấn GUI (input) đến màn hình LCD (output)
    /**
     * @brief Xử lý sự kiện từ nút nhấn nhận được ở TaskScanButton
     * 
     * @param eventToHandle tên của sự kiện nút nhấn nhận được cần xử lý
     * @param eventName danh sách các sự kiện nút nhấn có thể được xử lý bởi hàm này quy định trong ButtonGUI.h
     * @param fKEY hàm xử lý sự kiện nếu con trỏ đang trỏ tới tên thông số(key)
     * @param fVALUE hàm xử lý sự kiện nếu con trỏ đang trỏ tới giá trị thông số(value)
     */
    void HandleEvent(EventBits_t eventToHandle, EventBits_t eventName,void(GUI_Navigator::*fKEY)(),void(GUI_Navigator::*fVALUE)()){
        if(eventToHandle != eventName) return;
        if(_pNow == IS_KEYWORD) {
            (this->*fKEY)();
        }
        else if(_pNow == IS_VALUE) {
            (this->*fVALUE)();
        }
    }
    /**
     * @brief Gửi sự kiện tăng giá trị của thông số đến GUI để xử lý khi nhấn nút trên màn hình
     */
    void IncreaseValue(){
        EventBits_t e = xEventGroupGetBits(_evgGUI);
        if(CHECKFLAG(e,SHIFT_BIT_LEFT(GUI_EVT_VALUE_REACH_LIMIT)) == false) 
            xEventGroupSetBits(_evgGUI,SHIFT_BIT_LEFT(GUI_EVT_INCREASE_VALUE));

    }

    /**
     * @brief Gửi sự kiện giảm giá trị của thông số đến GUI để xử lý khi nhấn nút trên màn hình
     */
    void DecreaseValue(){
        EventBits_t e = xEventGroupGetBits(_evgGUI);
        if(CHECKFLAG(e,SHIFT_BIT_LEFT(GUI_EVT_VALUE_REACH_LIMIT)) == false) 
            xEventGroupSetBits(_evgGUI,SHIFT_BIT_LEFT(GUI_EVT_DECREASE_VALUE));
    }

    /**
     * @brief Set bit cờ reset LCD khi lỗi
     */
    void SendEventResetLCD(){
        xEventGroupSetBits(_evgGUI,SHIFT_BIT_LEFT(GUI_EVT_RESET_LCD));
    }

    /**
     * @brief Set bit cờ ghi dữ liệu vào flash
     */
    void SendEventSaveParamsToFlash(){
        xEventGroupSetBits(_evgGUI,SHIFT_BIT_LEFT(GUI_EVT_WRITE_PARAMS_TO_FLASH));
    }


public:
    void Begin(uint8_t maxDisplayParamIndex, EventGroupHandle_t evgGUI){
        _px = 0;
        _py = 0;
        _page = (Page)1;
        _pNow = IS_KEYWORD;
        _paramDisplayIndex = 0;
        _maxDisplayParamIndex = maxDisplayParamIndex;
        _evgGUI = evgGUI;
    }




    /**
     * @brief Trả về phần tử thứ _paramDisplayIndex của Mảng ánh xạ thông số.
     * Giá trị từ phần tử này cho biết ID (ParamID) của thông số cần tìm.
     * @example: uint8_t i = GUI_Navigator::GetParamDisplayIndex(); 
     * ParamID id = paramMappingDisplay[i]; // lấy id thông số từ phần tử
     * @note: Đây chỉ là ví dụ, cần phải khởi tạo class GUI_Navigator thì mới sử dụng được phương thức GetParamDisplayIndex.
     * 
     * @return Phần tử hiện tại của Mảng ánh xạ thông số.
     */
    uint8_t GetParamDisplayIndex(){
        return _paramDisplayIndex;
    }

    /**
     * @brief Khi refresh màn hình, cần phải reset thông số hiển thị về phần tử thứ 0
     */
    void ResetParamDisplayIndex(){
        _paramDisplayIndex = 0;
    }

    /**
     * @brief Con trỏ màn hình LCD hiện tại đang trỏ tới giá trị thông số 
     * để tăng giảm số hoặc đổi chuỗi ký tự của thông số đang chọn.
     */
    void PointerNowIsValue(){
        _pNow = IS_VALUE;
        _px = LENGTH_OF_PARAM + 1;
    }

    /**
     * @brief Con trỏ màn hình LCD hiện tại đang trỏ tới tên của các thông số 
     * để lựa chọn thông số trong bảng paramMappingDisplay (mảng ánh xạ thông số)
     */
    void PointerNowIsKeyword(){
        _pNow = IS_KEYWORD;
        _px = 0;
    }

    /**
     * @brief Lựa chọn thông số tiếp theo trong Mảng ánh xạ thông số
     * 
     */
    void MoveNextParam(){
        // Nếu đạt tới giới hạn index thì không cho phép thay đổi hàng
        if(_paramDisplayIndex >= _maxDisplayParamIndex - 1) {
            _paramDisplayIndex = _maxDisplayParamIndex - 1;
            return;
        }
        else _paramDisplayIndex++;
        // ở hàng dưới cùng thì quay về đầu trên cùng
        if(_py >= LCD_ROWS - 1) {
            _py = 0;
            // Sự kiện yêu cầu load 4 thông số tiếp theo (từ hàng đầu tiên load xuống hàng cuối cùng) tính từ _paramDisplayIndex hiện tại
            xEventGroupSetBits(_evgGUI,SHIFT_BIT_LEFT(GUI_EVT_REFRESH_NEXT_PARAMS_DISPLAY));
        }
        else _py++;// tăng dần số hàng (con trỏ di chuyển xuống dưới màn hình)
    }

    /**
     * @brief Lựa chọn thông số trước đó trong Mảng ánh xạ thông số
     * 
     */
    void MovePreviousParam(){
        // Nếu đạt tới giới hạn index thì không cho phép thay đổi hàng
        if(_paramDisplayIndex == 0 || _paramDisplayIndex > _maxDisplayParamIndex - 1) {
            _paramDisplayIndex = 0;
            return;
        }
        else _paramDisplayIndex--;
        // ở hàng trên cùng thì quay về hàng dưới cùng
        if(_py == 0) {
            _py = LCD_ROWS - 1;
            // Sự kiện yêu cầu load 4 thông số trước đó (từ hàng dưới cùng load lên hàng đầu tiên) tính từ _paramDisplayIndex hiện tại
            xEventGroupSetBits(_evgGUI,SHIFT_BIT_LEFT(GUI_EVT_REFRESH_PREVIOUS_PARAMS_DISPLAY));
        }
        else _py--;//còn không thì giảm dần y (con trỏ di chuyển lên phía trên màn hình)
    }
    // WIP
    void MoveNextPage(){
        if((Page)(_page + 1) >= PAGE_END) _page = (Page)(PAGE_START + 1);
        else {
            _page = (Page)(_page + 1);
        }
        xEventGroupSetBits(_evgGUI,SHIFT_BIT_LEFT(GUI_EVT_NEXT_PAGE));
        ESP_LOGI("Page","Change to: %d",_page);
    }

    Page GetCurrentPage() {return _page; }

    PointerNow GetPointerNow(){return _pNow; }

    uint8_t GetPx(){return _px; }

    uint8_t GetPy(){return _py; }

    void ResetPointer() {_px = 0; _py = 0;}

    /**
     * @brief Chờ sự kiện từ nút nhấn GUI do người dùng nhấn và xử lý tương ứng
     * @param e Các sự kiện thuộc enum EventButton
     */
    void WaitForEvent(EventBits_t e){
        HandleEvent(e,SHIFT_BIT_LEFT(EVT_BTN_MENU),&GUI_Navigator::MoveNextPage,&GUI_Navigator::PointerNowIsKeyword);
        if(_page == PAGE_SETTING){
            HandleEvent(e,SHIFT_BIT_LEFT(EVT_BTN_SET),&GUI_Navigator::PointerNowIsValue,&GUI_Navigator::SendEventSaveParamsToFlash);
        }
        HandleEvent(e,SHIFT_BIT_LEFT(EVT_BTN_UP),&GUI_Navigator::MovePreviousParam,&GUI_Navigator::IncreaseValue);
        HandleEvent(e,SHIFT_BIT_LEFT(EVT_BTN_DOWN_RIGHT),&GUI_Navigator::MoveNextParam,&GUI_Navigator::DecreaseValue);
        HandleEvent(e,SHIFT_BIT_LEFT(EVT_BTN_LCD_RESET),&GUI_Navigator::SendEventResetLCD,&GUI_Navigator::SendEventResetLCD);
        ESP_LOGI("Navigator","px:%u, py:%u, pNow:%d, iDisplay:%u",_px,_py,_pNow,_paramDisplayIndex);
    }
};


#endif