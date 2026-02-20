#include "json_handles.h"
#include "jsonrpc2.h"
#include "screens.h"
#include "sys_state.h"
#include "ui.h"
#include <cstdio>

// todo: add elevation to controls container across all screens

//
// Section: Constants
//

static const char *labels[] = {
    "Left Wall",
    "Center",
    "Right Wall",
    NULL,
};

static const lv_img_dsc_t *icons[] = {
    &icon_bed_placement_left_wall,
    &icon_bed_placement_center,
    &icon_bed_placement_right_wall,
    NULL,
};

static const char *descs[] = {
    "• Wall directly next to the left side of the bed.\n",
    "• No wall directly next to either side of the bed.\n",
    "• Wall directly next to the right side of the bed.\n",
    NULL,
};

//
// Section: Type Definitions
//

// session data
typedef struct {
  BED_POS bed_position;
} session_data_t;

//
// Section: Variables
//

// root UI objects
static lv_obj_t *screen;
static lv_obj_t *bed_placement_selector, *label_desc;
static lv_obj_t *btn_save, *cont_btns_nav;
static lv_obj_t *title_cont_left_arrow_obj;
static lv_obj_t *title_cont_info_icon_obj;

static SETTINGS_MODE settings_mode;

// session data storage
static session_data_t session_data = {
    .bed_position = BED_POS_UNSET,
};
static session_data_t sd_load; // holds data that is present during load time.

// Section: Function Declarations

// Public API
lv_obj_t *screen_settings_get_bed_placement();
void clear_bed_placement_sessiondata();

// Control functions
static bool is_session_data_dirty();
static void btn_save_update_status();
static void jsonrpc_request_send(session_data_t *data);
static void set_bed_placement_level(lv_obj_t *obj, BED_POS bed_pos);
static BED_POS get_bed_placement_level(lv_obj_t *obj);
static void set_desc(BED_POS bed_pos);
static void screen_init();

// Event Handlers
static void btn_radio_click_cb(lv_event_t *e);
static void btn_save_click_cb(lv_event_t *e);
static void request_timeout_cb(lv_event_t *e);
static void set_state_fail_cb(lv_event_t *e);
static void set_state_ok_cb(lv_event_t *e);
static void mbtn_event_cb(lv_event_t *e);
static void left_arrow_click_cb(lv_event_t *e);
static void screen_load_cb(lv_event_t *e);
static void btn_prev_click_cb(lv_event_t *e);
static void btn_next_click_cb(lv_event_t *e);

// Section: Function Definitions

/**
 *  Resets the session data for bed placement.
 */
void clear_bed_placement_sessiondata() { session_data.bed_position = BED_POS_UNSET; }

/**
 *  Checks if the current session data differs from the data loaded at screen start.
 * return true if data is different (dirty), false otherwise.
 */
static bool is_session_data_dirty() {
  session_data_t *sd = &session_data;
  // const sys_state_t *ss = sys_state_get();
  if (sd->bed_position != sd_load.bed_position) {
    return true;
  }
  return false;
}

/**
 *  Updates the state of the save button  session data.
 */
static void btn_save_update_status() {
  if (settings_mode == SETTINGS_MODE_SAVE && is_session_data_dirty()) {
    btn_set_state(btn_save, BTN_STATE_ACTIVE);
  } else {
    btn_set_state(btn_save, BTN_STATE_DISABLED);
  }
}

/**
 *  Constructs and sends the JSON-RPC request to save the bed position.
 * param: data Pointer to the session data containing the bed position.
 */
static void jsonrpc_request_send(session_data_t *data) {
  //  const size_t capacity = 200; // Adjust this based on your needs
  JsonDocument doc;

  JsonObject params = doc["params"].to<JsonObject>();
  params["pos"] = data->bed_position;

  serializeJsonRpcRequest(0, CMD_SAVE_BED_POS, params);
}

/**
 *  Sets the selected button in the radio selector based on the BED_POS enum.
 * param: obj The radio selector object.
 * param: bed_pos The bed position enum value.
 */
static void set_bed_placement_level(lv_obj_t *obj, BED_POS bed_pos) {
  int8_t btn_id = bed_pos - 1;
  if (bed_pos == BED_POS_UNSET) {
    btn_id = bed_pos;
  }
  radio_selector_set_selected_btn_1(obj, btn_id);
}

