// clang-format off
#include "screens.h"
#include "ui.h"
#include "resp_state.h"
#include "jsonrpc2.h"
#include "images.h"

// clang-format on
// Carry on formatting

// Screens
static lv_obj_t *screen_room_view_check; // Screen obj for room view check page
static lv_obj_t *cont_room_view_check;   // container obj to hold the complete data
static lv_obj_t *img_room_view_check;    // image object
static lv_obj_t *img_cont;               // container to hold image
static lv_obj_t *btn_continue;

uint8_t *imgdata;
#define IMAGE_WIDTH 60
#define IMAGE_HEIGHT 38
#define COLOR_SIZE 2
#define IMAGE_DATA_SIZE IMAGE_WIDTH * IMAGE_HEIGHT * COLOR_SIZE

void show_room_view_help(const lv_image_dsc_t *icon, const lv_image_dsc_t *icon_main_text,
                         const char *main_text, const char *btn1_text);
void close_room_view_help();

uint8_t hexCharToInt(char ch) {
  if (ch >= '0' && ch <= '9') {
    return ch - '0';
  } else if (ch >= 'a' && ch <= 'f') {
    return ch - 'a' + 10;
  } else if (ch >= 'A' && ch <= 'F') {
    return ch - 'A' + 10;
  } else {
    return 0; // Invalid character
  }
}

// Function to convert a hex string to a uint8_t array
void hexStringToUint8Array(const char *hexString, uint8_t *outputArray, size_t outputSize) {
  size_t len = strlen(hexString);
  if (len % 2 != 0) {
    return;
  }

  for (size_t i = 0; i < len / 2 && i < outputSize; i++) {
    outputArray[i] = (hexCharToInt(hexString[2 * i]) << 4) | hexCharToInt(hexString[2 * i + 1]);
  }
}

void display_image_room_view_check_error() {
  if (lv_obj_is_valid(img_room_view_check)) {
    lv_obj_clear_flag(img_room_view_check, LV_OBJ_FLAG_HIDDEN);
  }
  lv_img_set_src(img_room_view_check, &icon_room_assign_fail);
  lv_obj_set_align(img_room_view_check, LV_ALIGN_CENTER);
  lv_image_set_scale(img_room_view_check, 255);
}

void display_image_room_view_check(JsonDocument doc) {
  // show the image object
  if (lv_obj_is_valid(img_room_view_check)) {
    lv_obj_clear_flag(img_room_view_check, LV_OBJ_FLAG_HIDDEN);
  }
  static lv_img_dsc_t img; // image descriptor
  imgdata = (uint8_t *)malloc(IMAGE_DATA_SIZE);
  if (imgdata == NULL) {
    return;
  }
  memset(imgdata, '\0', IMAGE_DATA_SIZE);

  const char *imgString = doc["result"];

  if (imgString != nullptr) {
    if (strcmp(imgString, "null") != 0) {
      uint16_t len = strlen(imgString);
      hexStringToUint8Array(imgString, imgdata, IMAGE_DATA_SIZE);

      img.header.cf = LV_COLOR_FORMAT_RGB565;
      img.header.magic = LV_IMAGE_HEADER_MAGIC;
      img.header.w = IMAGE_WIDTH;
      img.header.h = IMAGE_HEIGHT;
      img.data_size = IMAGE_DATA_SIZE;
      img.data = imgdata;

      lv_img_set_src(img_room_view_check, &img);
      lv_obj_set_align(img_room_view_check, LV_ALIGN_CENTER);
      lv_image_set_scale(img_room_view_check, 2048);
    } else {
      display_image_room_view_check_error();
    }
  } else {
    display_image_room_view_check_error();
  }
}

// static void title_left_arrow_click_cb(lv_event_t *e) {
//   SETTINGS_MODE settings_mode = screen_settings_get_mode();
//   if (settings_mode == SETTINGS_MODE_SAVE) {
//     lv_screen_load(screen_settings_home_get());
//   } else if (settings_mode == SETTINGS_MODE_ACTIVATION) {
//     ASSIGN_MODE assign_mode = room_assign_get_mode();
//     if (assign_mode == ASSIGN_MODE_NONE) {
//       // If assign mode is none, means room selection not happened and user came to room view
//       page
//       // from confirm room page. Show confirm room screen when left arrow is clicked to go back
//       lv_screen_load(screen_settings_confirm_room_get());
//     } else if (assign_mode == ROOM_SELECTION_MODE) {
//       // If assign mode is room selection, means user came to this page from room selection page.
//       // Show room number selection screen when left arrow is clicked to go back
//       room_assign_set_mode(ROOM_SELECTION_MODE);
//       lv_screen_load(screen_settings_select_room_get());
//     }
//   }
//   return;
// }

