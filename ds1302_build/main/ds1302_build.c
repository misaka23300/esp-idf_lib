#include <stdio.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "ds1302.h"

// DS1302 GPIO 定义
#define DS1302_CE   GPIO_NUM_17
#define DS1302_IO   GPIO_NUM_16
#define DS1302_SCLK GPIO_NUM_4

void app_main(void)
{
    ds1302_t rtc = {
        .ce_pin   = DS1302_CE,
        .io_pin   = DS1302_IO,
        .sclk_pin = DS1302_SCLK
    };

    if (ds1302_init(&rtc) != ESP_OK) {
        printf("DS1302 初始化失败!\n");
        return;
    }

    // 设置时间，例如 2025-08-15 15:30:00
    struct tm set_time = {0};
    set_time.tm_year = 2025 - 1900;
    set_time.tm_mon  = 7;
    set_time.tm_mday = 15;
    set_time.tm_hour = 15;
    set_time.tm_min  = 30;
    set_time.tm_sec  = 0;
    set_time.tm_wday = 5;

    if (ds1302_set_time(&rtc, &set_time) != ESP_OK) {
        printf("设置时间失败!\n");
        return;
    }
    printf("时间设置成功!\n");

    // 循环读取时间并打印
    while (1) {
        struct tm read_time = {0};
        if (ds1302_get_time(&rtc, &read_time) == ESP_OK) {
            printf("当前时间: %04d-%02d-%02d %02d:%02d:%02d 星期%d\n",
                   read_time.tm_year + 1900,
                   read_time.tm_mon + 1,
                   read_time.tm_mday,
                   read_time.tm_hour,
                   read_time.tm_min,
                   read_time.tm_sec,
                   read_time.tm_wday);
        } else {
            printf("读取时间失败!\n");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
