// clang-format off
#include "jsonrpc2.h"
#include "ui.h"
#include <cstdio>
#include "screens.h"
#include "resp_state.h"
#include "json_handles.h"
// clang-format on
// Carry on formatting

//
// Section: Constants
//

// deactivate confirmation message box
static const char *btns[] = {"Cancel", "Deactivate", ""};
static const char *msg = "Are you sure you want to DEACTIVATE the sensor for this room?";

static const char *shut_down_btns[] = {"Cancel", "Shut Down", ""};
static const char *shut_down_msg = "Are you sure you want to DEACTIVATE and SHUT DOWN the system?";

// static const char *btn_deactivate_state_label_text[] = {
//     "Deactivate", NULL, NULL, "Deactivating", "Deactivated",
// };

// menu items
typedef enum {
  MENU_DISPLAY,
  MENU_AUDIO,
  MENU_ALERTS,
  MENU_EXIT_ALERT_SCHEDULE,
  MENU_BMS,
  MENU_BED_WIDTH,
  MENU_BED_PLACEMENT,
  MENU_OCCUPANT_SIZE,
} MENU_ID;

static const char *menu_text[] = {
    "Display",
    "In-Room Alert Audio",
    "Alerts",
    "Exit Alert Schedule",
    "Bed Sensitivity",
    "Bed Width",
    "Bed Placement",
    "Occupant Size",
    NULL,
};

//
// Section: Variables
//

// root
static lv_obj_t *screen;

// ui components
static lv_obj_t *cont_menu;
static lv_obj_t *btn_deactivate;
static lv_obj_t *btns_container;
// array to hold menu buttons so that we can hide or show accordingly
static lv_obj_t *menu_btns[sizeof(menu_text) / sizeof(menu_text[0])];

//
// Section: Control functions
//

static void menu_set_clickable(lv_obj_t *cont_menu, bool clickable) {
  uint32_t i;
  for (i = 0; i < lv_obj_get_child_cnt(cont_menu); i++) {
    lv_obj_t *child = lv_obj_get_child(cont_menu, i);
    if (clickable) {
      lv_obj_add_flag(child, LV_OBJ_FLAG_CLICKABLE);
    } else {
      lv_obj_clear_flag(child, LV_OBJ_FLAG_CLICKABLE);
    }
  }
}

static void jsonrpc_request_send(CMD command) {
  JsonObject params;
  serializeJsonRpcRequest(0, command, params);
}

//
// Section: Event Handlers
//

// static void left_arrow_click_cb(lv_event_t *e) {
//   SETTINGS_MODE settings_mode = screen_settings_get_mode();
//   if (settings_mode == SETTINGS_MODE_SAVE) {
//     lv_obj_t *screen = screen_home_active_get();
//     lv_screen_load(screen);
//   }
// }

