#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

void task_A(void *param)
{
    while(1) {
        ESP_LOGI("main", "ciallo~");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void app_main(void)
{
    xTaskCreatePinnedToCore(task_A, "ciallo", 2048, NULL, 3, NULL, 1);
}
