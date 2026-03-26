/**
 * @file     data_base.c
 * @brief
 * @author   Mr.Five
 * @date     2026-03-06 14:00:32
 */

#include "data_base.h"
// pwm parameters
pwm_state_t pwm_state = {
    .cf = 1000,
    .cd = 10};
pwm_abnormal_t pwm_abnormal;
pwm_parameter_t pwm_parameter = {
    .ds = 1,
    .dr = 80,
    .fs = 100,
    .fr = 2000};
time_t time;
adc_t adc;
capture_t cap = {
    .samp_freq = 1000000, // 1MHz
};
