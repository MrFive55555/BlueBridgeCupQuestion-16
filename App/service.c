/**
 * @file     service.c
 * @brief
 * @author   Mr.Five
 * @date     2026-03-10 14:13:43
 */

#include "service.h"
#include "data_base.h"
#include "ui.h"
#include "key.h"
#include "adc.h"
#include "stdio.h"
#include "string.h"
static void service_time_update(void);
static void service_adjust_output(void);
static void service_pwm_capture(void);
static char msg[256];
static void service_uart_send_message(void)
{
    // 1. 先格式化基本信息
    sprintf(msg, "Freq: %lu Hz", cap.freq);

    // 2. 拼接捕获到的结果
    sprintf(msg + strlen(msg), " | Duty: %lu %%", cap.duty);

    sprintf(msg + strlen(msg), " | adc_a:%lu,adc_b:%lu", adc.a_value, adc.b_value);
    // 3. 串口发送
    HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), 100);
}
void service_tick_run(void)
{
    static uint8_t enable_last_press;
    enable_last_press = (ui_get_cur_index() == PAGE_MONITOR) ? 1 : 0;
    // 5ms
    if (HAL_GetTick() - time.last_adc_get_tick >= 5)
    {
        time.last_adc_get_tick = HAL_GetTick();
        adc_get_value();
        service_adjust_para_update();
        service_adjust_output();
        service_pwm_capture();
    }
    // 10ms
    if (HAL_GetTick() - time.last_key_tick >= 10)
    {
        time.last_key_tick = HAL_GetTick();
        switch (key_scan(enable_last_press))
        {
        case KEY_UI:
            ui_next_page();
            break;
        case KEY_FUNCTION:
            ui_next_parameter();
            break;
        case KEY_ADD:
            ui_add_parameter();
            break;
        case KEY_MIN:
            ui_min_parameter();
            break;
        case KEY_SHORT_PRESS:
            ui_set_lock();
            break;
        case KEY_LONG_PRESS:
            ui_reset_time();
            break;
        default:
            break;
        }
    }
    // 100ms refresh lcd
    if (HAL_GetTick() - time.last_refresh_tick >= 100)
    {
        time.last_refresh_tick = HAL_GetTick();
        ui_refresh_page();
        ui_manage_led();
    }
    // 1s update running time
    if (HAL_GetTick() - time.last_run_tick >= 1000)
    {
        time.last_run_tick = HAL_GetTick();
        service_time_update();
    }
    // 2s send message by uart
    if (HAL_GetTick() - time.last_uart_tick >= 2000)
    {
        time.last_uart_tick = HAL_GetTick();
        service_uart_send_message();
    }
}
static void service_time_update(void)
{
    if (++time.sec >= 60)
    {
        time.sec = 0;
        if (++time.min >= 60)
        {
            time.min = 0;
            time.hour++;
        }
    }
}
void service_adjust_para_update(void)
{
    adc.duty_segment = (pwm_parameter.dr - 10) / pwm_parameter.ds;
    adc.fre_segment = (pwm_parameter.fr - 1000) / pwm_parameter.fs;
    adc.duty_segment_value = 4096 / adc.duty_segment;
    adc.fre_segment_value = 4096 / adc.fre_segment;
}
uint16_t duty_segment_num, fre_segment_num;
static void service_adjust_output(void)
{

    uint16_t arr, cmp;
    duty_segment_num = adc.a_value / adc.duty_segment_value;
    fre_segment_num = adc.b_value / adc.fre_segment_value;
    pwm_state.cd = 10 + duty_segment_num * pwm_parameter.ds;
    pwm_state.cf = 1000 + fre_segment_num * pwm_parameter.fs;
    arr = PWM_BASE_FRE / pwm_state.cf - 1;
    cmp = (arr + 1) * pwm_state.cd / 100;
    __HAL_TIM_SET_AUTORELOAD(&htim3, arr);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, cmp);
}
static void service_pwm_capture(void)
{
    if (cap.flag)
    {
        // 计算高电平持续时间（考虑溢出处理）
        if (cap.buf[1] >= cap.buf[0])
        {
            cap.high_level_ticks = cap.buf[1] - cap.buf[0];
        }
        else
        {
            // TIM2是32位定时器，处理溢出
            cap.high_level_ticks = (0xFFFFFFFF - cap.buf[0]) + cap.buf[1] + 1;
        }
        // 计算低电平持续时间(考虑溢出时间)
        if (cap.buf[2] >= cap.buf[1])
        {

            cap.low_level_ticks = cap.buf[2] - cap.buf[1];
        }
        else
        {
            // TIM2是32位定时器，处理溢出
            cap.low_level_ticks = (0xFFFFFFFF - cap.buf[1]) + cap.buf[2] + 1;
        }
        cap.total_ticks = cap.high_level_ticks + cap.low_level_ticks;
        cap.duty = cap.high_level_ticks * 100 / cap.total_ticks;
        cap.freq = cap.samp_freq / cap.total_ticks;
        cap.flag = 0;
        pwm_state.df = cap.freq;
    }
}
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
        uint32_t current_val = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
        uint8_t pin_level = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_15);

        // 如果已经捕获完成还没处理，直接跳过，防止数据覆盖
        if (cap.flag == 1)
            return;

        if (cap.count == 0)
        {
            // 确保第一次捕获必须是上升沿，否则不计入
            if (pin_level == GPIO_PIN_SET)
            {
                cap.buf[cap.count++] = current_val;
            }
        }
        else
        {
            // 后续边沿正常记录
            cap.buf[cap.count++] = current_val;
        }
        if (cap.count >= 3)
        {
            cap.flag = 1;
            cap.count = 0;
            // 注意：这里不要在中断里把 count 清零，建议在 main 处理完数据后再清零
        }
    }
}
/* Your code here */
