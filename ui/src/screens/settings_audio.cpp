#include "jsonrpc2.h"
#include "screens.h"
#include "sys_state.h"
#include "ui.h"

#include "resp_state.h"

//
// Section: Constants
//

char *selected_lang_text;
lv_obj_t *cont_content;
bool flg_SaveClk = false;

static const char *labels[] = {
    "Low",
    "Medium",
    "High",
    NULL,
};

//
// Section: Variables
//

// root
static lv_obj_t *screen;

// ui components
static lv_obj_t *switch_audio, *selector_vol;
static lv_obj_t *cont_lang_vol;
static lv_obj_t *btn_save, *btn_activate, *cont_btns_nav;
static lv_obj_t *title_cont_left_arrow_obj;
static lv_obj_t *title_cont_info_icon_obj;
static lv_obj_t *cont_change_language;
static lv_obj_t *selected_lang_lbl;
bool flag_lang_selection = false;
static SETTINGS_MODE settings_mode;
static lv_obj_t *spinner;


typedef struct {
  uint16_t fts_state; // bit 0 - bed_alt, bit 1 - fall_alt, bit 2 - rep_alt
  char mon_start[5];  // "2130"
  char mon_end[5];    // "0700"
  BMS bms;
  uint8_t bed_wid; // in inch
  BED_POS bed_pos; // 1 - 3
  OCC_SIZE occ_size;
  AUDIO audio;
  char lang[21];
  VOLUME vol;

} session_data_t;

static session_data_t session_data = {
    .fts_state = 0,
    .mon_start = "2130",
    .mon_end = "0700",
    .bms = BMS_UNSET,
    .bed_wid = 44,
    .bed_pos = BED_POS_UNSET,
    .occ_size = OCC_SIZE_UNSET,
    .audio = AUDIO_UNSET,
    // .lang = "",
    .vol = VOL_UNSET,

};

static session_data_t sd_load; // holds data that is present during load time.

//
// Section: Control functions
//

void clear_audio_sessiondata() {
  session_data.fts_state = 0;
  memset(session_data.mon_start, '\0', sizeof(session_data.mon_start));
  memset(session_data.mon_end, '\0', sizeof(session_data.mon_end));
  session_data.bms = BMS_UNSET;
  session_data.bed_wid = 44;
  session_data.bed_pos = BED_POS_UNSET;
  session_data.occ_size = OCC_SIZE_UNSET;
  session_data.audio = AUDIO_UNSET;
  strcpy(session_data.lang, "");
  session_data.vol = VOL_UNSET;
}

static void jsonobj_audio_prepare(JsonObject params, session_data_t *data) {
  if (data->audio != AUDIO_UNSET) {
    params["audio"].set(data->audio);
  }

  if (data->audio == AUDIO_ON) {
    if (strcmp(data->lang, "") != 0) {
      params["lang"].set(data->lang);
    }

    if (data->vol != VOL_UNSET) {
      params["vol"].set(data->vol);
    }
  }
}

// Create a JSON-RPC request
static void jsonrpc_save_request_send(session_data_t *data) {
  //  const size_t capacity = 300; // Adjust this based on your needs
  JsonDocument doc;

  JsonObject params = doc["params"].to<JsonObject>();

  jsonobj_audio_prepare(params, data);

  serializeJsonRpcRequest(0, CMD_SAVE_AUDIO, params);
}

static void jsonrpc_activate_request_send(session_data_t *data) {
  //  const size_t capacity = 300; // Adjust this based on your needs
  JsonDocument doc;

  JsonObject params = doc["params"].to<JsonObject>();

  // params["bed_alt"].set(data->bed_alt ? 1 : 0); // for getting as 1/0 instead of true/false
  // params["fall_alt"].set(data->fall_alt ? 1 : 0);
  // params["rep_alt"].set(data->rep_alt ? 1 : 0);
  params["fts_state"].set(data->fts_state);
  params["mon_start"].set(data->mon_start);
  params["mon_end"].set(data->mon_end);
  params["bms"].set(data->bms);
  params["bed_wid"].set(data->bed_wid);
  params["bed_pos"].set(data->bed_pos);
  params["occ_size"].set(data->occ_size);

  jsonobj_audio_prepare(params, data);

  serializeJsonRpcRequest(0, CMD_ACTIVATE_SENSOR, params);
}

