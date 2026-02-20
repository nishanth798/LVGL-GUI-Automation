/**
 * @file screen_settings_exit_alert_sch.cpp
 * @brief Template source file for LVGL objects
 */

/*********************
 * INCLUDES
 *********************/

#include "exit_alert_schedule.h"
#include "json_handles.h"
#include "jsonrpc2.h"
#include "screens.h"
#include "sys_state.h"
#include "ui.h"
#include <cstdio>
#include <cstring> // Required for strcmp

/*********************
 * DEFINES
 *********************/

/**********************
 * TYPEDEFS
 **********************/

// Structure to hold monitoring session start/end times in 24-hour format (HHMM)
typedef struct {
  char mon_start[5]; // "2100" (HHMM) - 4 chars + null terminator
  char mon_end[5];   // "0700" (HHMM)
} session_data_t;

/***********************
 * STATIC VARIABLES
 **********************/
SETTINGS_MODE settings_mode;
// Main screen object and key interactive elements
static lv_obj_t *screen, *btn_save, *cont_btns_nav;
// Title bar elements for navigation/info
static lv_obj_t *title_cont_left_arrow_obj, *title_cont_info_icon_obj;
lv_obj_t *cont_time_fields; // Container for time setting fields
lv_obj_t *dropdown;         // Monitoring mode selection dropdown
// Local session data (HHMM format)
static session_data_t session_data = {0};
// time data at load time
static session_data_t sd_load = {0};

typedef enum { TIME_NONE, TIME_START, TIME_END } time_clicked_t;

typedef struct {
  char start_time_str[9]; // "9:00PM"
  char end_time_str[9];   // "7:00AM"
  lv_obj_t *label_start_time;
  lv_obj_t *label_end_time;
  time_clicked_t last_clicked;
} time_data_t;

time_data_t time_data = {
    "9:00PM", // start_time_str
    "7:00AM", // end_time_str
    NULL,     // label_start_time
    NULL,     // label_end_time
    TIME_NONE // last_clicked
};

/***********************
 * STATIC PROTOTYPES
 **********************/

static void btn_save_update_status(void);
static bool is_session_data_dirty(void);
static void set_shadow_style(lv_obj_t *obj, int offset_x, int offset_y, int width,int spread, int color, int opa);
static void btn_back_click_cb(lv_event_t *e);
static void btn_next_click_cb(lv_event_t *e);
static void mbtn_event_cb(lv_event_t *e);
static void left_arrow_click_cb(lv_event_t *e);
static void select_dropdown_option(lv_event_t *e);
static void info_icon_click_cb(lv_event_t *e);
static void screen_load_cb(lv_event_t *e);
static void btn_save_click_cb(lv_event_t *e);

/**********************
 * GLOBAL FUNCTIONS
 **********************/

/* "H:MMAM/PM" -> "HHMM" (24h). Returns 0 on success. */
int convert_time_to_24hr(const char *in_str, char *out_str, size_t out_size) {
  int hour_12, min;
  char ampm[3];

  if (sscanf(in_str, "%d:%2d%2s", &hour_12, &min, ampm) != 3)
    return -1;
  if (hour_12 < 1 || hour_12 > 12 || min < 0 || min > 59)
    return -1;

  ampm[0] = (char)toupper(ampm[0]);
  ampm[1] = (char)toupper(ampm[1]);
  ampm[2] = '\0';

  int hour_24 = 0;
  if (strcmp(ampm, "AM") == 0)
    hour_24 = (hour_12 == 12) ? 0 : hour_12;
  else if (strcmp(ampm, "PM") == 0)
    hour_24 = (hour_12 == 12) ? 12 : (hour_12 + 12);
  else
    return -1;

  if (out_size < 5)
    return -1; // needs "HHMM\0"
  snprintf(out_str, out_size, "%02d%02d", hour_24, min);
  return 0;
}

/* "HHMM" (24h) -> "H:MM AM/PM". Returns 0 on success. */
int convert_time_to_12hr(const char *in_str, char *out_str, size_t out_size) {
  int hour_24, min;

  if (sscanf(in_str, "%2d%2d", &hour_24, &min) != 2)
    return -1;
  if (hour_24 < 0 || hour_24 > 23 || min < 0 || min > 59)
    return -1;

  const char *ampm = (hour_24 < 12) ? "AM" : "PM";
  int hour_12 = (hour_24 == 0) ? 12 : (hour_24 <= 12 ? hour_24 : hour_24 - 12);

  if (out_size < 9)
    return -1; // needs "H:MM AM\0"
  snprintf(out_str, out_size, "%d:%02d %s", hour_12, min, ampm);
  return 0;
}

