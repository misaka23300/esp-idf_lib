#include <stdio.h>
#include "main.h"


typedef enum 
{
    STOP,             // 停止
    START,            // 起步
    RUN,              // 正常运行
    emergency_braking // 紧急制动

} hm_state_t;

hm_state_t haulage_motor_state = STOP;
uint16_t speed_now;

esp_err_t starting()
{
    // pwm 40%
    // 延时
    // pwm调整至100%
    haulage_motor_state = RUN;
    return ESP_OK;
}

esp_err_t start(uint16_t speed)
{
    int16_t differential = speed_now = speed;
    if (speed_now < speed) {
        // pwm 上升
    } else (speed_now > speed) {

    }
}


esp_err_t emergency_braking()
{
    // pwm -> 0
    // 刹车 on
    // pwm 反接制动
}




// 司机控制器
esp_err_t driver_controller(uint8_t response)
{
    if (response == false) {
        haulage_motor_state = emergency_braking;
    }

    return ESP_OK;
}


void app_main(void)
{



}


