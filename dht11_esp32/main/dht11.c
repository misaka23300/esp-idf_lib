#include "dht11.h"



static rmt_channel_handle_t rx_channel_handle = NULL;

static QueueHandle_t rx_receive_queue_handle = NULL;

// rmt框架下的接收数据
static rmt_symbol_word_t rmt_symbol_word[128];

// rmt框架存放的整体数据
static rmt_rx_done_event_data_t rx_data;

int DHT11_PIN = 0;

void RMT_init(unsigned int dht11_pin)
{
    DHT11_PIN = dht11_pin;
    rmt_rx_channel_config_t rmt_config = {
        .clk_src =  RMT_CLK_SRC_DEFAULT,             // 使用默认时钟
        .gpio_num = DHT11_PIN,
        .intr_priority = 1,                         // 中断优先级
        .mem_block_symbols = 64,                    // 内存块符号数
        .resolution_hz = 1 * 1000 * 1000,           // 1MHz 分辨率
        .flags.allow_pd = false,                    // 不允许关闭电源
        .flags.invert_in = false,                   // 不翻转输入信号
        .flags.io_loop_back = false,                // 关闭回环，仅测试用
        .flags.with_dma = false,                    // 不使用 DMA
    };

    ESP_ERROR_CHECK(rmt_new_rx_channel(&rmt_config, &rx_channel_handle));

    // 第一个参数是队列的数量 第二个参数是队列中每个项目的大小 返回一个句柄
    rx_receive_queue_handle = xQueueCreate(40, sizeof(rmt_rx_done_event_data_t));

    assert(rx_receive_queue_handle);

    ESP_LOGI("dht11", "register RX done callback");

    rmt_rx_event_callbacks_t call_back = {
        .on_recv_done = rmt_rx_done_callback,
    };

    // 回调中断函数 接收结束时执行
    // rx_receive_queue(队列的句柄) -> rmt_rx_done_callback *user_data
    ESP_ERROR_CHECK(rmt_rx_register_event_callbacks(rx_channel_handle, &call_back, rx_receive_queue_handle));


    ESP_ERROR_CHECK(rmt_enable(rx_channel_handle));
}


bool IRAM_ATTR rmt_rx_done_callback(rmt_channel_handle_t channel,
                                    const rmt_rx_done_event_data_t *done_event_data,
                                    void *user_data)
{
    BaseType_t high_task_wakeup = pdFALSE;

    QueueHandle_t rx_receive_queue = (QueueHandle_t)user_data;

    // 传入句柄 传入数据
    xQueueSendFromISR(rx_receive_queue, done_event_data, &high_task_wakeup);

    return high_task_wakeup == pdTRUE;
}


int dht11_get_data(int *temp_x10, int *humidity)
{
    BaseType_t rmt_receive_done = pdFALSE;
    /*
          -------     _________
    _____|       |____|
    */
    gpio_set_pull_mode(DHT11_PIN, GPIO_PULLUP_ONLY);

    gpio_set_direction(DHT11_PIN, GPIO_MODE_OUTPUT);

    gpio_set_level(DHT11_PIN, 1);

    ets_delay_us(1 * 1000);

    gpio_set_level(DHT11_PIN, 0);

    ets_delay_us(20 * 1000);

    gpio_set_level(DHT11_PIN, 1);

    ets_delay_us(20);

    gpio_set_direction(DHT11_PIN, GPIO_MODE_INPUT);
    rmt_receive_config_t rmt_receive_config = {
        .signal_range_min_ns = 100,
        .signal_range_max_ns = 1000 * 1000,
        .flags.en_partial_rx = false,
    };

    ESP_ERROR_CHECK(rmt_receive(rx_channel_handle, rmt_symbol_word, sizeof(rmt_symbol_word), &rmt_receive_config));

    // 执行中断
    // 中断后 将数据放在rx_data
    rmt_receive_done = xQueueReceive(rx_receive_queue_handle, &rx_data, pdMS_TO_TICKS(1000));

    if (rmt_receive_done == pdTRUE) {
        return parse_items(rx_data.received_symbols, rx_data.num_symbols, humidity, temp_x10);
    } else {
        return 0;
    }

}


