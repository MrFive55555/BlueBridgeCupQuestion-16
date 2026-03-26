/**
 * @file     ui.c
 * @brief
 * @author   Mr.Five
 * @date     2026-03-05 14:51:26
 */
#include "stdio.h"
#include "string.h"
#include "ui.h"
#include "lcd.h"
#include "led.h"
#include "data_base.h"
/**
 * global variables
 */
typedef struct
{
    uint8_t cf[10]; // frequency
    uint8_t cd[10]; // duty
    uint8_t df[10]; // capture frequency
    const uint8_t *st;
} monitor_t;
typedef struct
{
    uint8_t cf[10];
    uint8_t cd[10];
    uint8_t df[10];
    uint8_t xf[10];
} statistics_t;
typedef struct
{
    uint8_t ds[10];
    uint8_t dr[10];
    uint8_t fs[10];
    uint8_t fr[10];
} parameter_t;
typedef struct
{
    uint8_t sec[10];
    uint8_t min[10];
    uint8_t hour[10];
} ui_time_t;
static const uint8_t *const lock_map[] = {"UNLOCK", "LOCK  "};
static monitor_t monitor;
static statistics_t statistics;
static parameter_t parameter;
static ui_time_t ui_time;
static uint8_t ui_page_index;
static uint8_t ui_parameter_index;
static uint16_t *ui_adjust_parameter[4] = {
    &pwm_parameter.ds, &pwm_parameter.dr,
    &pwm_parameter.fs, &pwm_parameter.fr};
/**
 * function declaration
 */
static uint16_t ui_get_string_length(uint8_t *str);
static void ui_monitor_page(void);
static void ui_statistics_page(void);
static void ui_parameter_page(void);
static void ui_show_running_time(void);
static uint32_t ui_pow10(uint32_t pow);
typedef void (*ui_page_fun)(void);
static const ui_page_fun const page_fun[3] = {
    ui_monitor_page,
    ui_statistics_page,
    ui_parameter_page,
};
/**
 * support input row and column to show char.
 * font size = 16x24 = width x height = 10 row x 20 column
 */
static void lcd_show_string(uint16_t line, uint16_t column, uint8_t *str)
{
    uint16_t column_sum = 319;
    uint16_t start_column = column_sum - column * 16;
    if (line > Line9 || column > 20)
        return;
    while (*str != '\0')
    {
        LCD_DisplayChar(line, start_column, *str);
        start_column -= 16;
        if (start_column < 15)
        {
            line += 24; // go to the next line
            if (line > Line9)
                return;
            start_column = column_sum;
        }
        str++;
    }
}
/**
 * manage led
 */
void ui_manage_led(void)
{
    led_all_off();
    if (ui_page_index == PAGE_MONITOR)
        led_set(LED_1, GPIO_PIN_RESET);
    else
        led_set(LED_1, GPIO_PIN_SET);
    if (pwm_state.st)
        led_set(LED_2, GPIO_PIN_RESET);
    else
        led_set(LED_2, GPIO_PIN_SET);
    // if(pwm_abnormal == PAGE_MONITOR) led_set(LED_1,GPIO_PIN_RESET);
    // else led_set(LED_1,GPIO_PIN_SET);
}
/**
 * initialize ui when power up
 */
void ui_init(void)
{
    ui_refresh_page();
}
/**
 * refresh page
 */
void ui_refresh_page(void)
{
    ui_parameter_update(PWM_STATE);
    ui_parameter_update(PWM_ABNORMAL);
    ui_parameter_update(PWM_PARAMETER);
    ui_parameter_update(RUNNING_TIME_UPDATE);
    page_fun[ui_page_index]();
}
/**
 * get current page index
 */
uint8_t ui_get_cur_index(void)
{
    return ui_page_index;
}
/**
 * switch to next page
 */
void ui_next_page(void)
{
    LCD_Clear(Black);
    ui_page_index++;
    ui_page_index %= 3;
    ui_refresh_page();
}
/**
 * switch to next parameter
 */
void ui_next_parameter(void)
{
    if (ui_page_index != PAGE_PARAMETER)
        return;
    ui_parameter_index++;
    ui_parameter_index %= 4;
}
/**
 *set lock
 */
void ui_set_lock(void)
{
    if (pwm_state.st)
        pwm_state.st = 0;
    else
        pwm_state.st = 1;
    ui_parameter_update(PWM_STATE);
}
/**
 * calculate pow of ten
 */
static uint32_t ui_pow10(uint32_t pow)
{
    uint32_t num = 1;
    for (uint32_t i = 0; i < pow; i++)
    {
        num *= 10;
    }
    return num;
}
/**
 * add parameter
 */
