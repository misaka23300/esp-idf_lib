#include "main.h"

// MCPWM 各模块的句柄
mcpwm_timer_handle_t mcpwm_timer_handle = NULL;
mcpwm_oper_handle_t  mcpwm_oper_handle  = NULL;
mcpwm_cmpr_handle_t  mcpwm_cmpr_handle  = NULL;
mcpwm_gen_handle_t   mcpwm_gen_handle   = NULL;

esp_err_t mcpwm_init()
{
    // === 定时器参数 ===
    uint32_t pwm_freq = 300 * 1000;       // PWM频率 = 300kHz
    uint32_t resolution = 10 * 1000 * 1000; // 计数器时钟 = 10MHz（计数一次 = 0.1us）

    uint32_t period_ticks = resolution / pwm_freq; // 一个周期对应的计数值
    uint32_t cmp_ticks = period_ticks / 2;         // 占空比 50%，比较器触发点

    // === 定时器配置 ===
    mcpwm_timer_config_t mcpwm_timer_cfg = {
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT, // 使用默认时钟
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP, // 向上计数模式
        .group_id = 0,                           // 使用 group0
        .intr_priority = 0,                      // 中断优先级
        .period_ticks = period_ticks,            // PWM 周期对应多少 tick
        .resolution_hz = resolution,             // 定时器计数速度 = 10MHz
    };

    mcpwm_new_timer(&mcpwm_timer_cfg, &mcpwm_timer_handle);

    // === 操作器（operator）===
    mcpwm_operator_config_t mcpwm_operator_cfg = {
        .group_id = 0,
    };

    mcpwm_new_operator(&mcpwm_operator_cfg, &mcpwm_oper_handle);

    // operator必须绑定到 timer 才能工作
    mcpwm_operator_connect_timer(mcpwm_oper_handle, mcpwm_timer_handle);

    // === 比较器（决定PWM占空比）===
    mcpwm_comparator_config_t mcpwm_comparator_cfg = {
        .intr_priority = 0,
        // flags 默认即可，不需要手动更新时机
    };

    mcpwm_new_comparator(mcpwm_oper_handle, &mcpwm_comparator_cfg, &mcpwm_cmpr_handle);

    // 设置比较值（计数到这个值时触发事件）
    mcpwm_comparator_set_compare_value(mcpwm_cmpr_handle, cmp_ticks);

    // === PWM 生成器（输出究竟如何变化）===
    mcpwm_generator_config_t mcpwm_generator_cfg = {
        .gen_gpio_num = 47,  // PWM 输出脚
    };
    mcpwm_new_generator(mcpwm_oper_handle, &mcpwm_generator_cfg, &mcpwm_gen_handle);

    // === 配置波形行为 ===
    // 计数器为0（EMPTY）时输出置高
    mcpwm_gen_timer_event_action_t timer_action_t = {
        .action = MCPWM_GEN_ACTION_HIGH,      // 输出变高
        .direction = MCPWM_TIMER_DIRECTION_UP, // 仅向上计数时触发
        .event = MCPWM_TIMER_EVENT_EMPTY,      // counter == 0
    };
    mcpwm_generator_set_action_on_timer_event(mcpwm_gen_handle, timer_action_t);

    // 计数到比较器触发点时输出置低
    mcpwm_gen_compare_event_action_t compare_event_t = {
        .action = MCPWM_GEN_ACTION_LOW,        // 输出变低
        .comparator = mcpwm_cmpr_handle,       // 使用前面创建的比较器
        .direction = MCPWM_TIMER_DIRECTION_UP, // 向上计数时触发
    };
    mcpwm_generator_set_action_on_compare_event(mcpwm_gen_handle, compare_event_t);

    // === 启动定时器 ===
    mcpwm_timer_enable(mcpwm_timer_handle);
    mcpwm_timer_start_stop(mcpwm_timer_handle, MCPWM_TIMER_START_NO_STOP); // 一直运行

    return ESP_OK;
}

void app_main()
{
    mcpwm_init();
}