/**
 *  Gets the BED_POS enum value from the selected button in the radio selector.
 * param: obj The radio selector object.
 * return :The current BED_POS enum value.
 */
static BED_POS get_bed_placement_level(lv_obj_t *obj) {
  return (BED_POS)(radio_selector_get_selected_btn_1(obj) + 1);
}

/**
 *  Updates the description label based on the selected bed position.
 * param bed_pos The bed position enum value.
 */
static void set_desc(BED_POS bed_pos) {
  if (bed_pos == BED_POS_UNSET) {
    lv_label_set_text_static(label_desc, "");
    return;
  }
  lv_label_set_text_static(label_desc, descs[bed_pos - 1]);
}

//
// Subsection: Event Handlers
//

/**
 *  Handler for buttons inside the "Confirm Save" message box.
 */
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

// show information page
static void info_icon_click_cb(lv_event_t *e) {
  show_system_info_screen((char *)"settings_bed_placement");
}

/**
 *  Handler for the left arrow (back) button in the header.
 */
static void left_arrow_click_cb(lv_event_t *e) {
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
  }
}

/**
 *  Handler for when a radio button selection changes. Updates session data and description.
 */
static void btn_radio_click_cb(lv_event_t *e) {
  BED_POS bed_pos = get_bed_placement_level(bed_placement_selector);
  set_desc(bed_pos);
  session_data.bed_position = bed_pos;
  btn_save_update_status();
}

/**
 *  Handler for the "Back" button (Activation mode).
 */
static void btn_back_click_cb(lv_event_t *e) {

  lv_obj_t *screen;
  sys_state_t *ss = sys_state_get();
  if (ss->fts_avail.bed_wid) {
    screen = screen_settings_get_bed_width();
  } else {
    screen = screen_settings_bms_get();
  }
  lv_scr_load(screen);
}

/**
 *  Handler for the "Next" button (Activation mode).
 */
static void btn_next_click_cb(lv_event_t *e) {
  lv_obj_t *screen;
  send_bed_placement_session_data((BED_POS)session_data.bed_position);
  screen_settings_set_mode(SETTINGS_MODE_ACTIVATION);
  sys_state_t *ss = sys_state_get();
  if (ss->fts_avail.occ_size) {
    screen = screen_settings_get_occupant_size();
  } else {
    screen = screen_settings_audio_get();
  }
  lv_screen_load(screen);
}

/**
 *  Handler for the Save button click.
 */
static void btn_save_click_cb(lv_event_t *e) {
  jsonrpc_request_send(&session_data);
  btn_set_state(btn_save, BTN_STATE_SUBMITTING);
}

/**
 *  Handler for JSON-RPC request timeout.
 */
static void request_timeout_cb(lv_event_t *e) {
  btn_set_state(btn_save, BTN_STATE_ACTIVE);
  toast_show(screen, no_response_str);
}

/**
 *  Handler for failed JSON-RPC save response. 
 */
static void set_state_fail_cb(lv_event_t *e) {
  btn_set_state(btn_save, BTN_STATE_ACTIVE);
  toast_show(screen, invalid_response_str);
}

/**
 *  Handler for successful JSON-RPC save response.
 */
static void set_state_ok_cb(lv_event_t *e) {
  btn_set_state(btn_save, BTN_STATE_SUBMITTED);
  // update load time session data with latest session data after save is successful
  const sys_state_t *ss = sys_state_get();
  sd_load.bed_position = ss->bed_pos;
}

/**
 *  Handler for when the screen is starting to load. Initializes state based on settings mode.
 */
