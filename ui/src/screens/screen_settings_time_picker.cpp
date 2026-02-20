/**
 * @file screen_settings_time_picker.cpp
 * @brief Template source file for LVGL objects
 */

/*********************
 * INCLUDES
 *********************/

#include "exit_alert_schedule.h"
#include "screens.h"
#include "sys_state.h"
#include "ui.h"
#include <cstdio>
#include <cstring> // Required for strcmp and strcpy
#include <ctype.h> // Useful for string utility

/*********************
 * DEFINES
 *********************/
// Define the necessary buffer size for set_time (e.g., "12:59PM\0" requires 8 bytes)
#define TIME_STR_SIZE 9

/***********************
 * STATIC VARIABLES
 **********************/
static lv_obj_t *title_cont_left_arrow_obj;
static lv_obj_t *title_cont_info_icon_obj;
static lv_obj_t *screen;
static lv_obj_t *save_btn;
static bool is_time_picker_exit_requested = false; 

// Global buffer to store the final time selected by the user.
char time_set[TIME_STR_SIZE] = {0};

/**********************
 * TYPEDEFS
 **********************/
typedef struct {
  char set_time[TIME_STR_SIZE];
} session_data_t;

// Global session data for current unsaved time
session_data_t session_data = {"9:00PM"};

// Global session data for the last loaded/saved time
session_data_t sd_load;

/***********************
 * STATIC PROTOTYPES
 **********************/
static void update_set_time_string(char *dest_buffer, size_t dest_size);
static bool is_session_data_dirty();
static void btn_save_update_status();
static void save_btn_click_cb(lv_event_t *e);
static void left_arrow_click_cb(lv_event_t *e);
static void screen_load_cb(lv_event_t *e);

/**********************
 * GLOBAL FUNCTIONS
 **********************/

bool is_current_page_time_picker() {
  if (lv_scr_act() == screen) {
    return true;
  }
  return false;
}


/**
 * @brief Checks if the current session data differs from the last loaded data.
 * @return true if time has changed, false otherwise.
 */
static bool is_session_data_dirty() {
  // sd is  &session_data
  session_data_t *sd = &session_data;

  // Compare the current time with the  time stored at screen loading
  if (strcmp(sd->set_time, sd_load.set_time) != 0) {
    return true;
  }
  return false;
}

/**
 * @brief Updates the state of the save button (Active/Disabled) based on session data .
 */
static void btn_save_update_status() {
  if (is_session_data_dirty()) {
    // If dirty, enable the save button
    btn_set_state(save_btn, BTN_STATE_ACTIVE);
  } else {
    // If clean, disable the save button
    btn_set_state(save_btn, BTN_STATE_DISABLED);
  }
}

/**
 * @brief Helper function to read LVGL subjects and safely format the time string.
 * * @param dest_buffer The destination buffer (e.g., sd->set_time or time_set).
 * @param dest_size The size of the destination buffer.
 */
static void update_set_time_string(char *dest_buffer, size_t dest_size) {
  // Read global time subjects (assuming 'hour', 'minute', 'time_half' are global lv_subjects)
  int hr = lv_subject_get_int(&hour);
  int min = lv_subject_get_int(&minute);
  char am_pm[3] = {0};

  // Copy the time half string safely
  const char *time_half_str = lv_subject_get_string(&time_half);
  if (time_half_str) {
    strncpy(am_pm, time_half_str, sizeof(am_pm) - 1);
  }

  // CRITICAL FIX: Use snprintf instead of sprintf to prevent buffer overflow
  // The previous use of sprintf was the likely cause of the segmentation fault
  // due to corrupting memory near session_data or the stack.
  snprintf(dest_buffer, dest_size, "%d:%02d %s", hr, min, am_pm);
}

/**
 * @brief LVGL Observer callback. Called when 'hour', 'minute', or 'time_half' subject changes.
 */
void time_change_observer(lv_observer_t *observer, lv_subject_t *subject) {
  session_data_t *sd = &session_data;

  // 1. Safely update the current session data time string
  update_set_time_string(sd->set_time, sizeof(sd->set_time));

  // 2. Update the save button status
  btn_save_update_status();
}

/**
 * @brief Click callback for the 'Save' button.
 */
