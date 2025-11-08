#include "sonic.h"

static sonic_dev global_gpio;
static gptimer_handle_t gptimer = NULL;


esp_err_t sonic_init(sonic_dev *gpio)
{
    // gpio配置
    global_gpio = *gpio;    // 取指针的实际值

    gpio_config_t sonic_gpio_cfg = {
        .intr_type = 0,
        .mode = GPIO_MODE_INPUT_OUTPUT_OD,
        .pin_bit_mask = (1ULL << gpio->rx) | (1ULL << gpio->tx),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };

    ESP_ERROR_CHECK(gpio_config(&sonic_gpio_cfg));

    // 定时器配置
    gptimer_config_t gptimer_cfg = {
        .clk_src = SOC_MOD_CLK_APB,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1 * 1000 * 1000,
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&gptimer_cfg, &gptimer));
    ESP_ERROR_CHECK(gptimer_enable(&gptimer));
    //ESP_ERROR_CHECK(gptimer_start(&gptimer));

    // 报警配置
    gptimer_alarm_config_t alarm_cfg = {
        .alarm_count = 0,
        .flags.auto_reload_on_alarm = true,
        .reload_count = 1 * 1000 * 1000,
    };

    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_cfg));

    gptimer_event_callbacks_t event_cbs = {
        .on_alarm = timer_alarm_cb,
    };

    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &event_cbs, NULL));


    return ESP_OK;
}

esp_err_t sonic_send()
{
    uint8_t i;
    ESP_ERROR_CHECK(gpio_set_direction(global_gpio.tx, GPIO_MODE_OUTPUT));
    for (i = 0; i < 6; i++) {
        gpio_set_level(global_gpio.rx, 1);
        ets_delay_us(14);
        gpio_set_level(global_gpio.rx, 0);
        ets_delay_us(14);
    }

    return ESP_OK;
}

esp_err_t sonic_measure()
{
    uint16_t distance;

    ESP_ERROR_CHECK(gpio_set_direction(global_gpio.rx, GPIO_MODE_INPUT));
    ESP_ERROR_CHECK(sonic_send());

    while (gpio_get_level(global_gpio.rx) == 0);
    ESP_ERROR_CHECK(gptimer_start(&gptimer));

    return ESP_OK;
}

static bool timer_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
    return false;
}


/* uchar distance;

    TR1 = 0;
    TL1 = 0; TH1 = 0;
    TF1 = 0;

    sonic_send();
    while (rx == 0);

    TR1 = 1;

    while (rx == 1 && TF1 == 0);
    TR1 = 0;

    if (TF1 == 1)
    {
        distance = 255;
    }
    else
    {
        distance = (uchar) ((TH1 << 8) | TL1)* 0.017;
    }

    return distance;
 */