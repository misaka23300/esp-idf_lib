#include "ds1302.h"

// 地址
#define CH_REG 0x80
#define WP_REG 0x0E

// 位
#define CH_BIT (1 << 7)
#define WP_BIT (1 << 7)
#define HOUR12_BIT (1 << 7)
#define PM_BIT (1 << 5)

// 掩码
#define CH_MASK ((uint8_t)(~CH_BIT))
#define WP_MASK ((uint8_t)(~WP_BIT))

#define SECOUNDS_MASK 0x7F
#define HOUR12_MASK 0x1F
#define HOUR24_MASK 0x3F

#define CLOCK_BURST 0xBE
#define RAM_BURST 0xFE

#define GPIO_BIT(x) (1ULL << (x))

#define CHECK(x)          \
    do                    \
    {                     \
        esp_err_t __ = x; \
        if (__ != ESP_OK) \
            return __;    \
    } while (0)
#define CHECK_ARG(VAL)                  \
    do                                  \
    {                                   \
        if (!VAL)                       \
            return ESP_ERR_INVALID_ARG; \
    } while (0)

// esp32 互斥锁

#define PORT_ENTER_CRITICAL portENTER_CRITICAL(&mux)
#define PORT_EXIT_CRITICAL portEXIT_CRITICAL(&mux)

// 单核
/* #define PORT_ENTER_CRITICAL portENTER_CRITICAL()
#define PORT_EXIT_CRITICAL portEXIT_CRITICAL() */

#define CHECK_MUX(x)            \
    do                          \
    {                           \
        esp_err_t __ = x;       \
        if (__ != ESP_OK)       \
        {                       \
            PORT_EXIT_CRITICAL; \
            return __;          \
        }                       \
    } while (0)

static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;


inline static uint8_t bcd_to_dec(uint8_t bcd)
{
    return (bcd / 16 * 10) + (bcd % 16);
}

inline static uint8_t dec_to_bcd(uint8_t dec)
{
    return (dec / 10 * 16) + (dec % 10);
}

inline static esp_err_t chip_enable(ds1302_t *gpio)
{
    return gpio_set_level(gpio->CE, 1);
    ets_delay_us(4);
    return ESP_OK;
}

inline static esp_err_t chip_disable(ds1302_t *gpio)
{
    return gpio_set_level(gpio->CE, 0);
}

/// @brief 设置SDA输入输出方式，拉低时钟引脚
/// @param gpio
/// @param mode
/// @return
inline static esp_err_t prepare(ds1302_t *gpio, gpio_mode_t mode)
{
    ESP_ERROR_CHECK(gpio_set_direction(gpio->SDA, mode));
    ESP_ERROR_CHECK(gpio_set_level(gpio->SCL, 0));
    return chip_enable(gpio);
}

/// @brief 拉高时钟信号
/// @param gpio
/// @return
inline static esp_err_t toggle_clock(ds1302_t *gpio)
{
    ESP_ERROR_CHECK(gpio_set_level(gpio->SCL, 1));
    ets_delay_us(1);
    ESP_ERROR_CHECK(gpio_set_level(gpio->SCL, 0));
    ets_delay_us(1);

    return ESP_OK;
}

/// @brief 在时钟上升沿写入字节
/// @param gpio
/// @param write_data
/// @return
inline static esp_err_t write_byte(ds1302_t *gpio, uint8_t write_data)
{
    uint8_t i = 0;
    for (i = 0; i < 8; i++) {
        ESP_ERROR_CHECK(gpio_set_level(gpio->SDA, (write_data >> i) & 0x01));
        ESP_ERROR_CHECK(toggle_clock(gpio));
    }
    return ESP_OK;
}

/// @brief 在指定地址的寄存器写入一个字节
/// @param gpio
/// @param address
/// @param write_data
/// @return
static esp_err_t write_register(ds1302_t *gpio, uint8_t address, uint8_t write_data)
{
    PORT_ENTER_CRITICAL;
    CHECK_MUX(prepare(gpio, GPIO_MODE_OUTPUT));
    CHECK_MUX(write_byte(gpio, address));
    CHECK_MUX(write_byte(gpio, write_data));
    PORT_EXIT_CRITICAL;

    return chip_disable(gpio);
}

/// @brief 在下降沿之后读取一个字节
/// @param gpio
/// @param read_data
/// @return
static esp_err_t read_byte(ds1302_t *gpio, uint8_t *read_data)
{
    uint8_t i = 0;
    *read_data = 0;
    for (i = 0; i < 8; i++) {
        *read_data = *read_data | (gpio_get_level(gpio->SDA) << i);
        ESP_ERROR_CHECK(toggle_clock(gpio));
    }
    return ESP_OK;
}

/// @brief 在指定地址的寄存器读取一个字节
/// @param gpio
/// @param address
/// @param read_data
/// @return
static esp_err_t read_register(ds1302_t *gpio, uint8_t address, uint8_t *read_data)
{
    PORT_ENTER_CRITICAL;
    CHECK_MUX(prepare(gpio, GPIO_MODE_OUTPUT));
    CHECK_MUX(write_byte(gpio, address | 0x01));
    CHECK_MUX(prepare(gpio, GPIO_MODE_INPUT));
    CHECK_MUX(read_byte(gpio, read_data));
    PORT_EXIT_CRITICAL;

    return chip_disable(gpio);
}

