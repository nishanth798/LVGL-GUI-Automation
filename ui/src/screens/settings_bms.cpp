#include "json_handles.h"
#include "jsonrpc2.h"
#include "screens.h"
#include "sys_state.h"
#include "ui.h"
#include <cstdio>

bool flg_SaveClick = false;

// todo: add elevation to controls container across all screens

//
// Section: Constants
//

static const char *labels[] = {
    "Low",
    "High",
    "Ultra-High",
    NULL,
};

static const lv_img_dsc_t *icons[] = {
    &icon_bms_low,
    &icon_bms_high,
    &icon_bms_uhigh,
    NULL,
};

static const char *descs[] = {
    "• Alerts when occupant is standing.\n"
    "• Commonly used for ambulatory occupants.",
    "• Alerts before occupant exits bed.\n"
    "• Commonly used for high risk occupants.",
    "• Alerts with any motion near the bed edge.\n"
    "• Commonly used for impulsive or amputee occupants.",
    NULL,
};

//
// Section: Variables
//

// root
static lv_obj_t *screen;

static lv_obj_t *bms_selector, *label_desc;
static lv_obj_t *btn_save, *cont_btns_nav;
static lv_obj_t *title_cont_left_arrow_obj;
static lv_obj_t *title_cont_info_icon_obj;

static SETTINGS_MODE settings_mode;

// session data
typedef struct {
  BMS bms;
} session_data_t;

static session_data_t session_data = {
    .bms = BMS_UNSET,
};
static session_data_t sd_load; // holds data that is present during load time.

//
// Section: Control functions
//

void clear_bms_sessiondata() { session_data.bms = BMS_UNSET; }

static bool is_session_data_dirty() {
  session_data_t *sd = &session_data;
  // const sys_state_t *ss = sys_state_get();
  if (sd->bms != sd_load.bms) {
    return true;
  }
  return false;
}

static void btn_save_update_status() {
  if (settings_mode == SETTINGS_MODE_SAVE && is_session_data_dirty()) {
    btn_set_state(btn_save, BTN_STATE_ACTIVE);
  } else {
    btn_set_state(btn_save, BTN_STATE_DISABLED);
  }
}

static void jsonrpc_request_send(session_data_t *data) {
  //  const size_t capacity = 200; // Adjust this based on your needs
  JsonDocument doc;

  JsonObject params = doc["params"].to<JsonObject>();
  params["bms"] = data->bms;

  serializeJsonRpcRequest(0, CMD_SAVE_BMS, params);
}

void set_bms_level(lv_obj_t *obj, BMS bms) {
  int8_t btn_id = bms - 1;
  if (bms == BMS_UNSET) {
    btn_id = bms;
  }
  radio_selector_set_selected_btn_1(obj, btn_id);
}

BMS get_bms_level(lv_obj_t *obj) { return (BMS)(radio_selector_get_selected_btn_1(obj) + 1); }

void set_desc(BMS bms) {
  if (bms == BMS_UNSET) {
    lv_label_set_text_static(label_desc, "");
    return;
  }
  lv_label_set_text_static(label_desc, descs[bms - 1]);
}

//
// Section: Event Handlers
//

static void btn_radio_click_cb(lv_event_t *e) {
  BMS bms = get_bms_level(bms_selector);
  set_desc(bms);
  session_data.bms = bms;
  btn_save_update_status();
}

static void btn_save_click_cb(lv_event_t *e) {
  //  flg_SaveClick = true;
  jsonrpc_request_send(&session_data);
  btn_set_state(btn_save, BTN_STATE_SUBMITTING);
  radio_selector_set_clickable(bms_selector, false);
  tmpl_settings_label_set_clickable(screen, false);
}

static void request_timeout_cb(lv_event_t *e) {
  // if(flg_SaveClick == true)
  {
    btn_set_state(btn_save, BTN_STATE_ACTIVE);
    radio_selector_set_clickable(bms_selector, true);
    tmpl_settings_label_set_clickable(screen, true);

    toast_show(screen, no_response_str);
  }
  //  flg_SaveClick = false;
}

