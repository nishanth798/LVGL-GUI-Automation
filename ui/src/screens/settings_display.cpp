#include "backlight.h"
#include "disp_state.h"
#include "jsonrpc2.h"
#include "sys_state.h"
#include "ui.h"

//
// Section: Variables
//

// root

static SETTINGS_MODE settings_mode;

static lv_obj_t *screen;
static lv_obj_t *title_cont_left_arrow_obj;
static lv_obj_t *title_cont_info_icon_obj;

typedef struct {
  uint8_t brightness;
  ABC abc;
} session_data_t;

static session_data_t session_data = {
    .brightness = BRIGHTNESS_DEFAULT,
    .abc = ABC_UNSET,
};

static session_data_t sd_load;

// ui components
static lv_obj_t *slider_brightness, *switch_abc;
static lv_obj_t *btn_save;

static void screen_init();

static bool is_session_data_dirty() {
  session_data_t *sd = &session_data;
  // const sys_state_t *ss = sys_state_get();
  if (sd->abc != sd_load.abc) {
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

void update_dispBrightScreen() {
#if (SIMULATOR == 0) && (SENSCAP_EN == 0)
  if (lv_obj_has_state(switch_abc,
                       LV_STATE_CHECKED)) // if abc switch is turned on (checked)
  {
    if (flg_als_Enable == false) // if als is not turned on
    {
      als_on(); // turn on ALS
      flg_als_Enable = true;
      flg_process_als = true; // process als from ui.ino
      // disp_state_set_abc(ABC_ON); // update the state of ABC into structure
      lv_obj_add_state(slider_brightness,
                       LV_STATE_DISABLED); // disable the brightness slider when als is on
      session_data.abc = ABC_ON;
    }
  } // if abc switch is turned off (unchecked)
  else {
    if (flg_als_Enable == true) // if als is on
    {
      als_off();
      flg_als_Enable = false;  // turn off ALS
      flg_process_als = false; // don't process als from ui.ino
      // get the value from slider and update it into structure
      uint8_t brightValue = lv_slider_get_value(slider_brightness);
      disp_state_set_brightness(brightValue);
      // disp_state_set_abc(ABC_OFF); // update the state of ABC into structure
      lv_obj_clear_state(slider_brightness,
                         LV_STATE_DISABLED); // Enable the brightness slider when als is off
      session_data.abc = ABC_OFF;
    }
  }

#endif
#ifdef SIMULATOR
  // In simulator, we don't have ALS, so we just toggle the state of ABC
  if (lv_obj_has_state(switch_abc, LV_STATE_CHECKED)) {
    // disp_state_set_abc(ABC_ON);
    lv_obj_add_state(slider_brightness, LV_STATE_DISABLED);
    session_data.abc = ABC_ON;
  } else {
    // disp_state_set_abc(ABC_OFF);
    lv_obj_clear_state(slider_brightness, LV_STATE_DISABLED);
    session_data.abc = ABC_OFF;
  }
#endif
}

//
// Section: Event Handlers
//

static void set_state_ok_cb(lv_event_t *e) {
  // if(flg_SaveClick == true)
  {
    btn_set_state(btn_save, BTN_STATE_SUBMITTED);

    const sys_state_t *ss = sys_state_get();
    sd_load.abc = ss->abc;
  }
  // flg_SaveClick = false;
}

static void handle_screen_load_callback() {
  settings_mode = screen_settings_get_mode();
  if (settings_mode == SETTINGS_MODE_ACTIVATION) {
    // show the header icon symbol and hide the left arrow
    lv_obj_add_flag(title_cont_left_arrow_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(title_cont_info_icon_obj, LV_OBJ_FLAG_HIDDEN);
  } else if (settings_mode == SETTINGS_MODE_SAVE) {
    // show the header left arrow and hide the icon symbol
    lv_obj_clear_flag(title_cont_left_arrow_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(title_cont_info_icon_obj, LV_OBJ_FLAG_HIDDEN);
  }

  const disp_state_t *d = disp_state_get();
  sys_state_t *ss_t = sys_state_get(); // get the current system state
  if ((d->abc == ABC_OFF) || (ss_t->abc == ABC_OFF)) {
    lv_slider_set_value(slider_brightness, d->brightness, LV_ANIM_OFF);
  }

  if (!is_alert_toast_hidden()) {
    set_alert_toast_parent(screen);
  }
#if (SIMULATOR == 0) && (SENSCAP_EN == 1)
  // For alert display disable als and enable brightness slider all the time
  if (flg_alert2d_display == true) {
    lv_obj_clear_state(slider_brightness, LV_STATE_DISABLED);
    lv_obj_add_state(switch_abc, LV_STATE_DISABLED);
    btn_set_state(btn_save, BTN_STATE_DISABLED); // disable the save button
    return;
  }
#endif

#if !SENSCAP_EN
#ifndef SIMULATOR
  if (flg_alert2d_display == false) {
#endif
    session_data_t *sd = &session_data;
    const sys_state_t *ss = sys_state_get(); // get the current system state
    if (ss->abc == ABC_UNSET) {              // if abc is unset in system state, set it to default
      sd->abc = ABC_DEFAULT;
      sd_load.abc = ABC_DEFAULT;
    } else { // if abc is set in system state, set it to that value
      sd->abc = ss->abc;
      sd_load.abc = ss->abc;
    }

    if (sd->abc == ABC_ON) { // if abc is on, set the switch to checked
      lv_obj_add_state(switch_abc, LV_STATE_CHECKED);
    } else { // if abc is off, set the switch to unchecked
      lv_obj_clear_state(switch_abc, LV_STATE_CHECKED);
    }
    update_dispBrightScreen();
    btn_save_update_status();
#ifndef SIMULATOR
  }
#endif
#endif // !SENSCAP_EN
}

static void screen_load_cb(lv_event_t *e) { handle_screen_load_callback(); }

static void screen_unload_cb(lv_event_t *e) { handle_screen_load_callback(); }

static void jsonrpc_save_request_send(session_data_t *data) {
  //  const size_t capacity = 300; // Adjust this based on your needs
  JsonDocument doc;

  JsonObject params = doc["params"].to<JsonObject>();

  params["brightness"] = data->brightness;
  params["abc"] = data->abc;

  serializeJsonRpcRequest(0, CMD_SAVE_DISPLAY, params);
}

static void mbtn_event_cb(lv_event_t *e) {
  lv_obj_t *btn = lv_event_get_current_target_obj(e);
  lv_obj_t *msgbox = lv_obj_get_parent(lv_obj_get_parent(btn));
  lv_obj_t *label = lv_obj_get_child(btn, 0);
  const char *btn_text = lv_label_get_text(label);
  if (strcmp(btn_text, "Don't Save") == 0) {
    lv_msgbox_close(msgbox);
    lv_screen_load(screen_settings_home_get());
    handle_screen_load_callback(); // apply previous settings as user is not saving the changes
  } else if (strcmp(btn_text, "Save") == 0) {
    lv_msgbox_close(msgbox);
    jsonrpc_save_request_send(&session_data);
    btn_set_state(btn_save, BTN_STATE_SUBMITTING);
  }
}

static void left_arrow_click_cb(lv_event_t *e) {
  SETTINGS_MODE settings_mode = screen_settings_get_mode();
  if (settings_mode == SETTINGS_MODE_SAVE) {
    if (is_session_data_dirty() && !(lv_obj_has_state(btn_save, LV_STATE_DISABLED))) {
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

static void slider_brightness_value_changed_cb(lv_event_t *e) {
  lv_obj_t *slider = lv_event_get_target_obj(e);
  uint8_t brightValue = lv_slider_get_value(slider);
  disp_state_set_brightness(brightValue);
}

static void switch_abc_toggle_cb(lv_event_t *e) {
  update_dispBrightScreen();
  btn_save_update_status();
}

// show information page
static void info_icon_click_cb(lv_event_t *e) {}

static void cancel_icon_click_cb(lv_event_t *e) {
  lv_obj_t *screen = screen_home_active_get();
  lv_screen_load(screen);
  handle_screen_load_callback(); // apply previous settings as user is not saving the changes
}

static void btn_save_click_cb(lv_event_t *e) {
  session_data.brightness = lv_slider_get_value(slider_brightness);
  // flg_SaveClk = true;
  jsonrpc_save_request_send(&session_data);

  // lv_obj_t *btn_save = lv_event_get_target_obj(e);
  btn_set_state(btn_save, BTN_STATE_SUBMITTING);
  // widgets_set_clickable(false);
}

static void request_timeout_cb(lv_event_t *e) {
  // if(flg_SaveClick == true)
  {
    btn_set_state(btn_save, BTN_STATE_ACTIVE);

    toast_show(screen, no_response_str);
  }
  //  flg_SaveClick = false;
}

//
// Section: UI
//
static void screen_init() {
  screen = tmpl_settings_create_1(NULL, "Settings", true);

  lv_obj_t *lef_arrow_obj = get_img_left_arrow_obj();
  lv_obj_add_flag(lef_arrow_obj, LV_OBJ_FLAG_EVENT_BUBBLE); /// Flags

  title_cont_left_arrow_obj = get_cont_left_arrow_obj();
  lv_obj_add_event_cb(title_cont_left_arrow_obj, left_arrow_click_cb, LV_EVENT_CLICKED, NULL);

  title_cont_info_icon_obj = get_cont_info_icon_obj();
  lv_obj_add_event_cb(title_cont_info_icon_obj, info_icon_click_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *cont_cancel_icon = get_cont_close_icon_obj();
  lv_obj_add_event_cb(cont_cancel_icon, cancel_icon_click_cb, LV_EVENT_CLICKED, NULL);

  // lv_obj_add_event_cb(screen, label_back_click_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(screen, screen_load_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
  lv_obj_add_event_cb(screen, screen_unload_cb, LV_EVENT_SCREEN_UNLOAD_START, NULL);
  lv_obj_add_event_cb(screen, request_timeout_cb, (lv_event_code_t)MY_EVENT_REQUEST_TIMEOUT, NULL);
  lv_obj_add_event_cb(screen, set_state_ok_cb, (lv_event_code_t)MY_EVENT_SET_STATE_OK, NULL);

  // add content container
  lv_obj_t *cont_content = lv_obj_create(screen);
  lv_obj_add_flag(cont_content, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_set_size(cont_content, 452, 224);
  lv_obj_set_pos(cont_content, 16, 80);
  // lv_obj_set_flex_grow(cont_content, 1);
  lv_obj_set_flex_flow(cont_content, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(cont_content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);
  lv_obj_clear_flag(cont_content, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_border_width(cont_content, 0, LV_PART_MAIN);
  lv_obj_set_style_bg_color(cont_content, lv_color_hex(VST_COLOR_SCREEN_BG_GREY),
                            LV_PART_MAIN | LV_STATE_DEFAULT); // light grey background
  lv_obj_set_style_pad_all(cont_content, 0, LV_PART_MAIN);    // no padding
  lv_obj_set_style_pad_gap(cont_content, 12, LV_PART_MAIN);   // gap between children

  //  add main content container (display controls)
  lv_obj_t *cont_controls = lv_obj_create(cont_content);
  lv_obj_set_size(cont_controls, 448, 151);
  lv_obj_clear_flag(cont_controls, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_border_width(cont_controls, 1, LV_PART_MAIN);
  lv_obj_set_style_radius(cont_controls, 10, LV_PART_MAIN);
  lv_obj_set_style_border_color(cont_controls, lv_color_hex(0xD9D9D9), LV_PART_MAIN);
  // lv_obj_set_align(cont_controls, LV_ALIGN_TOP_MID);

  lv_obj_set_style_shadow_color(cont_controls, lv_color_hex(0x000000),
                                LV_PART_MAIN);                    // black shadow
  lv_obj_set_style_shadow_opa(cont_controls, 64, LV_PART_MAIN);   // 25% opacity
  lv_obj_set_style_shadow_width(cont_controls, 6, LV_PART_MAIN);  // shadow width
  lv_obj_set_style_shadow_spread(cont_controls, 0, LV_PART_MAIN); // shadow spread
  lv_obj_set_style_shadow_ofs_x(cont_controls, 0, LV_PART_MAIN);  // shadow offset x
  lv_obj_set_style_shadow_ofs_y(cont_controls, 3, LV_PART_MAIN);  // shadow offset y

  lv_obj_set_flex_flow(cont_controls, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(cont_controls, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START,
                        LV_FLEX_ALIGN_CENTER);

  lv_obj_set_style_pad_row(cont_controls, 14, LV_PART_MAIN); // row padding
  lv_obj_set_style_pad_top(cont_controls, 40, LV_PART_MAIN); // top padding

  // Brightness label
  lv_obj_t *ui_Label41 = lv_label_create(cont_controls);
  lv_obj_set_width(ui_Label41, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_height(ui_Label41, LV_SIZE_CONTENT); /// 1
  lv_label_set_text(ui_Label41, "Brightness");
  lv_obj_set_style_text_font(ui_Label41, &opensans_semibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_color(ui_Label41, lv_color_hex(VST_COLOR_PRIMARY_TEXT),
                              LV_PART_MAIN | LV_STATE_DEFAULT);

  // Brightness Slider
  slider_brightness = lv_slider_create(cont_controls);
  lv_slider_set_range(slider_brightness, BRIGHTNESS_MIN, 100);
  lv_obj_set_width(slider_brightness, 416); // width of the slider
  lv_obj_set_height(slider_brightness, 16); // height of the slider
  lv_obj_set_style_bg_color(slider_brightness, lv_color_hex(0xD9D9D9),
                            LV_PART_MAIN | LV_STATE_DEFAULT); // light grey background
  lv_obj_set_style_bg_opa(slider_brightness, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_set_style_bg_color(slider_brightness, lv_color_hex(0x31344E),
                            LV_PART_INDICATOR | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(slider_brightness, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(slider_brightness, lv_color_hex(0xD9D9D9),
                            LV_PART_INDICATOR | LV_STATE_DISABLED); // light grey background
  lv_obj_set_style_bg_opa(slider_brightness, 255, LV_PART_INDICATOR | LV_STATE_DISABLED);

  // Slider Knob
  lv_obj_set_style_pad_all(slider_brightness, 7, LV_PART_KNOB); // knob size
  lv_obj_set_style_bg_color(slider_brightness, lv_color_hex(0x828282),
                            LV_PART_KNOB | LV_STATE_DEFAULT); // grey knob
  lv_obj_set_style_bg_opa(slider_brightness, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(slider_brightness, lv_color_hex(0x828282),
                            LV_PART_KNOB | LV_STATE_DISABLED);
  lv_obj_set_style_bg_opa(slider_brightness, 255, LV_PART_KNOB | LV_STATE_DISABLED);
  lv_obj_set_style_shadow_color(slider_brightness, lv_color_hex(0x000000),
                                LV_PART_KNOB);                        // black shadow
  lv_obj_set_style_shadow_opa(slider_brightness, 64, LV_PART_KNOB);   // 25% opacity
  lv_obj_set_style_shadow_width(slider_brightness, 4, LV_PART_KNOB);  // shadow width
  lv_obj_set_style_shadow_spread(slider_brightness, 0, LV_PART_KNOB); // shadow spread
  lv_obj_set_style_shadow_ofs_x(slider_brightness, 0, LV_PART_KNOB);  // shadow offset x
  lv_obj_set_style_shadow_ofs_y(slider_brightness, 0, LV_PART_KNOB);  // shadow offset y

  lv_slider_set_value(slider_brightness, BRIGHTNESS_DEFAULT, LV_ANIM_OFF);

  // Container for Sunlight icons
  lv_obj_t *ui_Panel46 = lv_obj_create(cont_controls);
  lv_obj_set_size(ui_Panel46, lv_pct(100), LV_SIZE_CONTENT); /// 1
  lv_obj_set_style_border_width(ui_Panel46, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_top(ui_Panel46, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_left(ui_Panel46, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_right(ui_Panel46, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

  // Low Singlight icon
  lv_obj_t *ui_Img_lowsun = lv_img_create(ui_Panel46);
  lv_img_set_src(ui_Img_lowsun, &icon_lowsunlight);
  lv_obj_set_size(ui_Img_lowsun, LV_SIZE_CONTENT, LV_SIZE_CONTENT); /// 1
  lv_obj_set_align(ui_Img_lowsun, LV_ALIGN_TOP_LEFT);

  // High Sunlight icon
  lv_obj_t *ui_Img_highsun = lv_img_create(ui_Panel46);
  lv_img_set_src(ui_Img_highsun, &icon_highsunlight);
  lv_obj_set_size(ui_Img_highsun, LV_SIZE_CONTENT, LV_SIZE_CONTENT); /// 1
  lv_obj_set_align(ui_Img_highsun, LV_ALIGN_TOP_RIGHT);

  // Container for Adaptive Lighting
  lv_obj_t *ui_Panel47 = lv_obj_create(cont_content);
  lv_obj_set_size(ui_Panel47, 448, 57);
  lv_obj_set_style_border_color(ui_Panel47, lv_color_hex(0xD9D9D9), LV_PART_MAIN);
  lv_obj_set_style_border_width(ui_Panel47, 1, LV_PART_MAIN);
  lv_obj_set_style_radius(ui_Panel47, 10, LV_PART_MAIN);

  lv_obj_set_style_shadow_color(ui_Panel47, lv_color_hex(0x000000), LV_PART_MAIN); // black shadow
  lv_obj_set_style_shadow_opa(ui_Panel47, 64, LV_PART_MAIN);                       // 25% opacity
  lv_obj_set_style_shadow_width(ui_Panel47, 6, LV_PART_MAIN);                      // shadow width
  lv_obj_set_style_shadow_spread(ui_Panel47, 0, LV_PART_MAIN);                     // shadow spread
  lv_obj_set_style_shadow_ofs_x(ui_Panel47, 0, LV_PART_MAIN); // shadow offset x
  lv_obj_set_style_shadow_ofs_y(ui_Panel47, 3, LV_PART_MAIN); // shadow offset y

  lv_obj_set_align(ui_Panel47, LV_ALIGN_CENTER);
  lv_obj_clear_flag(ui_Panel47, LV_OBJ_FLAG_SCROLLABLE); /// Flags

  // Adaptive Lighting label
  lv_obj_t *ui_Label42 = lv_label_create(ui_Panel47);
  lv_obj_set_width(ui_Label42, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_height(ui_Label42, LV_SIZE_CONTENT); /// 1
  lv_obj_align(ui_Label42, LV_ALIGN_LEFT_MID, 0, 3);
  lv_label_set_text(ui_Label42, "Adaptive Lighting");
  lv_obj_set_style_text_font(ui_Label42, &opensans_semibold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_color(ui_Label42, lv_color_hex(VST_COLOR_PRIMARY_TEXT),
                              LV_PART_MAIN | LV_STATE_DEFAULT);

  // Adaptive Lighting Enable/Disable switch
  switch_abc = lv_switch_create(ui_Panel47);
  lv_obj_set_width(switch_abc, 61);  // switch width
  lv_obj_set_height(switch_abc, 28); // switch height
  lv_obj_set_align(switch_abc, LV_ALIGN_RIGHT_MID);

  lv_obj_set_style_outline_color(switch_abc, lv_color_hex(VST_COLOR_CONTAINER_BORDER),
                                 LV_PART_MAIN);                   // light grey outline
  lv_obj_set_style_outline_opa(switch_abc, 255, LV_PART_MAIN);    // fully opaque outline
  lv_obj_set_style_outline_width(switch_abc, 2.64, LV_PART_MAIN); // outline width
  lv_obj_set_style_outline_pad(switch_abc, 3, LV_PART_MAIN);      // outline padding

  // Switch styles
  lv_obj_set_style_pad_all(switch_abc, 0, LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(switch_abc, lv_color_hex(0xFFFFFF),
                            LV_PART_INDICATOR);                // white background
  lv_obj_set_style_bg_opa(switch_abc, 255, LV_PART_INDICATOR); // fully opaque
  lv_obj_set_style_bg_color(switch_abc,
                            lv_color_hex(0xBBE4CB), // light green background when checked
                            LV_PART_INDICATOR | LV_STATE_CHECKED);
  lv_obj_set_style_bg_opa(switch_abc, 255,
                          LV_PART_INDICATOR | LV_STATE_CHECKED); // fully opaque when checked

  lv_obj_set_style_pad_all(switch_abc, 0, LV_PART_KNOB); // No padding for knob
  lv_obj_set_style_bg_color(switch_abc, lv_color_hex(0x707070), LV_PART_KNOB); // dark grey knob
  lv_obj_set_style_bg_opa(switch_abc, 255, LV_PART_KNOB);                      // fully opaque knob
  lv_obj_set_style_bg_color(switch_abc, lv_color_hex(0x218448),
                            LV_PART_KNOB | LV_STATE_CHECKED); // dark green knob when checked
  lv_obj_set_style_bg_opa(switch_abc, 255,
                          LV_PART_KNOB | LV_STATE_CHECKED); // fully opaque when checked

  btn_save = btn_save_create_1(screen);
  lv_obj_add_flag(btn_save, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_set_align(btn_save, LV_ALIGN_BOTTOM_MID);
  lv_obj_set_pos(btn_save, 0, -16);
  lv_obj_set_size(btn_save, 448, 80);

#if SENSCAP_EN
  lv_obj_add_state(switch_abc, LV_STATE_DISABLED);
  lv_obj_clear_flag(switch_abc, LV_OBJ_FLAG_CLICKABLE); // disables click handling

  lv_obj_add_state(btn_save, LV_STATE_DISABLED);
  lv_obj_clear_flag(btn_save, LV_OBJ_FLAG_CLICKABLE); // disables click handling
#else
  lv_obj_add_event_cb(switch_abc, switch_abc_toggle_cb, LV_EVENT_VALUE_CHANGED, NULL);
  lv_obj_add_event_cb(btn_save, btn_save_click_cb, LV_EVENT_CLICKED, NULL);
#endif

  lv_obj_add_event_cb(slider_brightness, slider_brightness_value_changed_cb, LV_EVENT_VALUE_CHANGED,
                      NULL);
}

lv_obj_t *screen_settings_display_get() {
  if (screen == NULL) {
    screen_init();
  }
  return screen;
}

// to handle abc value in set_state command
void update_display_brightness_settings() {

  if (screen == NULL) {
    screen_init();
  }
#if (SIMULATOR == 0) && (SENSCAP_EN == 0)
  if (flg_alert2d_display == false) {
    const sys_state_t *ss = sys_state_get(); // get the current system state

    if (ss->abc == ABC_ON)                   //  abc = 1 in sys_state
    {
      if (flg_als_Enable == false) // if als is not turned on
      {
        als_on(); // turn on ALS
        flg_als_Enable = true;
        flg_process_als = true; // process als from ui.ino
        // disp_state_set_abc(ABC_ON); // update the state of ABC into structure
        lv_obj_add_state(slider_brightness,
                         LV_STATE_DISABLED); // disable the brightness slider when als is on
        session_data.abc = ABC_ON;
        lv_obj_add_state(switch_abc, LV_STATE_CHECKED);
      }
    } // if abc switch is turned off (unchecked)
    else {
      if (flg_als_Enable == true) // if als is on
      {
        als_off();
        flg_als_Enable = false;  // turn off ALS
        flg_process_als = false; // don't process als from ui.ino
        // get the value from slider and update it into structure
        uint8_t brightValue = lv_slider_get_value(slider_brightness);
        disp_state_set_brightness(brightValue);
        // disp_state_set_abc(ABC_OFF); // update the state of ABC into structure
        lv_obj_clear_state(slider_brightness,
                           LV_STATE_DISABLED); // Enable the brightness slider when als is off
        lv_obj_clear_state(switch_abc, LV_STATE_CHECKED);

        session_data.abc = ABC_OFF;
      }
    }
  }
#endif

  // btn_save_update_status();
}

//Used to turn off als and reset all its flags
void turn_off_als(){
#if (SIMULATOR == 0) && (SENSCAP_EN == 0)
  if (flg_als_Enable == true) // if als is on
  {
    als_off();
    flg_als_Enable = false;  // turn off ALS
    flg_process_als = false; // don't process als from ui.ino
    lv_obj_clear_state(slider_brightness, LV_STATE_DISABLED); // Enable the brightness slider when als is off
    session_data.abc = ABC_OFF;
  }
#endif
}