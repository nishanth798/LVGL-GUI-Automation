/*******************************************************************************
 * Touch libraries:
 * FT6X36: https://github.com/strange-v/FT6X36.git
 * GT911: https://github.com/TAMCTec/gt911-arduino.git
 * XPT2046: https://github.com/PaulStoffregen/XPT2046_Touchscreen.git
 ******************************************************************************/
#ifndef SIMULATOR

#ifndef _TOUCH_H
#define _TOUCH_H

/* uncomment for GT911 */

#if !SENSCAP_EN
#define TOUCH_GT911
#define TOUCH_SCL 18
#define TOUCH_SDA 17
#define TOUCH_RST 38
#define TOUCH_GT911_ROTATION ROTATION_NORMAL
#else
#define TOUCH_FT6X36
#define TOUCH_MODULES_FT3267 1
#define TOUCH_FT6X36_ADDR (uint8_t)0x48
#define TOUCH_SCL 40
#define TOUCH_SDA 39
#define TOUCH_RST (-1)
#define TOUCH_FT6X36_ROTATION ROTATION_NORMAL
#endif

extern int TOUCH_GT911_INT; // Will be set based on hardware version in ui.ino file

#define TOUCH_MAP_X1 480
#define TOUCH_MAP_X2 0
#define TOUCH_MAP_Y1 480
#define TOUCH_MAP_Y2 0

#define MY_DISP_HOR_RES 480
#define MY_DISP_VER_RES 480

#include "lvgl.h"

void scan_i2c_devices(int timeout);
void touch_init();
bool touch_has_signal();
bool touch_touched();
bool touch_released();
void my_touchpad_read(lv_indev_t *indev_drv, lv_indev_data_t *data);
void my_lv_drv_init();
#if !SENSCAP_EN
void set_liquid_touch_filter(int enable);
void set_liquid_touch_filter_debug_logs(int enable);
#endif

#endif // #define _TOUCH_H

#endif // #ifndef SIMULATOR