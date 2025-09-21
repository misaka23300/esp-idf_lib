#include <stdio.h>
#include "driver/gpio.h"
#include "rom/ets_sys.h"




static unsigned char SCL, SDA, CE;

void ds1302_init(unsigned char scl_pin,unsigned char sda_pin,unsigned char ce_pin)
{
    SCL = scl_pin;
    SDA = sda_pin;
    CE = ce_pin;

    gpio_config_t gpio_cfg = {
        .intr_type = 0,
        .mode = 0,
        .pin_bit_mask = SCL,
        .pull_down_en = 0,
        .pull_up_en = 0,
    };

    gpio_config(&gpio_cfg);

    gpio_config_t gpio_cfg = {
        .intr_type = 0,
        .mode = 0,
        .pin_bit_mask = SDA,
        .pull_down_en = 0,
        .pull_up_en = 0,
    };

    gpio_config(&gpio_cfg);

    gpio_config_t gpio_cfg = {
        .intr_type = 0,
        .mode = 0,
        .pin_bit_mask = CE,
        .pull_down_en = 0,
        .pull_up_en = 0,
    };

    gpio_config(&gpio_cfg);
    




}


unsigned int t_cc;
unsigned int t_dc;
unsigned int t_cdh;
unsigned int t_cdd;
unsigned int t_cdw;

void ds1302_write(unsigned char str)
{
    // CE -> 1  SCL -> 0
    // 上升沿写入
    unsigned char i;
    
    for (i = 0;i < 8;i++) {
        gpio_set_level(SDA, str & 0x01);
        str = str >> 1;
        ets_delay_us(t_dc);

        gpio_set_level(SCL, 1);
        ets_delay_us(t_cdh);

        gpio_set_level(SCL, 0);
    }
}

void ds1302_write_byte(unsigned char address, unsigned char str)
{
    unsigned char i;
    gpio_set_direction(CE, GPIO_MODE_OUTPUT);
    gpio_set_direction(SCL, GPIO_MODE_OUTPUT);
    gpio_set_direction(SDA, GPIO_MODE_OUTPUT);

    // 初始化电平为低电平
    gpio_set_level(CE, 0);
      
    gpio_set_level(SCL, 0);

    ets_delay_us(t_cdw);

    gpio_set_level(CE, 1);

    ets_delay_us(t_cc - t_dc);           // tcc最小值：4ns


    ds1302_write(address);

    etc_delay_us(t_cdd);
    
    ds1302_write(str);

    gpio_set_level(CE, 0);
}

unsigned char ds1302_read_byte(unsigned char address)
{
    unsigned char i;
    unsigned char temp = 0x00;

    gpio_set_direction(SDA, GPIO_MODE_INPUT);

    gpio_set_level(CE, 0);
    gpio_set_level(SCL, 0);
    gpio_set_level(CE, 1);

    ds1302_write(address);

    for (i = 0;i < 8;i++) {
        gpio_set_level(SCL, 0);
        temp = temp >> 1;
        if (gpio_get_level(SDA) == 1) {
            temp |= 0x80;
        }
        gpio_set_level(SCL, 1);
    }
}
void app_main(void)
{

}

