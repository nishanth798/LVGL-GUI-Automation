
#ifndef SIMULATOR

#ifndef _DISPLAY_H
#define _DISPLAY_H

#include "lvgl.h"
#include <Arduino_GFX_Library.h>

#define MY_DISP_HOR_RES 480

#define MY_DISP_VER_RES 480

void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);
void my_lv_disp_init();

#endif

#endif // #ifndef SIMULATOR
