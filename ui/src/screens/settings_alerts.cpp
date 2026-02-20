#include "json_handles.h"
#include "jsonrpc2.h"
#include "resp_state.h"
#include "screens.h"
#include "ui.h"
#include <cstdio>

/*******************************************************************************
 * STATIC VARIABLES AND UI OBJECTS 
 ******************************************************************************/

static lv_obj_t *screen;
static lv_obj_t *cont_bed_alerts;
static lv_obj_t *cont_fall_alerts;
static lv_obj_t *cont_reposition_alerts;
static lv_obj_t *btn_next, *btn_save;
static lv_obj_t *title_cont_left_arrow_obj;
static lv_obj_t *title_cont_info_icon_obj;
static SETTINGS_MODE settings_mode;
static lv_obj_t *cont_btns_nav;

// session data
typedef struct {
  bool bed_alt;  // 1 - enabled, 0 - disabled
  bool fall_alt; // 1 - enabled, 0 - disabled
  bool rep_alt;  // 1 - enabled, 0 - disabled
} session_data_t;

static session_data_t session_data = {
    .bed_alt = false,
    .fall_alt = false,
    .rep_alt = false,
};

static session_data_t sd_load;

// Clear session data
void clear_alerts_sessiondata() {
  session_data.bed_alt = false;
  session_data.fall_alt = false;
  session_data.rep_alt = false;
}

//  Check if session data is different from loaded data
static bool is_session_data_dirty() {
  session_data_t *sd = &session_data;
  if (sd->bed_alt != sd_load.bed_alt) {
    return true;
  }
  if (sd->fall_alt != sd_load.fall_alt) {
    return true;
  }
  if (sd->rep_alt != sd_load.rep_alt) {
    return true;
  }
  return false;
}

// Update save button state based on session data changes
static void btn_save_update_status() {
  session_data_t *sd = &session_data;

  if (settings_mode == SETTINGS_MODE_SAVE && is_session_data_dirty()) {
    btn_set_state(btn_save, BTN_STATE_ACTIVE);
  } else {
    btn_set_state(btn_save, BTN_STATE_DISABLED);
  }
  if(sd->bed_alt == 0 && sd->fall_alt == 0 && sd->rep_alt == 0) {
    btn_set_state(btn_save, BTN_STATE_DISABLED);
  }
}

// Send JSON-RPC request to save alert settings
static void jsonrpc_request_send(session_data_t *data) {

  JsonDocument doc;

  JsonObject params = doc["params"].to<JsonObject>();
  uint8_t fts_state =
      (data->bed_alt ? 1 : 0) | ((data->fall_alt ? 1 : 0) << 1) | ((data->rep_alt ? 1 : 0) << 2);
  params["fts_state"] = fts_state;

  serializeJsonRpcRequest(0, CMD_SAVE_ALERTS, params);
}

/*******************************************************************************
 *  UI COMPONENT CREATION HELPERS
 ******************************************************************************/

static void set_shadow_style(lv_obj_t *obj, int offset_x, int offset_y, int width, int spread,
                             int opacity, uint32_t color) {
  lv_obj_set_style_shadow_color(obj, lv_color_hex(color), LV_PART_MAIN);
  lv_obj_set_style_shadow_width(obj, width, LV_PART_MAIN);
  lv_obj_set_style_shadow_spread(obj, spread, LV_PART_MAIN);
  lv_obj_set_style_shadow_opa(obj, opacity, LV_PART_MAIN);
  lv_obj_set_style_shadow_offset_x(obj, offset_x, LV_PART_MAIN);
  lv_obj_set_style_shadow_offset_y(obj, offset_y, LV_PART_MAIN);
}

