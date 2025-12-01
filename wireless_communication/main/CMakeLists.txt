idf_component_register(SRCS "uart.c" "main.c"
                    INCLUDE_DIRS "../include"
                    REQUIRES esp_driver_uart
                    )
