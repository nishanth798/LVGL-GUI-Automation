/**
 * @file exit_alert_schedule_gen.h
 */

#ifndef EXIT_ALERT_SCHEDULE_GEN_H
#define EXIT_ALERT_SCHEDULE_GEN_H

#ifndef UI_SUBJECT_STRING_LENGTH
#define UI_SUBJECT_STRING_LENGTH 256
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL VARIABLES
 **********************/

/*-------------------
 * Permanent screens
 *------------------*/

/*----------------
 * Global styles
 *----------------*/

/*----------------
 * Fonts
 *----------------*/

extern lv_font_t * open_sans_bold_18;

extern lv_font_t * open_sans_bold_20;

extern lv_font_t * open_sans_bold_24;

extern lv_font_t * open_sans_semibold_18;

extern lv_font_t * open_sans_semibold_36;

extern lv_font_t * open_sans_regular_18;

/*----------------
 * Images
 *----------------*/

extern const void * image_left_arrow;
extern const void * image_close;
extern const void * image_up_arrow;
extern const void * image_down_arrow;

/*----------------
 * Subjects
 *----------------*/

extern lv_subject_t selected_mon;
extern lv_subject_t hour;
extern lv_subject_t minute;
extern lv_subject_t time_half;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/*----------------
 * Event Callbacks
 *----------------*/

/**
 * Initialize the component library
 */

void exit_alert_schedule_init_gen(const char * asset_path);

/**********************
 *      MACROS
 **********************/

/**********************
 *   POST INCLUDES
 **********************/

/*Include all the widget and components of this library*/
#include "components/column_gen.h"
#include "components/cont_schedule_time_gen.h"
#include "components/header_gen.h"
#include "components/row_gen.h"
#include "components/save_button/save_button_gen.h"
#include "components/time_setting_gen.h"
#include "screens/screen_exit_schedule_gen.h"
#include "screens/screen_set_time_gen.h"

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*EXIT_ALERT_SCHEDULE_GEN_H*/