// set shadow style to object
// offset_x: horizontal offset
// offset_y: vertical offset
// width: shadow width
// spread: shadow spread
// color: shadow color in hex
// opa: shadow opacity  
void set_shadow_style(lv_obj_t *obj, int offset_x, int offset_y, int width,int spread, int color, int opa) {
  lv_obj_set_style_shadow_offset_x(obj, offset_x,0);
  lv_obj_set_style_shadow_offset_y(obj, offset_y,0);
  lv_obj_set_style_shadow_width(obj, width,0);
  lv_obj_set_style_shadow_spread(obj, spread,0);
  lv_obj_set_style_shadow_color(obj, lv_color_hex(color),0);
  lv_obj_set_style_shadow_opa(obj, opa,0);
}


/**
 * @brief Event handler for the Start Time change button.
 * Converts 24hr session data to 12hr, initializes LVGL subjects for the Time Picker screen,
 * sets a flag, and loads the Time Picker screen.
 *
 * @param e LVGL event structure.
 */
void start_time_change_event_cb(lv_event_t *e) {
  session_data_t *sd = &session_data;

  // Convert stored 24hr time (HHMM) to 12hr display format (H:MMAM/PM)
  convert_time_to_12hr(sd->mon_start, time_data.start_time_str, sizeof(time_data.start_time_str));

  // Parse the 12hr buffer to pre-populate the time picker subjects
  int hr, min;
  char am_pm[3];
  // This sscanf assumes format like "9:00 PM" or "12:30 AM"
  if (sscanf(time_data.start_time_str, "%d%*c%2d %2s", &hr, &min, am_pm) == 3) {
    lv_subject_set_int(&hour, hr);
    lv_subject_set_int(&minute, min);
    lv_subject_copy_string(&time_half, am_pm);
  }

  time_data.last_clicked = TIME_START;
  lv_obj_t *screen = screen_settings_time_picker_get();
  lv_scr_load(screen);
 // Update the time picker title to indicate Start Time selection
  update_time_picker_title("Scheduled Monitoring Start Time");
}

/**
 * @brief Event handler for the End Time change button.
 * Converts 24hr session data to 12hr, initializes LVGL subjects for the Time Picker screen,
 * sets a flag, and loads the Time Picker screen.
 *
 * @param e LVGL event structure.
 */
void end_time_change_event_cb(lv_event_t *e) {
  session_data_t *sd = &session_data;
  convert_time_to_12hr(sd->mon_end, time_data.end_time_str, sizeof(time_data.end_time_str));

  int hr, min;
  char am_pm[3];
  if (sscanf(time_data.end_time_str, "%d%*c%2d %2s", &hr, &min, am_pm) == 3) {
    lv_subject_set_int(&hour, hr);
    lv_subject_set_int(&minute, min);
    lv_subject_copy_string(&time_half, am_pm);
  }
  
  time_data.last_clicked = TIME_END;
  lv_obj_t *screen = screen_settings_time_picker_get();
  lv_scr_load(screen);
  // Update the time picker title to indicate End Time selection
  update_time_picker_title("Scheduled Monitoring End Time");
}

/**
 * @brief Clears the local session data for the Exit Alert Schedule.
 */
void clear_exit_alert_schedule_sessiondata() {
  session_data_t *sd = &session_data;
  strcpy(sd->mon_start, "");
  strcpy(sd->mon_end, "");
  time_data.last_clicked = TIME_NONE;
}

/**********************
 * STATIC FUNCTIONS
 **********************/

/**
 * @brief Checks if the current session data differs from the loaded data.
 * Used to determine if the "Save" button should be enabled.
 *
 * @return true if data is changed, false otherwise.
 */
static bool is_session_data_dirty() {
  session_data_t *sd = &session_data;
  if (strcmp(sd->mon_start, sd_load.mon_start) != 0 || strcmp(sd->mon_end, sd_load.mon_end) != 0) {
    return true;
  }
  return false;
}

/**
 * @brief Updates the state (Enabled/Disabled) of the save button .
 */
static void btn_save_update_status() {
  if (settings_mode == SETTINGS_MODE_SAVE && is_session_data_dirty()) {
    btn_set_state(btn_save, BTN_STATE_ACTIVE);
  } else {
    btn_set_state(btn_save, BTN_STATE_DISABLED);
  }
}

