#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "string.h"

QueueHandle_t queue_handle = NULL;

typedef struct {
    int value;
} queue_data_t;


void recieve_queue()
{
    queue_data_t data;
    static int last_data = 0;
    while (1) {
        if (xQueueReceive(queue_handle, &data, 100) == pdTRUE) {
            if (last_data != data.value) {
                ESP_LOGI("freertos", "receive queue value: %d", data.value);
                last_data = data.value;
            }

        }
    }
}

void add_queue()
{
    queue_data_t data;
    memset(&data, 0, sizeof(queue_data_t));
    while (1) {
        xQueueSend(queue_handle, &data, 100);
        vTaskDelay(pdMS_TO_TICKS(1000));
        data.value++;
    }
}

void app_main(void)
{
    queue_handle = xQueueCreate(10, sizeof(queue_data_t));
    ESP_LOGI("233", "print(%d)", sizeof(queue_data_t));
    xTaskCreatePinnedToCore(recieve_queue, "taskA", 2048, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(add_queue, "taskB", 2048, NULL, 3, NULL, 1);
}
