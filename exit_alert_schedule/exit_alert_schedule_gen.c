/**
 * @file exit_alert_schedule_gen.c
 */

/*********************
 *      INCLUDES
 *********************/

#include "exit_alert_schedule_gen.h"

#if LV_USE_XML
#endif /* LV_USE_XML */

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

lv_font_t * open_sans_bold_18;
extern lv_font_t open_sans_bold_18_data;
lv_font_t * open_sans_bold_20;
extern lv_font_t open_sans_bold_20_data;
lv_font_t * open_sans_bold_24;
extern lv_font_t open_sans_bold_24_data;
lv_font_t * open_sans_semibold_18;
extern lv_font_t open_sans_semibold_18_data;
lv_font_t * open_sans_semibold_36;
extern lv_font_t open_sans_semibold_36_data;
lv_font_t * open_sans_regular_18;
extern lv_font_t open_sans_regular_18_data;

/*----------------
 * Images
 *----------------*/

const void * image_left_arrow;
extern const void * image_left_arrow_data;
const void * image_close;
extern const void * image_close_data;
const void * image_up_arrow;
extern const void * image_up_arrow_data;
const void * image_down_arrow;
extern const void * image_down_arrow_data;

/*----------------
 * Subjects
 *----------------*/

lv_subject_t selected_mon;
lv_subject_t hour;
lv_subject_t minute;
lv_subject_t time_half;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void exit_alert_schedule_init_gen(const char * asset_path)
{
    char buf[256];

    /*----------------
     * Global styles
     *----------------*/

    /*----------------
     * Fonts
     *----------------*/

    /* get font 'open_sans_bold_18' from a C array */
    open_sans_bold_18 = &open_sans_bold_18_data;
    /* get font 'open_sans_bold_20' from a C array */
    open_sans_bold_20 = &open_sans_bold_20_data;
    /* get font 'open_sans_bold_24' from a C array */
    open_sans_bold_24 = &open_sans_bold_24_data;
    /* get font 'open_sans_semibold_18' from a C array */
    open_sans_semibold_18 = &open_sans_semibold_18_data;
    /* get font 'open_sans_semibold_36' from a C array */
    open_sans_semibold_36 = &open_sans_semibold_36_data;
    /* get font 'open_sans_regular_18' from a C array */
    open_sans_regular_18 = &open_sans_regular_18_data;


    /*----------------
     * Images
     *----------------*/
    image_left_arrow = &image_left_arrow_data;
    image_close = &image_close_data;
    image_up_arrow = &image_up_arrow_data;
    image_down_arrow = &image_down_arrow_data;

    /*----------------
     * Subjects
     *----------------*/
    lv_subject_init_int(&selected_mon, 1);
    lv_subject_init_int(&hour, 9);
    lv_subject_set_min_value_int(&hour, 0);
    lv_subject_set_max_value_int(&hour, 11);
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

#if LV_USE_XML
    /* Register widgets */

    /* Register fonts */
    lv_xml_register_font(NULL, "open_sans_bold_18", open_sans_bold_18);
    lv_xml_register_font(NULL, "open_sans_bold_20", open_sans_bold_20);
    lv_xml_register_font(NULL, "open_sans_bold_24", open_sans_bold_24);
    lv_xml_register_font(NULL, "open_sans_semibold_18", open_sans_semibold_18);
    lv_xml_register_font(NULL, "open_sans_semibold_36", open_sans_semibold_36);
    lv_xml_register_font(NULL, "open_sans_regular_18", open_sans_regular_18);

    /* Register subjects */
    lv_xml_register_subject(NULL, "selected_mon", &selected_mon);
    lv_xml_register_subject(NULL, "hour", &hour);
    lv_xml_register_subject(NULL, "minute", &minute);
    lv_xml_register_subject(NULL, "time_half", &time_half);

    /* Register callbacks */
#endif

    /* Register all the global assets so that they won't be created again when globals.xml is parsed.
     * While running in the editor skip this step to update the preview when the XML changes */
#if LV_USE_XML && !defined(LV_EDITOR_PREVIEW)
    /* Register images */
    lv_xml_register_image(NULL, "image_left_arrow", image_left_arrow);
    lv_xml_register_image(NULL, "image_close", image_close);
    lv_xml_register_image(NULL, "image_up_arrow", image_up_arrow);
    lv_xml_register_image(NULL, "image_down_arrow", image_down_arrow);
#endif

#if LV_USE_XML == 0
    /*--------------------
     *  Permanent screens
     *-------------------*/
    /* If XML is enabled it's assumed that the permanent screens are created
     * manaully from XML using lv_xml_create() */
#endif
}

/* Callbacks */

/**********************
 *   STATIC FUNCTIONS
 **********************/