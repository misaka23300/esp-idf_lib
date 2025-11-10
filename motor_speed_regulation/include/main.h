#pragma once

#include "ads1115.h"
#include "stdio.h"
#include "esp_err.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void ads1115_read_task(void *param);
float ads1115_raw_to_voltage_6V144(uint16_t raw);