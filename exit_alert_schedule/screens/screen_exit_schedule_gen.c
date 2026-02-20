/**
 * @file screen_exit_schedule_gen.c
 * @brief Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/

#include "screen_exit_schedule_gen.h"
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

lv_obj_t * screen_exit_schedule_create(void)
{
    LV_TRACE_OBJ_CREATE("begin");

    static lv_style_t style_alert_schedule;

    static bool style_inited = false;

    if (!style_inited) {
        lv_style_init(&style_alert_schedule);
        lv_style_set_bg_color(&style_alert_schedule, lv_color_hex(0xffffff));
        lv_style_set_border_color(&style_alert_schedule, lv_color_hex(0xD1D1D1));
        lv_style_set_border_width(&style_alert_schedule, 1);
        lv_style_set_radius(&style_alert_schedule, 7);

        style_inited = true;
    }

    lv_obj_t * lv_obj_0 = lv_obj_create(NULL);

    lv_obj_t * header_0 = header_create(lv_obj_0, image_left_arrow, "Settings", image_close);
    
    lv_obj_t * lv_obj_1 = lv_obj_create(lv_obj_0);
    lv_obj_set_width(lv_obj_1, 448);
    lv_obj_set_height(lv_obj_1, LV_SIZE_CONTENT);
    lv_obj_set_x(lv_obj_1, 16);
    lv_obj_set_y(lv_obj_1, 80);
    lv_obj_set_flag(lv_obj_1, LV_OBJ_FLAG_SCROLLABLE, false);
    lv_obj_set_flex_flow(lv_obj_1, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_style(lv_obj_1, &style_alert_schedule, 0);
    lv_obj_t * lv_label_0 = lv_label_create(lv_obj_1);
    lv_label_set_text(lv_label_0, "Exit Alert Schedule");
    lv_obj_set_style_text_font(lv_label_0, open_sans_semibold_18, 0);
    
    lv_obj_t * dropdown = lv_dropdown_create(lv_obj_1);
    lv_obj_set_name(dropdown, "dropdown");
    lv_obj_set_width(dropdown, 415);
    lv_obj_set_height(dropdown, 64);
    lv_obj_set_y(dropdown, 36);
    lv_obj_set_style_radius(dropdown, 10, 0);
    lv_obj_set_style_border_width(dropdown, 1, 0);
    lv_obj_set_style_border_color(dropdown, lv_color_hex(0xD1D1D1), 0);
    lv_obj_set_style_text_font(dropdown, open_sans_regular_18, 0);
    lv_dropdown_set_options(dropdown, "Continuous Monitoring\nScheduled Monitoring");
    lv_obj_t * lv_dropdown_list_0 = lv_dropdown_get_list(dropdown);
    lv_obj_add_subject_toggle_event(lv_dropdown_list_0, &selected_mon, LV_EVENT_CLICKED);
    
    lv_obj_t * timer = lv_obj_create(lv_obj_1);
    lv_obj_set_name(timer, "timer");
    lv_obj_set_align(timer, LV_ALIGN_CENTER);
    lv_obj_set_width(timer, 448);
    lv_obj_set_height(timer, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(timer, 0, 0);
    lv_obj_set_flag(timer, LV_OBJ_FLAG_SCROLLABLE, false);
    lv_obj_set_flex_flow(timer, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_border_width(timer, 0, 0);
    lv_obj_bind_flag_if_eq(timer, &selected_mon, LV_OBJ_FLAG_HIDDEN, 1);
    lv_obj_t * time_setting_0 = time_setting_create(timer, "Start time", "9:00 PM");
    
    lv_obj_t * time_setting_1 = time_setting_create(timer, "End Time", "7:00 AM");
    
    lv_obj_t * save_button_0 = save_button_create(lv_obj_0, "Save");
    lv_obj_set_align(save_button_0, LV_ALIGN_BOTTOM_MID);
    lv_obj_set_y(save_button_0, -16);

    LV_TRACE_OBJ_CREATE("finished");

    lv_obj_set_name(lv_obj_0, "screen_exit_schedule");

    return lv_obj_0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

