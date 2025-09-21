#pragma once


#include <stdio.h>
#include <time.h>




#include "driver/gpio.h"
#include "rom/ets_sys.h"
#include "esp_err.h"
#include "esp_idf_lib_helpers.h"





typedef struct {
    gpio_mode_t SCL;
    gpio_mode_t SDA;
    gpio_mode_t CE;
    bool work;
} ds1302_t;

/// @brief 检查GPIO参数合法性，配置GPIO，检查DS1302是否工作
/// @param gpio DS1302句柄
/// @return ESP_OK成功，其他失败
esp_err_t ds1302_init(ds1302_t *gpio);

/// @brief 获取计时位，检查是否工作
/// @param gpio DS1302句柄
/// @param running 结果指针，true为正在计时
/// @return ESP_OK成功，其他失败
esp_err_t ds1302_is_running(ds1302_t *gpio, bool *running);

/// @brief 设置计时位，设置是否计时
/// @param gpio DS1302句柄
/// @param start true为开始计时，false为停止
/// @return ESP_OK成功，其他失败
esp_err_t ds1302_start(ds1302_t *gpio, bool start);

/// @brief 获取写保护位
/// @param gpio DS1302句柄
/// @param wp 结果指针，true为写保护
/// @return ESP_OK 成功，其他失败
esp_err_t ds1302_get_write_protect(ds1302_t *gpio, bool *wp);

/// @brief 设置写保护位
/// @param gpio DS1302句柄
/// @param wp true为写保护，false为可写
/// @return ESP_OK成功，其他失败
esp_err_t ds1302_set_write_protect(ds1302_t *gpio, bool wp);

/// @brief 获取时间
/// @param gpio DS1302句柄
/// @param time tm结构体指针，返回时间
/// @return ESP_OK 成功，其他失败
esp_err_t ds1302_get_time(ds1302_t *gpio, struct tm *time);

/**
 * @brief 设置时间
 * @param gpio
 * @param time
 * @return ESP_OK
 */
esp_err_t ds1302_set_time(ds1302_t *gpio, const struct tm *time);




