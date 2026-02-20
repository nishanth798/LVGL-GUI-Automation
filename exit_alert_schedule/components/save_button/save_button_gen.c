/**
 * @file save_button_gen.c
 * @brief Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/

#include "save_button_gen.h"
#include "exit_alert_schedule.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/***********************
 *  STATIC VARIABLES
 **********************/

/***********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t * save_button_create(lv_obj_t * parent, const char * text)
{
    LV_TRACE_OBJ_CREATE("begin");

    static lv_style_t style_save_button;

    static bool style_inited = false;

    if (!style_inited) {
        lv_style_init(&style_save_button);
        lv_style_set_bg_color(&style_save_button, lv_color_hex(0x1a1040));
        lv_style_set_width(&style_save_button, 448);
        lv_style_set_height(&style_save_button, 80);
        lv_style_set_bg_opa(&style_save_button, 153);
        lv_style_set_radius(&style_save_button, 5);
        lv_style_set_shadow_width(&style_save_button, 4);
        lv_style_set_shadow_color(&style_save_button, lv_color_hex(0x000000));
        lv_style_set_shadow_opa(&style_save_button, 64);
        lv_style_set_shadow_offset_x(&style_save_button, 0);
        lv_style_set_shadow_offset_y(&style_save_button, 2);
        lv_style_set_pad_hor(&style_save_button, 25);
        lv_style_set_pad_ver(&style_save_button, 10);

        style_inited = true;
    }

    lv_obj_t * lv_button_0 = lv_button_create(parent);

    lv_obj_add_style(lv_button_0, &style_save_button, 0);
    lv_obj_t * lv_label_0 = lv_label_create(lv_button_0);
    lv_label_set_text(lv_label_0, text);
    lv_obj_set_align(lv_label_0, LV_ALIGN_CENTER);
    lv_obj_set_style_text_font(lv_label_0, open_sans_bold_24, 0);

    LV_TRACE_OBJ_CREATE("finished");

    lv_obj_set_name(lv_button_0, "save_button_#");

    return lv_button_0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

