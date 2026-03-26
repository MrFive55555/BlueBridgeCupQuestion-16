#ifndef KEY_H
#define KEY_H

/**
 * @file     key.h
 * @brief    
 * @author   Mr.Five
 * @date     2026-03-05 14:18:44
 */
#include "main.h"
typedef enum{
    KEY_NO = 0,
    KEY_UI,
    KEY_FUNCTION,
    KEY_ADD,
    KEY_MIN,
    KEY_SHORT_PRESS,
    KEY_LONG_PRESS,
}key_name_t;
uint8_t key_scan(uint8_t enable_last_press);
#endif /* KEY_H */