static void screen_load_cb(lv_event_t *e) {

  settings_mode = screen_settings_get_mode();
  settings_show_mode(screen, btn_save, cont_btns_nav, settings_mode);

  session_data_t *sd = &session_data;
  if (settings_mode == SETTINGS_MODE_ACTIVATION) {
    // show the header icon symbol and hide the left arrow
    lv_obj_add_flag(title_cont_left_arrow_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(title_cont_info_icon_obj, LV_OBJ_FLAG_HIDDEN);
    // set bed placement level to default if unset
    if (sd->bed_position == BED_POS_UNSET) {
      sd->bed_position = BED_POS_CENTER;
    }
  } else if (settings_mode == SETTINGS_MODE_SAVE) {
    // show the header left arrow and hide the icon symbol
    lv_obj_clear_flag(title_cont_left_arrow_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(title_cont_info_icon_obj, LV_OBJ_FLAG_HIDDEN);

    const sys_state_t *ss = sys_state_get();
    if (ss->bed_pos == BED_POS_UNSET) {
      sd->bed_position = BED_POS_CENTER;
      sd_load.bed_position = BED_POS_CENTER;
    } else {
      sd->bed_position = ss->bed_pos;
      sd_load.bed_position = ss->bed_pos;
    }

    btn_save_update_status();
  }
  set_bed_placement_level(bed_placement_selector, sd->bed_position);
  set_desc(sd->bed_position);

  if (!is_alert_toast_hidden()) {
    set_alert_toast_parent(screen);
  }
}

/**
 *  Initializes all LVGL objects for the Bed Placement screen.
 */
static void screen_init() {
  screen = tmpl_settings_create_1(NULL, "Settings", true);
  lv_obj_set_style_bg_color(screen, lv_color_hex(VST_COLOR_SCREEN_BG_GREY), LV_PART_MAIN);
  lv_obj_t *lef_arrow_obj = get_img_left_arrow_obj();
  lv_obj_add_flag(lef_arrow_obj, LV_OBJ_FLAG_EVENT_BUBBLE); /// Flags

  title_cont_left_arrow_obj = get_cont_left_arrow_obj();
  lv_obj_add_event_cb(title_cont_left_arrow_obj, left_arrow_click_cb, LV_EVENT_CLICKED, NULL);

  title_cont_info_icon_obj = get_cont_info_icon_obj();
  lv_obj_add_event_cb(title_cont_info_icon_obj, info_icon_click_cb, LV_EVENT_CLICKED, NULL);

  // Register event handlers
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

  // add controls container
  lv_obj_t *cont_controls = lv_obj_create(cont_content);
  lv_obj_set_size(cont_controls, lv_pct(100), 216);
  lv_obj_clear_flag(cont_controls, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_border_width(cont_controls, 1, LV_PART_MAIN);
  lv_obj_set_style_border_color(cont_controls, lv_color_hex(0xD1D1D1), LV_PART_MAIN);

  // Add shadow/elevation styles
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
  lv_label_set_text_static(label_subtitle, "Bed Placement");
  lv_obj_set_style_text_font(label_subtitle, &opensans_semibold_18, LV_PART_MAIN);
  lv_obj_set_style_text_color(label_subtitle, lv_color_hex(VST_COLOR_PRIMARY_TEXT),
                              LV_PART_MAIN); // text color

  // add bed placement controller
  bed_placement_selector = radio_selector_create_1(cont_controls, labels, icons);
  lv_obj_set_style_pad_left(bed_placement_selector, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_right(bed_placement_selector, 0, LV_PART_MAIN);
  lv_obj_add_event_cb(bed_placement_selector, btn_radio_click_cb, LV_EVENT_VALUE_CHANGED, NULL);

  // add desc label
  label_desc = lv_label_create(cont_controls);
  lv_obj_set_width(label_desc, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_height(label_desc, LV_SIZE_CONTENT); /// 1
  lv_obj_set_pos(label_desc, 12, 160);
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
  lv_obj_t *btn_back = btn_secondary_create_1(cont_btns_nav, "Back");
  lv_obj_set_align(btn_back, LV_ALIGN_LEFT_MID);
  lv_obj_add_event_cb(btn_back, btn_back_click_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *btn_next = btn_primary_create_1(cont_btns_nav, "Next", VST_COLOR_DARKBLUE);
  lv_obj_set_align(btn_next, LV_ALIGN_RIGHT_MID);
  lv_obj_add_event_cb(btn_next, btn_next_click_cb, LV_EVENT_CLICKED, NULL);
}

//
// Subsection: Public API
//

/**
 *  Gets the main screen object for Bed Placement settings, initializing it if necessary.
 * return The LVGL object pointer for the screen.
 */
lv_obj_t *screen_settings_get_bed_placement() {
  if (screen == NULL) {
    screen_init();
  }
  return screen;
}