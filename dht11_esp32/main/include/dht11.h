
/* #ifndef __dht11_h__
#define __dht11_h__ */
#pragma once
#include "esp_system.h"

#include <stdbool.h>
#include "soc/rmt_reg.h"

#include "driver/rmt_rx.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "assert.h"
#include "esp_log.h"
#include "esp_err.h"
#include "rom/ets_sys.h"


// 转换

void RMT_init(unsigned int DHT11_PIN);
int dht11_get_data(int *temp_x10, int *humidity);
bool rmt_rx_done_callback( rmt_channel_handle_t channel, const rmt_rx_done_event_data_t *done_event_data, void *user_data);
int parse_items(rmt_symbol_word_t *item, int item_num, int *humidity, int *temp_x10);

