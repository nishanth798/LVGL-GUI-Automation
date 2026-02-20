// clang-format off
#include "screens.h"
#include "ui.h"
#include "math.h"
#include "jsonrpc2.h"
#include "resp_state.h"

// clang-format on
// Carry on formatting

// Screens
static lv_obj_t *screen_assign_room;  // Screen obj for Assign Room to Sensor page
static lv_obj_t *screen_confirm_room; // Screen obj for Confirm Room Assignment

// Main containers
lv_obj_t *cont_assign_room;  // Container for Assign Room to Sensor
lv_obj_t *cont_confirm_room; // Container for Confirm Room Assignment

static void send_get_units_request() {
  static resp_session_data_t *resp_session_data = resp_session_data_get();
  resp_method_clear(resp_session_data);
  strcpy(resp_session_data->response_method, cmd_str[CMD_GET_UNITS]);
  resp_session_data->resp_id++;

  JsonObject params;

  serializeJsonRpcRequest(resp_session_data->resp_id, CMD_GET_UNITS, params);
}

/*****************************Assign Room*************************************/

static void btn_view_rooms_click_cb(lv_event_t *e) {
  // show spinner screen as indicating, we are waiting for response
  create_spinner_screen(screen_assign_room);
  send_get_units_request();
}

// static void assign_confirm_title_left_arrow_click_cb(lv_event_t *e) {
//   lv_obj_t *screen = scr_home_inactive_get();
//   lv_screen_load(screen);
// }

static void screen_assign_room_load_cb(lv_event_t *e) {}

