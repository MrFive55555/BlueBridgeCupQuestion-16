#ifndef DATA_BASE_H
#define DATA_BASE_H

/**
 * @file     data_base.h
 * @brief
 * @author   Mr.Five
 * @date     2026-03-06 14:00:41
 */
#include "main.h"
typedef struct
{
    uint16_t cf;
    uint16_t cd;
    uint16_t df;
    uint8_t st;
} pwm_state_t;
typedef struct
{
    uint16_t cf;
    uint16_t cd;
    uint16_t df;
    uint16_t xf;
    uint16_t flag;
} pwm_abnormal_t;
typedef struct
{
    uint16_t ds;
    uint16_t dr;
    uint16_t fs;
    uint16_t fr;
} pwm_parameter_t;
typedef struct
{
    uint8_t sec;
    uint8_t min;
    uint32_t hour;
    uint32_t last_run_tick;
    uint32_t last_refresh_tick;
    uint32_t last_key_tick;
    uint32_t last_adc_get_tick;
    uint32_t last_uart_tick;
} time_t;
typedef struct
{
    uint16_t a_value;
    uint16_t b_value;
    uint16_t duty_segment;
    uint16_t fre_segment;
    uint16_t duty_segment_value;
    uint16_t fre_segment_value;
} adc_t;
typedef struct
{
  uint32_t samp_freq;
  uint32_t buf[3];
  uint8_t count; // 捕获计数
  uint8_t flag;
  uint32_t high_level_ticks;
  uint32_t low_level_ticks;
  uint32_t total_ticks;
  uint32_t duty;
  uint32_t freq;
} capture_t;
#define PWM_BASE_FRE 1000000
extern capture_t cap;
extern adc_t adc;
extern time_t time;
extern pwm_state_t pwm_state;
extern pwm_abnormal_t pwm_abnormal;
extern pwm_parameter_t pwm_parameter;
#endif /* DATA_BASE_H */
