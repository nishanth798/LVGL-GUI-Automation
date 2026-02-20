/**
 * @file header_gen.c
 * @brief Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/

#include "header_gen.h"
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

lv_obj_t * header_create(lv_obj_t * parent, const void * left_icon, const char * title, const void * right_icon)
{
    LV_TRACE_OBJ_CREATE("begin");

    static lv_style_t style_header;

    static bool style_inited = false;

    if (!style_inited) {
        lv_style_init(&style_header);
        lv_style_set_bg_color(&style_header, lv_color_hex(0xffffff));
        lv_style_set_border_color(&style_header, lv_color_hex(0xd1d1d1));
        lv_style_set_border_opa(&style_header, 255);
        lv_style_set_width(&style_header, 480);
        lv_style_set_height(&style_header, 64);
        lv_style_set_border_width(&style_header, 1);
        lv_style_set_border_side(&style_header, LV_BORDER_SIDE_BOTTOM);
        lv_style_set_radius(&style_header, 0);

        style_inited = true;
    }

    lv_obj_t * lv_obj_0 = lv_obj_create(parent);

    lv_obj_add_style(lv_obj_0, &style_header, 0);
    lv_obj_t * lv_image_0 = lv_image_create(lv_obj_0);
    lv_image_set_src(lv_image_0, left_icon);
    lv_obj_set_align(lv_image_0, LV_ALIGN_LEFT_MID);
    
    lv_obj_t * lv_image_1 = lv_image_create(lv_obj_0);
    lv_image_set_src(lv_image_1, right_icon);
    lv_obj_set_align(lv_image_1, LV_ALIGN_RIGHT_MID);
    
    lv_obj_t * lv_label_0 = lv_label_create(lv_obj_0);
    lv_label_set_text(lv_label_0, title);
    lv_obj_set_align(lv_label_0, LV_ALIGN_CENTER);
    lv_obj_set_style_text_font(lv_label_0, open_sans_bold_20, 0);

    LV_TRACE_OBJ_CREATE("finished");

    lv_obj_set_name(lv_obj_0, "header_#");

    return lv_obj_0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

