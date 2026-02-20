#include "device_info.h"
#include "jsonrpc2.h"
#include "resp_state.h"
#include "screens.h"
#include "ui.h"

static lv_obj_t *screen;
static lv_obj_t *lbl_compute_id;
static lv_obj_t *lbl_sw_ver;

char parent_scr_name[50] = {0}; // to store the name of the parent screen

lv_obj_t *get_parent_of_system_info_screen() {
  if (strcmp(parent_scr_name, "settings_bms") == 0) {
    return screen_settings_bms_get();
  } else if (strcmp(parent_scr_name, "settings_audio") == 0) {
    return screen_settings_audio_get();
  } else if (strcmp(parent_scr_name, "settings_home") == 0) {
    return screen_settings_home_get();
  } else if (strcmp(parent_scr_name, "settings_alerts") == 0) {
    return screen_settings_alerts_get();
  } else if (strcmp(parent_scr_name, "settings_exit_alert_schedule") == 0) {
    return screen_settings_exit_alert_sch_get();
  } else if (strcmp(parent_scr_name, "settings_bed_width") == 0) {
    return screen_settings_get_bed_width();
  } else if (strcmp(parent_scr_name, "settings_bed_placement") == 0) {
    return screen_settings_get_bed_placement();
  } else if (strcmp(parent_scr_name, "settings_occupant_size") == 0) {
    return screen_settings_get_occupant_size();
  }

  return NULL;
}

static void btn_close_cb(lv_event_t *e) {
  lv_obj_t *screen = get_parent_of_system_info_screen();
  lv_screen_load(screen);
  return;
}

bool is_current_page_system_info() {
  if (lv_scr_act() == screen) {
    return true;
  }
  return false;
}

static void screen_load_cb(lv_event_t *e) {
  // get the compute id and sw_ver from resp_session_data structure
  static resp_session_data_t *resp_session_data = resp_session_data_get();
  lv_label_set_text(lbl_compute_id, resp_session_data->compute_id);
  lv_label_set_text(lbl_sw_ver, resp_session_data->sw_ver);
  // lv_label_set_text(lbl_sw_ver, "v2025-24Aug-25-46066775-dc4ff6be-nuc");
  // lv_label_set_text(lbl_compute_id, "00000123");
}