static lv_obj_t *create_alert_selection(lv_obj_t *parent, const char *alert_name) {
  lv_obj_t *cont_alert = lv_obj_create(parent);
  lv_obj_clear_flag(cont_alert, LV_OBJ_FLAG_SCROLLABLE); // Disable scrolling
  lv_obj_set_size(cont_alert, 416, 57);
  lv_obj_set_style_radius(cont_alert, 10, LV_PART_MAIN);
  lv_obj_set_style_pad_all(cont_alert, 16, LV_PART_MAIN);
  lv_obj_set_style_border_width(cont_alert, 1, LV_PART_MAIN);
  lv_obj_set_style_border_color(cont_alert, lv_color_hex(0xD9D9D9), LV_PART_MAIN);

  set_shadow_style(cont_alert, 0, 2, 4, 0, 64, 0x000000);

  lv_obj_t *label = lv_label_create(cont_alert);
  lv_label_set_text(label, alert_name);
  lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 3);                             // Align to left middle
  lv_obj_set_style_text_font(label, &opensans_semibold_18, LV_PART_MAIN);   // Set item font
  lv_obj_set_style_text_color(label, lv_color_hex(VST_COLOR_INACTIVE_TEXT), LV_PART_MAIN); // Set item color

  lv_obj_t *switch_obj = lv_switch_create(cont_alert);
  lv_obj_set_width(switch_obj, 46);  // Set switch width
  lv_obj_set_height(switch_obj, 21); // Set switch height
  lv_obj_align(switch_obj, LV_ALIGN_RIGHT_MID, 0, 0);

  lv_obj_set_style_outline_color(switch_obj, lv_color_hex(0xD1D1D1), LV_PART_MAIN);
  lv_obj_set_style_outline_opa(switch_obj, 255, LV_PART_MAIN);
  lv_obj_set_style_outline_width(switch_obj, 2, LV_PART_MAIN);
  lv_obj_set_style_outline_pad(switch_obj, 2, LV_PART_MAIN);

  lv_obj_set_style_pad_all(switch_obj, 0, LV_PART_INDICATOR); // No padding for indicator
  lv_obj_set_style_bg_color(switch_obj, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR);
  lv_obj_set_style_bg_opa(switch_obj, 255, LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(switch_obj, lv_color_hex(VST_COLOR_MINT_GREEN),
                            LV_PART_INDICATOR | LV_STATE_CHECKED);
  lv_obj_set_style_bg_opa(switch_obj, 255, LV_PART_INDICATOR | LV_STATE_CHECKED);

  lv_obj_set_style_pad_all(switch_obj, 0, LV_PART_KNOB); // No padding for knob
  lv_obj_set_style_bg_color(switch_obj, lv_color_hex(0x707070), LV_PART_KNOB);
  lv_obj_set_style_bg_opa(switch_obj, 255, LV_PART_KNOB);
  lv_obj_set_style_bg_color(switch_obj, lv_color_hex(0x218448), LV_PART_KNOB | LV_STATE_CHECKED);
  lv_obj_set_style_bg_opa(switch_obj, 255, LV_PART_KNOB | LV_STATE_CHECKED);

  // Event handler for switch is added in screen_init
  return cont_alert;
}