static void assign_room_cont_design() {
  screen_assign_room =
      tmpl_settings_create_1(NULL, "Assign Room to System", false); // Updated header title
  lv_obj_add_event_cb(screen_assign_room, screen_assign_room_load_cb, LV_EVENT_SCREEN_LOAD_START,
                      NULL);

  lv_obj_t *cont_header = lv_obj_get_child(screen_assign_room, 0);
  lv_obj_set_style_shadow_color(cont_header, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_shadow_opa(cont_header, 64, LV_PART_MAIN);
  lv_obj_set_style_shadow_width(cont_header, 4, LV_PART_MAIN);
  lv_obj_set_style_shadow_spread(cont_header, 0, LV_PART_MAIN);
  lv_obj_set_style_shadow_offset_x(cont_header, 0, LV_PART_MAIN);
  lv_obj_set_style_shadow_offset_y(cont_header, 2, LV_PART_MAIN);

  // lv_obj_t *title_left_arrow_obj = get_img_left_arrow_obj();
  // lv_obj_add_flag(title_left_arrow_obj, LV_OBJ_FLAG_EVENT_BUBBLE); /// Flags

  // lv_obj_t *title_cont_left_arrow_obj = get_cont_left_arrow_obj();
  // lv_obj_add_event_cb(title_cont_left_arrow_obj, assign_confirm_title_left_arrow_click_cb,
  //                     LV_EVENT_CLICKED, NULL);

  // add content container
  // cont_assign_room = lv_obj_create(screen_assign_room);
  // lv_obj_set_width(cont_assign_room, lv_pct(100));
  // lv_obj_set_flex_grow(cont_assign_room, 1);
  // lv_obj_set_flex_flow(cont_assign_room, LV_FLEX_FLOW_COLUMN);
  // lv_obj_set_flex_align(cont_assign_room, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
  //                       LV_FLEX_ALIGN_CENTER);
  // lv_obj_clear_flag(cont_assign_room, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  // lv_obj_set_style_border_width(cont_assign_room, 3, LV_PART_MAIN);

  // information label
  lv_obj_t *lbl_info = lv_label_create(
      screen_assign_room); // lbl_info is created as child of main screen(screen_assign_room)
  lv_obj_set_size(lbl_info, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_label_set_text(lbl_info, "The system does not have a room assigned.\nPlease tap 'View Rooms' "
                              "to view a list of rooms.");
  lv_obj_set_style_text_font(lbl_info, &opensans_italic_18,
                             LV_PART_MAIN | LV_STATE_DEFAULT); // Updated font style
  lv_obj_set_style_text_color(lbl_info, lv_color_hex(VST_COLOR_INACTIVE_TEXT), LV_PART_MAIN);
  lv_obj_add_flag(lbl_info, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_set_style_text_align(lbl_info, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(lbl_info, LV_ALIGN_TOP_MID, 0, 202);
  lv_obj_set_style_text_line_space(lbl_info, 4, LV_PART_MAIN); // added line spacing

  // Scan button
  lv_obj_t *btn_view_rooms = btn_create(
      screen_assign_room,
      "View Rooms"); // btn_view_rooms is created as child of main screen(screen_assign_room)
  lv_obj_set_style_bg_color(btn_view_rooms, lv_color_hex(VST_COLOR_DARKBLUE), LV_PART_MAIN);
  lv_obj_add_flag(btn_view_rooms, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_set_size(btn_view_rooms, 448, 80); // size
  lv_obj_set_align(btn_view_rooms, LV_ALIGN_BOTTOM_MID);
  lv_obj_set_pos(btn_view_rooms, 0, -16); // padding from bottom
  lv_obj_t *label_view_rooms =
      lv_obj_get_child(btn_view_rooms, 0); // Get the label child of the "View Rooms" button
  lv_obj_set_style_text_font(label_view_rooms, &opensans_bold_20,
                             LV_PART_MAIN); // Applied font  20 to "View Rooms" button label
  lv_obj_add_event_cb(btn_view_rooms, btn_view_rooms_click_cb, LV_EVENT_CLICKED, NULL);
}

/*********************************Confirm Room***********************************/
lv_obj_t *lbl_assigned_rm_num;

static void btn_change_room_click_cb(lv_event_t *e) {
  // show spinner screen as indicating, we are waiting for response
  create_spinner_screen(screen_confirm_room);
  send_get_units_request();
}

static void btn_confirm_room_click_cb(lv_event_t *e) {
  // show bms configuration page
  lv_obj_t *screen = screen_settings_alerts_get();
  lv_scr_load(screen);
}

static void screen_confirm_room_load_cb(lv_event_t *e) {

  // get the room response handling structure, which holds the assigned room number if any.
  resp_session_data_t *resp_session_data = resp_session_data_get();
  char rm_num_arr[20] = "Room #";                       // default text
  strcat(rm_num_arr, resp_session_data->assigned_room); // concatanate the room number received
  lv_label_set_text(lbl_assigned_rm_num, rm_num_arr);
}

static void confirm_room_cont_design() {
  screen_confirm_room = tmpl_settings_create_1(NULL, "Confirm Room Assignment", false);
  lv_obj_add_event_cb(screen_confirm_room, screen_confirm_room_load_cb, LV_EVENT_SCREEN_LOAD_START,
                      NULL);

  lv_obj_t *cont_header = lv_obj_get_child(screen_confirm_room, 0);
  lv_obj_set_style_shadow_color(cont_header, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_shadow_opa(cont_header, 64, LV_PART_MAIN);
  lv_obj_set_style_shadow_width(cont_header, 4, LV_PART_MAIN);
  lv_obj_set_style_shadow_spread(cont_header, 0, LV_PART_MAIN);
  lv_obj_set_style_shadow_offset_x(cont_header, 0, LV_PART_MAIN);
  lv_obj_set_style_shadow_offset_y(cont_header, 2, LV_PART_MAIN);

  // lv_obj_t *title_left_arrow_obj = get_img_left_arrow_obj();
  // lv_obj_add_flag(title_left_arrow_obj, LV_OBJ_FLAG_EVENT_BUBBLE); /// Flags

  // lv_obj_t *title_cont_left_arrow_obj = get_cont_left_arrow_obj();
  // lv_obj_add_event_cb(title_cont_left_arrow_obj, assign_confirm_title_left_arrow_click_cb,
  //                     LV_EVENT_CLICKED, NULL);

  lv_obj_t *lbl1 = lv_label_create(screen_confirm_room);
  lv_obj_set_size(lbl1, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_add_flag(lbl1, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_align(lbl1, LV_ALIGN_TOP_MID, 0, 89);
  lv_label_set_text(lbl1, "Previously Assigned Room");
  lv_obj_set_style_text_font(lbl1, &opensans_semibold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_color(lbl1, lv_color_hex(VST_COLOR_PRIMARY_TEXT), LV_PART_MAIN);

  lv_obj_t *lbl2 = lv_label_create(screen_confirm_room);
  lv_obj_set_size(lbl2, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_label_set_text(lbl2, "This system has already been assigned.\nPlease verify that this is the "
                          "same room you would \nlike to have assigned to this system.");
  lv_obj_set_style_text_font(lbl2, &opensans_regular_18, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_color(lbl2, lv_color_hex(VST_COLOR_PRIMARY_TEXT), LV_PART_MAIN);
  lv_obj_add_flag(lbl2, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_set_style_text_align(lbl2, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(lbl2, LV_ALIGN_TOP_MID, 0, 123);
  lv_obj_set_style_text_line_space(lbl2, 4, LV_PART_MAIN);
  // Room Number
  lbl_assigned_rm_num = lv_label_create(screen_confirm_room);
  lv_obj_set_size(lbl_assigned_rm_num, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_label_set_text(lbl_assigned_rm_num, "Room 0000");
  lv_obj_set_style_text_font(lbl_assigned_rm_num, &opensans_semibold_48,
                             LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_color(lbl_assigned_rm_num, lv_color_hex(VST_COLOR_INACTIVE_TEXT),
                              LV_PART_MAIN);
  lv_obj_add_flag(lbl_assigned_rm_num, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_set_pos(lbl_assigned_rm_num, 96, 259);

  // add Change Room & Confirm Room buttons
  lv_obj_t *btn_change_room = btn_secondary_create_1(screen_confirm_room, "Change Room");
  lv_obj_set_size(btn_change_room, 216, 80);
  lv_obj_add_flag(btn_change_room, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_align(btn_change_room, LV_ALIGN_BOTTOM_LEFT, 16, -16);
  lv_obj_t *btn_change_room_label = lv_obj_get_child(btn_change_room, 0);
  lv_obj_set_style_text_font(btn_change_room_label, &opensans_bold_24,
                             LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_add_event_cb(btn_change_room, btn_change_room_click_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *btn_confirm_room =
      btn_primary_create_1(screen_confirm_room, "Confirm Room", VST_COLOR_DARKBLUE);
  lv_obj_add_flag(btn_confirm_room, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_set_size(btn_confirm_room, 216, 80);
  lv_obj_align(btn_confirm_room, LV_ALIGN_BOTTOM_RIGHT, -16, -16);
  lv_obj_t *btn_confirm_room_label = lv_obj_get_child(btn_confirm_room, 0);
  lv_obj_set_style_text_font(btn_confirm_room_label, &opensans_bold_24,
                             LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_event_cb(btn_confirm_room, btn_confirm_room_click_cb, LV_EVENT_CLICKED, NULL);
}

/***********************************************************************************/

lv_obj_t *screen_settings_assign_room_get() {
  if (screen_assign_room == NULL) {
    assign_room_cont_design(); // for assign room to sensor page design
  }
  return screen_assign_room;
}

lv_obj_t *screen_settings_confirm_room_get() {
  if (screen_confirm_room == NULL) {
    confirm_room_cont_design(); // for confirm room assignment page design
  }
  return screen_confirm_room;
}
