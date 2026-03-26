#ifndef LED_H
#define LED_H
#include "main.h"
#define LED_LATCH_START() HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET)
#define LED_LATCH_END() HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET)
typedef enum {
    LED_1 = 0, LED_2, LED_3, LED_4, LED_5, LED_6, LED_7, LED_8, LED_ALL
} LED_ID;
void led_set(LED_ID LED_x,GPIO_PinState state);
void led_all_off(void);
#endif
