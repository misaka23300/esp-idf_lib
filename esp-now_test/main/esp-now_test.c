#include <stdio.h>
#include "esp_now.h"
#include "stdbool.h"

void app_main(void)
{

}


void esp_now_proc(void)
{
    // 调用前需要启动wifi
    esp_now_init();

    esp_now_peer_info_t now_info = {
        .channel = 0,
        .encrypt = false,   // 是否加密
        .ifidx = 0,
        .peer_addr = 0,
    };

    esp_now_add_peer(&now_info);

    // 发送数据
    esp_now_send();
}