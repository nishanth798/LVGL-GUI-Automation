/**
 * @file cont_schedule_time_gen.c
 * @brief Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/

#include "cont_schedule_time_gen.h"
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

lv_obj_t * cont_schedule_time_create(lv_obj_t * parent, const char * title)
{
    LV_TRACE_OBJ_CREATE("begin");

    static lv_style_t style_main_container;
    static lv_style_t style_time_disp_btn;

    static bool style_inited = false;

    if (!style_inited) {
        lv_style_init(&style_main_container);
        lv_style_set_bg_color(&style_main_container, lv_color_hex(0xffffff));
        lv_style_set_border_color(&style_main_container, lv_color_hex(0xd9d9d9));
        lv_style_set_border_opa(&style_main_container, 255);
        lv_style_set_border_width(&style_main_container, 1);
        lv_style_set_width(&style_main_container, 448);
        lv_style_set_height(&style_main_container, LV_SIZE_CONTENT);
        lv_style_set_radius(&style_main_container, 10);
        lv_style_set_shadow_width(&style_main_container, 6);
        lv_style_set_shadow_color(&style_main_container, lv_color_hex(0x000000));
        lv_style_set_shadow_opa(&style_main_container, 64);
        lv_style_set_shadow_offset_x(&style_main_container, 0);
        lv_style_set_shadow_offset_y(&style_main_container, 3);
        lv_style_set_pad_all(&style_main_container, 16);

        lv_style_init(&style_time_disp_btn);
        lv_style_set_bg_color(&style_time_disp_btn, lv_color_hex(0xfafafa));
        lv_style_set_border_color(&style_time_disp_btn, lv_color_hex(0xd1d1d1));
        lv_style_set_border_opa(&style_time_disp_btn, 255);
        lv_style_set_border_width(&style_time_disp_btn, 3);
        lv_style_set_width(&style_time_disp_btn, 120);
        lv_style_set_height(&style_time_disp_btn, 72);
        lv_style_set_radius(&style_time_disp_btn, 5);
        lv_style_set_pad_hor(&style_time_disp_btn, 41);
        lv_style_set_pad_ver(&style_time_disp_btn, 34);
        lv_style_set_text_font(&style_time_disp_btn, open_sans_semibold_36);

        style_inited = true;
    }

    lv_obj_t * lv_obj_0 = lv_obj_create(parent);
    lv_obj_set_flex_flow(lv_obj_0, LV_FLEX_FLOW_COLUMN);

    lv_obj_add_style(lv_obj_0, &style_main_container, 0);
    lv_obj_t * lv_label_0 = lv_label_create(lv_obj_0);
    lv_label_set_text(lv_label_0, title);
    lv_obj_set_style_text_font(lv_label_0, open_sans_semibold_18, 0);
    
    lv_obj_t * lv_obj_1 = lv_obj_create(lv_obj_0);
    lv_obj_set_style_pad_all(lv_obj_1, 0, 0);
    lv_obj_set_height(lv_obj_1, LV_SIZE_CONTENT);
    lv_obj_set_width(lv_obj_1, lv_pct(100));
    lv_obj_set_style_border_width(lv_obj_1, 0, 0);
    lv_obj_set_flex_flow(lv_obj_1, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flag(lv_obj_1, LV_OBJ_FLAG_SCROLLABLE, false);
    lv_obj_t * row_0 = row_create(lv_obj_1);
    lv_obj_set_style_pad_column(row_0, 28, 0);
    lv_obj_t * column_0 = column_create(row_0);
    lv_obj_set_style_border_width(column_0, 0, 0);
    lv_obj_set_style_pad_row(column_0, 16, 0);
    lv_obj_t * lv_button_0 = lv_button_create(column_0);
    lv_obj_set_style_pad_all(lv_button_0, 0, 0);
    lv_obj_t * lv_image_0 = lv_image_create(lv_button_0);
    lv_image_set_src(lv_image_0, image_up_arrow);
    
    lv_obj_add_subject_increment_event(lv_button_0, &hour, LV_EVENT_CLICKED, 1, true);
    
    lv_obj_t * lv_button_1 = lv_button_create(column_0);
    lv_obj_remove_style_all(lv_button_1);
    lv_obj_add_style(lv_button_1, &style_time_disp_btn, 0);
    lv_obj_t * lv_label_1 = lv_label_create(lv_button_1);
    lv_label_bind_text(lv_label_1, &hour, NULL);
    lv_obj_set_align(lv_label_1, LV_ALIGN_CENTER);
    
    lv_obj_t * lv_button_2 = lv_button_create(column_0);
    lv_obj_set_style_pad_all(lv_button_2, 0, 0);
    lv_obj_t * lv_image_1 = lv_image_create(lv_button_2);
    lv_image_set_src(lv_image_1, image_down_arrow);
    
    lv_obj_add_subject_increment_event(lv_button_2, &hour, LV_EVENT_CLICKED, -1, true);
    
    lv_obj_t * column_1 = column_create(row_0);
    lv_obj_set_style_border_width(column_1, 0, 0);
    lv_obj_set_style_pad_row(column_1, 16, 0);
    lv_obj_t * lv_button_3 = lv_button_create(column_1);
    lv_obj_set_style_pad_all(lv_button_3, 0, 0);
    lv_obj_t * lv_image_2 = lv_image_create(lv_button_3);
    lv_image_set_src(lv_image_2, image_up_arrow);
    
    lv_obj_add_subject_increment_event(lv_button_3, &minute, LV_EVENT_CLICKED, 1, true);
    
    lv_obj_t * lv_button_4 = lv_button_create(column_1);
    lv_obj_remove_style_all(lv_button_4);
    lv_obj_add_style(lv_button_4, &style_time_disp_btn, 0);
    lv_obj_t * lv_label_2 = lv_label_create(lv_button_4);
    lv_label_bind_text(lv_label_2, &minute, NULL);
    lv_obj_set_align(lv_label_2, LV_ALIGN_CENTER);
    
    lv_obj_t * lv_button_5 = lv_button_create(column_1);
    lv_obj_set_style_pad_all(lv_button_5, 0, 0);
    lv_obj_t * lv_image_3 = lv_image_create(lv_button_5);
    lv_image_set_src(lv_image_3, image_down_arrow);
    
    lv_obj_add_subject_increment_event(lv_button_5, &minute, LV_EVENT_CLICKED, -1, true);
    
    lv_obj_t * column_2 = column_create(row_0);
    lv_obj_set_style_border_width(column_2, 0, 0);
    lv_obj_set_style_pad_row(column_2, 16, 0);
    lv_obj_t * up_b = lv_button_create(column_2);
    lv_obj_set_style_pad_all(up_b, 0, 0);
    lv_obj_set_name(up_b, "up_b");
    lv_obj_t * lv_image_4 = lv_image_create(up_b);
    lv_image_set_src(lv_image_4, image_up_arrow);
    
    lv_obj_add_subject_set_string_event(up_b, &time_half, LV_EVENT_CLICKED, "AM");
    
    lv_obj_t * lv_button_6 = lv_button_create(column_2);
    lv_obj_remove_style_all(lv_button_6);
    lv_obj_add_style(lv_button_6, &style_time_disp_btn, 0);
    lv_obj_t * lv_label_3 = lv_label_create(lv_button_6);
    lv_label_bind_text(lv_label_3, &time_half, NULL);
    lv_obj_set_align(lv_label_3, LV_ALIGN_CENTER);
    
    lv_obj_t * lv_button_7 = lv_button_create(column_2);
    lv_obj_set_style_pad_all(lv_button_7, 0, 0);
    lv_obj_t * lv_image_5 = lv_image_create(lv_button_7);
    lv_image_set_src(lv_image_5, image_down_arrow);
    
    lv_obj_add_subject_set_string_event(lv_button_7, &time_half, LV_EVENT_CLICKED, "PM");

    LV_TRACE_OBJ_CREATE("finished");

    lv_obj_set_name(lv_obj_0, "cont_schedule_time_#");

    return lv_obj_0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