int parse_items(rmt_symbol_word_t *item, int item_num, int *humidity, int *temp_x10)
{
    int i = 0;
    unsigned int duration = 0;

    unsigned int relative_humidity = 0;
    unsigned int temp = 0;
    unsigned int check_sum = 0;
    unsigned int sum = 0;






    // 是否有足够的脉冲数
    if (item_num < 41) {
        ESP_LOGI("DHT11", "测量后没有足够的脉冲数");
        return 0;
    }

    // 跳过开始脉冲
    if (item_num > 41) {
        item++;
    }

    // 8bit 湿度整数数据 + 8bit湿度小数数据 + 8bit温度整数数据 + 8bit温度小数数据 + 8bit校验位。

    // 提取湿度数据
    for (i = 0; i < 16; i++, item++) {

        if (item -> level0) {
            duration = item -> duration0;
        } else {
            duration = item -> duration1;
        }

        relative_humidity = (relative_humidity << 1) + (duration < 35 ? 0 : 1);
    }

    // 提取温度数据
    for (i = 0; i < 16; i++, item++) {

        if (item -> level0) {
            duration = item -> duration0;
        } else {
            duration = item -> duration1;
        }

        temp = (temp << 1) + (duration < 35 ? 0 : 1);
    }

    // 提取校验位
    for (i = 0; i < 8; i++, item++) {
        duration = 0;
        if (item -> level0) {
            duration = item -> duration0;
        } else {
            duration = item -> duration1;
        }

        check_sum = (check_sum << 1) + (duration < 35 ? 0 : 1);
    }

    sum = ((temp >> 8) + temp + (relative_humidity >> 8) + relative_humidity);

    // 开始校验
    if ((sum & 0xFF) != check_sum) {
        ESP_LOGI("dht11", "check sum failure %4X %4X %2X", temp, relative_humidity, check_sum);
        return 0;
    }


    // 分离数据字节
    uint8_t rh_high = (relative_humidity >> 8) & 0xFF;
    uint8_t rh_low = relative_humidity & 0xFF;
    uint8_t temp_high = (temp >> 8) & 0xFF;
    uint8_t temp_low = temp & 0xFF;

    // 计算校验和
    uint8_t calc_sum = rh_high + rh_low + temp_high + temp_low;

    // 校验检查
    if (calc_sum != check_sum) {
        ESP_LOGI("dht11", "校验失败: 计算值=%02X, 接收值=%02X", calc_sum, check_sum);
        return 0;
    }

    // 数据处理
    *humidity = rh_high;
    *temp_x10 = temp_high * 10 + temp_low;

    // 范围检查
    if (*humidity > 100) {
        ESP_LOGI("dht11", "湿度超限: %d", *humidity);
        return 0;
    }
    if (*temp_x10 > 600) {
        ESP_LOGI("dht11", "温度超限: %d", *temp_x10);
        return 0;
    }





    /* // 提取数据
    relative_humidity = relative_humidity >> 8;
    temp = (temp >> 8) * 10 + (temp & 0xFF);

    // 范围校验
    if (relative_humidity <= 100) {
        *humidity = relative_humidity;
    }
    if (temp <= 600) {
        *temp_x10 = temp;
    } */

    return 1;
}

/*
┌─────────────────────┐
│      main程序调用     │
│     ┌───────────┐    │
│     │  RMT_init  │────┐
│     └───────────┘    │
│                      │
│     ┌─────────────┐  │
│     │ dht11_get_data│ │
│     └─────────────┘  │
│          │           │
│          ▼           │
│   设置GPIO模式和电平    │
│          │           │
│          ▼           │
│    调用 rmt_receive   │
│          │           │
│          ▼           │
│    触发RMT硬件接收     │
│          │           │
│          ▼           │
│  数据接收完成触发中断    │
│   ┌─────────────────┐│
│   │rmt_rx_done_callback│
│   └─────────────────┘│
│          │           │
│          ▼           │
│ 将接收数据通过队列发送   │
│  (xQueueSendFromISR) │
│          │           │
│          ▼           │
│ dht11_get_data调用xQueueReceive等待接收完成 │
│          │           │
│          ▼           │
│  接收数据放入rx_data   │
│          │           │
│          ▼           │
│ 调用 parse_items解析数据│
│          │           │
│          ▼           │
│   解析湿度、温度、校验码 │
│          │           │
│          ▼           │
│ 校验成功则返回湿度和温度 │
│ 校验失败返回0          │
└──────────────────────┘

*/