// Sends JSON-RPC request to save the current mon_start and mon_end values
static void jsonrpc_request_send(session_data_t *data) {
  JsonDocument doc;
  JsonObject params = doc["params"].to<JsonObject>();
  params["mon_start"] = data->mon_start;
  params["mon_end"] = data->mon_end;
  serializeJsonRpcRequest(0, CMD_SAVE_MON_SCH, params);
}

// Event handler for the Save button .
// Sends JSON-RPC request and updates save state to submitting.
static void btn_save_click_cb(lv_event_t *e) {
  jsonrpc_request_send(&session_data);
  btn_set_state(btn_save, BTN_STATE_SUBMITTING);
}

/**
 * @brief Event handler for the "Back" navigation button .
 * Loads the main alerts settings screen.
 *
 * @param e LVGL event structure.
 */
static void btn_back_click_cb(lv_event_t *e) {
  lv_obj_t *screen = screen_settings_alerts_get();
  lv_scr_load(screen);
}

/**
 * @brief Event handler for the "Next" navigation button .
 * Saves the current schedule and loads the next screen in the activation sequence (BMS settings).
 *
 * @param e LVGL event structure.
 */
static void btn_next_click_cb(lv_event_t *e) {
  // Pass the schedule settings to the in-room audio screen context, so activate button pressed it
  // can request should use mon_start and mon_end values
  screen_settings_audio_set_exit_schedule(session_data.mon_start, session_data.mon_end);
  lv_obj_t *screen = screen_settings_bms_get();
  lv_scr_load(screen);
}

/**
 * @brief Event handler for message box buttons .
 * Used when navigating away with unsaved changes.
 *
 * @param e LVGL event structure.
 */
static void mbtn_event_cb(lv_event_t *e) {
  lv_obj_t *btn = lv_event_get_current_target_obj(e);
  // Find the message box parent
  lv_obj_t *msgbox = lv_obj_get_parent(lv_obj_get_parent(btn));
  lv_obj_t *label = lv_obj_get_child(btn, 0);
  const char *btn_text = lv_label_get_text(label);

  if (strcmp(btn_text, "Don't Save") == 0) {
    lv_msgbox_close(msgbox);
    lv_scr_load(screen_settings_home_get());
  } else if (strcmp(btn_text, "Save") == 0) {
    lv_msgbox_close(msgbox);
    jsonrpc_request_send(&session_data);
    btn_set_state(btn_save, BTN_STATE_SUBMITTING);
  }
}

/**
 * @brief Event handler for the back arrow in the title bar (Save mode).
 * @param e LVGL event structure.
 */
static void left_arrow_click_cb(lv_event_t *e) {
  settings_mode = screen_settings_get_mode();
  if (settings_mode == SETTINGS_MODE_SAVE) {
    if (is_session_data_dirty()) {
      // Show confirmation message box
      lv_obj_t *mbox = msgbox_confirm_save_create();
      lv_obj_t *footer = lv_msgbox_get_footer(mbox);
      for (uint32_t i = 0; i < lv_obj_get_child_count(footer); i++) {
        lv_obj_t *child = lv_obj_get_child(footer, i);
        lv_obj_add_event_cb(child, mbtn_event_cb, LV_EVENT_CLICKED, NULL);
      }
      return;
    }
    lv_scr_load(screen_settings_home_get());
  }
}

// Event handler for request timeout during save operation
static void request_timeout_cb(lv_event_t *e) {
  btn_set_state(btn_save, BTN_STATE_ACTIVE);
  toast_show(screen, no_response_str);
}

// Event handler for failed save operation
static void set_state_fail_cb(lv_event_t *e) {
  btn_set_state(btn_save, BTN_STATE_ACTIVE);
  toast_show(screen, invalid_response_str);
}

// Event handler for successful save operation
static void set_state_ok_cb(lv_event_t *e) {
  btn_set_state(btn_save, BTN_STATE_SUBMITTED);
  // update load time session data with latest session data after save is successful
  const sys_state_t *ss = sys_state_get();
  strcpy(sd_load.mon_start, ss->mon_start);
  strcpy(sd_load.mon_end, ss->mon_end);
}

// Event handler for the info icon in the title bar.
static void info_icon_click_cb(lv_event_t *e) {
  show_system_info_screen((char *)"settings_exit_alert_schedule");
}

