#include "uart.h"


QueueHandle_t uart_queue = NULL;

esp_err_t uart_config()
{

    uart_port_t port = 0;
    uart_driver_install(port, 300, 300, 300, &uart_queue, 0);

    return ESP_OK;
}
