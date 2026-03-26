#include "led.h"
#define LED_GPIO_PORT GPIOC
#define LED_LATCH_START() HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET)
#define LED_LATCH_END() HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET)
static uint16_t led_pins[] = {
    GPIO_PIN_8, GPIO_PIN_9, GPIO_PIN_10, GPIO_PIN_11, 
    GPIO_PIN_12, GPIO_PIN_13, GPIO_PIN_14, GPIO_PIN_15
};
void led_set(LED_ID LED_x,GPIO_PinState state){
    HAL_GPIO_WritePin(LED_GPIO_PORT,led_pins[LED_x],state);
    LED_LATCH_START();
    LED_LATCH_END();
}
void led_all_off(void){
    for (uint8_t i = 0; i < LED_ALL; i++)
    {
        HAL_GPIO_WritePin(LED_GPIO_PORT,led_pins[i],GPIO_PIN_SET);
    }
    LED_LATCH_START();
    LED_LATCH_END();
}
