/**
 * @file ads1115_driver.c
 * @brief ADS1115 ADC传感器驱动代码
 * 该文件包含ADS1115的I2C初始化和读取函数
 */


#include "ads1115.h"

// 全局变量定义
i2c_master_bus_handle_t i2c_handle = NULL;      // I2C主机总线句柄
i2c_master_dev_handle_t i2c_ads1115_handle = NULL;  // I2C设备句柄


// ads1115一共有4个寄存器，第一个和第二个记录了读取的值，
// 第三个和第四个用于配置芯片.
uint8_t ads1115_registers[4] = {0, 0, 0, 0};


/**
 * @brief I2C总线初始化函数
 * 
 * 初始化I2C主机总线并配置ADS1115设备
 * 设置SCL引脚为GPIO21，SDA引脚为GPIO22
 * 总线时钟频率为100kHz
 */

esp_err_t iic_init(gpio_num_t SCL, gpio_num_t SDA)
{
    // 配置I2C主机总线参数
    i2c_master_bus_config_t ads1115_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,      // 使用默认时钟源
        .flags.allow_pd = false,                // 禁止电源管理
        .flags.enable_internal_pullup = true,   // 启用内部上拉电阻
        .glitch_ignore_cnt = 7,                 // 毛刺过滤计数
        .i2c_port = 0,                          // 使用I2C端口0
        .intr_priority = 3,                     // 中断优先级
        .scl_io_num = SCL,              // SCL引脚
        .sda_io_num = SDA,              // SDA引脚
    };

    // 创建I2C主机总线
    return i2c_new_master_bus(&ads1115_config, &i2c_handle);

}


/**
*@brief 将ads1115挂在设备总线
*/
esp_err_t ads1115_init(uint8_t ads1115_address)
{
    if (ads1115_address == 0x00) {
        ads1115_address = 0x48;
    }

    // 配置ADS1115设备参数
    i2c_device_config_t i2c_ads1115_device_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,  // 7位设备地址
        .device_address = ads1115_address,      // ADS1115设备地址(0x48 << 1)
        .scl_speed_hz = 100000,                 // I2C时钟频率100kHz
        .scl_wait_us = 0,                       // SCL等待时间
    };

    // 将ADS1115设备添加到I2C总线
    return i2c_master_bus_add_device(i2c_handle, &i2c_ads1115_device_config, &i2c_ads1115_handle);
}


/**
 * @brief 配置ads1115功能
 */
esp_err_t ads1115_config(ads1115_config_t *ads1115_config)
{

    uint8_t temp = 0;
   
    temp = temp | (ads1115_config->mode & 0x01);
    temp = temp | (ads1115_config->fsr & 0x07) << 1; 
    temp = temp | (ads1115_config->channel & 0x07) << 4;
    temp = temp | 0x80;

    ads1115_registers[2] = temp;

    temp = 0x18;
    temp = temp | (ads1115_config->speed & 0x07) << 5;

    ads1115_registers[3] = temp;

    return ads1115_write(0x02, ads1115_registers);
}

/**
 * @brief ADS1115写入函数
 * @param write_address 要写入的寄存器地址
 * @param write_data 要写入的数据，16位
 * @return esp_err_t I2C通信结果
 */
esp_err_t ads1115_write(uint8_t write_address, uint8_t *write_data)
{
    uint8_t write_buff[3];

    write_buff[0] = write_address;
    write_buff[1] = write_data[2];
    write_buff[2] = write_data[3]; 
    
    return i2c_master_transmit(i2c_ads1115_handle, write_buff, sizeof(write_buff), -1);
}


/**
 * @brief 读取ads1115电压值
 */
esp_err_t ads1115_read(uint16_t *read_data_p)
{
    uint8_t read_buff[2];
    ESP_ERROR_CHECK(i2c_master_receive(i2c_ads1115_handle, read_buff, 2, 1));

    uint8_t read_data = read_buff[0] << 8 | read_buff[1];

    *read_data_p = read_data; 
    return ESP_OK;
}