static void btn_menu_item_click_cb(lv_event_t *e) {
  lv_obj_t *btn = lv_event_get_target_obj(e);
  lv_obj_t *label = lv_obj_get_child(btn, 0);
  const char *btn_text = lv_label_get_text(label);
  // myprint(btn_text);
  lv_obj_t *screen;
  if (strcmp(btn_text, menu_text[MENU_DISPLAY]) == 0) {
    screen = screen_settings_display_get();
  } else if (strcmp(btn_text, menu_text[MENU_AUDIO]) == 0) {
    screen = screen_settings_audio_get();
  } else if (strcmp(btn_text, menu_text[MENU_BMS]) == 0) {
    screen = screen_settings_bms_get();
  } else if (strcmp(btn_text, menu_text[MENU_OCCUPANT_SIZE]) == 0) {
    screen = screen_settings_get_occupant_size();
  } else if (strcmp(btn_text, menu_text[MENU_BED_WIDTH]) == 0) {
    screen = screen_settings_get_bed_width();
  } else if (strcmp(btn_text, menu_text[MENU_BED_PLACEMENT]) == 0) {
    screen = screen_settings_get_bed_placement();
  } else if (strcmp(btn_text, menu_text[MENU_ALERTS]) == 0) {
    screen = screen_settings_alerts_get();
  } else if (strcmp(btn_text, menu_text[MENU_EXIT_ALERT_SCHEDULE]) == 0) {
    screen = screen_settings_exit_alert_sch_get();
  }
  screen_settings_set_mode(SETTINGS_MODE_SAVE);
  lv_screen_load(screen);
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
  } else if (strcmp(btn_text, "Deactivate") == 0) {
    lv_msgbox_close(msgbox);
    jsonrpc_request_send(CMD_DEACTIVATE_SENSOR);

    // Set button state based on current variant
    NKD_VARIANT firmware_variant = get_firmware_variant();
    if (firmware_variant == NKD_GO) {
      // For NKD_GO, only set submitting state for the deactivate button in the container
      lv_obj_t *button_deactivate = lv_obj_get_child(btns_container, 1);
      if (button_deactivate)
        btn_set_state(button_deactivate, BTN_STATE_SUBMITTING);
    } else {
      btn_set_state(btn_deactivate, BTN_STATE_SUBMITTING);
    }

    menu_set_clickable(cont_menu, false);
    tmpl_settings_label_set_clickable(screen, false);
  } else if (strcmp(btn_text, "Shut Down") == 0) {
    lv_msgbox_close(msgbox);
    jsonrpc_request_send(CMD_SHUTDOWN);

    // For shutdown, only set submitting state for the shutdown button
    lv_obj_t *btn_shutdown = lv_obj_get_child(btns_container, 0);
    if (btn_shutdown)
      btn_set_state(btn_shutdown, BTN_STATE_SUBMITTING);

    menu_set_clickable(cont_menu, false);
    tmpl_settings_label_set_clickable(screen, false);
  }
}

static void btn_shutdown_cb(lv_event_t *e) {
  lv_obj_t *mbox =
      msgbox_confirm_shutdown_create(shut_down_btns, shut_down_msg, VST_COLOR_ALERT_RED);

  lv_obj_t *footer = lv_msgbox_get_footer(mbox);
  for (uint32_t i = 0; i < lv_obj_get_child_count(footer); i++) {
    lv_obj_t *child = lv_obj_get_child(footer, i);
    lv_obj_add_event_cb(child, mbtn_event_cb, LV_EVENT_CLICKED, NULL);
  }
}

static void btn_deactivate_click_cb(lv_event_t *e) {
  lv_obj_t *mbox = msgbox_confirm_shutdown_create(btns, msg, VST_COLOR_ALERT_RED);
  //  lv_obj_add_event_cb(mbox, btn_deactivate_confirm_click_cb,
  //                      LV_EVENT_VALUE_CHANGED, NULL);

  lv_obj_t *footer = lv_msgbox_get_footer(mbox);
  for (uint32_t i = 0; i < lv_obj_get_child_count(footer); i++) {
    lv_obj_t *child = lv_obj_get_child(footer, i);
    lv_obj_add_event_cb(child, mbtn_event_cb, LV_EVENT_CLICKED, NULL);
  }
  // lv_screen_load(screen_settings_home_get());
}

static void request_timeout_cb(lv_event_t *e) {
  toast_show(screen, no_response_str);

  // Reset button states based on current variant
  NKD_VARIANT firmware_variant = get_firmware_variant();
  if (firmware_variant == NKD_GO) {
    // Reset the individual buttons in the container
    lv_obj_t *btn_shutdown = lv_obj_get_child(btns_container, 0);
    lv_obj_t *button_deactivate = lv_obj_get_child(btns_container, 1);
    if (btn_shutdown)
      btn_set_state(btn_shutdown, BTN_STATE_ACTIVE);
    if (button_deactivate)
      btn_set_state(button_deactivate, BTN_STATE_ACTIVE);
  } else {
    btn_set_state(btn_deactivate, BTN_STATE_ACTIVE);
  }

  menu_set_clickable(cont_menu, true);
  tmpl_settings_label_set_clickable(screen, true);
}

static void set_menu_btn_visibility(lv_obj_t *btn, bool visible) {
  if (visible)
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_HIDDEN);
  else
    lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);
}