static lv_obj_t *create_labels_container(lv_obj_t *parent, char *main_title) {
  lv_obj_t *cont_labels = lv_obj_create(parent);
  lv_obj_set_size(cont_labels, lv_pct(100), 56);
  lv_obj_set_style_border_width(cont_labels, 0, LV_PART_MAIN);
  // lv_obj_set_flex_flow(cont_labels, LV_FLEX_FLOW_COLUMN);
  // lv_obj_set_flex_align(cont_labels, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
  //                       LV_FLEX_ALIGN_CENTER);
  lv_obj_clear_flag(cont_labels, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_pad_all(cont_labels, 2, LV_PART_MAIN);

  lv_obj_t *lbl_title = lv_label_create(cont_labels);
  lv_label_set_text(lbl_title, main_title);
  lv_obj_set_size(lbl_title, LV_SIZE_CONTENT, LV_SIZE_CONTENT); /// 1
  lv_obj_set_style_text_color(lbl_title, lv_color_hex(VST_COLOR_INACTIVE_TEXT), LV_PART_MAIN);
  lv_obj_set_style_text_font(lbl_title, &opensans_semibold_20, LV_PART_MAIN);
  lv_obj_add_flag(lbl_title, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_set_align(lbl_title, LV_ALIGN_TOP_LEFT); // Align to top left with offset

  return cont_labels;
}

static void add_label_style(lv_obj_t *label) {
  lv_obj_set_size(label, LV_SIZE_CONTENT, LV_SIZE_CONTENT); /// 1
  lv_obj_set_style_text_color(label, lv_color_hex(VST_COLOR_INACTIVE_TEXT), LV_PART_MAIN);
  lv_obj_set_style_text_font(label, &opensans_regular_20, LV_PART_MAIN);
  lv_obj_add_flag(label, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_set_align(label, LV_ALIGN_BOTTOM_LEFT); // Align to top left with offset
}

static void screen_init() {
  screen = tmpl_settings_create_1(NULL, "System Info", false);
  lv_obj_set_style_bg_color(screen, lv_color_hex(VST_COLOR_SCREEN_BG_GREY), LV_PART_MAIN);

  lv_obj_add_event_cb(screen, screen_load_cb, LV_EVENT_SCREEN_LOAD_START, NULL);

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
  lv_obj_t *cont_controls_spacing = lv_obj_create(cont_content);
  lv_obj_set_size(cont_controls_spacing, lv_pct(100), 248);
  lv_obj_set_flex_flow(cont_controls_spacing, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(cont_controls_spacing, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);
  lv_obj_clear_flag(cont_controls_spacing, LV_OBJ_FLAG_SCROLLABLE); /// Flags

  lv_obj_set_style_pad_row(cont_controls_spacing, 16, LV_PART_MAIN);
  lv_obj_set_style_border_width(cont_controls_spacing, 1, LV_PART_MAIN);
  lv_obj_set_style_pad_all(cont_controls_spacing, 24, LV_PART_MAIN);
  lv_obj_set_style_bg_color(cont_controls_spacing, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_radius(cont_controls_spacing, 7, LV_PART_MAIN); // Rounded corners
  // shadow
  lv_obj_set_style_shadow_width(cont_controls_spacing, 2, 0);
  lv_obj_set_style_shadow_color(cont_controls_spacing, lv_color_hex(0x000000), 0);
  lv_obj_set_style_shadow_ofs_y(cont_controls_spacing, 2, 0);
  lv_obj_set_style_shadow_ofs_x(cont_controls_spacing, 2, 0);
  lv_obj_set_style_shadow_opa(cont_controls_spacing, 64, 0);

  // compute id container
  lv_obj_t *cont_compute_id = create_labels_container(cont_controls_spacing, (char *)"Compute ID");

  lbl_compute_id = lv_label_create(cont_compute_id);
  lv_label_set_text(lbl_compute_id, "00000622");
  add_label_style(lbl_compute_id); // add label styles

  // software version container
  lv_obj_t *cont_sw_ver =
      create_labels_container(cont_controls_spacing, (char *)"Software Version");

  lbl_sw_ver = lv_label_create(cont_sw_ver);
  lv_label_set_text(lbl_sw_ver, "v2025-02Feb-24-46066775-dc4ff6be-nuc");
  add_label_style(lbl_sw_ver); // add label styles

  // firmware version container
  lv_obj_t *cont_fw_ver =
      create_labels_container(cont_controls_spacing, (char *)"Firmware Version");
  lv_obj_t *lbl_fw_ver = lv_label_create(cont_fw_ver);

  // get firmware version from device info structure
  dev_info_t *dev_info = get_dev_info(); // returns the device info structure pointer
  lv_label_set_text(lbl_fw_ver, dev_info->fwVer);

  add_label_style(lbl_fw_ver); // add label styles

  lv_obj_t *btn_close = lv_btn_create(cont_content);
  lv_obj_add_flag(btn_close, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_set_size(btn_close, lv_pct(100), 80);         /// 1
  lv_obj_set_style_radius(btn_close, 5, LV_PART_MAIN); // Rounded corners
  lv_obj_set_style_bg_color(btn_close, lv_color_hex(VST_COLOR_DARKBLUE), LV_PART_MAIN);
  lv_obj_set_align(btn_close, LV_ALIGN_BOTTOM_MID); // Align to bottom center with offset

  lv_obj_t *label_close = lv_label_create(btn_close);
  lv_label_set_text(label_close, "Close");
  lv_obj_set_size(label_close, LV_SIZE_CONTENT, LV_SIZE_CONTENT); /// 1
  lv_obj_set_style_text_color(label_close, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_text_font(label_close, &opensans_bold_20, LV_PART_MAIN);
  lv_obj_align(label_close, LV_ALIGN_CENTER, 0, 0); // Center the label in the button

  lv_obj_add_event_cb(btn_close, btn_close_cb, LV_EVENT_CLICKED, NULL);
}

// // return screen object
// lv_obj_t *screen_system_info_get() {
//   if (screen == NULL) {
//     screen_init();
//   }
//   return screen;
// }

static void send_sys_info_request() {
  lv_obj_t *parent_screen = get_parent_of_system_info_screen();
  create_spinner_screen(parent_screen);
  static resp_session_data_t *resp_session_data = resp_session_data_get();
  resp_method_clear(resp_session_data);
  strcpy(resp_session_data->response_method, cmd_str[CMD_GET_SYS_INFO]);
  resp_session_data->resp_id++;

  JsonObject params;

  serializeJsonRpcRequest(resp_session_data->resp_id, CMD_GET_SYS_INFO, params);
}

void show_system_info_screen(char *parent_screen_name) {
  strcpy(parent_scr_name, parent_screen_name); // store the parent screen name
  static resp_session_data_t *resp_session_data = resp_session_data_get();

  if (screen == NULL) {
    screen_init();
  }
  // clear the compute_id and sw_ver in resp_session_data struct upon initialization of the screen
  memset(resp_session_data->compute_id, '\0', sizeof(resp_session_data->compute_id));
  memset(resp_session_data->sw_ver, '\0', sizeof(resp_session_data->sw_ver));
  // if(strcmp(resp_session_data->compute_id, "") == 0 || strcmp(resp_session_data->sw_ver, "") ==
  // 0){ if compute_id or sw_ver is empty, then we need to fetch the data from the device
  send_sys_info_request();
  //   return;
  // }
  // lv_screen_load(screen);
  return;
}

lv_obj_t *screen_system_info_get() { return screen; }