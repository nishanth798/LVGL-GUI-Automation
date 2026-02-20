#include "json_handles.h"
#include "jsonrpc2.h"
#include "screens.h"
#include "sys_state.h"
#include "ui.h"
#include "widgets.h"
//
// Section: Constants
//

//
// Section: Variables
//

// this screen
static lv_obj_t *screen;

// widgets on this screen
static lv_obj_t *btnm_mode;
static lv_obj_t *btn_pause_short, *btn_pause_long;
static lv_obj_t *status_panel;
static lv_obj_t *settings_gear_panel;
static lv_obj_t *lbl_room_info; // for room number displaying
static lv_obj_t *btn_bed_chair_mode_disabled;
static lv_obj_t *btn_clr_alerts,
    *lbl_clr_alrt_btn;            // for clearing Reposition alert and resuming fall monitoring
static lv_obj_t *cont_pause_btns; // container for navigation buttons()

//
// Section: Control functions
//

// todo: change to generic set command instead of CMD_ACTIVATE_BED_MODE or
// CMD_ACTIVATE_CHAIR_MODE
static void jsonrpc_request_send_mode(MONITOR_MODE mode) {
  // const size_t capacity = 200; // Adjust this based on your needs
  JsonDocument doc;

  CMD cmd;
  switch (mode) {
  case MODE_BED:
    cmd = CMD_ACTIVATE_BED_MODE;
    break;
  case MODE_CHAIR:
    cmd = CMD_ACTIVATE_CHAIR_MODE;
    break;
  default:
    return;
    break;
  }
  JsonObject params = doc["params"].to<JsonObject>(); //  doc.createNestedObject("params");

  serializeJsonRpcRequest(0, cmd, params);
}

// todo: change to generic set command instead of CMD_PAUSE_SENSOR
static void jsonrpc_request_send_pause(PAUSE pause) {
  // const size_t capacity = 200; // Adjust this based on your needs
  JsonDocument doc;

  JsonObject params = doc["params"].to<JsonObject>();

  params["pause"] = pause;

  serializeJsonRpcRequest(0, CMD_PAUSE_SENSOR, params);
}

void hide_room_number_label(bool hide) {
  // hide the room number label
  if (hide) {
    lv_obj_add_flag(lbl_room_info, LV_OBJ_FLAG_HIDDEN);
  } else {
    // show the room number label
    lv_obj_clear_flag(lbl_room_info, LV_OBJ_FLAG_HIDDEN);
  }
}