void update_settings_menu_buttons(void) {
  // get the sys_state struct and check the fts_avail struct params and show/hide the settings
  // buttons accordingly
  const sys_state_t *ss_t = sys_state_get();

  set_menu_btn_visibility(menu_btns[MENU_EXIT_ALERT_SCHEDULE], ss_t->fts_avail.sch_mon);
  set_menu_btn_visibility(menu_btns[MENU_BED_WIDTH], ss_t->fts_avail.bed_wid);
  set_menu_btn_visibility(menu_btns[MENU_BED_PLACEMENT], ss_t->fts_avail.bed_pos);
  set_menu_btn_visibility(menu_btns[MENU_OCCUPANT_SIZE], ss_t->fts_avail.occ_size);
}

static void screen_load_cb(lv_event_t *e) {
  menu_set_clickable(cont_menu, true);
  tmpl_settings_label_set_clickable(screen, true);

  update_settings_menu_buttons();
  // Show/hide buttons based on firmware variant
  NKD_VARIANT firmware_variant = get_firmware_variant();
  if (firmware_variant == NKD_GO) {
    // NKD_GO variant: Show container with both buttons, hide single deactivate button
    lv_obj_clear_flag(btns_container, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(btn_deactivate, LV_OBJ_FLAG_HIDDEN);
    lv_obj_t *btn_shutdown = lv_obj_get_child(btns_container, 0);
    lv_obj_t *button_deactivate = lv_obj_get_child(btns_container, 1);
    if (btn_shutdown)
      btn_set_state(btn_shutdown, BTN_STATE_ACTIVE);
    if (button_deactivate)
      btn_set_state(button_deactivate, BTN_STATE_ACTIVE);

  } else {
    // Other variants: Show single deactivate button, hide container
    lv_obj_clear_flag(btn_deactivate, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(btns_container, LV_OBJ_FLAG_HIDDEN);
    btn_set_state(btn_deactivate, BTN_STATE_ACTIVE);
  }
}

//
// Section: UI
//

// todo: convert to button matrix
static lv_obj_t *btn_menu_item_create(lv_obj_t *parent_container, const char *btn_txt,
                                      lv_event_cb_t event_cb) {
  lv_obj_t *btn = lv_btn_create(parent_container);
  lv_obj_set_size(btn, 221, 64);
  lv_obj_set_style_bg_color(btn, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_text_color(btn, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_border_color(btn, lv_color_hex(0xD1D1D1), LV_PART_MAIN);
  lv_obj_set_style_border_width(btn, 1, LV_PART_MAIN);

  lv_obj_t *btn_label = lv_label_create(btn);
  lv_obj_set_size(btn_label, LV_SIZE_CONTENT, LV_SIZE_CONTENT); /// 1
  lv_obj_align(btn_label, LV_ALIGN_LEFT_MID, 0, 3);
  lv_label_set_text(btn_label, btn_txt);
  lv_obj_set_style_text_font(btn_label, &opensans_semibold_19, LV_PART_MAIN);
  lv_obj_set_style_text_color(btn_label, lv_color_black(), LV_PART_MAIN);

  // lv_obj_t *arrow = lv_img_create(btn);
  // lv_img_set_src(arrow, &icon_arrow);
  // lv_obj_set_size(arrow, LV_SIZE_CONTENT, LV_SIZE_CONTENT); /// 1
  // lv_obj_set_align(arrow, LV_ALIGN_RIGHT_MID);

  lv_obj_add_event_cb(btn, event_cb, LV_EVENT_CLICKED, NULL);

  return btn;
}

// show information page
static void info_icon_click_cb(lv_event_t *e) { show_system_info_screen((char *)"settings_home"); }

static void screen_init(void) {
  screen = tmpl_settings_create_1(NULL, "Settings", true);
  lv_obj_set_style_bg_color(screen, lv_color_hex(VST_COLOR_SCREEN_BG_GREY), LV_PART_MAIN);

  // lv_obj_t *lef_arrow_obj = get_img_left_arrow_obj();
  // lv_obj_add_flag(lef_arrow_obj, LV_OBJ_FLAG_EVENT_BUBBLE); /// Flags

  // lv_obj_t *title_cont_left_arrow_obj = get_cont_left_arrow_obj();
  // lv_obj_add_event_cb(title_cont_left_arrow_obj, left_arrow_click_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *title_cont_info_icon_obj = get_cont_info_icon_obj();
  lv_obj_add_event_cb(title_cont_info_icon_obj, info_icon_click_cb, LV_EVENT_CLICKED, NULL);

  //  lv_obj_add_event_cb(screen, label_back_click_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(screen, screen_load_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
  lv_obj_add_event_cb(screen, request_timeout_cb, (lv_event_code_t)MY_EVENT_REQUEST_TIMEOUT, NULL);

  // add content container
  cont_menu = lv_obj_create(screen);
  lv_obj_set_width(cont_menu, 452);
  lv_obj_set_x(cont_menu, 16);
  lv_obj_set_flex_grow(cont_menu, 1);
  lv_obj_set_flex_flow(cont_menu, LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_flex_align(cont_menu, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
  lv_obj_clear_flag(cont_menu, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_border_width(cont_menu, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_row(cont_menu, 10, LV_PART_MAIN);
  lv_obj_set_style_pad_top(cont_menu, 0, LV_PART_MAIN);
  lv_obj_set_style_bg_color(cont_menu, lv_color_hex(VST_COLOR_SCREEN_BG_GREY), LV_PART_MAIN);
  lv_obj_set_style_pad_all(cont_menu, 0, LV_PART_MAIN);

  // add settings menu
  for (int i = 0; menu_text[i] != NULL; i++) {
    menu_btns[i] = btn_menu_item_create(cont_menu, menu_text[i], btn_menu_item_click_cb);
  }

  btn_deactivate = btn_create(cont_menu, "Deactivate");
  lv_obj_set_style_bg_color(btn_deactivate, lv_color_hex(VST_COLOR_ALERT_RED), LV_PART_MAIN);
  lv_obj_add_flag(btn_deactivate, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_add_event_cb(btn_deactivate, btn_deactivate_click_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_set_y(btn_deactivate, -16);

  // Create button container for NKD_GO variant (holds both shutdown and deactivate buttons)
  btns_container = lv_obj_create(cont_menu);
  lv_obj_set_size(btns_container, 448, 84);
  lv_obj_set_flex_flow(btns_container, LV_FLEX_FLOW_ROW);
  lv_obj_set_align(btns_container, LV_ALIGN_BOTTOM_MID);
  lv_obj_set_flex_align(btns_container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);
  lv_obj_clear_flag(btns_container, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_border_width(btns_container, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_gap(btns_container, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(btns_container, 0, LV_PART_MAIN);
  lv_obj_add_flag(btns_container, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_set_y(btns_container, -16);

  // Create shutdown button (left side) - white background with red border/text
  lv_obj_t *btn_shutdown = btn_create(btns_container, "Shut Down");
  lv_obj_set_style_width(btn_shutdown, 216, LV_PART_MAIN);
  lv_obj_set_style_bg_color(btn_shutdown, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_border_color(btn_shutdown, lv_color_hex(VST_COLOR_ALERT_RED), LV_PART_MAIN);
  lv_obj_set_style_text_color(btn_shutdown, lv_color_hex(VST_COLOR_ALERT_RED), LV_PART_MAIN);
  lv_obj_set_style_text_font(btn_shutdown, &opensans_bold_24, LV_PART_MAIN);
  lv_obj_set_style_border_width(btn_shutdown, 3, LV_PART_MAIN);
  lv_obj_add_event_cb(btn_shutdown, btn_shutdown_cb, LV_EVENT_CLICKED, NULL);

  // Create deactivate button for NKD_GO variant (right side) - red background
  lv_obj_t *button_deactivate = btn_create(btns_container, "Deactivate");
  lv_obj_set_style_width(button_deactivate, 216, LV_PART_MAIN);
  lv_obj_set_style_bg_color(button_deactivate, lv_color_hex(VST_COLOR_ALERT_RED), LV_PART_MAIN);
  lv_obj_add_event_cb(button_deactivate, btn_deactivate_click_cb, LV_EVENT_CLICKED, NULL);
}

//
// Section: API
//

lv_obj_t *screen_settings_home_get() {
  if (screen == NULL) {
    screen_init();
  }
  return screen;
}
