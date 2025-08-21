#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "driver/ledc.h"

#define LED_GPIO GPIO_NUM_2

static EventGroupHandle_t ledc_event_handle;

bool ledc_finish_cb(const ledc_cb_param_t *param, void *user_arg)
{

}

void led_run_task()
{
    static unsigned char gpio_level = 0;

    while (1) {
        if (gpio_level) {
            gpio_level = 0;
        } else {
            gpio_level = 1;
        }
        gpio_set_level(LED_GPIO, gpio_level);
        vTaskDelay(pdMS_TO_TICKS(500));

    }
}

void app_main()
{
    // GPIO配置
    gpio_config_t led_cfg = {
        .pin_bit_mask = (1 << LED_GPIO),
        .pull_up_en =  GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .intr_type = GPIO_INTR_DISABLE
    };

    gpio_config(&led_cfg);

    // 定时器配置
    ledc_timer_config_t ledc_timer = {
        .speed_mode      = LEDC_LOW_SPEED_MODE,         // 定时器速度
        .timer_num       = LEDC_TIMER_0,                // 定时器0
        .clk_cfg         = LEDC_AUTO_CLK,               // 定时器时钟
        .freq_hz         = 5000,                        // 频率
        .duty_resolution = LEDC_TIMER_13_BIT            // 占空比

    };

    ledc_timer_config(&ledc_timer);

    // ledc通道配置
    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = LEDC_CHANNEL_0,
        .timer_sel  = LEDC_TIMER_0,
        .gpio_num   = LED_GPIO,
        .duty       = 0,
        .intr_type  = LEDC_INTR_DISABLE
    };

    ledc_channel_config(&ledc_channel);

    ledc_fade_func_install(0);
    ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 4095, 2000);
    ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, LEDC_FADE_NO_WAIT);


    ledc_cbs_t cbs = {
        .fade_cb = ledc_finish_cb
    };
    ledc_cb_register(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, &cbs, NULL);
    xTaskCreatePinnedToCore(led_run_task, "led", 2048, NULL, 3, NULL, 1);
}



typedef struct {
    unsigned char speed;
} CAR;

CAR car = {
    .speed = 23,
};

