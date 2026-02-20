/**
 * @file exit_alert_schedule.h
 * @brief Declarations for Exit Alert Schedule and related UI components.
 */

#ifndef EXIT_ALERT_SCHEDULE_H
#define EXIT_ALERT_SCHEDULE_H

// Maximum length for subject strings (used for LVGL subjects)
#ifndef UI_SUBJECT_STRING_LENGTH
#define UI_SUBJECT_STRING_LENGTH 3
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

// Include LVGL core header (handles both simple and nested include paths)
#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#include "fonts.h"  // Custom font declarations
#include "images.h" // Custom image declarations
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
// (Add extern declarations for permanent screen objects here if needed)

/*----------------
 * Global styles
 *----------------*/
// (Add extern declarations for global LVGL styles here if needed)

/*----------------
 * Fonts
 *----------------*/


/*----------------
 * Images
 *----------------*/
// Image resources for up and down arrows (used in time pickers)
extern const void *image_up_arrow;
extern const void *image_down_arrow;

/*----------------
 * Subjects
 *----------------*/
// LVGL subjects for monitoring selection and time picker values
extern lv_subject_t hour;         // Subject for hour value
extern lv_subject_t minute;       // Subject for minute value
extern lv_subject_t time_half;    // Subject for AM/PM value

/**********************
 * GLOBAL PROTOTYPES
 **********************/

// Get the main Exit Alert Schedule screen object
lv_obj_t *screen_settings_exit_alert_sch_get(void);

// Get the time picker screen object
lv_obj_t *screen_settings_time_picker_get(void);

// Get the currently set time as a string (from the time picker)
char *get_time_set(void);

// Create the continuous schedule time picker widget
lv_obj_t *cont_schedule_time_create(lv_obj_t *parent, const char *title);

// Observer callback for time changes (hour/minute/AM-PM)
void time_change_observer(lv_observer_t *observer, lv_subject_t *subject);

// Set the exit schedule for audio settings (start/end times)
void screen_settings_audio_set_exit_schedule(const char *mon_start, const char *mon_end);

// Clear session data for the exit alert schedule screen
void clear_exit_alert_schedule_sessiondata(void);

// Create a time setting row (label + time + event callback)
lv_obj_t *time_setting_create(lv_obj_t *parent, const char *text, const char *time,
                              lv_event_cb_t event_cb);

// Update the title of the time picker widget
void update_time_picker_title(const char *title);

/*----------------
 * Event Callbacks
 *----------------*/
// (Add event callback prototypes here if needed)

/**
 * @brief Initialize the Exit Alert Schedule component library.
 * @param asset_path Path to assets (fonts, images, etc.)
 */
void exit_alert_schedule_init(const char *asset_path);

/**********************
 *      MACROS
 **********************/

/**********************
 *   POST INCLUDES
 **********************/

/* Include all the widget and components of this library */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*EXIT_ALERT_SCHEDULE_H*/