/// @brief 在指定地址的寄存器修改位
/// @param gpio
/// @param address
/// @param write_mask
/// @param write_bit
/// @return
static esp_err_t update_register(ds1302_t *gpio, uint8_t address, uint8_t write_mask, uint8_t write_bit)
{
    uint8_t read_data;
    ESP_ERROR_CHECK(read_register(gpio, address, &read_data));
    return write_register(gpio, address, (read_data & write_mask) | write_bit);
}

/// @brief 连续读取，读取指定的地址和之后的len个字节；
/// @param gpio
/// @param address
/// @param read_data
/// @param len
/// @return
static esp_err_t burst_read(ds1302_t *gpio, uint8_t address, uint8_t *read_data, uint8_t len)
{
    uint8_t i = 0;
    PORT_ENTER_CRITICAL;
    CHECK_MUX(prepare(gpio, GPIO_MODE_OUTPUT));
    CHECK_MUX(write_byte(gpio, address));
    CHECK_MUX(prepare(gpio, GPIO_MODE_INPUT));

    for (i = 0; i < len; i++, read_data++) {
        CHECK_MUX(read_byte(gpio, read_data));
    }
    PORT_EXIT_CRITICAL;
    return chip_disable(gpio);
}

/// @brief 连续写入
/// @param gpio
/// @param address
/// @param write_data
/// @param len
/// @return
static esp_err_t burst_write(ds1302_t *gpio, uint8_t address, uint8_t *write_data, uint8_t len)
{
    uint8_t i = 0;
    PORT_ENTER_CRITICAL;
    CHECK_MUX(prepare(gpio, GPIO_MODE_OUTPUT));
    CHECK_MUX(write_byte(gpio, address));

    for (i = 0; i < len; i++, write_data++) {
        CHECK_MUX(write_byte(gpio, write_data[i]));
    }

    PORT_EXIT_CRITICAL;
    return chip_disable(gpio);
}
////////////////////////////////////////////////////////////////////

/// @brief 检查GPIO参数合法性，配置GPIO，检查DS1302是否工作
/// @param gpio
/// @return
esp_err_t ds1302_init(ds1302_t *gpio)
{
    bool r;

    CHECK_ARG(gpio);
    gpio_config_t io_cfg = {
        .intr_type = 0,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = GPIO_BIT(gpio->SCL) | GPIO_BIT(gpio->SDA) | GPIO_BIT(gpio->CE),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };

    ESP_ERROR_CHECK(gpio_config(&io_cfg));
    ESP_ERROR_CHECK(ds1302_is_running(gpio, &r));

    return ESP_OK;
}

esp_err_t ds1302_is_running(ds1302_t *gpio, bool *running)
{
    uint8_t r;
    CHECK_ARG(gpio && running);
    ESP_ERROR_CHECK(read_register(gpio, CH_REG, &r));
    *running = !(r & CH_BIT);
    gpio->work = !*running;

    return ESP_OK;
}

esp_err_t ds1302_start(ds1302_t *gpio, bool start)
{
    ESP_ERROR_CHECK(update_register(gpio, CH_REG, CH_MASK, start ? 0 : CH_BIT));
    gpio->work = !start;

    return ESP_OK;
}

esp_err_t ds1302_get_write_protect(ds1302_t *gpio, bool *wp)
{
    uint8_t read;
    ESP_ERROR_CHECK(read_register(gpio, WP_REG, &read));
    *wp = (read & WP_BIT) != 0;
    return ESP_OK;
}

esp_err_t ds1302_set_write_protect(ds1302_t *gpio, bool wp)
{
    return update_register(gpio, WP_REG, WP_MASK, wp ? WP_BIT : 0);
}

esp_err_t ds1302_get_time(ds1302_t *gpio, struct tm *time)
{
    uint8_t cache[7];

    burst_read(gpio, CLOCK_BURST, cache, 7);

    // 秒
    time->tm_sec = bcd_to_dec(cache[0] & SECOUNDS_MASK);
    // 分
    time->tm_min = bcd_to_dec(cache[1]);

    // 时
    if (cache[2] & HOUR12_BIT) {
        time->tm_hour = bcd_to_dec(cache[2] & HOUR12_MASK) - 1;
        if (cache[2] & PM_BIT) {
            time->tm_hour += 12;
        }
    } else {
        time->tm_hour = bcd_to_dec(cache[2] & HOUR24_MASK);
    }
    time->tm_mday = bcd_to_dec(cache[3]);       // 天
    time->tm_mon = bcd_to_dec(cache[4]) - 1;    // 月
    time->tm_wday = bcd_to_dec(cache[5]) - 1;   // 星期
    time->tm_year = bcd_to_dec(cache[6]) + 100; // 年

    return ESP_OK;
}

esp_err_t ds1302_set_time(ds1302_t *gpio, const struct tm *time)
{
    uint8_t cache[8] = {
        dec_to_bcd(time->tm_sec) | (gpio->work ? CH_BIT : 0),
        dec_to_bcd(time->tm_min),
        dec_to_bcd(time->tm_hour),
        dec_to_bcd(time->tm_mday),
        dec_to_bcd(time->tm_mon + 1),
        dec_to_bcd(time->tm_wday + 1),
        dec_to_bcd(time->tm_year - 100),
        0
    };
    return (burst_write(gpio, CLOCK_BURST, cache, 8));
}
