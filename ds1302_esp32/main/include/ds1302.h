#pragma once

#include "freertos/FreeRTOS.h"
#include <stdio.h>

#include "driver/gpio.h"
#include "rom/ets_sys.h"
#include "esp_err.h"


typedef struct {
    gpio_mode_t SCL;
    gpio_mode_t SDA;
    gpio_mode_t CE;
    bool work;
} ds1302_t;