void ui_add_parameter(void)
{
    if (ui_page_index != PAGE_PARAMETER)
        return;
    *ui_adjust_parameter[ui_parameter_index] += ui_pow10(ui_parameter_index);
    if (ui_parameter_index == 0)
    {
        if (*ui_adjust_parameter[ui_parameter_index] > (pwm_parameter.dr - 10))
        {
            *ui_adjust_parameter[ui_parameter_index] = (pwm_parameter.dr - 10);
        }
    }
    else if (ui_parameter_index == 1)
    {
        if (*ui_adjust_parameter[ui_parameter_index] > 100)
        {
            *ui_adjust_parameter[ui_parameter_index] = 100;
        }
    }
    else if (ui_parameter_index == 2)
    {
        if (*ui_adjust_parameter[ui_parameter_index] > (pwm_parameter.fr - 1000))
        {
            *ui_adjust_parameter[ui_parameter_index] = (pwm_parameter.fr - 1000);
        }
    }
    else if (ui_parameter_index == 3)
    {
        if (*ui_adjust_parameter[ui_parameter_index] > 10000)
        {
            *ui_adjust_parameter[ui_parameter_index] = 10000;
        }
    }
    ui_parameter_update(PWM_PARAMETER);
}
/**
 * minus parameter
 */
void ui_min_parameter(void)
{
    if (ui_page_index != PAGE_PARAMETER)
        return;
    *ui_adjust_parameter[ui_parameter_index] -= ui_pow10(ui_parameter_index);
    if (ui_parameter_index == 0)
    {
        if (*ui_adjust_parameter[ui_parameter_index] < 1)
        {
            *ui_adjust_parameter[ui_parameter_index] = 1;
        }
    }
    else if (ui_parameter_index == 1)
    {
        if (*ui_adjust_parameter[ui_parameter_index] < 10)
        {
            *ui_adjust_parameter[ui_parameter_index] = 10;
        }
    }
    else if (ui_parameter_index == 2)
    {
        if (*ui_adjust_parameter[ui_parameter_index] < 100)
        {
            *ui_adjust_parameter[ui_parameter_index] = 100;
        }
    }
    else if (ui_parameter_index == 3)
    {
        if (*ui_adjust_parameter[ui_parameter_index] < 1000)
        {
            *ui_adjust_parameter[ui_parameter_index] = 1000;
        }
    }
    ui_parameter_update(PWM_PARAMETER);
}
/**
 * update pararmeters of ui
 */
void ui_parameter_update(ui_update_t type)
{
    if (type == PWM_STATE)
    {
        snprintf(monitor.cf, sizeof(monitor.cf), "%d", pwm_state.cf);
        snprintf(monitor.cd, sizeof(monitor.cd), "%d", pwm_state.cd);
        snprintf(monitor.df, sizeof(monitor.df), "%d", pwm_state.df);
        monitor.st = lock_map[pwm_state.st];
    }
    else if (type == PWM_ABNORMAL)
    {
        snprintf(statistics.cf, sizeof(statistics.cf), "%d", pwm_abnormal.cf);
        snprintf(statistics.cd, sizeof(statistics.cd), "%d", pwm_abnormal.cd);
        snprintf(statistics.df, sizeof(statistics.df), "%d", pwm_abnormal.df);
        int16_t xf = pwm_state.cf - pwm_state.df;
        if (xf < 0)
            xf = -xf;
        snprintf(statistics.xf, sizeof(statistics.xf), "%d", pwm_abnormal.xf);
    }
    else if (type == PWM_PARAMETER)
    {
        snprintf(parameter.ds, sizeof(parameter.ds), "%d", pwm_parameter.ds);
        snprintf(parameter.dr, sizeof(parameter.dr), "%d", pwm_parameter.dr);
        snprintf(parameter.fs, sizeof(parameter.fs), "%d", pwm_parameter.fs);
        snprintf(parameter.fr, sizeof(parameter.fr), "%d", pwm_parameter.fr);
    }
    else if (type == RUNNING_TIME_UPDATE)
    {

        if (time.sec < 10)
        {
            ui_time.sec[0] = '0';
            ui_time.sec[1] = '0' + time.sec;
            ui_time.sec[2] = '\0';
        }
        else
        {
            snprintf(ui_time.sec, sizeof(ui_time.sec), "%d", time.sec);
        }
        if (time.min < 10)
        {
            ui_time.min[0] = '0';
            ui_time.min[1] = '0' + time.min;
            ui_time.min[2] = '\0';
        }
        else
        {
            snprintf(ui_time.min, sizeof(ui_time.min), "%d", time.min);
        }
        if (time.hour < 10)
        {
            ui_time.hour[0] = '0';
            ui_time.hour[1] = '0' + time.hour;
            ui_time.hour[2] = '\0';
        }
        else
        {
            snprintf(ui_time.hour, sizeof(ui_time.hour), "%d", time.hour);
        }
    }
}
/**
 * get length of str
 */
