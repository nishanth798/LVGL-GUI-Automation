#include "json_handles.h"
#include "jsonrpc2.h"
#include "lvgl.h"
#include "screens.h"
#include "sys_state.h"
#include "ui.h"
#include <cstdio>  // For sprintf
#include <cstdlib> // For atoi
#include <cstring> // For strcmp, strncpy

// --- Section: Constants and Macros -------------------------------------------

// Bed Width Constants (used for internal logic)
#define BED_WIDTH_UNSET 0
#define BED_WIDTH_STANDARD 38 // Radio button level 1
#define BED_WIDTH_WIDE 42     // Radio button level 2

// Extended/Preset Custom Widths (used in custom selection mode)
#define FULL_XL_BED_WIDTH 54
#define QUEEN_BED_WIDTH 60
#define CALI_KING_BED_WIDTH 72
#define KING_BED_WIDTH 76

// Radio Selector Configuration Data
static const char *labels[] = {
    "Standard",
    "Wide",
    "Custom",
    NULL,
};

static const lv_img_dsc_t *icons[] = {
    &icon_bed_width_standard,
    &bed_width_wide,
    &bed_width_custom,
    NULL,
};

static const char *descs[] = {
    "• Standard default size is 38 in, twin bed.\n",
    "• Wide default size is 42 in, small bariatric.\n",
    NULL,
};

// --- Section: Static Variables -----------------------------------------------

// LVGL Objects
static lv_obj_t *screen;
static lv_obj_t *bed_width_selector, *label_desc, *selected_width_label;
static lv_obj_t *btn_save, *cont_btns_nav, *cont_controls, *cont_width_selection_parent;
static lv_obj_t *title_cont_left_arrow_obj, *title_cont_info_icon_obj;
static lv_obj_t *label_subtitle;

// State and Data Variables
static bool is_custom_width_selected = false;
char *selected_bed_width_text; // Stores the display text for the selected width
static SETTINGS_MODE settings_mode;

// Session Data Structure
typedef struct {
  uint8_t bed_width;
} session_data_t;

static session_data_t session_data = {
    .bed_width = BED_WIDTH_UNSET,
};
static session_data_t sd_load; // Holds data that is present during load time.

// --- Section: Forward Declarations -------------------------------------------

// Core Logic/Getters/Setters
static bool is_session_data_dirty();
static void btn_save_update_status();
static void jsonrpc_request_send(session_data_t *data);
static void set_bed_width_level(lv_obj_t *obj, uint8_t bedWidth);
static uint8_t get_bed_width_level(lv_obj_t *obj);
static void set_desc(uint8_t bedWidth);
static void update_width_selection(uint8_t bedWidth);
void update_session_data_width_value(uint8_t bedWidth_level);

// Event Handlers
static void btn_radio_click_cb(lv_event_t *e);
static void mbtn_event_cb(lv_event_t *e);
static void info_icon_click_cb(lv_event_t *e);
static void left_arrow_click_cb(lv_event_t *e);
static void change_width_event_cb(lv_event_t *e);
static void btn_back_click_cb(lv_event_t *e);
static void btn_next_click_cb(lv_event_t *e);
static void btn_save_click_cb(lv_event_t *e);
static void request_timeout_cb(lv_event_t *e);
static void set_state_fail_cb(lv_event_t *e);
static void set_state_ok_cb(lv_event_t *e);
static void screen_load_cb(lv_event_t *e);

// UI/API
static void screen_init();

// --- Section: Session Data Management (Public API) ---------------------------

/**
 *  Clears the bed width in the session data.
 */
void clear_bed_width_sessiondata() { session_data.bed_width = BED_WIDTH_UNSET; }

/**
 * Gets the current bed width from session data.
 * returns The current bed width (in inches).
 */
uint8_t settings_get_bed_width() { return session_data.bed_width; }

/**
 *  Sets the bed width in session data.
 *  "bed_width" The bed width value (in inches).
 */
void settings_set_bed_width(uint8_t bed_width) { session_data.bed_width = bed_width; }

/**
 *  Sets the flag indicating if the custom width screen was entered.
 *  flag True if custom width screen was visited, false otherwise.
 */
void set_bed_width_selection_flag(bool flag) { is_custom_width_selected = flag; }

// --- Section: Core Logic and Helpers -----------------------------------------

