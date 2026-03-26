#ifndef SERVICE_H
#define SERVICE_H

/**
 * @file     service.h
 * @brief    
 * @author   Mr.Five
 * @date     2026-03-10 14:13:20
 */

#include "main.h"
void service_tick_run(void);
void service_adjust_para_update(void);
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim);
#endif /* SERVICE_H */
