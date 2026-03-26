/**
 * @file     adc.c
 * @brief
 * @author   Mr.Five
 * @date     2026-03-20 17:21:04
 */

#include "adc.h"
#include "data_base.h"
ADC_HandleTypeDef *hadcx[2] = {&hadc1, &hadc2};
static uint16_t *adc_value[2] = {&adc.b_value, &adc.a_value};
void adc_get_value(void)
{
    static uint8_t adc_num;
    HAL_ADC_Start(hadcx[adc_num]);
    HAL_Delay(1);
    *adc_value[adc_num] = HAL_ADC_GetValue(hadcx[adc_num]);
    adc_num++;
    adc_num %= 2;
}
/* Your code here */