static uint16_t ui_get_string_length(uint8_t *str)
{
    uint16_t length = 0;
    while (*str != '\0')
    {
        length++;
        str++;
    }
    return length;
}
/**
 * show running time
 */
static void ui_show_running_time(void)
{
    uint8_t hour_text_length = ui_get_string_length(ui_time.hour) + 4;
    lcd_show_string(Line7, 4, ui_time.hour);
    lcd_show_string(Line7, hour_text_length, "H");
    lcd_show_string(Line7, hour_text_length + 1, ui_time.min);
    lcd_show_string(Line7, hour_text_length + 3, "M");
    lcd_show_string(Line7, hour_text_length + 4, ui_time.sec);
    lcd_show_string(Line7, hour_text_length + 6, "S");
}
/**
 * reset time
 */
void ui_reset_time(void)
{
    time.hour = 0;
    time.min = 0;
    time.sec = 0;
    ui_parameter_update(RUNNING_TIME_UPDATE);
}
/**
 * monitor page
 */
static void ui_monitor_page(void)
{
    uint16_t str_len;
    lcd_show_string(Line0, 8, "PWM");
    str_len = 7 + ui_get_string_length(monitor.cf);
    lcd_show_string(Line3, 4, "CF=");
    lcd_show_string(Line3, 7, monitor.cf);
    lcd_show_string(Line3, str_len, "Hz");
    lcd_show_string(Line3, str_len + 2, " ");

    str_len = 7 + ui_get_string_length(monitor.cd);
    lcd_show_string(Line4, 4, "CD=");
    lcd_show_string(Line4, 7, monitor.cd);
    lcd_show_string(Line4, str_len, "%");
    lcd_show_string(Line4, str_len + 1, " ");

    str_len = 7 + ui_get_string_length(monitor.df);
    lcd_show_string(Line5, 4, "DF=");
    lcd_show_string(Line5, 7, monitor.df);
    lcd_show_string(Line5, str_len, "Hz");
    lcd_show_string(Line5, str_len + 2, " ");

    lcd_show_string(Line6, 4, "ST=");
    lcd_show_string(Line6, 7, (uint8_t *)monitor.st);
    ui_show_running_time();
}
/**
 * statistic page
 */
static void ui_statistics_page(void)
{
    uint16_t str_len;
    lcd_show_string(Line0, 8, "RECD");
    str_len = 7 + ui_get_string_length(statistics.cf);
    lcd_show_string(Line3, 4, "CF=");
    lcd_show_string(Line3, 7, statistics.cf);
    lcd_show_string(Line3, str_len, "Hz");
    lcd_show_string(Line3, str_len + 2, " ");

    lcd_show_string(Line4, 4, "CD=");
    str_len = 7 + ui_get_string_length(statistics.cd);
    lcd_show_string(Line4, 7, statistics.cd);
    lcd_show_string(Line4, str_len, "%");
    lcd_show_string(Line4, str_len + 1, " ");

    lcd_show_string(Line5, 4, "DF=");
    str_len = 7 + ui_get_string_length(statistics.df);
    lcd_show_string(Line5, 7, statistics.df);
    lcd_show_string(Line5, str_len, "Hz");
    lcd_show_string(Line5, str_len + 2, " ");

    lcd_show_string(Line6, 4, "XF=");
    str_len = 7 + ui_get_string_length(statistics.xf);
    lcd_show_string(Line6, 7, statistics.xf);
    lcd_show_string(Line6, str_len, "Hz");
    lcd_show_string(Line6, str_len + 2, " ");
    ui_show_running_time();
}
/**
 * parameter page
 */
static void ui_parameter_page(void)
{
    uint16_t str_len;
    lcd_show_string(Line0, 8, "PARA");

    lcd_show_string(Line3, 4, "DS=");
    str_len = 7 + ui_get_string_length(parameter.ds);
    lcd_show_string(Line3, 7, parameter.ds);
    lcd_show_string(Line3, str_len, "%");
    lcd_show_string(Line3, str_len + 1, " ");

    lcd_show_string(Line4, 4, "DR=");
    str_len = 7 + ui_get_string_length(parameter.dr);
    lcd_show_string(Line4, 7, parameter.dr);
    lcd_show_string(Line4, str_len, "%");
    lcd_show_string(Line4, str_len + 1, " ");

    lcd_show_string(Line5, 4, "FS=");
    str_len = 7 + ui_get_string_length(parameter.fs);
    lcd_show_string(Line5, 7, parameter.fs);
    lcd_show_string(Line5, str_len, "Hz");
    lcd_show_string(Line5, str_len + 2, " ");

    lcd_show_string(Line6, 4, "FR=");
    str_len = 7 + ui_get_string_length(parameter.fr);
    lcd_show_string(Line6, 7, parameter.fr);
    lcd_show_string(Line6, str_len, "Hz");
    lcd_show_string(Line6, str_len + 2, " ");
}