#include "ds1302.h"
#include "stdio.h"
#include "time.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void ds1302_task();


void app_main()
{
    xTaskCreatePinnedToCore(ds1302_task, "ds1302", 2048, NULL, 1, NULL, 1);
}

void ds1302_task()
{
    ds1302_t ds1302_gpio = {
        .SCL = GPIO_NUM_4,
        .SDA = GPIO_NUM_16,
        .CE  = GPIO_NUM_17,
        .work = 0,
    };

    ds1302_init(&ds1302_gpio);
    ds1302_set_write_protect(&ds1302_gpio, 0);

    struct tm time = {
        .tm_sec  = 23,
        .tm_min  = 24,
        .tm_hour = 8,
        .tm_mday = 13,   // 日期
        .tm_mon  = 7,    // 月份（0 = 一月，所以 7 = 八月）
        .tm_year = 125,  // 年份（从 1900 开始，所以 125 = 2025）
    };

    ds1302_set_time(&ds1302_gpio, &time);
    ds1302_set_write_protect(&ds1302_gpio, 1);

    struct tm rtc;

    while (1) {
        ds1302_get_time(&ds1302_gpio, &rtc);
        ESP_LOGI("ds1302", "当前时间: %04d-%02d-%02d %02d:%02d:%02d",
                 rtc.tm_year + 1900,
                 rtc.tm_mon + 1,
                 rtc.tm_mday,
                 rtc.tm_hour,
                 rtc.tm_min,
                 rtc.tm_sec);
        vTaskDelay(pdMS_TO_TICKS(1000)); // 延时 1 秒
    }
}
