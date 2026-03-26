/**
 * @file     key.c
 * @brief
 * @author   Mr.Five
 * @date     2026-03-05 14:18:50
 */

#include "key.h"
#define KEY_NUM 4
typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
} key_config_t;
static key_config_t keys[] = {
    {GPIOB, GPIO_PIN_0}, // B1
    {GPIOB, GPIO_PIN_1}, // B2
    {GPIOB, GPIO_PIN_2}, // B3
    {GPIOA, GPIO_PIN_0}  // B4
};
uint8_t key_scan(uint8_t enable_last_press)
{
    static uint8_t key_state = 0; // 0:等待按下, 1:消抖确认, 2:等待释放
    static uint32_t key_tick = 0;
    uint8_t current_val = KEY_NO;
    static uint32_t key_short_long_tick;
    static uint8_t had_short_press;
    static uint8_t had_long_press;
    static uint8_t save_key_value;

    // 读取所有按键逻辑（以 B1 为例，其他按键同理）
    for (uint8_t i = 0; i < 4; i++)
    {
        if (!HAL_GPIO_ReadPin(keys[i].port, keys[i].pin))
            current_val = i + 1;
    }
    switch (key_state)
    {
    case 0: // 等待按下
        if (current_val != 0)
        {
            key_state = 1;
            key_tick = HAL_GetTick();
        }
        break;

    case 1: // 消抖检测（10ms-20ms）
        if (HAL_GetTick() - key_tick >= 15)
        {
            if (current_val != 0)
            {
                key_state = 2;
                save_key_value = current_val;
                if (current_val == KEY_FUNCTION)
                {
                    key_short_long_tick = HAL_GetTick();
                }
            }
            else
            {
                key_state = 0; // 抖动，回到初始状态
            }
        }
        break;

    case 2: // 等待释放
        if (current_val == KEY_FUNCTION && enable_last_press)
        {
            if (HAL_GetTick() - key_short_long_tick >= 250 && !had_short_press)
            {
                had_short_press = 1;
            }
            else if (HAL_GetTick() - key_short_long_tick >= 2000)
            {
                had_long_press = 1;
            }
        }
        else if (current_val == 0)
        {
            key_state = 0;
            if (had_long_press)
            {
                had_short_press = 0;
                had_long_press = 0;
                return KEY_LONG_PRESS;
            }
            if (had_short_press)
            {
                had_short_press = 0;
                return KEY_SHORT_PRESS;
            }
            return save_key_value;
        }
        break;
    }
    return KEY_NO; // 无有效按键
}
