#pragma once

#include "stdio.h"
#include "stdbool.h"

#include "driver/gpio.h"
#include "rom/ets_sys.h"
#include "driver/gptimer.h"

typedef struct {
    gpio_num_t rx;
    gpio_num_t tx;
} sonic_dev;

