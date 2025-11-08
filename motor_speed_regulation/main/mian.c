#include "main.h"


void app_main(void)
{
    // 初始化I2C总线
    iic_init(GPIO_NUM_43, GPIO_NUM_45);
    
    ads1115_init(0x00);
    
    ads1115_config_t ads1115_cfg = {
        .channel = 0x01,
        .fsr = sfr_6,
        .mode = 0,
        .speed = SPS_860
    };

    ads1115_config(&ads1115_cfg);
  
     xTaskCreatePinnedToCore(ads1115_read_task, "ciallo", 2048, NULL, 3, NULL, 1);
}



void ads1115_read_task(void *param)
{
    uint16_t *read_data = NULL;

    ads1115_read(read_data);
   

    ESP_LOGI("ads1115", "ADC value: %d (0x%04X)", read_data, read_data);

   
}
