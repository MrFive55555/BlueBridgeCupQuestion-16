#ifndef UI_H
#define UI_H

/**
 * @file     ui.h
 * @brief    
 * @author   Mr.Five
 * @date     2026-03-05 14:51:19
 */
#include "main.h"
typedef enum
{
    PWM_STATE = 0,
    PWM_ABNORMAL,
    PWM_PARAMETER,
    RUNNING_TIME_UPDATE,
} ui_update_t;
typedef enum
{
    PAGE_MONITOR = 0,
    PAGE_STATISTICS,
    PAGE_PARAMETER,
} ui_page_name_t;
void ui_manage_led(void);
void ui_init(void);
uint8_t ui_get_cur_index(void);
void ui_next_page(void);
void ui_next_parameter(void);
void ui_add_parameter(void);
void ui_min_parameter(void);
void ui_reset_time(void);
void ui_refresh_page(void);
void ui_set_lock(void);
void ui_parameter_update(ui_update_t type);
#endif /* UI_H */