/**
 * @brief Event handler for the Monitoring Dropdown.
 * Updates session data and UI visibility based on selected option.
 *
 * @param e LVGL event structure.
 */
static void select_dropdown_option(lv_event_t *e) {
  char buf[32];
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target_obj(e);

  if (code != LV_EVENT_VALUE_CHANGED) {
    return;
  }
  
  lv_dropdown_get_selected_str(obj, buf, sizeof(buf));
  settings_mode = screen_settings_get_mode();
  session_data_t *sd = &session_data;

  bool is_continuous = (strcmp(buf, "Continuous Monitoring") == 0);
  bool is_scheduled = (strcmp(buf, "Scheduled Monitoring") == 0);

  if (is_continuous) {
    strcpy(sd->mon_start, "");
    strcpy(sd->mon_end, "");
    if (settings_mode == SETTINGS_MODE_ACTIVATION) {
      strcpy(time_data.start_time_str, "");
      strcpy(time_data.end_time_str, "");
    }
    lv_obj_add_flag(cont_time_fields, LV_OBJ_FLAG_HIDDEN);
  } else if (is_scheduled) {
    // Only set defaults if switching from continuous (empty session data)
    if (sd->mon_start[0] == '\0' && sd->mon_end[0] == '\0') {
      if (settings_mode == SETTINGS_MODE_ACTIVATION) {
        strcpy(sd->mon_start, "2100");
        strcpy(sd->mon_end, "0700");
      } else if (settings_mode == SETTINGS_MODE_SAVE) {
        if (sd->mon_start[0] == '\0' && sd->mon_end[0] == '\0') {
          strcpy(time_data.start_time_str, "9:00 PM");
          strcpy(time_data.end_time_str, "7:00 AM");
        }
        convert_time_to_24hr(time_data.start_time_str, sd->mon_start, sizeof(sd->mon_start));
        convert_time_to_24hr(time_data.end_time_str, sd->mon_end, sizeof(sd->mon_end));
      }
      // Update 12hr buffers and labels
      convert_time_to_12hr(sd->mon_start, time_data.start_time_str, sizeof(time_data.start_time_str));
      convert_time_to_12hr(sd->mon_end, time_data.end_time_str, sizeof(time_data.end_time_str));
      lv_label_set_text(time_data.label_start_time, time_data.start_time_str);
      lv_label_set_text(time_data.label_end_time, time_data.end_time_str);
    }
    lv_obj_clear_flag(cont_time_fields, LV_OBJ_FLAG_HIDDEN);
  }

  btn_save_update_status();
}

// Event handler to adjust dropdown list position on click
static void dropdown_click_cb(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_target_obj(e);
  lv_obj_t *list = lv_dropdown_get_list(obj);
  lv_obj_set_y(list, 204); // 204 pixels gap; 
}

// Helper function to handle time picker results and update data/labels
static void handle_time_picker_result(session_data_t *sd) {
  if (time_data.last_clicked == TIME_START) {
    strcpy(time_data.start_time_str, get_time_set());
    convert_time_to_24hr(time_data.start_time_str, sd->mon_start, sizeof(sd->mon_start));
    lv_label_set_text(time_data.label_start_time, time_data.start_time_str);
    lv_obj_clear_flag(cont_time_fields, LV_OBJ_FLAG_HIDDEN);
  } else if (time_data.last_clicked == TIME_END) {
    strcpy(time_data.end_time_str, get_time_set());
    convert_time_to_24hr(time_data.end_time_str, sd->mon_end, sizeof(sd->mon_end));
    lv_label_set_text(time_data.label_end_time, time_data.end_time_str);
    lv_obj_clear_flag(cont_time_fields, LV_OBJ_FLAG_HIDDEN);
  }
  time_data.last_clicked = TIME_NONE;
}

/**
 * @brief Event handler that runs when the screen starts loading.
 *
 * @param e LVGL event structure.
 */