static lv_obj_t *create_action_button(lv_obj_t *parent, const char *text, bool is_next_btn) {
  // Create button with text using custom btn_create function
  lv_obj_t *btn = btn_create(parent, text);

  // Set button dimensions and basic styling
  lv_obj_set_size(btn, 216, 80);                 // Button size
  lv_obj_set_style_radius(btn, 5, LV_PART_MAIN); // Rounded corners
  lv_obj_set_style_border_color(btn, lv_color_hex(VST_COLOR_DARKBLUE), LV_PART_MAIN);
  lv_obj_set_style_border_width(btn, 3, LV_PART_MAIN); // 3px border

  // Set padding for text positioning
  lv_obj_set_style_pad_top(btn, 10, LV_PART_MAIN);
  lv_obj_set_style_pad_bottom(btn, 10, LV_PART_MAIN);
  lv_obj_set_style_pad_left(btn, 25, LV_PART_MAIN);
  lv_obj_set_style_pad_right(btn, 25, LV_PART_MAIN);

  set_shadow_style(btn, 0, 2, 4, 0, 64, 0x000000);

  // Text styling
  lv_obj_set_style_text_align(btn, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_set_style_text_font(btn, &opensans_bold_24, LV_PART_MAIN | LV_STATE_DEFAULT);

  // Apply different color schemes based on button type
  if (is_next_btn) {
    // Next button: Dark blue background with white text (primary action)
    lv_obj_set_style_bg_color(btn, lv_color_hex(VST_COLOR_DARKBLUE), LV_PART_MAIN);
    lv_obj_set_style_text_color(btn, lv_color_white(), LV_PART_MAIN);
  } else {
    // Back button: White background with dark blue text (secondary action)
    lv_obj_set_style_bg_color(btn, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_color(btn, lv_color_hex(VST_COLOR_DARKBLUE), LV_PART_MAIN);
  }

  return btn;
}

/*******************************************************************************
 * EVENT CALLBACKS 
 ******************************************************************************/

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

static void left_arrow_click_cb(lv_event_t *e) {
  settings_mode = screen_settings_get_mode();
  if (settings_mode == SETTINGS_MODE_SAVE) {
    session_data_t *sd = &session_data;
    if (sd->bed_alt == 0 && sd->fall_alt == 0 && sd->rep_alt == 0) {
      lv_screen_load(screen_settings_home_get());
      return;
    }
    if (is_session_data_dirty()) {
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

static void btn_back_click_cb(lv_event_t *e) {
  // Handle back button click
  NKD_VARIANT firmware_variant = get_firmware_variant();
  if (firmware_variant == NKD_GO) {
    ASSIGN_MODE assign_mode = room_assign_get_mode();
    if (assign_mode == ASSIGN_MODE_NONE) {
      // If assign mode is none, means room selection not happened and user came to room view page
      // from confirm room page. Show confirm room screen when left arrow is clicked to go back
      lv_screen_load(screen_settings_confirm_room_get());
    } else if (assign_mode == ROOM_SELECTION_MODE) {
      // If assign mode is room selection, means user came to this page from room selection page.
      // Show room number selection screen when left arrow is clicked to go back
      room_assign_set_mode(ROOM_SELECTION_MODE);
      lv_screen_load(screen_settings_select_room_get());
    }
  } else if (firmware_variant == NKD_INTERACTIVE) {
    lv_screen_load(scr_home_inactive_get());
  }
}

static void info_icon_click_cb(lv_event_t *e) {
  // show information page
  show_system_info_screen((char *)"settings_alerts");
}

// Event handler for alert switches
static void switch_event_cb(lv_event_t *e) {
  lv_obj_t *switch_obj = lv_event_get_target_obj(e);
  lv_obj_t *cont_alert = lv_obj_get_parent(switch_obj);
  lv_obj_t *lbl_alert_name = lv_obj_get_child(cont_alert, 0); // Get the first child (label)

  if (lv_obj_has_state(switch_obj, LV_STATE_CHECKED)) {
    // update session data
    if (cont_alert == cont_bed_alerts) {
      session_data.bed_alt = true;
    } else if (cont_alert == cont_fall_alerts) {
      session_data.fall_alt = true;
    } else if (cont_alert == cont_reposition_alerts) {
      session_data.rep_alt = true;
    }
    // Switch is ON - set text color to active/enabled color
    lv_obj_set_style_bg_color(switch_obj, lv_color_hex(VST_COLOR_MINT_GREEN), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(lbl_alert_name, lv_color_hex(0x31344E),
                                LV_PART_MAIN); // Dark text when enabled
  } else {
    // update session data
    if (cont_alert == cont_bed_alerts) {
      session_data.bed_alt = false;
    } else if (cont_alert == cont_fall_alerts) {
      session_data.fall_alt = false;
    } else if (cont_alert == cont_reposition_alerts) {
      session_data.rep_alt = false;
    }
    // Switch is OFF - set text color to disabled/muted color
    lv_obj_set_style_text_color(lbl_alert_name, lv_color_hex(VST_COLOR_INACTIVE_TEXT),
                                LV_PART_MAIN); // Gray text when disabled
  }

  settings_mode = screen_settings_get_mode();
  if (settings_mode == SETTINGS_MODE_ACTIVATION) {
    // check if any alert is enabled and not hidden, if yes enable next button else disable next
    // button
    if ((session_data.bed_alt && !(lv_obj_has_flag(cont_bed_alerts, LV_OBJ_FLAG_HIDDEN))) ||
        (session_data.fall_alt && !(lv_obj_has_flag(cont_fall_alerts, LV_OBJ_FLAG_HIDDEN))) ||
        (session_data.rep_alt && !(lv_obj_has_flag(cont_reposition_alerts, LV_OBJ_FLAG_HIDDEN)))) {
      lv_obj_add_flag(btn_next, LV_OBJ_FLAG_CLICKABLE);
      lv_obj_clear_state(btn_next, LV_STATE_DISABLED);
    } else {
      lv_obj_clear_flag(btn_next, LV_OBJ_FLAG_CLICKABLE);
      lv_obj_add_state(btn_next, LV_STATE_DISABLED);
      return;
    }
  } else if (settings_mode == SETTINGS_MODE_SAVE) {
    btn_save_update_status();
  }
}

// Next button click handler
static void btn_next_click_cb(lv_event_t *e) {
  screen_settings_set_alerts_state(session_data.bed_alt, session_data.fall_alt,
                                   session_data.rep_alt);
  // Handle next button click
  sys_state_t *ss_t = sys_state_get();
  lv_obj_t *screen;
  if (ss_t->fts_avail.sch_mon) {
    // Load exit alert schedule screen if scheduled monitoring is enabled
    screen = screen_settings_exit_alert_sch_get();
  } else {
    // Load BMS settings screen
    screen = screen_settings_bms_get();
  }

  lv_screen_load(screen);
}

// Save button click handler
static void btn_save_click_cb(lv_event_t *e) {
  jsonrpc_request_send(&session_data);
  btn_set_state(btn_save, BTN_STATE_SUBMITTING);
}

// Request timeout handler
static void request_timeout_cb(lv_event_t *e) {
  btn_set_state(btn_save, BTN_STATE_ACTIVE);
  toast_show(screen, no_response_str);
}

// Set state fail handler
static void set_state_fail_cb(lv_event_t *e) {
  btn_set_state(btn_save, BTN_STATE_ACTIVE);
  toast_show(screen, invalid_response_str);
}

// Set state ok handler
static void set_state_ok_cb(lv_event_t *e) {
  btn_set_state(btn_save, BTN_STATE_SUBMITTED);
  // update load time session data with latest session data after save is successful
  const sys_state_t *ss = sys_state_get();
  sd_load.bed_alt = ss->fts_state.bed_alt;
  sd_load.fall_alt = ss->fts_state.fall_alt;
  sd_load.rep_alt = ss->fts_state.rep_alt;
}

/*******************************************************************************
 * SCREEN LOAD HANDLER
 ******************************************************************************/

static void screen_load_cb(lv_event_t *e) {
  settings_mode = screen_settings_get_mode();
  settings_show_mode(screen, btn_save, cont_btns_nav, settings_mode);

  if (settings_mode == SETTINGS_MODE_ACTIVATION) {
    lv_obj_add_flag(title_cont_left_arrow_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(title_cont_info_icon_obj, LV_OBJ_FLAG_HIDDEN);
  } else if (settings_mode == SETTINGS_MODE_SAVE) {
    lv_obj_clear_flag(title_cont_left_arrow_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(title_cont_info_icon_obj, LV_OBJ_FLAG_HIDDEN);
  }

  const sys_state_t *ss_t = sys_state_get();

  if (ss_t->fts_avail.fall_alt) {
    lv_obj_clear_flag(cont_fall_alerts, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(cont_fall_alerts, LV_OBJ_FLAG_HIDDEN);
  }

  if (ss_t->fts_avail.rep_alt) {
    lv_obj_clear_flag(cont_reposition_alerts, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(cont_reposition_alerts, LV_OBJ_FLAG_HIDDEN);
  }

  lv_obj_t *switch_bed = lv_obj_get_child(cont_bed_alerts, 1);
  lv_obj_t *switch_fall = lv_obj_get_child(cont_fall_alerts, 1);
  lv_obj_t *switch_rep = lv_obj_get_child(cont_reposition_alerts, 1);

  if (settings_mode == SETTINGS_MODE_ACTIVATION) {

    // turn off or on the alert switches based on session data
    if (session_data.bed_alt == true) {
      lv_obj_add_state(switch_bed, LV_STATE_CHECKED);
      lv_obj_set_style_bg_color(switch_bed, lv_color_hex(VST_COLOR_MINT_GREEN),
                                LV_PART_MAIN | LV_STATE_CHECKED);
      lv_obj_set_style_text_color(lv_obj_get_child(cont_bed_alerts, 0), lv_color_hex(0x31344E),
                                  LV_PART_MAIN); // Dark text when enabled
    } else {
      lv_obj_clear_state(switch_bed, LV_STATE_CHECKED);
      lv_obj_set_style_text_color(lv_obj_get_child(cont_bed_alerts, 0), lv_color_hex(VST_COLOR_INACTIVE_TEXT),
                                  LV_PART_MAIN); // Gray text when disabled
    }
    if (session_data.fall_alt == true) {
      lv_obj_add_state(switch_fall, LV_STATE_CHECKED);
      lv_obj_set_style_bg_color(switch_fall, lv_color_hex(VST_COLOR_MINT_GREEN),
                                LV_PART_MAIN | LV_STATE_CHECKED);
      lv_obj_set_style_text_color(lv_obj_get_child(cont_fall_alerts, 0), lv_color_hex(0x31344E),
                                  LV_PART_MAIN); // Dark text when enabled
    } else {
      lv_obj_clear_state(switch_fall, LV_STATE_CHECKED);
      lv_obj_set_style_text_color(lv_obj_get_child(cont_fall_alerts, 0), lv_color_hex(VST_COLOR_INACTIVE_TEXT),
                                  LV_PART_MAIN); // Gray text when disabled
    }
    if (session_data.rep_alt == true) {
      lv_obj_add_state(switch_rep, LV_STATE_CHECKED);
      lv_obj_set_style_bg_color(switch_rep, lv_color_hex(VST_COLOR_MINT_GREEN),
                                LV_PART_MAIN | LV_STATE_CHECKED);
      lv_obj_set_style_text_color(lv_obj_get_child(cont_reposition_alerts, 0),
                                  lv_color_hex(0x31344E),
                                  LV_PART_MAIN); // Dark text when enabled
    } else {
      lv_obj_clear_state(switch_rep, LV_STATE_CHECKED);
      lv_obj_set_style_text_color(lv_obj_get_child(cont_reposition_alerts, 0),
                                  lv_color_hex(VST_COLOR_INACTIVE_TEXT),
                                  LV_PART_MAIN); // Gray text when disabled
    }
  }

  if (settings_mode == SETTINGS_MODE_SAVE) {
    if (ss_t->fts_avail.bed_alt == true) {
      sd_load.bed_alt = ss_t->fts_state.bed_alt;
    } else {
      sd_load.bed_alt = false;
    }
    if (ss_t->fts_avail.fall_alt == true) {
      sd_load.fall_alt = ss_t->fts_state.fall_alt;
    } else {
      sd_load.fall_alt = false;
    }
    if (ss_t->fts_avail.rep_alt == true) {
      sd_load.rep_alt = ss_t->fts_state.rep_alt;
    } else {
      sd_load.rep_alt = false;
    }
    if (ss_t->fts_avail.bed_alt && ss_t->fts_state.bed_alt) {
      lv_obj_add_state(switch_bed, LV_STATE_CHECKED);
      lv_obj_set_style_bg_color(switch_bed, lv_color_hex(VST_COLOR_MINT_GREEN),
                                LV_PART_MAIN | LV_STATE_CHECKED);
      lv_obj_set_style_text_color(lv_obj_get_child(cont_bed_alerts, 0), lv_color_hex(0x31344E),
                                  LV_PART_MAIN); // Dark text when enabled
      session_data.bed_alt = true;
    } else {
      lv_obj_clear_state(switch_bed, LV_STATE_CHECKED);
      lv_obj_set_style_text_color(lv_obj_get_child(cont_bed_alerts, 0), lv_color_hex(VST_COLOR_INACTIVE_TEXT),
                                  LV_PART_MAIN); // Gray text when disabled
      session_data.bed_alt = false;
    }
    if (ss_t->fts_avail.fall_alt && ss_t->fts_state.fall_alt) {
      lv_obj_add_state(switch_fall, LV_STATE_CHECKED);
      lv_obj_set_style_bg_color(switch_fall, lv_color_hex(VST_COLOR_MINT_GREEN),
                                LV_PART_MAIN | LV_STATE_CHECKED);
      lv_obj_set_style_text_color(lv_obj_get_child(cont_fall_alerts, 0), lv_color_hex(0x31344E),
                                  LV_PART_MAIN); // Dark text when enabled
      session_data.fall_alt = true;
    } else {
      lv_obj_clear_state(switch_fall, LV_STATE_CHECKED);
      lv_obj_set_style_text_color(lv_obj_get_child(cont_fall_alerts, 0), lv_color_hex(VST_COLOR_INACTIVE_TEXT),
                                  LV_PART_MAIN); // Gray text when disabled
      session_data.fall_alt = false;
    }
    if (ss_t->fts_avail.rep_alt && ss_t->fts_state.rep_alt) {
      lv_obj_add_state(switch_rep, LV_STATE_CHECKED);
      lv_obj_set_style_bg_color(switch_rep, lv_color_hex(VST_COLOR_MINT_GREEN),
                                LV_PART_MAIN | LV_STATE_CHECKED);
      lv_obj_set_style_text_color(lv_obj_get_child(cont_reposition_alerts, 0),
                                  lv_color_hex(0x31344E),
                                  LV_PART_MAIN); // Dark text when enabled
      session_data.rep_alt = true;
    } else {
      lv_obj_clear_state(switch_rep, LV_STATE_CHECKED);
      lv_obj_set_style_text_color(lv_obj_get_child(cont_reposition_alerts, 0),
                                  lv_color_hex(VST_COLOR_INACTIVE_TEXT),
                                  LV_PART_MAIN); // Gray text when disabled
      session_data.rep_alt = false;
    }
    btn_save_update_status();
  }
  settings_mode = screen_settings_get_mode();
  if (settings_mode == SETTINGS_MODE_ACTIVATION) {
    // disable next button if no alerts are enabled and also if enabled that respective alert is
    // not hidden
    if ((session_data.bed_alt && !(lv_obj_has_flag(cont_bed_alerts, LV_OBJ_FLAG_HIDDEN))) ||
        (session_data.fall_alt && !(lv_obj_has_flag(cont_fall_alerts, LV_OBJ_FLAG_HIDDEN))) ||
        (session_data.rep_alt && !(lv_obj_has_flag(cont_reposition_alerts, LV_OBJ_FLAG_HIDDEN)))) {
      lv_obj_add_flag(btn_next, LV_OBJ_FLAG_CLICKABLE);
      lv_obj_clear_state(btn_next, LV_STATE_DISABLED);
    } else {
      lv_obj_clear_flag(btn_next, LV_OBJ_FLAG_CLICKABLE);
      lv_obj_add_state(btn_next, LV_STATE_DISABLED);
    }
  }
}

/*******************************************************************************
 * SCREEN INITIALIZATION
 ******************************************************************************/

static void screen_init() {
  screen = tmpl_settings_create_1(NULL, "Settings", true);

  lv_obj_add_event_cb(screen, screen_load_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
  lv_obj_add_event_cb(screen, request_timeout_cb, (lv_event_code_t)MY_EVENT_REQUEST_TIMEOUT, NULL);
  lv_obj_add_event_cb(screen, set_state_ok_cb, (lv_event_code_t)MY_EVENT_SET_STATE_OK, NULL);
  lv_obj_add_event_cb(screen, set_state_fail_cb, (lv_event_code_t)MY_EVENT_SET_STATE_FAIL, NULL);

  title_cont_left_arrow_obj = get_cont_left_arrow_obj();
  lv_obj_add_event_cb(title_cont_left_arrow_obj, left_arrow_click_cb, LV_EVENT_CLICKED, NULL);

  title_cont_info_icon_obj = get_cont_info_icon_obj();
  lv_obj_add_event_cb(title_cont_info_icon_obj, info_icon_click_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *alerts_container = lv_obj_create(screen);
  lv_obj_clear_flag(alerts_container, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_size(alerts_container, 448, 271);
  lv_obj_set_style_pad_gap(alerts_container, 10, LV_PART_MAIN);
  lv_obj_set_style_bg_color(alerts_container, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_border_color(alerts_container, lv_color_hex(VST_COLOR_CONTAINER_BORDER),
                                LV_PART_MAIN);
  lv_obj_set_style_border_width(alerts_container, 1, LV_PART_MAIN);
  lv_obj_set_style_radius(alerts_container, 7, LV_PART_MAIN);

  set_shadow_style(alerts_container, 0, 3, 6, 0, 64, 0x000000);

  lv_obj_t *alert_item = lv_obj_create(alerts_container);
  lv_obj_clear_flag(alert_item, LV_OBJ_FLAG_SCROLLABLE); // Disable scrolling
  lv_obj_set_size(alert_item, 416, 227);
  lv_obj_align(alert_item, LV_ALIGN_CENTER, 0, 2); // Align to center of parent
  lv_obj_set_style_pad_gap(alert_item, 16, LV_PART_MAIN);
  lv_obj_set_style_pad_all(alert_item, 0, LV_PART_MAIN);
  lv_obj_set_style_border_width(alert_item, 0, LV_PART_MAIN);
  lv_obj_set_flex_flow(alert_item, LV_FLEX_FLOW_ROW_WRAP);

  lv_obj_t *label = lv_label_create(alert_item);
  lv_label_set_text(label, "Alerts");
  lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 2);
  lv_obj_set_style_text_font(label, &opensans_semibold_18, LV_PART_MAIN); // Set title font
  lv_obj_set_style_text_color(label, lv_color_hex(VST_COLOR_PRIMARY_TEXT), LV_PART_MAIN);

  lv_obj_t *alert_desc = lv_obj_create(alert_item);
  lv_obj_clear_flag(alert_desc, LV_OBJ_FLAG_SCROLLABLE); // Disable scrolling
  lv_obj_set_size(alert_desc, 416, 194);                 // increased container size to show shadow
  lv_obj_set_style_pad_all(alert_desc, 0, LV_PART_MAIN);
  lv_obj_set_style_border_width(alert_desc, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_gap(alert_desc, 10, LV_PART_MAIN);
  lv_obj_set_flex_flow(alert_desc, LV_FLEX_FLOW_ROW_WRAP);

  cont_bed_alerts = create_alert_selection(alert_desc, "Bed/Chair Exit Alerts");
  cont_fall_alerts = create_alert_selection(alert_desc, "Fall Alerts");
  cont_reposition_alerts = create_alert_selection(alert_desc, "Reposition Alerts");

  // Add event callbacks to the switches created inside the alert selections
  lv_obj_add_event_cb(lv_obj_get_child(cont_bed_alerts, 1), switch_event_cb, LV_EVENT_VALUE_CHANGED,
                      NULL);
  lv_obj_add_event_cb(lv_obj_get_child(cont_fall_alerts, 1), switch_event_cb,
                      LV_EVENT_VALUE_CHANGED, NULL);
  lv_obj_add_event_cb(lv_obj_get_child(cont_reposition_alerts, 1), switch_event_cb,
                      LV_EVENT_VALUE_CHANGED, NULL);

  btn_save = btn_save_create_1(screen);

  lv_obj_add_event_cb(btn_save, btn_save_click_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_flag(btn_save, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_set_size(btn_save, 448, 80);
  lv_obj_align(btn_save, LV_ALIGN_BOTTOM_MID, 0, -16);

  cont_btns_nav = lv_obj_create(screen);
  lv_obj_set_size(cont_btns_nav, 448, 80);                       // Action bar size
  lv_obj_add_flag(cont_btns_nav, LV_OBJ_FLAG_IGNORE_LAYOUT);     // Manual positioning
  lv_obj_set_pos(cont_btns_nav, 16, 384);                        // Position at bottom
  lv_obj_set_style_border_width(cont_btns_nav, 0, LV_PART_MAIN); // No border
  lv_obj_set_style_bg_opa(cont_btns_nav, LV_OPA_TRANSP,
                          LV_PART_MAIN);                    // Transparent background
  lv_obj_set_style_pad_all(cont_btns_nav, 0, LV_PART_MAIN); // No padding
  lv_obj_set_flex_flow(cont_btns_nav, LV_FLEX_FLOW_ROW);    // Horizontal layout
  lv_obj_set_flex_align(cont_btns_nav, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);              // Distribute buttons
  lv_obj_clear_flag(cont_btns_nav, LV_OBJ_FLAG_SCROLLABLE); // Disable scrolling

  lv_obj_t *btn_back = create_action_button(cont_btns_nav, "Back", false);
  btn_next = create_action_button(cont_btns_nav, "Next", true);

  lv_obj_add_event_cb(btn_back, btn_back_click_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(btn_next, btn_next_click_cb, LV_EVENT_CLICKED, NULL);
}

/*******************************************************************************
 * PUBLIC ACCESSOR FUNCTION
 ******************************************************************************/

lv_obj_t *screen_settings_alerts_get() {
  if (screen == NULL) {
    screen_init();
  }
  return screen;
}