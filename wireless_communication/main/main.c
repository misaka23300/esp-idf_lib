#include <stdio.h>
#include "main.h"

void app_main(void)
{


}

const uint8_t *terminal_esm[] = {"FF9199924252"};



const uint8_t bit_rate[] = "AT+SYMBOL=3\r\n";




// 网关配置
esp_err_t TP2210_config()
{
    return ESP_OK;
}

// 终端控制
esp_err_t TP1107_config()
{
    return ESP_OK;
}