static void btn_help_click_cb(lv_event_t *e) {
  show_room_view_help(&img_room_view_help_correct, &icon_room_view_correct, "Correct Positioning",
                      "See Next");
}

static void btn_rescan_click_cb(lv_event_t *e) {
  static resp_session_data_t *resp_session_data = resp_session_data_get();
  resp_method_clear(resp_session_data);
  strcpy(resp_session_data->response_method, cmd_str[CMD_GET_ROOM_VIEW]);
  resp_session_data->resp_id++;

  JsonObject params;

  serializeJsonRpcRequest(resp_session_data->resp_id, CMD_GET_ROOM_VIEW, params);
  // show spinner screen
  create_spinner_screen(screen_room_view_check);
}

static void btn_continue_click_cb(lv_event_t *e) {
  static resp_session_data_t *resp_session_data = resp_session_data_get();
  resp_method_clear(resp_session_data);
  strcpy(resp_session_data->response_method, cmd_str[CMD_SET_ROOM_VIEW]);
  resp_session_data->resp_id++;

  JsonObject params;

  serializeJsonRpcRequest(resp_session_data->resp_id, CMD_SET_ROOM_VIEW, params);
  // show spinner screen
  create_spinner_screen(screen_room_view_check);
  // LV_LOG_USER("room_view_check continue dbg 1 %s", &resp_session_data->response_method);
}

static void screen_room_view_check_load_cb(lv_event_t *e) {
  close_room_view_help();
  SETTINGS_MODE settings_mode = screen_settings_get_mode();
  lv_obj_t *label = lv_obj_get_child(btn_continue, 0);
  if (settings_mode == SETTINGS_MODE_SAVE) {
    if (lv_obj_is_valid(img_room_view_check)) {
      lv_obj_add_flag(img_room_view_check, LV_OBJ_FLAG_HIDDEN); /// Flags
    }
    lv_label_set_text(label, "Confirm");
  } else if (settings_mode == SETTINGS_MODE_ACTIVATION) {
    lv_label_set_text(label, "Continue");
  }

  if (!is_alert_toast_hidden()) {
    // LV_LOG_USER("room_view_check screen is parent for alert toast");
    set_alert_toast_parent(screen_room_view_check);
  }
}

