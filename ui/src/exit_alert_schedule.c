/**
 * @file exit_alert_schedule.c
 */

/*********************
 *      INCLUDES
 *********************/

#include "exit_alert_schedule.h"


/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/*----------------
 * Translations
 *----------------*/

/**********************
 *  GLOBAL VARIABLES
 **********************/

/*--------------------
 *  Permanent screens
 *-------------------*/

/*----------------
 * Global styles
 *----------------*/

/*----------------
 * Fonts
 *----------------*/


/*----------------
 * Images
 *----------------*/

const void * image_up_arrow;
extern const void * image_up_arrow_data;
const void * image_down_arrow;
extern const void * image_down_arrow_data;

/*----------------
 * Subjects
 *----------------*/
lv_subject_t hour;
lv_subject_t minute;
lv_subject_t time_half;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void exit_alert_schedule_init(const char * asset_path)
{
    char buf[256];

    /*----------------
     * Global styles
     *----------------*/

    /*----------------
     * Fonts
     *----------------*/

    /*----------------
     * Images
     *----------------*/
    image_up_arrow = &image_up_arrow_data;
    image_down_arrow = &image_down_arrow_data;

    /*----------------
     * Subjects
     *----------------*/
    lv_subject_init_int(&hour, 9);
    lv_subject_set_min_value_int(&hour, 1);
    lv_subject_set_max_value_int(&hour, 12);
    lv_subject_init_int(&minute, 0);
    lv_subject_set_min_value_int(&minute, 0);
    lv_subject_set_max_value_int(&minute, 59);
    static char time_half_buf[UI_SUBJECT_STRING_LENGTH];
    static char time_half_prev_buf[UI_SUBJECT_STRING_LENGTH];
    lv_subject_init_string(&time_half,
                           time_half_buf,
                           time_half_prev_buf,
                           UI_SUBJECT_STRING_LENGTH,
                           "AM"
                          );

    /*----------------
     * Translations
     *----------------*/


}

/* Callbacks */

/**********************
 *   STATIC FUNCTIONS
 **********************/