static void screen_load_cb(lv_event_t *e) {
  settings_mode = screen_settings_get_mode();
  settings_show_mode(screen, btn_save, cont_btns_nav, settings_mode);
  session_data_t *sd = &session_data;
  sys_state_t *sys_state = sys_state_get();

  //  Initialising Data Load (Only if not returning from time picker)
  if (time_data.last_clicked == TIME_NONE) {

    // Loading data into session_data based on mode
    if (settings_mode == SETTINGS_MODE_SAVE) {
      // SAVE MODE: Load existing system state into session_data for editing
      strcpy(sd->mon_start, sys_state->mon_start);
      strcpy(sd->mon_end, sys_state->mon_end);

      // Also load into sd_loading
      strcpy(sd_load.mon_start, sys_state->mon_start);
      strcpy(sd_load.mon_end, sys_state->mon_end);
    }
    // If SETTINGS_MODE_ACTIVATION, sd is already initialized to the default "9:00PM" / "" in
    // globals.

    // B. Set UI and 12hr buffers based on session_data
    bool is_scheduled = (strcmp(sd->mon_start, "") != 0 && strcmp(sd->mon_end, "") != 0);
    int selected_dropdown_index = is_scheduled ? 1 : 0;

    lv_dropdown_set_selected(dropdown, selected_dropdown_index);

    if (is_scheduled) {
      // Update 12hr buffers and labels from 24hr session data
      convert_time_to_12hr(sd->mon_start, time_data.start_time_str, sizeof(time_data.start_time_str));
      convert_time_to_12hr(sd->mon_end, time_data.end_time_str, sizeof(time_data.end_time_str));
      lv_label_set_text(time_data.label_start_time, time_data.start_time_str);
      lv_label_set_text(time_data.label_end_time, time_data.end_time_str);
      lv_obj_clear_flag(cont_time_fields, LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_add_flag(cont_time_fields, LV_OBJ_FLAG_HIDDEN);
    }
  }

  // Handling Time Picker Return
  // This logic runs AFTER the initial load if flags are set.
  handle_time_picker_result(sd);

  // 3. UI Mode and Button Status
  if (settings_mode == SETTINGS_MODE_ACTIVATION) {
    lv_obj_add_flag(title_cont_left_arrow_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(title_cont_info_icon_obj, LV_OBJ_FLAG_HIDDEN);
  } else if (settings_mode == SETTINGS_MODE_SAVE) {
    lv_obj_clear_flag(title_cont_left_arrow_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(title_cont_info_icon_obj, LV_OBJ_FLAG_HIDDEN);
  }

  btn_save_update_status(); // Update button state after all data updates

  if (!is_alert_toast_hidden()) {
    set_alert_toast_parent(screen);
  }
}

/**
 * @brief Creates the LVGL screen for Exit Alert Schedule settings.
 *
 * @return lv_obj_t* Pointer to the created screen object.
 */
static void screen_init(void) {
  exit_alert_schedule_init(NULL);
  LV_TRACE_OBJ_CREATE("begin");

  static lv_style_t style_alert_schedule;
  static bool style_inited = false;

  // Initialize custom style for the main container
  if (!style_inited) {
    lv_style_init(&style_alert_schedule);
    lv_style_set_bg_color(&style_alert_schedule, lv_color_hex(0xffffff));
    lv_style_set_border_color(&style_alert_schedule, lv_color_hex(0xD1D1D1));
    lv_style_set_border_width(&style_alert_schedule, 1);
    lv_style_set_radius(&style_alert_schedule, 7);
    style_inited = true;
  }

  // Create the base screen using a template function
  screen = tmpl_settings_create_1(NULL, "Settings", true);

  // Get title bar elements and assign event callbacks
  title_cont_left_arrow_obj = get_cont_left_arrow_obj();
  lv_obj_add_event_cb(title_cont_left_arrow_obj, left_arrow_click_cb, LV_EVENT_CLICKED, NULL);

  title_cont_info_icon_obj = get_cont_info_icon_obj();
  lv_obj_add_event_cb(title_cont_info_icon_obj, info_icon_click_cb, LV_EVENT_CLICKED, NULL);

  // Register screen load event handler
  lv_obj_add_event_cb(screen, screen_load_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
  lv_obj_add_event_cb(screen, request_timeout_cb, (lv_event_code_t)MY_EVENT_REQUEST_TIMEOUT, NULL);
  lv_obj_add_event_cb(screen, set_state_ok_cb, (lv_event_code_t)MY_EVENT_SET_STATE_OK, NULL);
  lv_obj_add_event_cb(screen, set_state_fail_cb, (lv_event_code_t)MY_EVENT_SET_STATE_FAIL, NULL);

  // --- Main Content Container (lv_obj_1) ---
  lv_obj_t *lv_obj_1 = lv_obj_create(screen);
  lv_obj_set_width(lv_obj_1, 448);
  lv_obj_set_height(lv_obj_1, LV_SIZE_CONTENT);
  lv_obj_set_x(lv_obj_1, 16);
  lv_obj_set_y(lv_obj_1, 80);
  lv_obj_set_flag(lv_obj_1, LV_OBJ_FLAG_SCROLLABLE, false);
  lv_obj_set_flex_flow(lv_obj_1, LV_FLEX_FLOW_COLUMN);
  lv_obj_add_style(lv_obj_1, &style_alert_schedule, 0);

  // Adding shadow to main container
  // parameters: object, offset_x, offset_y, width, spread, color, opa
  set_shadow_style(lv_obj_1, 0, 3, 6, 0, 0x000000, 64);

  // Container for Exit Alert Schedule Title and Dropdown 
  lv_obj_t *cont_ttl_dropdown = lv_obj_create(lv_obj_1);
  lv_obj_set_width(cont_ttl_dropdown, 448);
  lv_obj_set_height(cont_ttl_dropdown, 100);
  lv_obj_set_style_pad_all(cont_ttl_dropdown, 0, 0);
  lv_obj_set_style_border_width(cont_ttl_dropdown, 0, 0);

  // Title inside the container
  lv_obj_t *lv_label_0 = lv_label_create(cont_ttl_dropdown);
  lv_label_set_text(lv_label_0, "Exit Alert Schedule");
  lv_obj_set_y(lv_label_0, 2);
  lv_obj_set_style_text_font(lv_label_0, &opensans_semibold_18, 0);

  // Monitoring Mode Dropdown
  dropdown = lv_dropdown_create(cont_ttl_dropdown);
  lv_obj_set_name(dropdown, "dropdown");
  lv_obj_set_width(dropdown, 415);
  lv_obj_set_height(dropdown, 64);
  lv_obj_set_y(dropdown, 34);
  lv_obj_set_style_radius(dropdown, 10, 0);
  lv_obj_set_style_border_width(dropdown, 1, 0);
  lv_obj_set_style_border_color(dropdown, lv_color_hex(0xD1D1D1), 0);
  lv_obj_set_style_text_font(dropdown, &opensans_regular_18, 0);
  lv_obj_set_style_text_color(dropdown, lv_color_hex(VST_COLOR_ACTIVATE_SENSOR_BLUE),   LV_PART_MAIN);
  lv_obj_set_style_border_color(dropdown, lv_color_hex(0x74B4FF), LV_STATE_CHECKED);
  lv_dropdown_set_symbol(dropdown, &icon_down_arrow);
  lv_obj_set_style_transform_rotation(dropdown, 1800, LV_PART_INDICATOR | LV_STATE_CHECKED);
  lv_obj_set_style_pad_all(dropdown, 15, 0);

  lv_dropdown_set_options(dropdown, "Continuous Monitoring\n" "Scheduled Monitoring");
  lv_obj_add_event_cb(dropdown, select_dropdown_option, LV_EVENT_VALUE_CHANGED, NULL);
 
  // Styling the dropdown list
  lv_obj_t * dropdown_list = lv_dropdown_get_list(dropdown);
  lv_obj_set_style_bg_color(dropdown_list, lv_color_hex(VST_COLOR_BTN_SELECTED_SKYBLUE), LV_PART_SELECTED | LV_STATE_CHECKED);
  lv_obj_set_style_text_line_space(dropdown_list, 46, LV_PART_MAIN);
  lv_obj_set_style_max_height(dropdown_list, LV_COORD_MAX, 0);
  lv_obj_set_style_pad_ver(dropdown_list, 30, LV_PART_MAIN);
  lv_obj_set_style_border_width(dropdown_list, 1, 0);
  lv_obj_set_style_border_color(dropdown_list, lv_color_hex(VST_COLOR_CONTAINER_BORDER), 0);
  lv_obj_set_style_radius(dropdown_list, 10, 0);
  lv_obj_set_style_text_font(dropdown_list, &opensans_regular_18, 0);
  lv_obj_set_style_text_color(dropdown_list, lv_color_hex(VST_COLOR_INACTIVE_TEXT),   LV_PART_MAIN);
  
  // Adjust dropdown list position on click
  lv_obj_add_event_cb(dropdown, dropdown_click_cb, LV_EVENT_CLICKED, NULL);

  // Adding shadow to dropdown list
  // parameters: object, offset_x, offset_y, width, spread, color, opa
  set_shadow_style(dropdown_list, 2, 4, 4, 0, 0x000000, 64);

  // Adding shadow to dropdown
  // parameters: object, offset_x, offset_y, width, spread, color, opa
  set_shadow_style(dropdown, 0, 2, 4, 0, 0x000000, 64);

  // Time Setting Container (Visible only for Scheduled Monitoring)
  cont_time_fields = lv_obj_create(lv_obj_1);
  lv_obj_set_name(cont_time_fields, "cont_time_fields");
  lv_obj_set_align(cont_time_fields, LV_ALIGN_CENTER);
  lv_obj_set_width(cont_time_fields, 448);
  lv_obj_set_height(cont_time_fields, LV_SIZE_CONTENT);
  lv_obj_set_style_pad_all(cont_time_fields, 0, 0);
  lv_obj_set_flag(cont_time_fields, LV_OBJ_FLAG_SCROLLABLE, false);
  lv_obj_set_flex_flow(cont_time_fields, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_border_width(cont_time_fields, 0, 0);
  lv_obj_add_flag(cont_time_fields, LV_OBJ_FLAG_HIDDEN); // Hidden by default

  // Start Time Picker Component
  lv_obj_t *time_setting_0 = time_setting_create(cont_time_fields, "Start time",
                                                 time_data.start_time_str, start_time_change_event_cb);
  // Get the label object within the time setting component to update it later
  // time_setting_create returns a container where the label is at path [1][0]
  time_data.label_start_time = lv_obj_get_child(lv_obj_get_child(time_setting_0, 1), 0);

  // End Time Picker Component
  lv_obj_t *time_setting_1 = time_setting_create(cont_time_fields, "End Time", time_data.end_time_str,
                                                 end_time_change_event_cb);
  // Get the label object within the time setting component to update it later
  time_data.label_end_time = lv_obj_get_child(lv_obj_get_child(time_setting_1, 1), 0);

  // --- Save Button (for SETTINGS_MODE_SAVE) ---
  btn_save = btn_save_create_1(screen);
  lv_obj_add_flag(btn_save, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_set_align(btn_save, LV_ALIGN_BOTTOM_MID);
  lv_obj_add_flag(btn_save, LV_OBJ_FLAG_HIDDEN); // Hidden by default (shown only in SAVE mode)
  lv_obj_set_y(btn_save, -16);
  lv_obj_set_width(btn_save, 448);
  lv_obj_add_event_cb(btn_save, btn_save_click_cb, LV_EVENT_CLICKED, NULL);

  // --- Navigation Buttons Container (for SETTINGS_MODE_ACTIVATION) ---
  cont_btns_nav = lv_obj_create(screen);
  lv_obj_set_size(cont_btns_nav, 448, 80);
  lv_obj_clear_flag(cont_btns_nav, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_border_width(cont_btns_nav, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_left(cont_btns_nav, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_right(cont_btns_nav, 0, LV_PART_MAIN);
  lv_obj_add_flag(cont_btns_nav, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_set_align(cont_btns_nav, LV_ALIGN_BOTTOM_MID);
  lv_obj_set_y(cont_btns_nav, -16);
  lv_obj_set_style_bg_color(cont_btns_nav, lv_color_hex(VST_COLOR_SCREEN_BG_GREY),
                            LV_PART_MAIN); // background color

  // "Back" Button
  lv_obj_t *btn_back = btn_secondary_create_1(cont_btns_nav, "Back");
  lv_obj_set_align(btn_back, LV_ALIGN_LEFT_MID);
  lv_obj_add_event_cb(btn_back, btn_back_click_cb, LV_EVENT_CLICKED, NULL);

  // "Next" Button
  lv_obj_t *btn_next = btn_primary_create_1(cont_btns_nav, "Next", VST_COLOR_DARKBLUE);
  lv_obj_set_align(btn_next, LV_ALIGN_RIGHT_MID);
  lv_obj_add_event_cb(btn_next, btn_next_click_cb, LV_EVENT_CLICKED, NULL);

  LV_TRACE_OBJ_CREATE("finished");
  lv_obj_set_name(screen, "screen_exit_schedule");

}

/**
 * @brief Getter for the exit schedule screen object.
 * Creates the screen if it doesn't exist (singleton pattern).
 *
 * @return lv_obj_t* Pointer to the screen object.
 */
lv_obj_t *screen_settings_exit_alert_sch_get() {
  if (screen == NULL) {
    screen_init();
  }
  return screen;
}