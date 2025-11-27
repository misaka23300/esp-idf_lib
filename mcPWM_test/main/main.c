#include "main.h"

mcpwm_timer_handle_t mcpwm_timer_handle = NULL;
mcpwm_oper_handle_t mcpwm_oper_handle = NULL;
mcpwm_cmpr_handle_t mcpwm_cmpr_handle = NULL;
mcpwm_gen_handle_t mcpwm_gen_handle = NULL;

esp_err_t mcpwm_init()
{

    // 定时器
    uint32_t pwm_freq = 300 * 1000;
    uint32_t resolution = 10 * 1000 * 1000;
    
    uint32_t period_ticks = resolution / pwm_freq;
    uint32_t cmp_ticks = period_ticks / 2;
    mcpwm_timer_config_t mcpwm_timer_cfg = {
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
        .group_id = 0,
        .intr_priority = 0,
        .period_ticks = period_ticks,
        .resolution_hz = resolution,
    };

    mcpwm_new_timer(&mcpwm_timer_cfg, &mcpwm_timer_handle);

    // 操作器
    mcpwm_operator_config_t mcpwm_operator_cfg = {
        .group_id = 0,
    };

    mcpwm_new_operator(&mcpwm_operator_cfg, &mcpwm_oper_handle);
    mcpwm_operator_connect_timer(mcpwm_oper_handle, mcpwm_timer_handle);

    // 比较器
    mcpwm_comparator_config_t mcpwm_comparator_cfg = {
        .intr_priority = 0,
        /* .flags.update_cmp_on_sync = 1,
        .flags.update_cmp_on_tep = 2,
        .flags.update_cmp_on_tez = 3 */
    };
    mcpwm_new_comparator(mcpwm_oper_handle, &mcpwm_comparator_cfg, &mcpwm_cmpr_handle);
    mcpwm_comparator_set_compare_value(mcpwm_cmpr_handle, cmp_ticks);

    // 生成器
    mcpwm_generator_config_t mcpwm_generator_cfg = {
        .gen_gpio_num = 47,
    };
    mcpwm_new_generator(mcpwm_oper_handle, &mcpwm_generator_cfg, &mcpwm_gen_handle);

    // action
    mcpwm_gen_timer_event_action_t timer_action_t = {
        .action = MCPWM_GEN_ACTION_HIGH,
        .direction = MCPWM_TIMER_DIRECTION_UP,
        .event = MCPWM_TIMER_EVENT_EMPTY,
    };

    mcpwm_generator_set_action_on_timer_event(mcpwm_gen_handle, timer_action_t);

    mcpwm_gen_compare_event_action_t compare_event_t = {
        .action = MCPWM_GEN_ACTION_LOW,
        .comparator = mcpwm_cmpr_handle,
        .direction = MCPWM_TIMER_DIRECTION_UP,
    };

    mcpwm_generator_set_action_on_compare_event(mcpwm_gen_handle, compare_event_t);


    mcpwm_timer_enable(mcpwm_timer_handle);
    mcpwm_timer_start_stop(mcpwm_timer_handle, MCPWM_TIMER_START_NO_STOP);

    return ESP_OK;
}

void app_main()
{
    mcpwm_init();
}