lv_obj_t *create_button(lv_obj_t *parent_obj, const char *btn_text, int btn_width, int btn_height) {
  lv_obj_t *btn = lv_btn_create(parent_obj);
  lv_obj_set_size(btn, btn_width, btn_height);
  // lv_obj_set_style_bg_color(btn, lv_color_hex(VST_COLOR_DARKBLUE), LV_PART_MAIN);
  lv_obj_set_style_radius(btn, 0, LV_PART_MAIN);

  lv_obj_t *lbl = lv_label_create(btn);
  lv_label_set_text(lbl, btn_text);
  lv_obj_set_align(lbl, LV_ALIGN_CENTER);
  lv_obj_set_size(lbl, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_style_text_font(lbl, &opensans_bold_20, LV_PART_MAIN);

  return btn;
}

static void room_view_check_cont_design() {
  screen_room_view_check = tmpl_settings_create_1(NULL, "Check Room View", false);
  lv_obj_add_event_cb(screen_room_view_check, screen_room_view_check_load_cb,
                      LV_EVENT_SCREEN_LOAD_START, NULL);

  // lv_obj_t *title_left_arrow_obj = get_img_left_arrow_obj();
  // lv_obj_add_event_cb(title_left_arrow_obj, title_left_arrow_click_cb, LV_EVENT_CLICKED, NULL);

  // add content container
  cont_room_view_check = lv_obj_create(screen_room_view_check);
  lv_obj_set_width(cont_room_view_check, lv_pct(100));
  lv_obj_set_flex_grow(cont_room_view_check, 1);
  lv_obj_set_flex_flow(cont_room_view_check, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(cont_room_view_check, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);
  lv_obj_clear_flag(cont_room_view_check, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_border_width(cont_room_view_check, 3, LV_PART_MAIN);
  lv_obj_set_style_border_side(cont_room_view_check, LV_BORDER_SIDE_TOP, LV_PART_MAIN);
  lv_obj_set_style_pad_row(cont_room_view_check, 10, LV_PART_MAIN);
  lv_obj_set_style_pad_all(cont_room_view_check, 0, LV_PART_MAIN);

  // Image container to hold the image
  img_cont = lv_obj_create(cont_room_view_check);
  lv_obj_set_size(img_cont, lv_pct(100), 304);
  lv_obj_set_style_border_width(img_cont, 0, LV_PART_MAIN);
  lv_obj_set_align(img_cont, LV_ALIGN_CENTER);
  lv_obj_clear_flag(img_cont, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_pad_all(img_cont, 0, LV_PART_MAIN);

  // Image object
  img_room_view_check = lv_img_create(img_cont);
  // lv_img_set_src(img_room_view_check, &icon_bed); // Todo: change it later
  //  lv_obj_set_size(img_room_view_check, LV_SIZE_CONTENT, LV_SIZE_CONTENT); /// 1
  lv_obj_set_size(img_room_view_check, lv_pct(100), 304);
  lv_obj_set_align(img_room_view_check, LV_ALIGN_CENTER);
  lv_obj_clear_flag(img_room_view_check, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_clear_flag(img_room_view_check, LV_OBJ_FLAG_SCROLLABLE); /// Flags

  // Container to hold the buttons
  lv_obj_t *cont_btns = lv_obj_create(cont_room_view_check);
  lv_obj_set_size(cont_btns, lv_pct(100), LV_SIZE_CONTENT);
  // lv_obj_set_flex_grow(cont_btns, 1);
  lv_obj_set_flex_flow(cont_btns, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(cont_btns, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_clear_flag(cont_btns, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_border_width(cont_btns, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_top(cont_btns, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_bottom(cont_btns, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_column(cont_btns, 15, LV_PART_MAIN);

  // Help button
  lv_obj_t *btn_help = create_button(cont_btns, "Help", 138, 80);
  lv_obj_set_style_bg_color(btn_help, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_border_width(btn_help, 3, LV_PART_MAIN);
  lv_obj_set_style_border_color(btn_help, lv_color_hex(VST_COLOR_DARKBLUE), LV_PART_MAIN);

  lv_obj_t *lbl_help = lv_obj_get_child(btn_help, 0);
  lv_obj_set_style_text_color(lbl_help, lv_color_hex(VST_COLOR_DARKBLUE), LV_PART_MAIN);

  lv_obj_add_event_cb(btn_help, btn_help_click_cb, LV_EVENT_CLICKED, NULL);

  // Help button
  lv_obj_t *btn_rescan = create_button(cont_btns, "Rescan", 138, 80);
  lv_obj_set_style_bg_color(btn_rescan, lv_color_hex(VST_COLOR_DARKBLUE), LV_PART_MAIN);

  lv_obj_add_event_cb(btn_rescan, btn_rescan_click_cb, LV_EVENT_CLICKED, NULL);

  // Help button
  btn_continue = create_button(cont_btns, "Continue", 138, 80);
  lv_obj_set_style_bg_color(btn_continue, lv_color_hex(VST_COLOR_MONITOR_GREEN), LV_PART_MAIN);

  lv_obj_add_event_cb(btn_continue, btn_continue_click_cb, LV_EVENT_CLICKED, NULL);
}

lv_obj_t *screen_settings_room_view_get() {
  if (screen_room_view_check == NULL) {
    room_view_check_cont_design(); // for room view check page design
  }
  return screen_room_view_check;
}

/********************** Popup message windows ***********************/
lv_obj_t *opacity_screen_room_view_help;

// to see previous and next screens in the help popup
static void room_view_help_see_prev_next_click_cb(lv_event_t *e) {
  lv_obj_t *btn = lv_event_get_current_target_obj(e);
  lv_obj_t *label = lv_obj_get_child(btn, 0);
  const char *btn_text = lv_label_get_text(label);
  if (strcmp(btn_text, "See Next") == 0) {
    show_room_view_help(&img_room_view_help_incorrect, &icon_room_view_incorrect,
                        "Incorrect Positioning", "See Previous");
  } else if (strcmp(btn_text, "See Previous") == 0) {
    show_room_view_help(&img_room_view_help_correct, &icon_room_view_correct, "Correct Positioning",
                        "See Next");
  }
}

static void room_view_help_close_click_cb(lv_event_t *e) { close_room_view_help(); }

void show_room_view_help(const lv_image_dsc_t *icon, const lv_image_dsc_t *icon_main_text,
                         const char *main_text, const char *btn1_text) {
  // if already open then destroy it first.
  if (lv_obj_is_valid(opacity_screen_room_view_help)) {
    lv_obj_delete(opacity_screen_room_view_help);
  }

  // Opacity screen
  opacity_screen_room_view_help = lv_obj_create(screen_room_view_check);
  lv_obj_set_size(opacity_screen_room_view_help, lv_pct(100), lv_pct(100));
  lv_obj_set_style_bg_color(opacity_screen_room_view_help, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(opacity_screen_room_view_help, LV_OPA_40, LV_PART_MAIN);
  lv_obj_add_flag(opacity_screen_room_view_help, LV_OBJ_FLAG_IGNORE_LAYOUT);

  // Main Content container
  lv_obj_t *main_cont = lv_obj_create(opacity_screen_room_view_help);
  lv_obj_set_size(main_cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT); /// 1
  lv_obj_set_flex_flow(main_cont, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(main_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_clear_flag(main_cont, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_border_width(main_cont, 0, LV_PART_MAIN);
  lv_obj_set_align(main_cont, LV_ALIGN_CENTER);

  // Container to hold icon along with main heading text
  lv_obj_t *main_text_cont = lv_obj_create(main_cont);
  lv_obj_set_size(main_text_cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT); /// 1
  lv_obj_set_flex_flow(main_text_cont, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(main_text_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);
  lv_obj_clear_flag(main_text_cont, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_border_width(main_text_cont, 0, LV_PART_MAIN);

  // icon relate to main heading text
  lv_obj_t *img_main_text = lv_img_create(main_text_cont);
  lv_img_set_src(img_main_text, icon_main_text);
  lv_obj_set_size(img_main_text, LV_SIZE_CONTENT, LV_SIZE_CONTENT); /// 1
  // lv_obj_set_align(img_main_text, LV_ALIGN_CENTER);
  lv_obj_clear_flag(img_main_text, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_clear_flag(img_main_text, LV_OBJ_FLAG_SCROLLABLE); /// Flags

  // Main heading text
  lv_obj_t *lbl_mainTxt = lv_label_create(main_text_cont);
  lv_obj_set_size(lbl_mainTxt, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_label_set_text(lbl_mainTxt, main_text);
  lv_obj_set_style_text_font(lbl_mainTxt, &opensans_semibold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_color(lbl_mainTxt, lv_color_hex(VST_COLOR_PRIMARY_TEXT), LV_PART_MAIN);
  // lv_obj_set_align(lbl_mainTxt, LV_ALIGN_CENTER);
  lv_obj_set_style_text_align(lbl_mainTxt, LV_TEXT_ALIGN_CENTER, 0);

  // Image to show
  lv_obj_t *img_obj = lv_img_create(main_cont);
  lv_img_set_src(img_obj, icon);
  lv_obj_set_size(img_obj, 328, 261);
  lv_obj_set_align(img_obj, LV_ALIGN_CENTER);
  lv_image_set_scale(img_obj, 512); // zoom by *2
  lv_obj_clear_flag(img_obj, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_clear_flag(img_obj, LV_OBJ_FLAG_SCROLLABLE); /// Flags

  // Container to hold the buttons
  lv_obj_t *cont_btns = lv_obj_create(main_cont);
  lv_obj_set_size(cont_btns, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  // lv_obj_set_flex_grow(cont_btns, 1);
  lv_obj_set_flex_flow(cont_btns, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(cont_btns, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_clear_flag(cont_btns, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_border_width(cont_btns, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_top(cont_btns, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_bottom(cont_btns, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_column(cont_btns, 15, LV_PART_MAIN);

  // See Next/Previous button
  lv_obj_t *btn_see_prev_next = create_button(cont_btns, btn1_text, 156, 62);
  lv_obj_set_style_bg_color(btn_see_prev_next, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_border_width(btn_see_prev_next, 3, LV_PART_MAIN);
  lv_obj_set_style_border_color(btn_see_prev_next, lv_color_hex(VST_COLOR_DARKBLUE), LV_PART_MAIN);

  lv_obj_t *label = lv_obj_get_child(btn_see_prev_next, 0);
  lv_obj_set_style_text_color(label, lv_color_hex(VST_COLOR_DARKBLUE), LV_PART_MAIN);

  lv_obj_add_event_cb(btn_see_prev_next, room_view_help_see_prev_next_click_cb, LV_EVENT_CLICKED,
                      NULL);

  // Close button
  lv_obj_t *btn_close = create_button(cont_btns, "Close", 156, 62);
  lv_obj_set_style_bg_color(btn_close, lv_color_hex(VST_COLOR_DARKBLUE), LV_PART_MAIN);

  lv_obj_add_event_cb(btn_close, room_view_help_close_click_cb, LV_EVENT_CLICKED, NULL);
}

void close_room_view_help() {
  if (lv_obj_is_valid(opacity_screen_room_view_help)) {
    lv_obj_add_flag(opacity_screen_room_view_help, LV_OBJ_FLAG_HIDDEN);
  }
}