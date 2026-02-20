/**
 * @file time_setting_gen.c
 * @brief Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/

#include "time_setting_gen.h"
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

lv_obj_t * time_setting_create(lv_obj_t * parent, const char * text, const char * time)
{
    LV_TRACE_OBJ_CREATE("begin");

    static lv_style_t style_main_cont;
    static lv_style_t style_time_box;
    static lv_style_t style_change_btn;

    static bool style_inited = false;

    if (!style_inited) {
        lv_style_init(&style_main_cont);
        lv_style_set_width(&style_main_cont, 415);
        lv_style_set_height(&style_main_cont, LV_SIZE_CONTENT);
        lv_style_set_pad_all(&style_main_cont, 0);
        lv_style_set_border_width(&style_main_cont, 0);
        lv_style_set_pad_row(&style_main_cont, 2);

        lv_style_init(&style_time_box);
        lv_style_set_bg_color(&style_time_box, lv_color_hex(0xf6f6f6));
        lv_style_set_width(&style_time_box, 234);
        lv_style_set_height(&style_time_box, 48);
        lv_style_set_radius(&style_time_box, 5);
        lv_style_set_pad_hor(&style_time_box, 18);
        lv_style_set_pad_ver(&style_time_box, 15);

        lv_style_init(&style_change_btn);
        lv_style_set_bg_color(&style_change_btn, lv_color_hex(0xffffff));
        lv_style_set_border_color(&style_change_btn, lv_color_hex(0x1a1040));
        lv_style_set_border_opa(&style_change_btn, 255);
        lv_style_set_border_width(&style_change_btn, 3);
        lv_style_set_width(&style_change_btn, 171);
        lv_style_set_height(&style_change_btn, 48);
        lv_style_set_radius(&style_change_btn, 5);
        lv_style_set_shadow_width(&style_change_btn, 4);
        lv_style_set_shadow_color(&style_change_btn, lv_color_hex(0x000000));
        lv_style_set_shadow_opa(&style_change_btn, 64);
        lv_style_set_shadow_offset_x(&style_change_btn, 0);
        lv_style_set_shadow_offset_y(&style_change_btn, 2);
        lv_style_set_pad_hor(&style_change_btn, 25);
        lv_style_set_pad_ver(&style_change_btn, 10);

        style_inited = true;
    }

    lv_obj_t * lv_obj_0 = lv_obj_create(parent);
    lv_obj_set_flex_flow(lv_obj_0, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flag(lv_obj_0, LV_OBJ_FLAG_SCROLLABLE, false);

    lv_obj_add_style(lv_obj_0, &style_main_cont, 0);
    lv_obj_t * lv_obj_1 = lv_obj_create(lv_obj_0);
    lv_obj_set_width(lv_obj_1, 415);
    lv_obj_set_height(lv_obj_1, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(lv_obj_1, 0, 0);
    lv_obj_set_style_border_width(lv_obj_1, 0, 0);
    lv_obj_t * lv_label_0 = lv_label_create(lv_obj_1);
    lv_label_set_text(lv_label_0, text);
    lv_obj_set_style_text_font(lv_label_0, open_sans_semibold_18, 0);
    
    lv_obj_t * lv_obj_2 = lv_obj_create(lv_obj_0);
    lv_obj_set_flag(lv_obj_2, LV_OBJ_FLAG_SCROLLABLE, false);
    lv_obj_add_style(lv_obj_2, &style_time_box, 0);
    lv_obj_t * lv_label_1 = lv_label_create(lv_obj_2);
    lv_label_set_text(lv_label_1, time);
    lv_obj_set_style_text_font(lv_label_1, open_sans_regular_18, 0);
    
    lv_obj_t * lv_obj_3 = lv_obj_create(lv_obj_0);
    lv_obj_set_flag(lv_obj_3, LV_OBJ_FLAG_SCROLLABLE, false);
    lv_obj_set_flag(lv_obj_3, LV_OBJ_FLAG_CLICKABLE, true);
    lv_obj_add_style(lv_obj_3, &style_change_btn, 0);
    lv_obj_t * lv_label_2 = lv_label_create(lv_obj_3);
    lv_label_set_text(lv_label_2, "Change");
    lv_obj_set_align(lv_label_2, LV_ALIGN_CENTER);
    lv_obj_set_style_text_font(lv_label_2, open_sans_bold_18, 0);

    LV_TRACE_OBJ_CREATE("finished");

    lv_obj_set_name(lv_obj_0, "time_setting_#");

    return lv_obj_0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

