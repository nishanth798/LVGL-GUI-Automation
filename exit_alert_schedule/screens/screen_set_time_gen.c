/**
 * @file screen_set_time_gen.c
 * @brief Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/

#include "screen_set_time_gen.h"
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

lv_obj_t * screen_set_time_create(void)
{
    LV_TRACE_OBJ_CREATE("begin");


    static bool style_inited = false;

    if (!style_inited) {

        style_inited = true;
    }

    lv_obj_t * lv_obj_0 = lv_obj_create(NULL);
    lv_obj_set_flex_flow(lv_obj_0, LV_FLEX_FLOW_COLUMN);

    lv_obj_t * header_0 = header_create(lv_obj_0, image_left_arrow, "Settings", image_close);
    
    lv_obj_t * cont_schedule_time_0 = cont_schedule_time_create(lv_obj_0, "Scheduled Monitoring Start Time");
    lv_obj_set_flag(cont_schedule_time_0, LV_OBJ_FLAG_IGNORE_LAYOUT, true);
    lv_obj_set_x(cont_schedule_time_0, 16);
    lv_obj_set_y(cont_schedule_time_0, 76);
    
    lv_obj_t * save_button_0 = save_button_create(lv_obj_0, "Save");
    lv_obj_set_flag(save_button_0, LV_OBJ_FLAG_IGNORE_LAYOUT, true);
    lv_obj_set_align(save_button_0, LV_ALIGN_BOTTOM_MID);
    lv_obj_set_y(save_button_0, -16);

    LV_TRACE_OBJ_CREATE("finished");

    lv_obj_set_name(lv_obj_0, "screen_set_time");

    return lv_obj_0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

