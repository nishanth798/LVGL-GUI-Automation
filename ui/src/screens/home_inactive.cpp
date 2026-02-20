#include "json_handles.h"
#include "jsonrpc2.h"
#include "resp_state.h"
#include "ui.h"
#include <stdio.h>
#include <stdlib.h>

// this screen
static lv_obj_t *screen;
lv_obj_t *btn_shutdown;
lv_obj_t *btn_activate;

// Shutdown confirmation message box
static const char *mboxBtns[] = {"Cancel", "Shut Down", ""};
static const char *mboxMsg = "Are you sure you want to SHUT DOWN the system?";

void hide_shutdown_btn(bool hide) {
  // hide the room number label
  if (hide) {
    lv_obj_add_flag(btn_shutdown, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_size(btn_activate, 374, 374);                                     /// 1
    lv_obj_set_style_radius(btn_activate, 187, LV_PART_MAIN | LV_STATE_DEFAULT); // 187
  } else {
    // show the room number label
    lv_obj_clear_flag(btn_shutdown, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_size(btn_activate, 320, 320);                                     /// 1
    lv_obj_set_style_radius(btn_activate, 187, LV_PART_MAIN | LV_STATE_DEFAULT); // 187
  }
}

static void screen_load_cb(lv_event_t *e) {
  // set this screen as parent for firmware upgrate status toast message.
  set_firmmware_toast_parent(screen);
  // get firmware variant
  NKD_VARIANT firmware_variant = get_firmware_variant();
  if (firmware_variant == NKD_GO) {
    // If variant is NKD_GO, show the room number label
    hide_shutdown_btn(false);
    btn_set_state(btn_shutdown, BTN_STATE_ACTIVE);
  } else {
    // If variant is not NKD_GO, hide the room number label
    hide_shutdown_btn(true);
  }
  screen_settings_set_mode(SETTINGS_MODE_ACTIVATION); // set mode as activation mode
}

static void btn_activate_sensor_click_cb(lv_event_t *e) {
  // To clear the old session data of bms screen and audio screen before showing
  // bms settings page and audio settings page.
  clear_alerts_sessiondata();
  clear_bms_sessiondata();
  clear_audio_sessiondata();
  clear_exit_alert_schedule_sessiondata();
  clear_bed_width_sessiondata();
  clear_occupant_size_sessiondata();
  clear_bed_placement_sessiondata();

  // lv_obj_t *screen = screen_settings_bms_get();
  // clear old data
  unit_num_info_t *unit_num_info = unit_num_info_get();
  unit_num_info_clear(unit_num_info);
  rm_num_info_t *rm_num_info = rm_num_info_get();
  rm_num_info_clear(rm_num_info);
  rm_num_info_clear(rm_num_info);
  // bug fix:clearing data (when inactive timeout happens, the control comes to inactive screen.
  // while starting the process again clearing the data)
  resp_session_data_t *resp_session_data = resp_session_data_get();
  resp_session_data_clear(resp_session_data);
  room_assign_set_mode(ASSIGN_MODE_NONE);             // make assign mode as none
  screen_settings_set_mode(SETTINGS_MODE_ACTIVATION); // set mode as activation mode
  close_one_btn_msg_box();
  close_two_btn_msg_box();

  // Get firmware variant
  NKD_VARIANT firmware_variant = get_firmware_variant();

  if (firmware_variant == NKD_INTERACTIVE) {
    // lv_obj_t *screen = screen_settings_bms_get();
    lv_obj_t *screen = screen_settings_alerts_get();
    // screen_settings_set_mode(SETTINGS_MODE_ACTIVATION);
    lv_screen_load(screen);
  } else if (firmware_variant == NKD_GO) {
    // ToDo: check if the room is already assigned.
    //  1. If assigned, then show confirm room page
    //  2. If not assigned, then show assign room page

    strcpy(resp_session_data->response_method, cmd_str[CMD_GET_ROOM]);
    resp_session_data->resp_id++;

    JsonObject params;

    serializeJsonRpcRequest(resp_session_data->resp_id, CMD_GET_ROOM, params);
    // show spinner screen as indicating, we are waiting for response
    create_spinner_screen(screen);
  }
  return;
}

static void jsonrpc_request_send(CMD command) {
  JsonObject params;
  serializeJsonRpcRequest(0, command, params);
}

static void request_timeout_cb(lv_event_t *e) {
  toast_show(screen, no_response_str);
  btn_set_state(btn_shutdown, BTN_STATE_ACTIVE);
}

static void mbtn_event_cb(lv_event_t *e) {
  lv_obj_t *btn = lv_event_get_current_target_obj(e);
  lv_obj_t *msgbox = lv_obj_get_parent(lv_obj_get_parent(btn));
  lv_obj_t *label = lv_obj_get_child(btn, 0);
  //  LV_UNUSED(label);
  const char *btn_text = lv_label_get_text(label);
  if (strcmp(btn_text, "Cancel") == 0) {
    lv_msgbox_close(msgbox);
    //  lv_screen_load(screen_settings_home_get());
  } else if (strcmp(btn_text, "Shut Down") == 0) {
    lv_msgbox_close(msgbox);
    jsonrpc_request_send(CMD_SHUTDOWN);
    btn_set_state(btn_shutdown, BTN_STATE_SUBMITTING);

    // btn_set_state(btn_shutdown, BTN_STATE_SUBMITTING);
    // tmpl_settings_label_set_clickable(screen, false);
  }
}

void btn_shutdown_click_cb(lv_event_t *e) {
  lv_obj_t *mbox = msgbox_confirm_shutdown_create(mboxBtns, mboxMsg, VST_COLOR_ALERT_RED);

  lv_obj_t *footer = lv_msgbox_get_footer(mbox);
  for (uint32_t i = 0; i < lv_obj_get_child_count(footer); i++) {
    lv_obj_t *child = lv_obj_get_child(footer, i);
    lv_obj_add_event_cb(child, mbtn_event_cb, LV_EVENT_CLICKED, NULL);
  }
}

static void screen_init() {
  screen = lv_obj_create(NULL);

  // Main container
  lv_obj_t *cont_main = lv_obj_create(screen);
  lv_obj_clear_flag(cont_main, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_size(cont_main, lv_pct(100), lv_pct(100));
  // lv_obj_set_style_border_width(cont_main, 1, LV_PART_MAIN);
  lv_obj_clear_flag(cont_main, LV_OBJ_FLAG_CLICKABLE);
  // lv_obj_set_style_pad_bottom(cont_main, 0, LV_PART_MAIN);
  lv_obj_set_flex_flow(cont_main, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(cont_main, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_bg_color(cont_main, lv_color_white(), LV_PART_MAIN); // white background
  lv_obj_set_style_border_width(cont_main, 0, LV_PART_MAIN);            // no border

  // add event callbacks
  lv_obj_add_event_cb(screen, request_timeout_cb, (lv_event_code_t)MY_EVENT_REQUEST_TIMEOUT, NULL);

  // Main container
  // add container for mode switcher and settings gear
  lv_obj_t *cont_sub1 = lv_obj_create(cont_main);
  lv_obj_clear_flag(cont_sub1, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_size(cont_sub1, lv_pct(100), 320);         /// 1
  lv_obj_set_flex_grow(cont_sub1, 1);                   // Let it expand if space available
  // lv_obj_set_style_border_width(cont_sub1, 1, LV_PART_MAIN);
  lv_obj_clear_flag(cont_sub1, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_style_border_width(cont_sub1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

  // add activate sensor button.
  // A generic container was used instead of a button widget, as the rounder it
  // got, slower it was to render clicks.
  btn_activate = lv_obj_create(cont_sub1);
  lv_obj_set_size(btn_activate, lv_pct(100), lv_pct(100)); /// 1
  // lv_obj_set_width(btn_activate, 320);  // 374
  // lv_obj_set_height(btn_activate, 320); // 374
  lv_obj_set_align(btn_activate, LV_ALIGN_CENTER);
  lv_obj_clear_flag(btn_activate, LV_OBJ_FLAG_SCROLLABLE);                     /// Flags
  lv_obj_set_style_radius(btn_activate, 187, LV_PART_MAIN | LV_STATE_DEFAULT); // 187
  // btn_activate button color VST_COLOR_DARKBLUE(#1A1040)
  lv_obj_set_style_bg_color(btn_activate, lv_color_hex(VST_COLOR_DARKBLUE),
                            LV_PART_MAIN | LV_STATE_DEFAULT); // 0x43BA7F

  lv_obj_set_style_bg_color(btn_activate,
                            lv_color_darken(lv_color_hex(VST_COLOR_ACTIVATE_SENSOR_BLUE),
                                            50),              // use LV_OPA_ names instead
                            LV_PART_MAIN | LV_STATE_PRESSED); // 0x6135DF

  // add button label
  lv_obj_t *label = lv_label_create(btn_activate);
  lv_obj_set_size(label, LV_SIZE_CONTENT, LV_SIZE_CONTENT); /// 1
  // lv_obj_set_height(label, LV_SIZE_CONTENT); /// 1
  lv_obj_set_align(label, LV_ALIGN_CENTER);
  lv_label_set_text(label, "Activate\nSystem");
  lv_obj_set_style_text_line_space(label, 12, LV_PART_MAIN); // space between two lines
  lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(label, &opensans_bold_40, LV_PART_MAIN | LV_STATE_DEFAULT);

  // shutdown button with label for NKD-G variant

  // btn_create(cont_menu, "Deactivate");

  btn_shutdown = btn_create(cont_main, "Shut Down");
  lv_obj_set_size(btn_shutdown, lv_pct(100), 80); // Full width, 50 height
  lv_obj_set_style_bg_color(btn_shutdown, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(btn_shutdown, lv_color_hex(VST_COLOR_ALERT_RED),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(btn_shutdown, 3, LV_PART_MAIN | LV_STATE_DEFAULT);

  // shutdown label
  // lv_obj_t *shutdown_label = lv_label_create(btn_shutdown);
  // lv_obj_set_size(shutdown_label, LV_SIZE_CONTENT, LV_SIZE_CONTENT); /// 1
  // lv_obj_set_align(shutdown_label, LV_ALIGN_CENTER);
  // lv_label_set_text(shutdown_label, "Shutdown");
  lv_obj_set_style_text_color(btn_shutdown, lv_color_hex(VST_COLOR_ALERT_RED),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(btn_shutdown, &opensans_bold_24, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_add_event_cb(btn_activate, btn_activate_sensor_click_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(screen, screen_load_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
  lv_obj_add_event_cb(btn_shutdown, btn_shutdown_click_cb, LV_EVENT_CLICKED, NULL);
}

lv_obj_t *scr_home_inactive_get() {
  if (screen == NULL) {
    screen_init();
  }
  return screen;
}
