#include <stdio.h>
#include <stdbool.h>
#include "driver/i2c_types.h"
#include "driver/i2c_master.h"
#include "driver/i2c_slave.h"

// 资源分配
i2c_master_bus_handle_t i2c_handle = NULL;
i2c_master_dev_handle_t i2c_dev_handle = NULL;

void iic_init()
{
    // 安装I2C主机总线
    i2c_master_bus_config_t ads1115_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .flags.allow_pd = false,
        .flags.enable_internal_pullup = true,
        .glitch_ignore_cnt = 7,
        .i2c_port = 0,
        .intr_priority = 3,
        .scl_io_num = GPIO_NUM_21,
        .sda_io_num = GPIO_NUM_22,
    };

    i2c_new_master_bus(&ads1115_config, &i2c_handle);

    // 安装I2C主机设备
    i2c_device_config_t i2c_ads1115_device_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x90,
        .scl_speed_hz = 100000,
        .scl_wait_us = 0,
    };
    i2c_master_bus_add_device(&ads1115_config, &i2c_ads1115_device_config, &i2c_dev_handle);
}

void app_main(void)
{
    iic_init();
    i2c_master_transmit(&i2c_dev_handle, 0x23, sizeof(0x23), -1);
}
