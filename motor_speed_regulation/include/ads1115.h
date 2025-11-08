#pragma once

#include <stdio.h>
#include <stdbool.h>
#include "driver/gpio.h"
#include "driver/i2c_types.h"
#include "driver/i2c_master.h"
#include "driver/i2c_slave.h"


typedef enum {
    sfr_6 = 0x00,
    sfr_4 = 0x01,
    sfr_2 = 0x02,
    sfr_1 = 0x03,
} fsr_t;

typedef enum {
    SPS_8 = 0x00,
    SPS_16 = 0x01,
    SPS_32 = 0x02,
    SPS_64 = 0x03,
    SPS_128 = 0x04,
    SPS_250 = 0x05,
    SPS_475 = 0x06,
    SPS_860 = 0x07,
} speed_t;

typedef struct {
    uint8_t channel;    // 要读取电压的通道
    fsr_t fsr;          // 读取的量程
    uint8_t mode;       // 读取模式 0为单次读取，1为连续读取
    speed_t speed;      // 读取速度
} ads1115_config_t;




esp_err_t iic_init(gpio_num_t SCL, gpio_num_t SDA);

esp_err_t ads1115_init(uint8_t ads1115_address);

esp_err_t ads1115_write(uint8_t write_address, uint8_t *write_data);

esp_err_t ads1115_config(ads1115_config_t *ads1115_config);

esp_err_t ads1115_read(uint16_t *read_data_p);


 /*
    1
    ads1115_config->channel 
    ads1115_config->channel 
    ads1115_config->channel 
    ads1115_config->fsr
    ads1115_config->fsr
    ads1115_config->fsr
    ads1115_config->mode

    ads1115_config->speed
    ads1115_config->speed
    ads1115_config->speed
    0
    0
    0
    1
    1
    */