static void save_btn_click_cb(lv_event_t *e) {
  // Update the final time_set string (for retrieval via get_time_set)
  is_time_picker_exit_requested = true;
  update_set_time_string(time_set, sizeof(time_set));
  lv_obj_t *screen = screen_settings_exit_alert_sch_get();
  lv_scr_load(screen);
  
}

/**
 * @brief Getter for the final saved time string.
 * @return Pointer to the time_set string.
 */
char *get_time_set(void) { return time_set; }

/**
 * @brief Click callback for the 'Back' arrow.
 */
static void left_arrow_click_cb(lv_event_t *e) {
  // Navigate back to the exit alert schedule screen
  is_time_picker_exit_requested = true;
  lv_obj_t *screen = screen_settings_exit_alert_sch_get();
  lv_scr_load(screen);
}

// Called when the screen is unloaded.
static void screen_unload_cb(lv_event_t *e) {
  if (is_time_picker_exit_requested) {
    // Reset the flag and do nothing else if left arrow was clicked
    is_time_picker_exit_requested = false;
    return;
  } else {
    // Clear exit alert schedule session data if navigating away without left arrow
    clear_exit_alert_schedule_sessiondata();
  }
   
}

/**
 * @brief LVGL Screen Load callback. Initializes state when the screen is shown.
 */
void screen_load_cb(lv_event_t *e) {
  // Set screen label unclickable (template function, kept for completeness)
  tmpl_settings_label_set_clickable(screen, true);

  session_data_t *sd = &session_data;

  // 1. Read current subject values and initialize sd->set_time
  update_set_time_string(sd->set_time, sizeof(sd->set_time));

  // 2. Initialize the loaded state (sd_load) and time_set with the current time.
  strcpy(sd_load.set_time, sd->set_time);
  strcpy(time_set, sd->set_time);

  // 3. Update button status
  btn_save_update_status();
}

/**
 * @brief Creates the LVGL screen and all its objects.
 * @return The created LVGL object (screen).
 */
static void screen_init(){
  screen = tmpl_settings_create_1(NULL, "Settings", true);
  lv_obj_set_flex_flow(screen, LV_FLEX_FLOW_COLUMN);

  // Attach the load callback to initialize data when the screen is shown
  lv_obj_add_event_cb(screen, screen_load_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
  lv_obj_add_event_cb(screen, screen_unload_cb, LV_EVENT_SCREEN_UNLOAD_START, NULL);

  // Back Arrow Setup
  title_cont_left_arrow_obj = get_cont_left_arrow_obj();
  lv_obj_clear_flag(title_cont_left_arrow_obj, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_event_cb(title_cont_left_arrow_obj, left_arrow_click_cb, LV_EVENT_CLICKED, NULL);

  // Info Icon Setup
  title_cont_info_icon_obj = get_cont_info_icon_obj();
  lv_obj_add_flag(title_cont_info_icon_obj, LV_OBJ_FLAG_HIDDEN);

  // Save Button Setup
  save_btn = btn_save_create_1(screen);
  lv_obj_set_flag(save_btn, LV_OBJ_FLAG_IGNORE_LAYOUT, true);
  lv_obj_set_align(save_btn, LV_ALIGN_BOTTOM_MID);
  lv_obj_set_y(save_btn, -16);
  lv_obj_add_event_cb(save_btn, save_btn_click_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_set_width(save_btn, 448);

  // Time PickerContainer Setup
  lv_obj_t *cont_schedule_time_0 =
      cont_schedule_time_create(screen, "Scheduled Monitoring Start Time");
  lv_obj_set_flag(cont_schedule_time_0, LV_OBJ_FLAG_IGNORE_LAYOUT, true);
  lv_obj_set_x(cont_schedule_time_0, 16);
  lv_obj_set_y(cont_schedule_time_0, 79);

  lv_obj_set_name(screen, "screen_set_time");

}

/**
 * @brief Getter for the screen object. Creates it if it doesn't exist.
 * @return The screen object.
 */
lv_obj_t *screen_settings_time_picker_get(void) {
  if (screen == NULL) {
   screen_init();
  }
  return screen;
}

/**********************
 * STATIC FUNCTIONS (Implementation above)
 **********************/