// Check if session data has unsaved changes
static bool is_session_data_dirty() {
  session_data_t *sd = &session_data;
  if (sd->bed_width != sd_load.bed_width) {
    return true;
  }
  return false;
}

// Update the save button state based on session data dirty status
static void btn_save_update_status() {
  if (settings_mode == SETTINGS_MODE_SAVE && is_session_data_dirty()) {
    btn_set_state(btn_save, BTN_STATE_ACTIVE);
  } else {
    btn_set_state(btn_save, BTN_STATE_DISABLED);
  }
}

// Send JSON-RPC request to save bed width setting
static void jsonrpc_request_send(session_data_t *data) {
  JsonDocument doc;
  JsonObject params = doc["params"].to<JsonObject>();
  params["width"] = data->bed_width;
  serializeJsonRpcRequest(0, CMD_SAVE_BED_WIDTH, params);
}

static void set_bed_width_level(lv_obj_t *obj, uint8_t bedWidth) {
  uint8_t btn_id;
  if (bedWidth == BED_WIDTH_STANDARD) {
    btn_id = 0;
  } else if (bedWidth == BED_WIDTH_WIDE) {
    btn_id = 1;
  } else {
    btn_id = 2; // Custom level
  }
  radio_selector_set_selected_btn_1(obj, btn_id);
}

static uint8_t get_bed_width_level(lv_obj_t *obj) {
  // Returns 0, 1, or 2; add 1 to get level 1, 2, or 3.
  return (uint8_t)(radio_selector_get_selected_btn_1(obj) + 1);
}

