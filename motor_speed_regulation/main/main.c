#include "main.h"


void app_main(void)
{
    ESP_LOGI("ads1115", "main函数开始初始化");
    // 初始化I2C总线
    ESP_ERROR_CHECK(iic_init(GPIO_NUM_41, GPIO_NUM_42));
    
    ESP_ERROR_CHECK(ads1115_init(0x48));
    
    ads1115_config_t ads1115_cfg = {
        .channel = ain_0, 
        .fsr = sfr_6,
        .mode = 0,
        .speed = SPS_860
    };

    ESP_ERROR_CHECK(ads1115_config(&ads1115_cfg));
    ESP_LOGI("ads1115", "开始执行freeRTOS");
    xTaskCreatePinnedToCore(ads1115_read_task, "ciallo", 4096, NULL, 3, NULL, 1);
}



void ads1115_read_task(void *param)
{
    uint16_t read_data = 0;
    uint16_t *read_data_p = &read_data;
    while (1) {
        ads1115_read(read_data_p);
        float out_data = ads1115_raw_to_voltage_6V144(read_data);
        //ESP_LOGI("ads1115", "ADC value: %d (0x%04X)", out_data, out_data);
        ESP_LOGI("ADS1115", "Voltage: %.4f V", out_data);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
    

   
}


float ads1115_raw_to_voltage_6V144(uint16_t raw)
{
    int16_t val = (int16_t)raw;   // 转换为有符号
    return val * (6.144f / 32768.0f);
}