static bool is_session_data_dirty() {
  session_data_t *sd = &session_data;
  // const sys_state_t *ss = sys_state_get();
  if (sd->audio != sd_load.audio) {
    return true;
  }

  if (sd->audio == AUDIO_ON) {
    if (strcmp(sd->lang, sd_load.lang) != 0) {
      return true;
    }
    if (sd->vol != sd_load.vol) {
      return true;
    }
  }
  return false;
}

static void widgets_set_clickable(bool clickable) {
  if (clickable) {
    lv_obj_add_flag(switch_audio, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(cont_change_language, LV_OBJ_FLAG_CLICKABLE);
    // lv_obj_add_flag(dropdown_lang, LV_OBJ_FLAG_CLICKABLE);
  } else {
    lv_obj_clear_flag(switch_audio, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(cont_change_language, LV_OBJ_FLAG_CLICKABLE);
    // lv_obj_clear_flag(dropdown_lang, LV_OBJ_FLAG_CLICKABLE);
  }
  radio_selector_set_clickable(selector_vol, clickable);
  tmpl_settings_label_set_clickable(screen, clickable);
}

// static bool is_session_data_set(session_data_t *sd) {
//   if (sd->audio == AUDIO_UNSET || sd->vol == VOL_UNSET) {
//     return false;
//   }
//   return true;
// }

// static bool switch_audio_get_value() {}
// static LANG dropdown_lang_get_value() {}
// static VOLUME selector_vol_get_value() {}

static void switch_audio_set_value(AUDIO audio) {
  if (audio == AUDIO_ON) {
    lv_obj_add_state(switch_audio, LV_STATE_CHECKED);
    lv_obj_clear_flag(cont_lang_vol, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_clear_state(switch_audio, LV_STATE_CHECKED);
    lv_obj_add_flag(cont_lang_vol, LV_OBJ_FLAG_HIDDEN);
  }
}

static void selector_vol_set_value(VOLUME vol) {
  int8_t btn_id = vol - 1;
  if (vol == VOL_UNSET) {
    btn_id = vol;
  }
  radio_selector_set_selected_btn_1(selector_vol, btn_id);
}

// static void cont_lang_vol_show(bool show) {}

// static void screen_widgets_add_clickable() {}
// static void screen_widgets_clear_clickable() {}

// void settings_audio_set_lang(LANG lang) { session_data.lang = lang; }

// LANG settings_audio_get_lang() { return session_data.lang; }

static void update_language_selection(char *lang) {
  if (flag_lang_selection) {
    lang_info_t *lang_info = lang_info_get();
    if(strcmp(lang_info->selected_language, "") != 0) {
      strcpy(session_data.lang, lang_info->selected_language);
    }
  }
  lv_label_set_text(selected_lang_lbl, session_data.lang);
}

static void set_widgets(session_data_t *sess) {
  switch_audio_set_value(sess->audio);
  selector_vol_set_value(sess->vol);
  update_language_selection(sess->lang);
}

static void btn_save_update_status() {
  if (settings_mode == SETTINGS_MODE_SAVE && is_session_data_dirty()) {
    btn_set_state(btn_save, BTN_STATE_ACTIVE);
  } else {
    btn_set_state(btn_save, BTN_STATE_DISABLED);
  }
}

char *get_selected_language() { return session_data.lang; }

// //
// // Section: Event Handlers
// //

static void screen_load_cb(lv_event_t *e) {
  settings_mode = screen_settings_get_mode();
  settings_show_mode(screen, btn_save, cont_btns_nav, settings_mode);
  widgets_set_clickable(true);

  if (settings_mode == SETTINGS_MODE_ACTIVATION) {
    btn_set_state(btn_activate, BTN_STATE_ACTIVE);
  }

  session_data_t *sd = &session_data;
  if (settings_mode == SETTINGS_MODE_ACTIVATION) {
    // show the header icon symbol and hide the left arrow
    lv_obj_add_flag(title_cont_left_arrow_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(title_cont_info_icon_obj, LV_OBJ_FLAG_HIDDEN);

    if (sd->audio == AUDIO_UNSET) {
      sd->audio = AUDIO_DEFAULT;
      strcpy(sd->lang, "English");
      sd->vol = VOL_DEFAULT;
    }
  } else if (settings_mode == SETTINGS_MODE_SAVE) {
    // show the header left arrow and hide the icon symbol
    lv_obj_clear_flag(title_cont_left_arrow_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(title_cont_info_icon_obj, LV_OBJ_FLAG_HIDDEN);
    const sys_state_t *ss = sys_state_get();

    if (ss->audio == AUDIO_UNSET) {
      sd->audio = AUDIO_OFF;
      sd_load.audio = AUDIO_OFF;
    } else {
      if (!flag_lang_selection) {
        sd->audio = ss->audio;
      }
      sd_load.audio = ss->audio;
    }

    if (strcmp(ss->lang, "") == 0) {
      strcpy(sd->lang, "English");
      strcpy(sd_load.lang, "English");
    } else {
      if (!flag_lang_selection) {
        strcpy(sd->lang, ss->lang);
      }
      strcpy(sd_load.lang, ss->lang);
    }

    if (ss->vol == VOL_UNSET) {
      sd->vol = VOL_DEFAULT;
      sd_load.vol = VOL_DEFAULT;
    } else {
      if (!flag_lang_selection) { // to avoid overwriting vol when returning from language selection
                                  // screen
        sd->vol = ss->vol;
      }
      sd_load.vol = ss->vol;
    }
  }

  set_widgets(&session_data);
  if(flag_lang_selection && lv_obj_is_valid(spinner)) {
    lv_obj_add_flag(spinner, LV_OBJ_FLAG_HIDDEN);
  }
  flag_lang_selection = false;
  btn_save_update_status();
  if (!is_alert_toast_hidden()) {
    set_alert_toast_parent(screen);
  }
}

static void mbtn_event_cb(lv_event_t *e) {
  lv_obj_t *btn = lv_event_get_current_target_obj(e);
  lv_obj_t *msgbox = lv_obj_get_parent(lv_obj_get_parent(btn));
  lv_obj_t *label = lv_obj_get_child(btn, 0);
  // LV_LOG_USER("Button %s clicked", lv_label_get_text(label));
  const char *btn_text = lv_label_get_text(label);
  if (strcmp(btn_text, "Don't Save") == 0) {
    lv_msgbox_close(msgbox);
    lv_screen_load(screen_settings_home_get());
  } else if (strcmp(btn_text, "Save") == 0) {
    lv_msgbox_close(msgbox);
    jsonrpc_save_request_send(&session_data);
    btn_set_state(btn_save, BTN_STATE_SUBMITTING);
    widgets_set_clickable(false);
  }
}

static void left_arrow_click_cb(lv_event_t *e) {
  settings_mode = screen_settings_get_mode();
  if (settings_mode == SETTINGS_MODE_SAVE) {
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
    lv_screen_load(screen_settings_bms_get());
  }
}

static void btn_prev_click_cb(lv_event_t *e) {
  sys_state_t *ss = sys_state_get();
  if (ss->fts_avail.occ_size) {
    lv_obj_t *screen = screen_settings_get_occupant_size();
    lv_screen_load(screen);
  } else if (ss->fts_avail.bed_pos) {
    lv_obj_t *screen = screen_settings_get_bed_placement();
    lv_screen_load(screen);
  } else if (ss->fts_avail.bed_wid) {
    lv_obj_t *screen = screen_settings_get_bed_width();
    lv_screen_load(screen);
  } else {
    lv_obj_t *screen = screen_settings_bms_get();
    lv_screen_load(screen);
  }
}

static void btn_save_click_cb(lv_event_t *e) {
  // flg_SaveClk = true;
  jsonrpc_save_request_send(&session_data);

  lv_obj_t *btn_save = lv_event_get_target_obj(e);
  btn_set_state(btn_save, BTN_STATE_SUBMITTING);
  widgets_set_clickable(false);
}

static void switch_audio_toggle_cb(lv_event_t *e) {
  lv_obj_t *switch_audio = lv_event_get_target_obj(e);
  bool on = lv_obj_has_state(switch_audio, LV_STATE_CHECKED);
  if (on) {
    lv_obj_clear_flag(cont_lang_vol, LV_OBJ_FLAG_HIDDEN);
    session_data.audio = AUDIO_ON;
  } else {
    lv_obj_add_flag(cont_lang_vol, LV_OBJ_FLAG_HIDDEN);
    session_data.audio = AUDIO_OFF;
  }
  btn_save_update_status();
}

VOLUME get_vol_level(lv_obj_t *obj) { return (VOLUME)(radio_selector_get_selected_btn_1(obj) + 1); }

static void btn_radio_click_cb(lv_event_t *e) {
  session_data.vol = get_vol_level(selector_vol);
  btn_save_update_status();
}

static void btn_activate_click_cb(lv_event_t *e) {
  jsonrpc_activate_request_send(&session_data);
  lv_obj_t *btn_activate = lv_event_get_target_obj(e);
  btn_set_state(btn_activate, BTN_STATE_SUBMITTING);
}

static void request_timeout_cb(lv_event_t *e) {

  // if(flg_SaveClk == true)
  {
    if (settings_mode == SETTINGS_MODE_SAVE) {
      btn_set_state(btn_save, BTN_STATE_ACTIVE);
    } else {
      btn_set_state(btn_activate, BTN_STATE_ACTIVE);
    }
    widgets_set_clickable(true);
    toast_show(screen, no_response_str);
  }
  //  flg_SaveClk = false;
}

void handle_language_change_timeout() {
  lang_info_t *lang_info = lang_info_get(); // Get the language info structure
  lang_info_clear(lang_info);               // Clear any previous language data

  lang_info->available_language_count = 2;              // Set the number of available languages
  strcpy(lang_info->available_languages[0], "English"); // Add "English" as the first language
  strcpy(lang_info->available_languages[1], "Spanish"); // Add "Spanish" as the second language

  strcpy(lang_info->selected_language,
         "English");                      // Set "English" as the currently selected language
  lang_info->selected_language_index = 0; // Set the selected language index to 0 (first language)

  lv_obj_t *screen =
      get_audio_language_settings_screen(); // Get the language selection screen object
  lv_scr_load(screen);                      // Load the language selection screen
}

static void set_state_fail_cb(lv_event_t *e) {
  // if(flg_SaveClk == true)
  {
    if (settings_mode == SETTINGS_MODE_SAVE) {
      btn_set_state(btn_save, BTN_STATE_ACTIVE);
    } else {
      btn_set_state(btn_activate, BTN_STATE_ACTIVE);
    }
    widgets_set_clickable(true);
    toast_show(screen, invalid_response_str);
  }
  // flg_SaveClk = false;
}

static void set_state_ok_cb(lv_event_t *e) {
  // if(flg_SaveClk == true)
  {
    if (settings_mode == SETTINGS_MODE_SAVE) {
      btn_set_state(btn_save, BTN_STATE_SUBMITTED);
      // update load time session data with latest session data after save is successful
      const sys_state_t *ss = sys_state_get();
      sd_load.bms = ss->bms;
      sd_load.audio = ss->audio;
      strcpy(sd_load.lang, ss->lang);
      sd_load.vol = ss->vol;
    } else {
      btn_set_state(btn_activate, BTN_STATE_ACTIVE);
    }
    widgets_set_clickable(true);
  }
  //  flg_SaveClk = false;
}

static void send_get_languages_request() {
  static resp_session_data_t *resp_session_data = resp_session_data_get();
  resp_method_clear(resp_session_data);
  strcpy(resp_session_data->response_method, cmd_str[CMD_GET_LANGUAGES]);
  resp_session_data->resp_id++;

  JsonObject params;

  serializeJsonRpcRequest(resp_session_data->resp_id, CMD_GET_LANGUAGES, params);
}

static void change_language_req_timeout(lv_timer_t *timer){
  lv_timer_del(timer);
  if(lv_obj_is_valid(spinner)) {
    lv_obj_add_flag(spinner, LV_OBJ_FLAG_HIDDEN);
  }
}

void change_language_event_cb(lv_event_t *e) {
  flag_lang_selection = true;
  send_get_languages_request();
  if(spinner == NULL){
    spinner = lv_spinner_create(cont_change_language);
    lv_spinner_set_anim_params(spinner, 1000, 90);
    lv_obj_set_width(spinner, 18);
    lv_obj_set_height(spinner, 18);
    lv_obj_set_style_arc_width(spinner, 4, LV_PART_MAIN);
    lv_obj_set_style_arc_width(spinner, 4, LV_PART_INDICATOR);
    lv_obj_align(spinner, LV_ALIGN_RIGHT_MID, 16, 0);
  } else{
    lv_obj_clear_flag(spinner, LV_OBJ_FLAG_HIDDEN);
  }
  lv_timer_t *timer = lv_timer_create(change_language_req_timeout, 3000, NULL);
}

// show information page
static void info_icon_click_cb(lv_event_t *e) { show_system_info_screen((char *)"settings_audio"); }

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

  //  lv_obj_add_event_cb(screen, label_back_click_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(screen, screen_load_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
  lv_obj_add_event_cb(screen, request_timeout_cb, (lv_event_code_t)MY_EVENT_REQUEST_TIMEOUT, NULL);
  lv_obj_add_event_cb(screen, set_state_ok_cb, (lv_event_code_t)MY_EVENT_SET_STATE_OK, NULL);
  lv_obj_add_event_cb(screen, set_state_fail_cb, (lv_event_code_t)MY_EVENT_SET_STATE_FAIL, NULL);

  // add content container
  cont_content = lv_obj_create(screen);
  lv_obj_set_width(cont_content, lv_pct(100));
  lv_obj_set_flex_grow(cont_content, 1);
  lv_obj_set_flex_flow(cont_content, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(cont_content, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);
  lv_obj_clear_flag(cont_content, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_border_width(cont_content, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_top(
      cont_content, 0,
      LV_PART_MAIN); // for removing the gap between main container and topside label
  lv_obj_set_style_bg_color(cont_content, lv_color_hex(VST_COLOR_SCREEN_BG_GREY),
                            LV_PART_MAIN); // light grey background

  // add container for controls
  lv_obj_t *cont_controls = lv_obj_create(cont_content);
  lv_obj_set_width(cont_controls, lv_pct(100));
  lv_obj_set_flex_grow(cont_controls, 1);
  lv_obj_clear_flag(cont_controls, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_border_width(cont_controls, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_top(cont_controls, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_left(cont_controls, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_right(cont_controls, 0, LV_PART_MAIN);
  lv_obj_set_style_bg_color(cont_controls, lv_color_hex(VST_COLOR_SCREEN_BG_GREY),
                            LV_PART_MAIN); // light grey background

  // add audio on / off control container
  lv_obj_t *cont_switch_audio = lv_obj_create(cont_controls);
  lv_obj_set_width(cont_switch_audio, lv_pct(100));      // 448
  lv_obj_set_height(cont_switch_audio, LV_SIZE_CONTENT); // 65
  lv_obj_set_align(cont_switch_audio, LV_ALIGN_TOP_MID);
  lv_obj_clear_flag(cont_switch_audio, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_radius(cont_switch_audio, 10, LV_PART_MAIN);
  lv_obj_set_style_border_width(cont_switch_audio, 1, LV_PART_MAIN);
  lv_obj_set_style_border_color(cont_switch_audio, lv_color_hex(0xD9D9D9), LV_PART_MAIN);

  lv_obj_set_style_shadow_color(cont_switch_audio, lv_color_hex(0x000000),
                                LV_PART_MAIN);                          // shadow color
  lv_obj_set_style_shadow_width(cont_switch_audio, 6, LV_PART_MAIN);    // shadow width
  lv_obj_set_style_shadow_spread(cont_switch_audio, 0, LV_PART_MAIN);   // shadow spread
  lv_obj_set_style_shadow_opa(cont_switch_audio, 64, LV_PART_MAIN);     // shadow opacity
  lv_obj_set_style_shadow_offset_x(cont_switch_audio, 0, LV_PART_MAIN); //  shadow offset x
  lv_obj_set_style_shadow_offset_y(cont_switch_audio, 3, LV_PART_MAIN); // shadow offset y

  lv_obj_t *ui_Label8 = lv_label_create(cont_switch_audio);
  lv_obj_set_width(ui_Label8, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_height(ui_Label8, LV_SIZE_CONTENT); /// 1
  lv_obj_set_align(ui_Label8, LV_ALIGN_LEFT_MID);
  lv_label_set_text(ui_Label8, "In-Room Alert Audio");
  lv_obj_set_style_text_font(ui_Label8, &opensans_semibold_18, LV_PART_MAIN);

  switch_audio = lv_switch_create(cont_switch_audio);
  lv_obj_set_width(switch_audio, 46);  // width of the switch
  lv_obj_set_height(switch_audio, 21); // height of the switch
  lv_obj_set_align(switch_audio, LV_ALIGN_RIGHT_MID);

  lv_obj_set_style_outline_color(switch_audio, lv_color_hex(VST_COLOR_CONTAINER_BORDER),
                                 LV_PART_MAIN); // border color
  lv_obj_set_style_outline_opa(switch_audio, 255, LV_PART_MAIN);
  lv_obj_set_style_outline_width(switch_audio, 2, LV_PART_MAIN);
  lv_obj_set_style_outline_pad(switch_audio, 2, LV_PART_MAIN);

  lv_obj_set_style_pad_all(switch_audio, 0, LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(switch_audio, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR); // bg color
  lv_obj_set_style_bg_opa(switch_audio, 255, LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(switch_audio, lv_color_hex(0xBBE4CB),
                            LV_PART_INDICATOR | LV_STATE_CHECKED); // bg color when checked
  lv_obj_set_style_bg_opa(switch_audio, 255, LV_PART_INDICATOR | LV_STATE_CHECKED);

  lv_obj_set_style_pad_all(switch_audio, 0, LV_PART_KNOB);
  lv_obj_set_style_bg_color(switch_audio, lv_color_hex(0x707070), LV_PART_KNOB); // knob color
  lv_obj_set_style_bg_opa(switch_audio, 255, LV_PART_KNOB);
  lv_obj_set_style_bg_color(switch_audio, lv_color_hex(0x218448),
                            LV_PART_KNOB | LV_STATE_CHECKED); // knob color when checked
  lv_obj_set_style_bg_opa(switch_audio, 255, LV_PART_KNOB | LV_STATE_CHECKED);

  lv_obj_add_event_cb(switch_audio, switch_audio_toggle_cb, LV_EVENT_VALUE_CHANGED, NULL);

  // add language, volume control container
  cont_lang_vol = lv_obj_create(cont_controls);
  lv_obj_set_width(cont_lang_vol, lv_pct(100)); // 448
  lv_obj_set_height(cont_lang_vol, lv_pct(84)); // 203
  lv_obj_set_flex_flow(cont_lang_vol, LV_FLEX_FLOW_COLUMN);
  lv_obj_clear_flag(cont_lang_vol, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_pad_all(cont_lang_vol, 16, LV_PART_MAIN);
  lv_obj_set_style_border_color(cont_lang_vol, lv_color_hex(0xD9D9D9), LV_PART_MAIN);
  lv_obj_set_style_border_width(cont_lang_vol, 1, LV_PART_MAIN);

  lv_obj_set_style_shadow_color(cont_lang_vol, lv_color_hex(0x000000),
                                LV_PART_MAIN);                      // shadow color
  lv_obj_set_style_shadow_width(cont_lang_vol, 6, LV_PART_MAIN);    // shadow width
  lv_obj_set_style_shadow_spread(cont_lang_vol, 0, LV_PART_MAIN);   // shadow spread
  lv_obj_set_style_shadow_opa(cont_lang_vol, 64, LV_PART_MAIN);     // shadow opacity
  lv_obj_set_style_shadow_offset_x(cont_lang_vol, 0, LV_PART_MAIN); //  shadow offset x
  lv_obj_set_style_shadow_offset_y(cont_lang_vol, 3, LV_PART_MAIN); // shadow offset y

  // add language control container
  lv_obj_t *cont_lang = lv_obj_create(cont_lang_vol);
  lv_obj_set_width(cont_lang, 416);
  lv_obj_set_height(cont_lang, LV_SIZE_CONTENT); /// 1
  lv_obj_set_style_pad_gap(cont_lang, 8,
                           LV_PART_MAIN); // for reducing row aps in the language container
  lv_obj_set_style_pad_all(cont_lang, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_left(cont_lang, 0,
                          LV_PART_MAIN); // for bringing language label downside a bit
  lv_obj_set_style_pad_top(cont_lang, 15, LV_PART_MAIN);
  lv_obj_set_style_border_width(cont_lang, 0, LV_PART_MAIN);
  lv_obj_set_flex_flow(cont_lang, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(cont_lang, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
  lv_obj_clear_flag(cont_lang, LV_OBJ_FLAG_SCROLLABLE); /// Flags

  lv_obj_t *ui_Label7 = lv_label_create(cont_lang);
  lv_obj_set_width(ui_Label7, LV_SIZE_CONTENT);
  lv_obj_set_height(ui_Label7, LV_SIZE_CONTENT); /// 1
  lv_obj_set_align(ui_Label7, LV_ALIGN_LEFT_MID);
  lv_label_set_text(ui_Label7, "Language");
  lv_obj_set_style_text_font(ui_Label7, &opensans_semibold_18, LV_PART_MAIN);

  // Create container that holds selected language and change language button
  lv_obj_t *cont_lang_selection = lv_obj_create(cont_lang);
  lv_obj_set_size(cont_lang_selection, 416, 48);
  lv_obj_set_flex_flow(cont_lang_selection, LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_style_pad_gap(cont_lang_selection, 10, 0); // 10px gap between children
  lv_obj_set_style_pad_all(cont_lang_selection, 0, 0);
  lv_obj_set_style_border_width(cont_lang_selection, 0, 0);
  lv_obj_set_style_radius(cont_lang_selection, 0, 0);

  // Create container for selected language (left side)
  lv_obj_t *cont_selected_lang = lv_obj_create(cont_lang_selection);
  lv_obj_set_scroll_dir(cont_selected_lang, LV_DIR_NONE);
  lv_obj_set_style_width(cont_selected_lang, 203, 0);
  lv_obj_set_style_height(cont_selected_lang, 48, 0);
  lv_obj_set_style_bg_color(cont_selected_lang, lv_color_hex(0xF6F6F6), 0); // Light gray background
  lv_obj_set_style_radius(cont_selected_lang, 5, 0);
  lv_obj_set_style_border_width(cont_selected_lang, 0, 0);
  selected_lang_lbl = lv_label_create(cont_selected_lang);
  lv_label_set_text(selected_lang_lbl, "English");
  lv_obj_set_style_text_font(selected_lang_lbl, &opensans_regular_16, 0);
  lv_obj_set_style_text_color(selected_lang_lbl, lv_color_hex(0x31344E), 0);
  lv_obj_align(selected_lang_lbl, LV_ALIGN_LEFT_MID, 0, 0);

  // Create "Change Language" obj (right side) which take you to language selection screen
  cont_change_language = lv_obj_create(cont_lang_selection);
  lv_obj_set_scroll_dir(cont_change_language, LV_DIR_NONE);
  lv_obj_set_style_width(cont_change_language, 203, 0);
  lv_obj_set_style_height(cont_change_language, 48, 0);
  lv_obj_set_style_radius(cont_change_language, 5, 0);
  lv_obj_set_style_bg_color(cont_change_language, lv_color_hex(0xFFFFFF), 0); // White background
  lv_obj_set_style_border_width(cont_change_language, 3, 0);
  lv_obj_set_style_border_color(cont_change_language, lv_color_hex(0x1A1040),
                                0); // Dark blue border
  lv_obj_set_style_shadow_width(cont_change_language, 2, 0);
  lv_obj_set_style_shadow_color(cont_change_language, lv_color_hex(0x000000), 0);
  lv_obj_set_style_shadow_ofs_y(cont_change_language, 2, 0);
  lv_obj_set_style_shadow_opa(cont_change_language, 64, 0);

  // Add label to cont_change_language
  lv_obj_t *chg_lang_lbl = lv_label_create(cont_change_language);
  lv_label_set_text(chg_lang_lbl, "Change Language");
  lv_obj_set_style_text_font(chg_lang_lbl, &opensans_bold_18, 0);
  lv_obj_set_style_text_color(chg_lang_lbl, lv_color_hex(0x31344E), 0);
  lv_obj_center(chg_lang_lbl);

  // Make cont_change_language clickable and add event handler
  lv_obj_add_flag(cont_change_language, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(cont_change_language, change_language_event_cb, LV_EVENT_CLICKED, NULL);

  // add volume control container
  lv_obj_t *cont_vol = lv_obj_create(cont_lang_vol);
  lv_obj_set_width(cont_vol, lv_pct(100));
  lv_obj_set_height(cont_vol, LV_SIZE_CONTENT);          /// 1
  lv_obj_set_style_pad_gap(cont_vol, 16, LV_PART_MAIN); 
  lv_obj_set_style_pad_all(cont_vol, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_top(cont_vol, 16, LV_PART_MAIN);
  lv_obj_set_style_border_width(cont_vol, 0, LV_PART_MAIN);
  lv_obj_set_flex_flow(cont_vol, LV_FLEX_FLOW_COLUMN);
  // lv_obj_set_flex_align(cont_vol, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_clear_flag(cont_vol, LV_OBJ_FLAG_SCROLLABLE); ///  Flags

  lv_obj_t *ui_Label11 = lv_label_create(cont_vol);
  lv_obj_set_width(ui_Label11, LV_SIZE_CONTENT);
  lv_obj_set_height(ui_Label11, LV_SIZE_CONTENT); /// 1
  lv_obj_align(ui_Label11, LV_ALIGN_TOP_LEFT, 0, 12);
  lv_label_set_text(ui_Label11, "Volume");
  lv_obj_set_style_text_font(ui_Label11, &opensans_semibold_18, LV_PART_MAIN);
  lv_obj_set_style_border_width(ui_Label11, 0, LV_PART_MAIN);

  selector_vol = radio_selector_create_1(cont_vol, labels, NULL);
  lv_obj_add_flag(selector_vol, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_set_pos(selector_vol, 0, 8);
  lv_obj_set_style_border_width(selector_vol, 0, LV_PART_MAIN);

  lv_obj_add_event_cb(selector_vol, btn_radio_click_cb, LV_EVENT_VALUE_CHANGED, NULL);

  // align language/volume control container below audio switch container
  lv_obj_align_to(cont_lang_vol, cont_switch_audio, LV_ALIGN_OUT_BOTTOM_MID, 0, -10);
  lv_obj_move_background(cont_lang_vol);

  // fixme: code duplication (btns)
  // add btn save
  btn_save = btn_save_create_1(cont_content);

  lv_obj_add_event_cb(btn_save, btn_save_click_cb, LV_EVENT_CLICKED, NULL);

  // add nav btns container
  cont_btns_nav = lv_obj_create(cont_content);
  lv_obj_set_size(cont_btns_nav, 448, 80);
  lv_obj_clear_flag(cont_btns_nav, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_border_width(cont_btns_nav, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_left(cont_btns_nav, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_right(cont_btns_nav, 0, LV_PART_MAIN);
  lv_obj_add_flag(cont_btns_nav, LV_OBJ_FLAG_HIDDEN);

  // add previous and next buttons
  lv_obj_t *btn_prev = btn_secondary_create_1(cont_btns_nav, "Back");
  lv_obj_set_align(btn_prev, LV_ALIGN_LEFT_MID);
  lv_obj_add_event_cb(btn_prev, btn_prev_click_cb, LV_EVENT_CLICKED, NULL);

  btn_activate = btn_primary_create_1(cont_btns_nav, "Activate", VST_COLOR_DARKBLUE);
  lv_obj_set_align(btn_activate, LV_ALIGN_RIGHT_MID);
  lv_obj_add_event_cb(btn_activate, btn_activate_click_cb, LV_EVENT_CLICKED, NULL);
}

lv_obj_t *screen_settings_audio_get() {
  if (screen == NULL) {
    screen_init();
  }
  return screen;
}

void screen_settings_audio_set_bms(BMS bms) { session_data.bms = bms; }

void screen_settings_set_alerts_state(bool bed_alt, bool fall_alt, bool rep_alt) {
  // printf("settings_audio: bed_alt: %d, fall_alt: %d, rep_alt: %d\n", bed_alt, fall_alt, rep_alt);
  // combine the three bools into a single uint16_t using bitwise operations as
  // fts_state bit 0 - bed_alt, bit 1 - fall_alt, bit 2 - rep_alt
  session_data.fts_state = (bed_alt ? 1 : 0) | ((fall_alt ? 1 : 0) << 1) | ((rep_alt ? 1 : 0) << 2);
}

void set_language_selection_flag(bool flag) { flag_lang_selection = flag; }

void screen_settings_audio_set_exit_schedule(const char *mon_start, const char *mon_end) {
  strncpy(session_data.mon_start, mon_start, sizeof(session_data.mon_start) - 1);
  strncpy(session_data.mon_end, mon_end, sizeof(session_data.mon_end) - 1);
}

// set bed width in session data
void send_bed_width_session_data(uint8_t bed_width) { session_data.bed_wid = bed_width; }

// set bed placement in session data
void send_bed_placement_session_data(BED_POS bed_position) { session_data.bed_pos = bed_position; }

// set occupant size in session data
void send_occupant_size_session_data(OCC_SIZE occ_size) { session_data.occ_size = occ_size; }