// Update description label and layout based on selected bed width
static void set_desc(uint8_t bedWidth) {
  if (bedWidth == BED_WIDTH_STANDARD) {
    lv_label_set_text_static(label_desc, descs[0]);
    lv_obj_clear_flag(label_desc, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(cont_width_selection_parent, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_size(cont_controls, 448, 209);
    lv_obj_align(bed_width_selector, LV_ALIGN_CENTER, 0, -7);
    lv_obj_align(label_subtitle, LV_ALIGN_TOP_LEFT, 0, 9);
  } else if (bedWidth == BED_WIDTH_WIDE) {
    lv_label_set_text_static(label_desc, descs[1]);
    lv_obj_clear_flag(label_desc, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(cont_width_selection_parent, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_size(cont_controls, 448, 209);
    lv_obj_align(bed_width_selector, LV_ALIGN_CENTER, 0, -7);
    lv_obj_align(label_subtitle, LV_ALIGN_TOP_LEFT, 0, 9);
  } else {
    // Custom width selected
    lv_label_set_text_static(label_desc, "");
    lv_obj_add_flag(label_desc, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(cont_width_selection_parent, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_size(cont_controls, 448, 227);
    lv_obj_align(bed_width_selector, LV_ALIGN_CENTER, 0, -22);
    lv_obj_align(label_subtitle, LV_ALIGN_TOP_LEFT, 0, 3);
  }
}

// Update the selected width label based on bed width value
static void update_width_selection(uint8_t bedWidth) {
  switch (bedWidth) {
  case FULL_XL_BED_WIDTH:
    selected_bed_width_text = (char *)"Full/Full XL (54 in.)";
    break;
  case QUEEN_BED_WIDTH:
    selected_bed_width_text = (char *)"Queen (60 in).";
    break;
  case CALI_KING_BED_WIDTH:
    selected_bed_width_text = (char *)"California King (72 in.)";
    break;
  case KING_BED_WIDTH:
    selected_bed_width_text = (char *)"King (76 in.)";
    break;
  default:
    if (bedWidth == BED_WIDTH_STANDARD || bedWidth == BED_WIDTH_WIDE) {
      selected_bed_width_text = (char *)"Full/Full XL (54 in.)"; // Fallback/default text
      break;
    }
    // True custom value (e.g., 55 in.)
    static char custom_value[7];
    sprintf(custom_value, "%d in.", bedWidth);
    selected_bed_width_text = custom_value;
    break;
  }

  lv_label_set_text_static(selected_width_label, selected_bed_width_text);
}

// Update session data based on selected radio button level
void update_session_data_width_value(uint8_t bedWidth_level) {
  if (bedWidth_level == 3) { // Custom selected
    if (strcmp(selected_bed_width_text, "Full/Full XL (54 in.)") == 0) {
      session_data.bed_width = FULL_XL_BED_WIDTH;
    } else if (strcmp(selected_bed_width_text, "Queen (60 in).") == 0) {
      session_data.bed_width = QUEEN_BED_WIDTH;
    } else if (strcmp(selected_bed_width_text, "California King (72 in.)") == 0) {
      session_data.bed_width = CALI_KING_BED_WIDTH;
    } else if (strcmp(selected_bed_width_text, "King (76 in.)") == 0) {
      session_data.bed_width = KING_BED_WIDTH;
    } else {
      // Must be a custom value (e.g., "55 in.")
      char first_two[3];
      strncpy(first_two, selected_bed_width_text, 2);
      first_two[2] = '\0';
      session_data.bed_width = atoi(first_two);
    }
  } else if (bedWidth_level == 2) { // Wide selected
    session_data.bed_width = BED_WIDTH_WIDE;
  } else { // Standard selected
    session_data.bed_width = BED_WIDTH_STANDARD;
  }
}

// --- Section: Event Handlers -------------------------------------------------

static void mbtn_event_cb(lv_event_t *e) {
  lv_obj_t *btn = lv_event_get_current_target_obj(e);
  lv_obj_t *msgbox = lv_obj_get_parent(lv_obj_get_parent(btn));
  lv_obj_t *label = lv_obj_get_child(btn, 0);
  const char *btn_text = lv_label_get_text(label);

  if (strcmp(btn_text, "Don't Save") == 0) {
    lv_msgbox_close(msgbox);
    lv_screen_load(screen_settings_home_get());
  } else if (strcmp(btn_text, "Save") == 0) {
    lv_msgbox_close(msgbox);
    jsonrpc_request_send(&session_data);
    btn_set_state(btn_save, BTN_STATE_SUBMITTING);
  }
}

static void info_icon_click_cb(lv_event_t *e) {
  show_system_info_screen((char *)"settings_bed_width");
}

static void left_arrow_click_cb(lv_event_t *e) {
  settings_mode = screen_settings_get_mode();
  if (settings_mode == SETTINGS_MODE_SAVE) {
    if (is_session_data_dirty()) {
      // Show confirmation dialog if unsaved changes exist
      lv_obj_t *mbox = msgbox_confirm_save_create();
      lv_obj_t *footer = lv_msgbox_get_footer(mbox);
      for (uint32_t i = 0; i < lv_obj_get_child_count(footer); i++) {
        lv_obj_t *child = lv_obj_get_child(footer, i);
        lv_obj_add_event_cb(child, mbtn_event_cb, LV_EVENT_CLICKED, NULL);
      }
      return;
    }
    lv_screen_load(screen_settings_home_get());
  }
}

static void btn_radio_click_cb(lv_event_t *e) {
  uint8_t bedWidth_level = get_bed_width_level(bed_width_selector);

  //  Manage radio selector level based on selection
  if (bedWidth_level == 3) {
    lv_obj_clear_flag(cont_width_selection_parent, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(label_desc, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_size(cont_controls, lv_pct(100), 227);
    lv_obj_align(bed_width_selector, LV_ALIGN_CENTER, 0, -22);
    lv_obj_align(label_subtitle, LV_ALIGN_TOP_LEFT, 0, 3);
  } else {
    lv_obj_add_flag(cont_width_selection_parent, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(label_desc, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_size(cont_controls, 448, 209);
    lv_obj_align(bed_width_selector, LV_ALIGN_CENTER, 0, -7);
    lv_obj_align(label_subtitle, LV_ALIGN_TOP_LEFT, 0, 9);
    if (bedWidth_level == 2) {
      set_desc(BED_WIDTH_WIDE);
    } else {
      set_desc(BED_WIDTH_STANDARD);
    }
  }

  // 2. Update session data and save button status
  update_session_data_width_value(bedWidth_level);
  btn_save_update_status();
}

static void change_width_event_cb(lv_event_t *e) {
  is_custom_width_selected = true;
  lv_obj_t *screen = get_change_width_settings_screen();
  lv_scr_load(screen);
}

static void btn_back_click_cb(lv_event_t *e) {
  lv_obj_t *screen = screen_settings_bms_get();
  lv_scr_load(screen);
}

static void btn_next_click_cb(lv_event_t *e) {
  send_bed_width_session_data(session_data.bed_width);
  screen_settings_set_mode(SETTINGS_MODE_ACTIVATION);

  sys_state_t *ss = sys_state_get();
  if (ss->fts_avail.bed_pos) {
    lv_obj_t *screen = screen_settings_get_bed_placement();
    lv_screen_load(screen);
  } else if (ss->fts_avail.occ_size) {
    lv_obj_t *screen = screen_settings_get_occupant_size();
    lv_screen_load(screen);
  } else {
    lv_obj_t *screen = screen_settings_audio_get();
    lv_screen_load(screen);
  }
}

static void btn_save_click_cb(lv_event_t *e) {
  jsonrpc_request_send(&session_data);
  btn_set_state(btn_save, BTN_STATE_SUBMITTING);
}

static void request_timeout_cb(lv_event_t *e) {
  btn_set_state(btn_save, BTN_STATE_ACTIVE);
  toast_show(screen, no_response_str);
}

static void set_state_fail_cb(lv_event_t *e) {
  btn_set_state(btn_save, BTN_STATE_ACTIVE);
  toast_show(screen, invalid_response_str);
}

static void set_state_ok_cb(lv_event_t *e) {
  btn_set_state(btn_save, BTN_STATE_SUBMITTED);
  // update load time session data with latest session data after save is successful
  const sys_state_t *ss = sys_state_get();
  sd_load.bed_width = ss->bed_wid;
}

// Called when the screen is loaded
static void screen_load_cb(lv_event_t *e) {

  // Set mode-specific UI elements(Back and Next buttons or Save button)
  settings_mode = screen_settings_get_mode();
  settings_show_mode(screen, btn_save, cont_btns_nav, settings_mode);

  session_data_t *sd = &session_data;
  if (settings_mode == SETTINGS_MODE_ACTIVATION) {
    // show the header info icon symbol and hide the left arrow
    lv_obj_add_flag(title_cont_left_arrow_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(title_cont_info_icon_obj, LV_OBJ_FLAG_HIDDEN);
    // set bed width level to default if unset
    if (sd->bed_width == BED_WIDTH_UNSET) {
      sd->bed_width = BED_WIDTH_STANDARD;
    }
  } else if (settings_mode == SETTINGS_MODE_SAVE) {
    // show the header left arrow and hide the info icon
    lv_obj_clear_flag(title_cont_left_arrow_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(title_cont_info_icon_obj, LV_OBJ_FLAG_HIDDEN);

    // Load existing bed width from system state if not already set in session data
    const sys_state_t *ss = sys_state_get();
    if (ss->bed_wid == BED_WIDTH_UNSET) {
      sd->bed_width = BED_WIDTH_STANDARD;
      sd_load.bed_width = BED_WIDTH_STANDARD;
    } else {
      // Only overwrite session data if user hasn't made a custom selection
      if (is_custom_width_selected == false) {
        sd->bed_width = ss->bed_wid;
      }
      sd_load.bed_width = ss->bed_wid;
    }
    //  Update save button status
    btn_save_update_status();
  }
  // Reset custom width selection flag
  is_custom_width_selected = false;
  // Set radio selector based on loaded/selected bed width
  set_bed_width_level(bed_width_selector, sd->bed_width);
  // Update description
  set_desc(sd->bed_width);
  // Update selected width label
  update_width_selection(sd->bed_width);

  if (!is_alert_toast_hidden()) {
    set_alert_toast_parent(screen);
  }
}

// --- Section: UI Initialization ----------------------------------------------

static void screen_init() {
  // 1. Header and Event Setup
  screen = tmpl_settings_create_1(NULL, "Settings", true);
  lv_obj_set_style_bg_color(screen, lv_color_hex(VST_COLOR_SCREEN_BG_GREY), LV_PART_MAIN);

  lv_obj_t *left_arrow_obj = get_img_left_arrow_obj();
  lv_obj_add_flag(left_arrow_obj, LV_OBJ_FLAG_EVENT_BUBBLE);

  title_cont_left_arrow_obj = get_cont_left_arrow_obj();
  lv_obj_add_event_cb(title_cont_left_arrow_obj, left_arrow_click_cb, LV_EVENT_CLICKED, NULL);

  title_cont_info_icon_obj = get_cont_info_icon_obj();
  lv_obj_add_event_cb(title_cont_info_icon_obj, info_icon_click_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_add_event_cb(screen, screen_load_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
  lv_obj_add_event_cb(screen, request_timeout_cb, (lv_event_code_t)MY_EVENT_REQUEST_TIMEOUT, NULL);
  lv_obj_add_event_cb(screen, set_state_ok_cb, (lv_event_code_t)MY_EVENT_SET_STATE_OK, NULL);
  lv_obj_add_event_cb(screen, set_state_fail_cb, (lv_event_code_t)MY_EVENT_SET_STATE_FAIL, NULL);

  // 2. Content Container
  lv_obj_t *cont_content = lv_obj_create(screen);
  lv_obj_set_size(cont_content, lv_pct(100), 400);
  lv_obj_set_flex_flow(cont_content, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(cont_content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);
  lv_obj_clear_flag(cont_content, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_border_width(cont_content, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_top(cont_content, 0, LV_PART_MAIN);
  lv_obj_set_style_bg_color(cont_content, lv_color_hex(VST_COLOR_SCREEN_BG_GREY), LV_PART_MAIN);

  // 3. Controls Container
  cont_controls = lv_obj_create(cont_content);
  lv_obj_set_size(cont_controls, 448, 209);
  lv_obj_clear_flag(cont_controls, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_border_width(cont_controls, 1, LV_PART_MAIN);
  lv_obj_set_style_border_color(cont_controls, lv_color_hex(0xD1D1D1), LV_PART_MAIN);

  // Shadow styles
  lv_obj_set_style_shadow_color(cont_controls, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_set_style_shadow_width(cont_controls, 6, LV_PART_MAIN);
  lv_obj_set_style_shadow_spread(cont_controls, 0, LV_PART_MAIN);
  lv_obj_set_style_shadow_opa(cont_controls, 64, LV_PART_MAIN);
  lv_obj_set_style_shadow_offset_x(cont_controls, 2, LV_PART_MAIN);
  lv_obj_set_style_shadow_offset_y(cont_controls, 2, LV_PART_MAIN);

  // Sub title
  label_subtitle = lv_label_create(cont_controls);
  lv_obj_set_size(label_subtitle, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_add_flag(label_subtitle, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_set_pos(label_subtitle, 0, 9);
  lv_label_set_text_static(label_subtitle, "Bed Width");
  lv_obj_set_style_text_font(label_subtitle, &opensans_semibold_18, LV_PART_MAIN);
  lv_obj_set_style_text_color(label_subtitle, lv_color_hex(VST_COLOR_PRIMARY_TEXT), LV_PART_MAIN);

  // Bed width radio selector
  bed_width_selector = radio_selector_create_1(cont_controls, labels, icons);
  lv_obj_set_style_pad_left(bed_width_selector, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_right(bed_width_selector, 0, LV_PART_MAIN);
  lv_obj_add_event_cb(bed_width_selector, btn_radio_click_cb, LV_EVENT_VALUE_CHANGED, NULL);
  lv_obj_align(bed_width_selector, LV_ALIGN_CENTER, 0, -7);

  // Description label (for Standard/Wide)
  label_desc = lv_label_create(cont_controls);
  lv_obj_set_width(label_desc, LV_SIZE_CONTENT);
  lv_obj_set_height(label_desc, LV_SIZE_CONTENT);
  lv_obj_set_pos(label_desc, 12, 150);
  lv_label_set_text_static(label_desc, descs[0]);
  lv_obj_set_style_text_font(label_desc, &opensans_italic_14, LV_PART_MAIN);
  lv_obj_set_style_text_color(label_desc, lv_color_hex(0x707070), LV_PART_MAIN);
  lv_obj_set_style_text_line_space(label_desc, 4, LV_PART_MAIN);

  // Custom Width Selection Container (Hidden by default)
  cont_width_selection_parent = lv_obj_create(cont_controls);
  lv_obj_set_size(cont_width_selection_parent, 416, 48);
  lv_obj_set_flex_flow(cont_width_selection_parent, LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_style_pad_gap(cont_width_selection_parent, 10, 0);
  lv_obj_set_style_pad_all(cont_width_selection_parent, 0, 0);
  lv_obj_set_style_border_width(cont_width_selection_parent, 0, 0);
  lv_obj_set_style_radius(cont_width_selection_parent, 0, 0);
  lv_obj_align(cont_width_selection_parent, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_add_flag(cont_width_selection_parent, LV_OBJ_FLAG_HIDDEN);

  // Container for Selected Width Display
  lv_obj_t *cont_selected_size = lv_obj_create(cont_width_selection_parent);
  lv_obj_set_scroll_dir(cont_selected_size, LV_DIR_NONE);
  lv_obj_set_style_width(cont_selected_size, 203, 0);
  lv_obj_set_style_height(cont_selected_size, 48, 0);
  lv_obj_set_style_bg_color(cont_selected_size, lv_color_hex(0xF6F6F6), 0);
  lv_obj_set_style_radius(cont_selected_size, 5, 0);
  lv_obj_set_style_border_width(cont_selected_size, 0, 0);

  // Label for Selected Width Text
  selected_width_label = lv_label_create(cont_selected_size);
  lv_obj_set_style_text_font(selected_width_label, &opensans_regular_16, 0);
  lv_obj_set_style_text_color(selected_width_label, lv_color_hex(0x31344E), 0);
  lv_obj_align(selected_width_label, LV_ALIGN_LEFT_MID, 0, 2);

  // "Change Width" Button
  lv_obj_t *cont_change_width = lv_obj_create(cont_width_selection_parent);
  lv_obj_set_scroll_dir(cont_change_width, LV_DIR_NONE);
  lv_obj_set_style_width(cont_change_width, 203, 0);
  lv_obj_set_style_height(cont_change_width, 48, 0);
  lv_obj_set_style_radius(cont_change_width, 5, 0);
  lv_obj_set_style_bg_color(cont_change_width, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_border_width(cont_change_width, 3, 0);
  lv_obj_set_style_border_color(cont_change_width, lv_color_hex(0x1A1040), 0);
  lv_obj_set_style_shadow_width(cont_change_width, 2, 0);
  lv_obj_set_style_shadow_color(cont_change_width, lv_color_hex(0x000000), 0);
  lv_obj_set_style_shadow_ofs_y(cont_change_width, 2, 0);
  lv_obj_set_style_shadow_opa(cont_change_width, 64, 0);

  // Label for "Change Width"
  lv_obj_t *chg_width_label = lv_label_create(cont_change_width);
  lv_label_set_text(chg_width_label, "Change Width");
  lv_obj_set_style_text_font(chg_width_label, &opensans_bold_18, 0);
  lv_obj_set_style_text_color(chg_width_label, lv_color_hex(0x31344E), 0);
  lv_obj_align(chg_width_label, LV_ALIGN_CENTER, 0, 2);

  lv_obj_add_flag(cont_change_width, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(cont_change_width, change_width_event_cb, LV_EVENT_CLICKED, NULL);

  // 4. Save Button (Save Mode Only)
  btn_save = btn_save_create_1(cont_content);
  lv_obj_add_event_cb(btn_save, btn_save_click_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_flag(btn_save, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_set_align(btn_save, LV_ALIGN_BOTTOM_MID);

  // 5. Navigation Buttons Container (Activation Mode)
  cont_btns_nav = lv_obj_create(cont_content);
  lv_obj_set_size(cont_btns_nav, 448, 80);
  lv_obj_clear_flag(cont_btns_nav, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_border_width(cont_btns_nav, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_left(cont_btns_nav, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_right(cont_btns_nav, 0, LV_PART_MAIN);
  lv_obj_add_flag(cont_btns_nav, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(cont_btns_nav, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_set_align(cont_btns_nav, LV_ALIGN_BOTTOM_MID);
  lv_obj_set_style_bg_color(cont_btns_nav, lv_color_hex(VST_COLOR_SCREEN_BG_GREY), LV_PART_MAIN);

  // Back Button
  lv_obj_t *btn_back = btn_secondary_create_1(cont_btns_nav, "Back");
  lv_obj_set_align(btn_back, LV_ALIGN_LEFT_MID);
  lv_obj_add_event_cb(btn_back, btn_back_click_cb, LV_EVENT_CLICKED, NULL);

  // Next Button
  lv_obj_t *btn_next = btn_primary_create_1(cont_btns_nav, "Next", VST_COLOR_DARKBLUE);
  lv_obj_set_align(btn_next, LV_ALIGN_RIGHT_MID);
  lv_obj_add_event_cb(btn_next, btn_next_click_cb, LV_EVENT_CLICKED, NULL);
}

// --- Section: Public API -----------------------------------------------------

lv_obj_t *screen_settings_get_bed_width() {
  if (screen == NULL) {
    screen_init();
  }
  return screen;
}