static void set_state_fail_cb(lv_event_t *e) {
  //  if(flg_SaveClick == true)
  {
    btn_set_state(btn_save, BTN_STATE_ACTIVE);
    radio_selector_set_clickable(bms_selector, true);
    tmpl_settings_label_set_clickable(screen, true);

    toast_show(screen, invalid_response_str);
  }
  //  flg_SaveClick = false;
}

static void set_state_ok_cb(lv_event_t *e) {
  // if(flg_SaveClick == true)
  {
    btn_set_state(btn_save, BTN_STATE_SUBMITTED);
    radio_selector_set_clickable(bms_selector, true);
    tmpl_settings_label_set_clickable(screen, true);

    // update load time session data with latest session data after save is successful
    const sys_state_t *ss = sys_state_get();
    sd_load.bms = ss->bms;
  }
  // flg_SaveClick = false;
}

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
    radio_selector_set_clickable(bms_selector, false);
    tmpl_settings_label_set_clickable(screen, false);
  }
}

static void left_arrow_click_cb(lv_event_t *e) {
  // get firmware variant
  NKD_VARIANT firmware_variant = get_firmware_variant();
  settings_mode = screen_settings_get_mode();
  if (settings_mode == SETTINGS_MODE_SAVE) {
    //  const sys_state_t *ss = sys_state_get();
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
  } else if (settings_mode == SETTINGS_MODE_ACTIVATION) {
    // if variant is
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
  return;
}

static void btn_prev_click_cb(lv_event_t *e) {
  // get firmware variant
  // NKD_VARIANT firmware_variant = get_firmware_variant();
  // if (firmware_variant == NKD_GO) {
  //   ASSIGN_MODE assign_mode = room_assign_get_mode();
  //   if (assign_mode == ASSIGN_MODE_NONE) {
  //     // If assign mode is none, means room selection not happened and user came to room view
  //     page
  //     // from confirm room page. Show confirm room screen when left arrow is clicked to go back
  //     lv_screen_load(screen_settings_confirm_room_get());
  //   } else if (assign_mode == ROOM_SELECTION_MODE) {
  //     // If assign mode is room selection, means user came to this page from room selection page.
  //     // Show room number selection screen when left arrow is clicked to go back
  //     room_assign_set_mode(ROOM_SELECTION_MODE);
  //     lv_screen_load(screen_settings_select_room_get());
  //   }
  // } else if (firmware_variant == NKD_INTERACTIVE) {
  //   lv_screen_load(scr_home_inactive_get());
  // }

  sys_state_t *ss_t = sys_state_get();
  lv_obj_t *screen;
  if (ss_t->fts_avail.sch_mon) {
    // Load exit alert schedule screen if scheduled monitoring is enabled
    screen = screen_settings_exit_alert_sch_get();
  } else {
    // Load alert settings screen
    screen = screen_settings_alerts_get();
  }
  lv_screen_load(screen);
}

static void btn_next_click_cb(lv_event_t *e) {
  screen_settings_audio_set_bms(session_data.bms);
  screen_settings_set_mode(SETTINGS_MODE_ACTIVATION);
  sys_state_t *ss = sys_state_get();
  if (ss->fts_avail.bed_wid) {
    lv_obj_t *screen = screen_settings_get_bed_width();
    lv_screen_load(screen);
  } else if (ss->fts_avail.bed_pos) {
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

static void screen_load_cb(lv_event_t *e) {

  settings_mode = screen_settings_get_mode();
  settings_show_mode(screen, btn_save, cont_btns_nav, settings_mode);

  radio_selector_set_clickable(bms_selector, true);
  tmpl_settings_label_set_clickable(screen, true);

  session_data_t *sd = &session_data;
  if (settings_mode == SETTINGS_MODE_ACTIVATION) {
    // show the header icon symbol and hide the left arrow
    lv_obj_add_flag(title_cont_left_arrow_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(title_cont_info_icon_obj, LV_OBJ_FLAG_HIDDEN);
    // set the bms level to default if unset
    if (sd->bms == BMS_UNSET) {
      sd->bms = BMS_DEFAULT;
    }
  } else if (settings_mode == SETTINGS_MODE_SAVE) {
    // show the header left arrow and hide the icon symbol
    lv_obj_clear_flag(title_cont_left_arrow_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(title_cont_info_icon_obj, LV_OBJ_FLAG_HIDDEN);

    const sys_state_t *ss = sys_state_get();
    if (ss->bms == BMS_UNSET) {
      sd->bms = BMS_DEFAULT;
      sd_load.bms = BMS_DEFAULT;
    } else {
      sd->bms = ss->bms;
      sd_load.bms = ss->bms;
    }

    btn_save_update_status();
  }
  set_bms_level(bms_selector, sd->bms);
  set_desc(sd->bms);

  if (!is_alert_toast_hidden()) {
    // LV_LOG_USER("bms screen is parent for alert toast");
    set_alert_toast_parent(screen);
  }
}

// show information page
static void info_icon_click_cb(lv_event_t *e) { show_system_info_screen((char *)"settings_bms"); }

//
// Section: UI
//

static void screen_init() {
  screen = tmpl_settings_create_1(NULL, "Settings", true);
  lv_obj_set_style_bg_color(screen, lv_color_hex(VST_COLOR_SCREEN_BG_GREY), LV_PART_MAIN);
  lv_obj_t *lef_arrow_obj = get_img_left_arrow_obj();
  lv_obj_add_flag(lef_arrow_obj, LV_OBJ_FLAG_EVENT_BUBBLE); /// Flags

  title_cont_left_arrow_obj = get_cont_left_arrow_obj();
  lv_obj_add_event_cb(title_cont_left_arrow_obj, left_arrow_click_cb, LV_EVENT_CLICKED, NULL);

  title_cont_info_icon_obj = get_cont_info_icon_obj();
  lv_obj_add_event_cb(title_cont_info_icon_obj, info_icon_click_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_add_event_cb(screen, screen_load_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
  lv_obj_add_event_cb(screen, request_timeout_cb, (lv_event_code_t)MY_EVENT_REQUEST_TIMEOUT, NULL);
  lv_obj_add_event_cb(screen, set_state_ok_cb, (lv_event_code_t)MY_EVENT_SET_STATE_OK, NULL);
  lv_obj_add_event_cb(screen, set_state_fail_cb, (lv_event_code_t)MY_EVENT_SET_STATE_FAIL, NULL);

  // add content container
  lv_obj_t *cont_content = lv_obj_create(screen);
  lv_obj_set_size(cont_content, lv_pct(100), 400); /// 1
  lv_obj_set_flex_flow(cont_content, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(cont_content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);
  lv_obj_clear_flag(cont_content, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_border_width(cont_content, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_top(cont_content, 0, LV_PART_MAIN);
  lv_obj_set_style_bg_color(cont_content, lv_color_hex(VST_COLOR_SCREEN_BG_GREY), LV_PART_MAIN);
  // lv_obj_set_style_bg_opa(cont_content,LV_OPA_0, LV_PART_MAIN | LV_PART_ITEMS |
  // LV_STATE_DEFAULT);

  // add spacing (top and bottom) container for controls
  // lv_obj_t *cont_controls_spacing = lv_obj_create(cont_content);
  // lv_obj_set_size(cont_controls_spacing, lv_pct(100), 220);
  // lv_obj_set_style_border_width(cont_controls_spacing, 0, LV_PART_MAIN);
  // lv_obj_set_style_pad_all(cont_controls_spacing, 0, LV_PART_MAIN);
  // lv_obj_set_style_bg_color(cont_controls_spacing, lv_color_hex(VST_COLOR_SCREEN_BG_GREY),
  //                           LV_PART_MAIN);

  // add controls container
  lv_obj_t *cont_controls = lv_obj_create(cont_content);
  lv_obj_set_size(cont_controls, lv_pct(100), 220);
  lv_obj_clear_flag(cont_controls, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_border_width(cont_controls, 1, LV_PART_MAIN);
  lv_obj_set_style_border_color(cont_controls, lv_color_hex(0xD1D1D1), LV_PART_MAIN);

  lv_obj_set_style_shadow_color(cont_controls, lv_color_hex(0x000000),
                                LV_PART_MAIN);                      // shadow color
  lv_obj_set_style_shadow_width(cont_controls, 6, LV_PART_MAIN);    // shadow width
  lv_obj_set_style_shadow_spread(cont_controls, 0, LV_PART_MAIN);   // shadow spread
  lv_obj_set_style_shadow_opa(cont_controls, 64, LV_PART_MAIN);     // shadow opacity
  lv_obj_set_style_shadow_offset_x(cont_controls, 2, LV_PART_MAIN); //  shadow offset x
  lv_obj_set_style_shadow_offset_y(cont_controls, 2, LV_PART_MAIN); // shadow offset y

  // add sub title
  lv_obj_t *label_subtitle = lv_label_create(cont_controls);
  lv_obj_set_size(label_subtitle, LV_SIZE_CONTENT, LV_SIZE_CONTENT); /// 1
  lv_obj_add_flag(label_subtitle, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_set_pos(label_subtitle, 0, 10); // position from top
  lv_label_set_text_static(label_subtitle, "Bed Mode Sensitivity");
  lv_obj_set_style_text_font(label_subtitle, &opensans_semibold_18, LV_PART_MAIN);
  lv_obj_set_style_text_color(label_subtitle, lv_color_hex(VST_COLOR_PRIMARY_TEXT),
                              LV_PART_MAIN); // text color

  // add bms controller
  bms_selector = radio_selector_create_1(cont_controls, labels, icons);
  // lv_obj_set_align(bms_selector, LV_ALIGN_CENTER);
  lv_obj_set_style_pad_left(bms_selector, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_right(bms_selector, 0, LV_PART_MAIN);
  lv_obj_add_event_cb(bms_selector, btn_radio_click_cb, LV_EVENT_VALUE_CHANGED, NULL);

  // add desc label
  label_desc = lv_label_create(cont_controls);
  lv_obj_set_width(label_desc, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_height(label_desc, LV_SIZE_CONTENT); /// 1
  lv_obj_set_pos(label_desc, 12, 150);
  // lv_obj_set_align(label_desc, LV_ALIGN_BOTTOM_LEFT);
  lv_label_set_text_static(label_desc, "");
  lv_obj_set_style_text_font(label_desc, &opensans_italic_14, LV_PART_MAIN);
  lv_obj_set_style_text_color(label_desc, lv_color_hex(0x707070), LV_PART_MAIN);
  lv_obj_set_style_text_line_space(label_desc, 4, LV_PART_MAIN);
  lv_label_set_text_static(label_desc, descs[0]);

  // add btn save
  btn_save = btn_save_create_1(cont_content);

  lv_obj_add_event_cb(btn_save, btn_save_click_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_flag(btn_save, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_set_align(btn_save, LV_ALIGN_BOTTOM_MID);

  // add nav btns container
  cont_btns_nav = lv_obj_create(cont_content);
  lv_obj_set_size(cont_btns_nav, 448, 80);
  lv_obj_clear_flag(cont_btns_nav, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_border_width(cont_btns_nav, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_left(cont_btns_nav, 0, LV_PART_MAIN);  // padding left
  lv_obj_set_style_pad_right(cont_btns_nav, 0, LV_PART_MAIN); // padding right
  lv_obj_add_flag(cont_btns_nav, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(cont_btns_nav, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_set_align(cont_btns_nav, LV_ALIGN_BOTTOM_MID);
  lv_obj_set_style_bg_color(cont_btns_nav, lv_color_hex(VST_COLOR_SCREEN_BG_GREY),
                            LV_PART_MAIN); // background color

  // add previous and next buttons
  lv_obj_t *btn_prev = btn_secondary_create_1(cont_btns_nav, "Back");
  lv_obj_set_align(btn_prev, LV_ALIGN_LEFT_MID);

  lv_obj_add_event_cb(btn_prev, btn_prev_click_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *btn_next = btn_primary_create_1(cont_btns_nav, "Next", VST_COLOR_DARKBLUE);
  lv_obj_set_align(btn_next, LV_ALIGN_RIGHT_MID);
  lv_obj_add_event_cb(btn_next, btn_next_click_cb, LV_EVENT_CLICKED, NULL);
}

//
// Section: API
//

lv_obj_t *screen_settings_bms_get() {
  if (screen == NULL) {
    screen_init();
  }
  return screen;
}