static void widgets_set_clickable(bool clickable) {
  if (clickable) {
    lv_obj_add_flag(btnm_mode, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(settings_gear_panel, LV_OBJ_FLAG_CLICKABLE);
  } else {
    lv_obj_clear_flag(btnm_mode, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(settings_gear_panel, LV_OBJ_FLAG_CLICKABLE);
  }
}

static void btnm_mode_click_cb(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_target_obj(e);
  MONITOR_MODE mode = btnm_mode_get(obj);
  jsonrpc_request_send_mode(mode);

  widgets_set_clickable(false);
  btn_set_state(btn_pause_short, BTN_STATE_INACTIVE);
  btn_set_state(btn_pause_long, BTN_STATE_INACTIVE);
  btn_set_state(btn_clr_alerts, BTN_STATE_INACTIVE);

  sys_state_t *ss = sys_state_get();
  if( ss->mode == MODE_SCHEDULED_MON) {
    // disable pause buttons for scheduled monitoring mode
    lv_obj_add_state(btn_pause_short, LV_STATE_DISABLED);
    lv_obj_add_state(btn_pause_long, LV_STATE_DISABLED);
  }
}

static void btnm_pause_click_cb(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_target_obj(e);
  lv_obj_t *label = lv_obj_get_child(obj, 0);
  const char *text = lv_label_get_text(label);
  if (strcmp(text, "Short Pause") == 0) {
    jsonrpc_request_send_pause(PAUSE_SHORT);
  } else if (strcmp(text, "Long Pause") == 0) {
    jsonrpc_request_send_pause(PAUSE_LONG);
  }

  widgets_set_clickable(false);
  sys_state_t *ss = sys_state_get();
  if( ss->mode != MODE_SCHEDULED_MON) {
  btn_set_state(btn_pause_short, BTN_STATE_INACTIVE);
  btn_set_state(btn_pause_long, BTN_STATE_INACTIVE);
  }

  btn_set_state(obj, BTN_STATE_SUBMITTING);
}

static void screen_refresh() {
  // refresh mode switcher
  const sys_state_t *ss = sys_state_get();
  btnm_mode_set(btnm_mode, ss->mode);
  set_reposition_alert_state( ss->fts_state.rep_alt, ss->pr_inj_tmr);

  // refresh status panel
  // set status panel
  // base panel: monitoring
  // overlay panels: alert, error, pause
  if (ss->syserr == SYSERR_SYSTEM_DISCONNECTED) {
    widget_status_panel_set_error(ss->syserr, ss->syserr_title);
  } else if (ss->alert != ALERT_NONE || strcmp(ss->alert_title, "") != 0) {
    widget_status_panel_set_alert(ss->alert, ss->alert_title);
  } else if (ss->syserr != SYSERR_NONE || strcmp(ss->syserr_title, "") != 0) {
    widget_status_panel_set_error(ss->syserr, ss->syserr_title);
  } else if (ss->pause_tmr > 0) {
    widget_status_panel_set_paused(ss->pause_tmr);
  } else if (ss->cal == CAL_ON || ss->cal == CAL_CHAIR) {
    widget_status_panel_set_calib(ss->cal);
  } else if (ss->mode >= MODE_BED || ss->mode <= MODE_SCHEDULED_MON) {
    widget_status_panel_set_monitoring(ss->mode);
    if( ss->mode == MODE_SCHEDULED_MON) {
      lv_obj_add_state(btn_pause_short, LV_STATE_DISABLED);
      lv_obj_add_state(btn_pause_long, LV_STATE_DISABLED);
    }
    else {
      lv_obj_clear_state(btn_pause_short, LV_STATE_DISABLED);
      lv_obj_clear_state(btn_pause_long, LV_STATE_DISABLED);
    }
  } else {
    widget_status_panel_set_unset();
  }
  /*
  get the fts_state and update the home active screen accordingly
     1) if the bed/chair mode is disabled,
        a) disable the mode switcher
        b) hide pause buttons
        c) show Clear alert btn (disabled state) for Prressure injury monitoring
          and resume fall monitoring btn (disabled state) for fall monitoring
        d) enable Clear ALert/Resume Fall Monitoring btn when there is an reposition alert
           or fall detected
      2) if the bed/chair mode is enabled,
         a) enable the mode switcher
         b) show pause buttons during monitoring mode
         c) when Reposition alert (pressure injury) happens, hide pause buttons and show
            Clear alert btn (enabled state)
  */
  // update buttons text only if the alert title is empty
  if (strcmp(ss->alert_title, "") == 0) {
    // Set the clear alert btn text based on the current mode type
    if (ss->mode == MODE_PRESSURE_INJURY) {
      lv_label_set_text(lbl_clr_alrt_btn, "Clear Alert");
    } else if (ss->mode == MODE_FALL_MON) {
      lv_label_set_text(lbl_clr_alrt_btn, "Resume Fall Monitoring");
    }
    // Set the clear alert btn text based on the current alert type.
    // As alert has hgher priority than mode, update the button text based on alert type
    if (ss->alert == ALERT_REPOSITION) {
      lv_label_set_text(lbl_clr_alrt_btn, "Clear Alert");
    } else if (ss->alert == ALERT_FALL_DETECTED) {
      lv_label_set_text(lbl_clr_alrt_btn, "Resume Fall Monitoring");
    }
  }

  if (ss->fts_state.bed_alt == false) {
    // if the bed/chair mode is disabled, show the disabled mode switcher container
    lv_obj_clear_flag(btn_bed_chair_mode_disabled, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(btnm_mode, LV_OBJ_FLAG_HIDDEN);

    // hide pause buttons only if alert tiltle is empty
    if (strcmp(ss->alert_title, "") == 0) {
      // hide pause buttons and show Clear alert for Prressure injury monitoring
      // and resume fall alert for fall monitoring
      lv_obj_add_flag(cont_pause_btns, LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(btn_clr_alerts, LV_OBJ_FLAG_HIDDEN);
      // disable Clear Alert/Resume Fall Monitoring btn
      lv_obj_clear_flag(btn_clr_alerts, LV_OBJ_FLAG_CLICKABLE);
      // btn_set_state(btn_clr_alerts, BTN_STATE_INACTIVE);
      lv_obj_add_state(btn_clr_alerts, LV_STATE_DISABLED);

      // enable Clear ALert/Resume Fall Monitoring btn when there is an reposition
      // alert or fall detected
      if (ss->alert == ALERT_REPOSITION || ss->alert == ALERT_FALL_DETECTED) {
        lv_obj_add_flag(btn_clr_alerts, LV_OBJ_FLAG_CLICKABLE);
        btn_set_state(btn_clr_alerts, BTN_STATE_ACTIVE);
        lv_obj_clear_state(btn_clr_alerts, LV_STATE_DISABLED);
      }
    }
  } else {
    // if the bed/chair mode is enabled, hide disabled mode switcher container
    lv_obj_add_flag(btn_bed_chair_mode_disabled, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(btnm_mode, LV_OBJ_FLAG_HIDDEN);
    // hide pause buttons only if alert tiltle is empty
    if (strcmp(ss->alert_title, "") == 0) {
      // show pause buttons and hide Clear alert btn
      lv_obj_clear_flag(cont_pause_btns, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(btn_clr_alerts, LV_OBJ_FLAG_HIDDEN);
      // Hide pause buttons, show Clear ALert button and enable it
      if (ss->alert == ALERT_REPOSITION) {
        lv_obj_add_flag(cont_pause_btns, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(btn_clr_alerts, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(btn_clr_alerts, LV_OBJ_FLAG_CLICKABLE);
        btn_set_state(btn_clr_alerts, BTN_STATE_ACTIVE);
        lv_obj_clear_state(btn_clr_alerts, LV_STATE_DISABLED);
      }
    }
  }

  // refresh bms level indicator
  // lv_obj_t *bms = widget_bms_level_indicator_get();
  // widget_bms_level_indicator_set(bms, ss->bms);

  // update the bms label text
  set_bms_level(ss->bms);
  // update the selected room number
  char rm_num_arr[20] = "Room ";       // default text
  strcat(rm_num_arr, ss->room_number); // concatanate the room number received
  lv_label_set_text(lbl_room_info, rm_num_arr);
}

static void set_bottom_btns_state() {
  sys_state_t *ss = sys_state_get();
  if (ss->mode != MODE_SCHEDULED_MON) {
  btn_set_state(btn_pause_short, BTN_STATE_ACTIVE);
  btn_set_state(btn_pause_long, BTN_STATE_ACTIVE);
  }
  
  btn_set_state(btn_clr_alerts, BTN_STATE_ACTIVE);
  lv_obj_clear_state(btn_clr_alerts, LV_STATE_DISABLED);
}

static void screen_load_cb(lv_event_t *e) {
  screen_settings_set_mode(SETTINGS_MODE_SAVE);
  widgets_set_clickable(true);
  set_bottom_btns_state();

  screen_refresh();
  // set this screen as parent for firmware upgrate status toast message.
  set_firmmware_toast_parent(screen);

  // get firmware variant
  NKD_VARIANT firmware_variant = get_firmware_variant();
  if (firmware_variant == NKD_GO) {
    // If variant is NKD_GO, show the room number label
    hide_room_number_label(false);
  } else {
    // If variant is not NKD_GO, hide the room number label
    hide_room_number_label(true);
  }
}

static void settings_gear_click_cb(lv_event_t *e) {
  // If status badge is already open in settings page, hide it before showing settings screen.
  alert_toast_hide();
  // Show settings screen
  lv_screen_load(screen_settings_home_get());
}

static void request_timeout_cb(lv_event_t *e) {
  const sys_state_t *ss = sys_state_get();
  btnm_mode_set(btnm_mode, ss->mode);
  toast_show(screen, no_response_str);

  widgets_set_clickable(true);
  set_bottom_btns_state();
}

static void set_state_fail_cb(lv_event_t *e) {
  const sys_state_t *ss = sys_state_get();
  btnm_mode_set(btnm_mode, ss->mode);
  toast_show(screen, invalid_response_str);

  widgets_set_clickable(true);
  set_bottom_btns_state();
}

static void set_state_ok_cb(lv_event_t *e) {
  widgets_set_clickable(true);
  set_bottom_btns_state();
  screen_refresh();
}

static void jsonrpc_request_send_clr_alert() {
  JsonObject params;
  serializeJsonRpcRequest(0, CMD_CLEAR_ALERT, params);
}

static void jsonrpc_request_send_resume_fall_mon() {
  JsonObject params;
  serializeJsonRpcRequest(0, CMD_RESUME_FALL_MONITORING, params);
}

static void btn_clr_alerts_click_cb(lv_event_t *e) {
  // compare lbl_clr_alrt_btn text and send thejson request accordingly
  const char *text = lv_label_get_text(lbl_clr_alrt_btn);
  if (strcmp(text, "Clear Alert") == 0) {
    jsonrpc_request_send_clr_alert();
  } else if (strcmp(text, "Resume Fall Monitoring") == 0) {
    jsonrpc_request_send_resume_fall_mon();
  }

  widgets_set_clickable(false);
  btn_set_state(btn_clr_alerts, BTN_STATE_INACTIVE);
  btn_set_state(btn_clr_alerts, BTN_STATE_SUBMITTING);
}

static void *screen_init() {
  screen = lv_obj_create(NULL);

  lv_obj_set_size(screen, lv_pct(100), lv_pct(100));
  lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_align(screen, LV_ALIGN_CENTER);
  lv_obj_set_flex_flow(screen, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(screen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_row(screen, 16, LV_PART_MAIN);
  lv_obj_clear_flag(screen, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_style_bg_color(screen, lv_color_white(), LV_PART_MAIN);

  lv_obj_add_event_cb(screen, screen_load_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
  lv_obj_add_event_cb(screen, request_timeout_cb, (lv_event_code_t)MY_EVENT_REQUEST_TIMEOUT, NULL);
  lv_obj_add_event_cb(screen, set_state_ok_cb, (lv_event_code_t)MY_EVENT_SET_STATE_OK, NULL);
  lv_obj_add_event_cb(screen, set_state_fail_cb, (lv_event_code_t)MY_EVENT_SET_STATE_FAIL, NULL);
  lv_obj_add_event_cb(screen, set_state_ok_cb, (lv_event_code_t)MY_EVENT_KEEPALIVE_TIMEOUT, NULL);

  // add container for mode switcher and settings gear
  lv_obj_t *cont_btnm_mode = lv_obj_create(screen);
  lv_obj_clear_flag(cont_btnm_mode, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_size(cont_btnm_mode, lv_pct(100), LV_SIZE_CONTENT); /// 1
  lv_obj_set_style_border_width(cont_btnm_mode, 0, LV_PART_MAIN);
  lv_obj_clear_flag(cont_btnm_mode, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_style_pad_bottom(cont_btnm_mode, 0, LV_PART_MAIN);

  // add mode button matrix
  btnm_mode = widget_btnm_mode_add(cont_btnm_mode);
  lv_obj_set_align(btnm_mode, LV_ALIGN_LEFT_MID);
  lv_obj_add_event_cb(btnm_mode, btnm_mode_click_cb, LV_EVENT_VALUE_CHANGED, NULL);

  // Add disabled mode button (for bed/chair alerts disabled mode)
  // Keep it disabled initially
  btn_bed_chair_mode_disabled = lv_obj_create(cont_btnm_mode);
  lv_obj_clear_flag(btn_bed_chair_mode_disabled, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_size(btn_bed_chair_mode_disabled, 352, 80);
  lv_obj_set_align(btn_bed_chair_mode_disabled, LV_ALIGN_LEFT_MID);
  lv_obj_set_style_opa(btn_bed_chair_mode_disabled, LV_OPA_60, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(btn_bed_chair_mode_disabled, lv_color_hex(0x31344E),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(btn_bed_chair_mode_disabled, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(btn_bed_chair_mode_disabled, 5, LV_PART_MAIN);
  lv_obj_add_flag(btn_bed_chair_mode_disabled, LV_OBJ_FLAG_HIDDEN);

  lv_obj_t *lbl_mode_disabled = lv_label_create(btn_bed_chair_mode_disabled);
  lv_label_set_text(lbl_mode_disabled, "Bed/Chair Mode Disabled");
  lv_obj_set_style_text_color(lbl_mode_disabled, lv_color_hex(VST_COLOR_DARKBLUE),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(lbl_mode_disabled, &opensans_bold_24, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_align(lbl_mode_disabled, LV_ALIGN_CENTER);

  // add settings gear
  settings_gear_panel = lv_obj_create(cont_btnm_mode);
  lv_obj_set_size(settings_gear_panel, 80, 80);
  lv_obj_set_align(settings_gear_panel, LV_ALIGN_RIGHT_MID);
  lv_obj_clear_flag(settings_gear_panel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_border_color(settings_gear_panel, lv_color_hex(VST_COLOR_DARKBLUE),
                                LV_PART_MAIN);
  lv_obj_set_style_border_width(settings_gear_panel, 3, LV_PART_MAIN);

  // add click event handler
  lv_obj_add_event_cb(settings_gear_panel, settings_gear_click_cb, LV_EVENT_CLICKED, NULL);

  // fixme: isnt the panel being clickable good enough?
  lv_obj_t *ui_img_gear = lv_img_create(settings_gear_panel);
  lv_img_set_src(ui_img_gear, &icon_gear);
  lv_obj_set_size(ui_img_gear, LV_SIZE_CONTENT, LV_SIZE_CONTENT); /// 1
  lv_obj_set_align(ui_img_gear, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_img_gear, LV_OBJ_FLAG_EVENT_BUBBLE); /// Flags
  lv_obj_clear_flag(ui_img_gear, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_img_recolor(ui_img_gear, lv_color_hex(0x999999),
                               LV_PART_MAIN | LV_STATE_DISABLED);
  lv_obj_set_style_img_recolor_opa(ui_img_gear, 255, LV_PART_MAIN | LV_STATE_DISABLED);

  lv_obj_t *status_panel_monitoring = lv_obj_create(screen);
  lv_obj_set_flex_flow(status_panel_monitoring, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_size(status_panel_monitoring, 448, 256);
  lv_obj_clear_flag(status_panel_monitoring, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_pad_all(status_panel_monitoring, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(status_panel_monitoring, 0, LV_PART_MAIN);
  lv_obj_set_flex_align(status_panel_monitoring, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);

  // add room number label
  lbl_room_info = lv_label_create(status_panel_monitoring);
  lv_label_set_text(lbl_room_info, "Room  0000");
  lv_obj_set_style_text_font(lbl_room_info, &opensans_bold_24, LV_PART_MAIN);

  // add status card
  status_panel = widget_status_panel_add(status_panel_monitoring);

  // add nav btns container
  cont_pause_btns = lv_obj_create(screen);
  lv_obj_add_flag(cont_pause_btns, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_set_size(cont_pause_btns, lv_pct(100), 80);
  lv_obj_clear_flag(cont_pause_btns, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_border_width(cont_pause_btns, 0, LV_PART_MAIN);
  lv_obj_set_pos(cont_pause_btns, 0, 384);

  // add Short Pause and Long Pause buttons
  btn_pause_short = btn_primary_create_1(cont_pause_btns, "Short Pause", VST_COLOR_DARKBLUE);
  lv_obj_set_align(btn_pause_short, LV_ALIGN_LEFT_MID);
  lv_obj_t *lbl_pause_short = lv_obj_get_child(btn_pause_short, 0);
  lv_obj_set_style_text_font(lbl_pause_short, &opensans_bold_24, LV_PART_MAIN);
  lv_obj_add_event_cb(btn_pause_short, btnm_pause_click_cb, LV_EVENT_CLICKED, NULL);

  btn_pause_long = btn_primary_create_1(cont_pause_btns, "Long Pause", VST_COLOR_DARKBLUE);
  lv_obj_set_align(btn_pause_long, LV_ALIGN_RIGHT_MID);
  lv_obj_t *lbl_pause_long = lv_obj_get_child(btn_pause_long, 0);
  lv_obj_set_style_text_font(lbl_pause_long, &opensans_bold_24, LV_PART_MAIN);
  lv_obj_add_event_cb(btn_pause_long, btnm_pause_click_cb, LV_EVENT_CLICKED, NULL);

  // Add Clear ALert button / Resume Fall Monitoring button
  // Keep it disabled initially
  btn_clr_alerts = btn_create(screen, "Resume Fall Monitoring");
  // get the label of the button such that text can be changed at runtime depending on the alert
  lbl_clr_alrt_btn = lv_obj_get_child(btn_clr_alerts, 0);

  lv_obj_add_event_cb(btn_clr_alerts, btn_clr_alerts_click_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_flag(btn_clr_alerts, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_set_align(btn_clr_alerts, LV_ALIGN_BOTTOM_MID);
  lv_obj_set_pos(btn_clr_alerts, 0, -16);
  lv_obj_set_size(btn_clr_alerts, 448, 80);
  lv_obj_set_style_bg_color(btn_clr_alerts, lv_color_hex(VST_COLOR_DARKBLUE),
                            LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(btn_clr_alerts, 5, LV_PART_MAIN);
  lv_obj_set_style_border_width(btn_clr_alerts, 0, LV_PART_MAIN | LV_STATE_DISABLED);
  lv_obj_add_flag(btn_clr_alerts, LV_OBJ_FLAG_HIDDEN); // By default hide clear alert button
  lv_obj_add_state(btn_clr_alerts, LV_STATE_DISABLED);

  return screen;
}

lv_obj_t *screen_home_active_get() {
  if (screen == NULL) {
    screen_init();
  }
  return screen;
}
