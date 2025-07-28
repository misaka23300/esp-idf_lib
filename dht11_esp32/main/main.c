#include "freertos\FreeRTOS.h"
#include "freertos\task.h"
#include "driver\gpio.h"
#include "dht11.h"
#include "esp_log.h"
#define DHT11_PIN GPIO_NUM_14

void taskA();


void app_main()
{
    ESP_LOGI("main", "esp32 s3 start");
    xTaskCreatePinnedToCore(taskA, "dht11",  2048*2, NULL, 1, NULL, 1);
    //taskA();
}


void taskA()
{
    int temp = 0;
    int humidity = 0;

    
    RMT_init(DHT11_PIN);
    while (1) {
        dht11_get_data(&temp, &humidity);
        //ESP_LOGI("main", "temp: %d.%d", temp / 10, temp % 10);
        ESP_LOGI("main", "temp->%i.%i C     hum->%i%%", temp / 10, temp % 10, humidity);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}