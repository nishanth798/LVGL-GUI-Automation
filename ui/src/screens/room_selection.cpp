#include "jsonrpc2.h"
#include "math.h"
#include "screens.h"
#include "ui.h"

#include "resp_state.h"

static rm_num_info_t *rm_num_info;
static unit_num_info_t *unit_num_info;
static ASSIGN_MODE assign_mode = ASSIGN_MODE_NONE;
static lv_obj_t *label_title; // title label object in the header

// static bool is_rm_scr_loaded_frm_next_btn_click = false; // Flag to check if "Select a Room"
// screen is loaded from "Select a Unit" screen next button cb

/******************************************************************************/
void room_assign_set_mode(ASSIGN_MODE mode) {
  assign_mode = mode;
  return;
}

ASSIGN_MODE room_assign_get_mode() { return assign_mode; }

/***************************************************************************/
void unit_num_info_clear(unit_num_info_t *unit_info) {
  // clear data in the arrays
  if (unit_num_info != NULL) {
    for (int i = 0; i < (sizeof(unit_info->unit_num_arr) / (sizeof(unit_info->unit_num_arr[0])));
         i++) {
      memset(unit_info->unit_num_arr[i], '\0', sizeof(unit_info->unit_num_arr[i]));
    }
    memset(unit_info->selected_unit_num, '\0', sizeof(unit_info->selected_unit_num));
    unit_info->unit_num_arr_idx = 0; // reset the array index to zero
    unit_info->unit_num_cnt = 0;     // reset the room number count to zero
  }
}

static void unit_num_info_init() {
  if (unit_num_info == NULL) {
    unit_num_info = (unit_num_info_t *)malloc(sizeof(unit_num_info_t));
    unit_num_info_clear(unit_num_info);
  }
}

unit_num_info_t *unit_num_info_get() {
  if (unit_num_info == NULL) {
    unit_num_info_init();
  }
  return unit_num_info;
}

/***************************************************************************************/

void rm_num_info_clear(rm_num_info_t *rm_info) {
  // clear data in the arrays
  if (rm_num_info != NULL) {
    for (int i = 0; i < (sizeof(rm_info->rm_num_arr) / (sizeof(rm_info->rm_num_arr[0]))); i++) {
      memset(rm_info->rm_num_arr[i], '\0', sizeof(rm_info->rm_num_arr[i]));
    }
    memset(rm_info->selected_rm_num, '\0', sizeof(rm_info->selected_rm_num));
    rm_info->rm_num_arr_idx = 0; // reset the array index to zero
    rm_info->rm_num_cnt = 0;     // reset the room number count to zero
  }
}

static void rm_num_info_init() {
  if (rm_num_info == NULL) {
    rm_num_info = (rm_num_info_t *)malloc(sizeof(rm_num_info_t));
    rm_num_info_clear(rm_num_info);
  }
}

rm_num_info_t *rm_num_info_get() {
  if (rm_num_info == NULL) {
    rm_num_info_init();
  }
  return rm_num_info;
}

/****************************** GLobal variables ****************************/
static lv_obj_t *screen_select_room; // Screen obj for Room Selection
lv_obj_t *cont_select_room;          // Container for Room Selection

/********************* Function Declarations ******************/
static void room_selection_cont_design();
// void create_oneBtn_msgbox(lv_obj_t *parent_obj, const lv_image_dsc_t *icon, const char
// *main_text, const char *sub_text, const char *btn_text); void create_twoBtn_msgbox(lv_obj_t
// *parent_obj, const lv_image_dsc_t *icon, const char *main_text, const char *sub_text, const char
// *btns_text[]);

/**************************************Room Selection**********************************/
// variables used in cont_select_room
static float rm_num_btn_cnt =
    6.0; // Total number of buttons, taken double because, to use "ceil" math function
lv_obj_t *btn_rm_num[6];            // button array
lv_obj_t *lbl_Pageinfo;             // page info label contains "<", "page x of y", ">"
static int currPgCnt = 1, totPgCnt; // to track page count

lv_obj_t *btn_next_roomSelection; // next button
lv_obj_t *go_to_first_page_btn, *go_to_last_page_btn, *previous_page_btn, *next_page_btn;

static void unselect_buttons();
static void change_selection_screen_title();

/**
 * Helper function to create navigation buttons for pagination
 * Creates a styled button with an icon (fast forward, fast backward, left, right)
 *
 * @param parent - Parent container where the button will be added
 * @param btn - Pointer to button object pointer (will be modified to point to created button)
 * @param img - Pointer to image object pointer (will be modified to point to created image)
 * @param icon - Icon image descriptor to display on the button
 */
static void create_nav_button(lv_obj_t *parent, lv_obj_t **btn, lv_obj_t **img,
                              const lv_image_dsc_t *icon) {
  // Create the button with standard navigation styling
  *btn = lv_btn_create(parent);
  lv_obj_set_size(*btn, 71.59, 69.42);                                   // Set button size
  lv_obj_set_style_bg_color(*btn, lv_color_hex(0x1A1040), LV_PART_MAIN); // Dark blue background
  lv_obj_set_style_radius(*btn, 5.42, LV_PART_MAIN);                     // Rounded corners

  // Create and center the icon image inside the button
  *img = lv_img_create(*btn);
  lv_img_set_src(*img, icon);
  lv_obj_center(*img);
}

/**
 * Helper function to create navigation group container
 * Creates a transparent container with horizontal flex layout and 10px gap between children
 * Used to group navigation buttons (left/right pairs) with consistent spacing
 *
 * @param parent - Parent container where the navigation group will be added
 * @return Pointer to the created navigation group container
 */
static lv_obj_t *create_nav_group(lv_obj_t *parent) {
  lv_obj_t *nav_group = lv_obj_create(parent);
  lv_obj_set_size(nav_group, LV_SIZE_CONTENT, LV_SIZE_CONTENT);    // Auto-size to content
  lv_obj_set_style_border_width(nav_group, 0, LV_PART_MAIN);       // No border
  lv_obj_set_style_bg_opa(nav_group, LV_OPA_TRANSP, LV_PART_MAIN); // Transparent background
  lv_obj_set_style_pad_all(nav_group, 0, LV_PART_MAIN);            // No padding
  lv_obj_set_flex_flow(nav_group, LV_FLEX_FLOW_ROW);               // Horizontal layout
  lv_obj_set_style_pad_gap(nav_group, 10, LV_PART_MAIN);           // 10px gap between buttons
  return nav_group;
}

/**
 * Helper function to create action buttons (Back/Next)
 * Creates styled buttons for primary actions with different styling based on button type
 *
 * @param parent - Parent container where the button will be added
 * @param text - Text to display on the button
 * @param is_next_btn - true for "Next" button (dark blue bg), false for "Back" button (white bg)
 * @return Pointer to the created button object
 */
static lv_obj_t *create_action_button(lv_obj_t *parent, const char *text, bool is_next_btn) {
  // Create button with text using custom btn_create function
  lv_obj_t *btn = btn_create(parent, text);

  // Set button dimensions and basic styling
  lv_obj_set_size(btn, 216, 80);                 // Button size
  lv_obj_set_style_radius(btn, 5, LV_PART_MAIN); // Rounded corners
  lv_obj_set_style_border_color(btn, lv_color_hex(VST_COLOR_DARKBLUE), LV_PART_MAIN);
  lv_obj_set_style_border_width(btn, 3, LV_PART_MAIN); // 3px border

  // Set padding for text positioning
  lv_obj_set_style_pad_top(btn, 10, LV_PART_MAIN);
  lv_obj_set_style_pad_bottom(btn, 10, LV_PART_MAIN);
  lv_obj_set_style_pad_left(btn, 25, LV_PART_MAIN);
  lv_obj_set_style_pad_right(btn, 25, LV_PART_MAIN);

  // Add shadow effect for depth
  lv_obj_set_style_shadow_offset_y(btn, 2, LV_PART_MAIN);                   // Shadow offset
  lv_obj_set_style_shadow_width(btn, 4, LV_PART_MAIN);                      // Shadow blur
  lv_obj_set_style_shadow_color(btn, lv_color_hex(0x000000), LV_PART_MAIN); // Black shadow
  lv_obj_set_style_shadow_opa(btn, 64, LV_PART_MAIN);                       // 25% opacity

  // Text styling
  lv_obj_set_style_text_align(btn, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_set_style_text_font(btn, &opensans_bold_24, LV_PART_MAIN | LV_STATE_DEFAULT);

  // Apply different color schemes based on button type
  if (is_next_btn) {
    // Next button: Dark blue background with white text (primary action)
    lv_obj_set_style_bg_color(btn, lv_color_hex(VST_COLOR_DARKBLUE), LV_PART_MAIN);
    lv_obj_set_style_text_color(btn, lv_color_white(), LV_PART_MAIN);
  } else {
    // Back button: White background with dark blue text (secondary action)
    lv_obj_set_style_bg_color(btn, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_color(btn, lv_color_hex(VST_COLOR_DARKBLUE), LV_PART_MAIN);
  }

  return btn;
}

/* Enable or disable clickable property of next button
 */
static void set_btn_next_roomSelect_clickable(bool clickable) {
  if (clickable) {
    lv_obj_add_flag(btn_next_roomSelection, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_state(btn_next_roomSelection, LV_STATE_DISABLED);
  } else {
    lv_obj_clear_flag(btn_next_roomSelection, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_state(btn_next_roomSelection, LV_STATE_DISABLED);
  }
}

static void update_room_numbers(uint8_t *arr_idx, char arr[100][11], int totCnt) {
  change_selection_screen_title(); // for updating screen title
  totPgCnt = (int)ceil(totCnt / rm_num_btn_cnt);

  for (int i = 0; i < rm_num_btn_cnt; i++) {
    lv_obj_t *label = lv_obj_get_child(btn_rm_num[i], 0);
    // if string is empty or array index reached total room count, then hide the remaining buttons
    if (strcmp(arr[*arr_idx], "") && ((*arr_idx) < totCnt)) {
      lv_label_set_text_fmt(label, "%s", arr[*arr_idx]);
      lv_obj_clear_flag(btn_rm_num[i], LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_add_flag(btn_rm_num[i], LV_OBJ_FLAG_HIDDEN);
    }
    (*arr_idx)++;
  }
  currPgCnt = (int)ceil((*arr_idx) / rm_num_btn_cnt);
  lv_label_set_text_fmt(lbl_Pageinfo, "%d of %d", currPgCnt, totPgCnt);

  // Manage navigation button states based on current page position
  // Disable backward navigation buttons when on first page
  if (currPgCnt == 1) {
    lv_obj_add_state(go_to_first_page_btn, LV_STATE_DISABLED); // Disable "go to first page" button
    lv_obj_add_state(previous_page_btn, LV_STATE_DISABLED);    // Disable "previous page" button
  } else {
    // Enable backward navigation when not on first page
    lv_obj_clear_state(go_to_first_page_btn, LV_STATE_DISABLED); // Enable "go to first page" button
    lv_obj_clear_state(previous_page_btn, LV_STATE_DISABLED);    // Enable "previous page" button
  }

  // Disable forward navigation buttons when on last page
  if (currPgCnt == totPgCnt) {
    lv_obj_add_state(go_to_last_page_btn, LV_STATE_DISABLED); // Disable "go to last page" button
    lv_obj_add_state(next_page_btn, LV_STATE_DISABLED);       // Disable "next page" button
  } else {
    // Enable forward navigation when not on last page
    lv_obj_clear_state(go_to_last_page_btn, LV_STATE_DISABLED); // Enable "go to last page" button
    lv_obj_clear_state(next_page_btn, LV_STATE_DISABLED);       // Enable "next page" button
  }

  unselect_buttons();
}

static void img_right_page_click_cb(lv_event_t *e) {
  if (assign_mode == UNIT_SELECTION_MODE) {
    if (unit_num_info->unit_num_arr_idx >= unit_num_info->unit_num_cnt) {
      return;
    }
    update_room_numbers(&unit_num_info->unit_num_arr_idx, unit_num_info->unit_num_arr,
                        unit_num_info->unit_num_cnt);
  } else if (assign_mode == ROOM_SELECTION_MODE) {
    if (rm_num_info->rm_num_arr_idx >= rm_num_info->rm_num_cnt) {
      return;
    }
    update_room_numbers(&rm_num_info->rm_num_arr_idx, rm_num_info->rm_num_arr,
                        rm_num_info->rm_num_cnt);
  }
}

static void img_left_page_click_cb(lv_event_t *e) {
  if (assign_mode == UNIT_SELECTION_MODE) {
    if (unit_num_info->unit_num_arr_idx <= rm_num_btn_cnt) {
      return;
    }
    // to go to previous page, this page button count and previous page button count we need to
    // reduce.(so, rm_num_btn_cnt *2)
    unit_num_info->unit_num_arr_idx = unit_num_info->unit_num_arr_idx - (rm_num_btn_cnt * 2);
    update_room_numbers(&unit_num_info->unit_num_arr_idx, unit_num_info->unit_num_arr,
                        unit_num_info->unit_num_cnt);
  } else if (assign_mode == ROOM_SELECTION_MODE) {
    if (rm_num_info->rm_num_arr_idx <= rm_num_btn_cnt) {
      return;
    }
    // to go to previous page, this page button count and previous page button count we need to
    // reduce.(so, rm_num_btn_cnt *2)
    rm_num_info->rm_num_arr_idx = rm_num_info->rm_num_arr_idx - (rm_num_btn_cnt * 2);
    update_room_numbers(&rm_num_info->rm_num_arr_idx, rm_num_info->rm_num_arr,
                        rm_num_info->rm_num_cnt);
  }
}

static void img_fast_forward_click_cb(lv_event_t *e) {
  if (assign_mode == UNIT_SELECTION_MODE) {
    if (unit_num_info->unit_num_arr_idx >= unit_num_info->unit_num_cnt) {
      return;
    }
    totPgCnt = (int)ceil(unit_num_info->unit_num_cnt /
                         rm_num_btn_cnt); // ceil the result i.e., if it is 6.2, ceil it to 7
    unit_num_info->unit_num_arr_idx =
        ((totPgCnt - 1) * 6); // to get the last page first button index
    update_room_numbers(&unit_num_info->unit_num_arr_idx, unit_num_info->unit_num_arr,
                        unit_num_info->unit_num_cnt);
  } else if (assign_mode == ROOM_SELECTION_MODE) {
    if (rm_num_info->rm_num_arr_idx >= rm_num_info->rm_num_cnt) {
      return;
    }
    totPgCnt = (int)ceil(rm_num_info->rm_num_cnt /
                         rm_num_btn_cnt); // ceil the result i.e., if it is 6.2, ceil it to 7
    rm_num_info->rm_num_arr_idx = ((totPgCnt - 1) * 6);
    update_room_numbers(&rm_num_info->rm_num_arr_idx, rm_num_info->rm_num_arr,
                        rm_num_info->rm_num_cnt);
  }
}

static void img_fast_backward_click_cb(lv_event_t *e) {
  if (assign_mode == UNIT_SELECTION_MODE) {
    if (unit_num_info->unit_num_arr_idx <= rm_num_btn_cnt) {
      return;
    }
    unit_num_info->unit_num_arr_idx = 0;
    update_room_numbers(&unit_num_info->unit_num_arr_idx, unit_num_info->unit_num_arr,
                        unit_num_info->unit_num_cnt);
  } else if (assign_mode == ROOM_SELECTION_MODE) {
    if (rm_num_info->rm_num_arr_idx <= rm_num_btn_cnt) {
      return;
    }
    rm_num_info->rm_num_arr_idx = 0;
    update_room_numbers(&rm_num_info->rm_num_arr_idx, rm_num_info->rm_num_arr,
                        rm_num_info->rm_num_cnt);
  }
}

// To select only one button at a time, and also to unselect the selected button on second click
static void unselect_buttons() {
  int selected_btn_cnt = 0;
  for (int i = 0; i < rm_num_btn_cnt; i++) {
    lv_obj_t *label = lv_obj_get_child(btn_rm_num[i], 0); // get label from the button
    // if the text of the button is same as the selected text and the button is not hidden then add
    // checked state, else remove checked state
    if (assign_mode == UNIT_SELECTION_MODE) {
      if (strcmp(unit_num_info->selected_unit_num, lv_label_get_text(label)) == 0 &&
          (!lv_obj_has_flag(btn_rm_num[i], LV_OBJ_FLAG_HIDDEN))) {
        lv_obj_add_state(btn_rm_num[i], LV_STATE_CHECKED);
        lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
        selected_btn_cnt++;
      } else {
        lv_obj_clear_state(btn_rm_num[i], LV_STATE_CHECKED);
        lv_obj_set_style_text_color(label, lv_color_hex(VST_COLOR_DARKBLUE), LV_PART_MAIN);
      }
    } else if (assign_mode == ROOM_SELECTION_MODE) {
      if (strcmp(rm_num_info->selected_rm_num, lv_label_get_text(label)) == 0 &&
          (!lv_obj_has_flag(btn_rm_num[i], LV_OBJ_FLAG_HIDDEN))) {
        lv_obj_add_state(btn_rm_num[i], LV_STATE_CHECKED);
        lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
        selected_btn_cnt++;
      } else {
        lv_obj_clear_state(btn_rm_num[i], LV_STATE_CHECKED);
        lv_obj_set_style_text_color(label, lv_color_hex(VST_COLOR_DARKBLUE), LV_PART_MAIN);
      }
    }
  }
  if (selected_btn_cnt > 0) {
    set_btn_next_roomSelect_clickable(true);
  } else {
    set_btn_next_roomSelect_clickable(false);
  }
}

static lv_obj_t *currentButton = NULL; // object to hold the currently selected button

static void rm_num_btn_click_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);        // get event code
  lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e); // get the object that triggered this event
  lv_obj_t *label = lv_obj_get_child(obj, 0);         // get label from the button
  if (code == LV_EVENT_CLICKED) {
    if (assign_mode == UNIT_SELECTION_MODE) {
      if ((currentButton == obj) &&
          (strcmp(unit_num_info->selected_unit_num, lv_label_get_text(label)) == 0)) {
        currentButton = NULL;
        set_btn_next_roomSelect_clickable(false);
      } else {
        currentButton = obj;
        set_btn_next_roomSelect_clickable(true);
      }
    } else if (assign_mode == ROOM_SELECTION_MODE) {
      if ((currentButton == obj) &&
          (strcmp(rm_num_info->selected_rm_num, lv_label_get_text(label)) == 0)) {
        currentButton = NULL;
        set_btn_next_roomSelect_clickable(false);
      } else {
        currentButton = obj;
        set_btn_next_roomSelect_clickable(true);
      }
    }

    // Assign the selected text to the respective variable
    if (assign_mode == UNIT_SELECTION_MODE) {
      if (strcmp(unit_num_info->selected_unit_num, lv_label_get_text(label)) != 0) {
        memset(unit_num_info->selected_unit_num, '\0',
               sizeof(unit_num_info->selected_unit_num)); // clear the array
        strcpy(unit_num_info->selected_unit_num,
               lv_label_get_text(label)); // copy label text into variable
      } else {
        memset(unit_num_info->selected_unit_num, '\0',
               sizeof(unit_num_info->selected_unit_num)); // clear the array
      }
    } else if (assign_mode == ROOM_SELECTION_MODE) {
      if (strcmp(rm_num_info->selected_rm_num, lv_label_get_text(label)) != 0) {
        memset(rm_num_info->selected_rm_num, '\0',
               sizeof(rm_num_info->selected_rm_num)); // clear the array
        strcpy(rm_num_info->selected_rm_num,
               lv_label_get_text(label)); // copy label text into variable
      } else {
        memset(rm_num_info->selected_rm_num, '\0',
               sizeof(rm_num_info->selected_rm_num)); // clear the array
      }
    }

    // for adding the checked state for the selected button and removing for other buttons
    lv_obj_t *parent = lv_obj_get_parent(obj);
    uint32_t i;
    for (i = 0; i < lv_obj_get_child_count(parent); i++) {
      lv_obj_t *child = lv_obj_get_child(parent, i);
      lv_obj_t *child_label = lv_obj_get_child(child, 0); // get label from the button
      if (child == currentButton) {
        lv_obj_add_state(child, LV_STATE_CHECKED);
        // Change label text color for selected button
        lv_obj_set_style_text_color(child_label, lv_color_white(), LV_PART_MAIN);
      } else {
        lv_obj_remove_state(child, LV_STATE_CHECKED);
        // Revert label text color for unselected buttons
        lv_obj_set_style_text_color(child_label, lv_color_hex(VST_COLOR_DARKBLUE), LV_PART_MAIN);
      }
    }
  }
}

/* common function used to send CMD_SET_ROOM command
 */
static void send_set_room_request() {
  static resp_session_data_t *resp_session_data = resp_session_data_get();
  resp_method_clear(resp_session_data);

  JsonDocument doc;
  JsonObject params = doc["params"].to<JsonObject>();

  resp_session_data->resp_id++; // increment the request id
  strcpy(resp_session_data->response_method,
         cmd_str[CMD_SET_ROOM]); // update the response method into session structure
  params["room"] = rm_num_info->selected_rm_num;
  // if sensor id is present, then send sensor id, else send it as null pointer
  if (strcmp(resp_session_data->sensor_id, "") != 0) {
    params["sensor"] = resp_session_data->sensor_id;
  } else {
    params["sensor"] = nullptr;
  }
  serializeJsonRpcRequest(resp_session_data->resp_id, CMD_SET_ROOM, params);
  // Formulate the message Assigning Room 455...
  char msg1[100] = "Assigning Room ";
  strcat(msg1, rm_num_info->selected_rm_num);
  strcat(msg1, "...");
  static const int cont_padding[4] = {32, 32, 32,
                                      32};  // popup container padding (top, bottom, left, right)
  static const int btn_size[2] = {352, 64}; // button size (width, height)
  create_oneBtn_msgbox(screen_select_room, &icon_assigning_room, msg1,
                       "Assigning room can take up to 45\nseconds.", "Cancel", cont_padding,
                       btn_size);
}

/* When Next button is selected from unit/room selection pages
 */
static void select_room_next_btn_click_cb(lv_event_t *e) {
  static resp_session_data_t *resp_session_data = resp_session_data_get();
  resp_method_clear(resp_session_data);

  JsonDocument doc;
  JsonObject params = doc["params"].to<JsonObject>();

  if (assign_mode == UNIT_SELECTION_MODE) {
    resp_session_data->resp_id++; // increment the request id
    strcpy(resp_session_data->response_method,
           cmd_str[CMD_GET_ROOMS]); // update the response method into session structure
    params["unit"] = unit_num_info->selected_unit_num;
    serializeJsonRpcRequest(resp_session_data->resp_id, CMD_GET_ROOMS, params);
    // show spinner screen as indicating, we are waiting for response
    create_spinner_screen(screen_select_room);
    // The changes for tracking next button load are commented out, keeping previous logic.
    // is_rm_scr_loaded_frm_next_btn_click = true;
  } else if (assign_mode == ROOM_SELECTION_MODE) {
    send_set_room_request();
  }
}

/*To change screen title according to the
screen switching between Unit selection and Room selection
*/
static void change_selection_screen_title() {
  // lv_obj_t *cont = lv_obj_get_child(screen_select_room, 2); // get container from the screen
  // lv_obj_t *label = lv_obj_get_child(cont, 0);              // get label from the container

  if (assign_mode == UNIT_SELECTION_MODE) {
    lv_label_set_text(label_title, "Select a Unit");
  } else if (assign_mode == ROOM_SELECTION_MODE) {
    lv_label_set_text(label_title, "Select a Room");
  }
  return;
}

/*clears all data pertaining to assign room
 */
void clear_all_assign_room_data() {
  unit_num_info_clear(unit_num_info);
  rm_num_info_clear(rm_num_info);
  assign_mode = ASSIGN_MODE_NONE;
  memset(unit_num_info->selected_unit_num, '\0', sizeof(unit_num_info->selected_unit_num));
  memset(rm_num_info->selected_rm_num, '\0', sizeof(rm_num_info->selected_rm_num));
}

/* Screen load call back function
 */
static void screen_select_room_load_cb(lv_event_t *e) {
  unselect_buttons();
  change_selection_screen_title();
  set_btn_next_roomSelect_clickable(false);
  if (assign_mode == UNIT_SELECTION_MODE) {
    unit_num_info->unit_num_arr_idx = 0;
    update_room_numbers(&unit_num_info->unit_num_arr_idx, unit_num_info->unit_num_arr,
                        unit_num_info->unit_num_cnt);
  } else if (assign_mode == ROOM_SELECTION_MODE) {
    // When room selection is loaded from the alert settings screen, then show the previously
    // selected room number page
    if (rm_num_info->rm_num_arr_idx != 0) {
      rm_num_info->rm_num_arr_idx = rm_num_info->rm_num_arr_idx - 6;
    }
    update_room_numbers(&rm_num_info->rm_num_arr_idx, rm_num_info->rm_num_arr,
                        rm_num_info->rm_num_cnt);

    // if(is_rm_scr_loaded_frm_next_btn_click){
    //   is_rm_scr_loaded_frm_next_btn_click = false; // Reset the flag after loading the "Select a
    //   Room" screen from "Unit Selection" screen.
    // }
    // else{
    //   set_btn_next_roomSelect_clickable(true); // Enable Next button if "Select a Room" screen is
    //   loaded from the "alerts settings" screen
    // }
  }
}

/* When left arrow on the top left corner is selected, this function is invoked to go to previous
 * page
 */
static void room_select_back_btn_click_cb(lv_event_t *e) {
  static resp_session_data_t *resp_session_data = resp_session_data_get();
  if (assign_mode == UNIT_SELECTION_MODE) {
    clear_all_assign_room_data();
    // if room is not assigned, show assign room page else show confirm room page
    if (strcmp(resp_session_data->assigned_room, "") == 0) {
      lv_obj_t *scr = screen_settings_assign_room_get();
      lv_screen_load(scr);
    } else {
      lv_obj_t *scr = screen_settings_confirm_room_get();
      lv_screen_load(scr);
    }
  } else if (assign_mode == ROOM_SELECTION_MODE) {
    assign_mode = UNIT_SELECTION_MODE;
    set_btn_next_roomSelect_clickable(true); // Enable "Select a Unit" screen "Next" button when
                                             // going back from "Select a Room" screen
    // clear the previously selected room number
    memset(rm_num_info->selected_rm_num, '\0', sizeof(rm_num_info->selected_rm_num));
    // As the index is already at the next page button, as it is increased in for loop.
    // So doing -6 to get back to same page first button index
    unit_num_info->unit_num_arr_idx = (unit_num_info->unit_num_arr_idx - 6);
    update_room_numbers(&unit_num_info->unit_num_arr_idx, unit_num_info->unit_num_arr,
                        unit_num_info->unit_num_cnt);
  }
}

/* This container design consists of six buttons and left and right navigate buttons
This is used for both unit selection and room selection based on ASSIGN_ROOM enum.
*/

static void room_selection_cont_design() {
  screen_select_room = tmpl_settings_create_1(NULL, "Select a Unit", false);
  lv_obj_add_event_cb(screen_select_room, screen_select_room_load_cb, LV_EVENT_SCREEN_LOAD_START,
                      NULL);

  // lv_obj_t *title_left_arrow_obj = get_img_left_arrow_obj();
  // lv_obj_add_flag(title_left_arrow_obj, LV_OBJ_FLAG_EVENT_BUBBLE); /// Flags

  // lv_obj_t *title_cont_left_arrow_obj = get_cont_left_arrow_obj();
  // lv_obj_add_event_cb(title_cont_left_arrow_obj, room_select_title_left_arrow_click_cb,
  // LV_EVENT_CLICKED, NULL);

  // lv_obj_add_flag(title_cont_left_arrow_obj, LV_OBJ_FLAG_HIDDEN); // Flags

  label_title = get_header_label_obj(); // get label from the header

  // Create main content container for room/unit selection buttons
  // Uses row wrap layout to arrange 6 buttons in a 2x3 grid (219x64 each)
  cont_select_room = lv_obj_create(screen_select_room);
  lv_obj_set_size(cont_select_room, 452, 216);                      // Container size
  lv_obj_add_flag(cont_select_room, LV_OBJ_FLAG_IGNORE_LAYOUT);     // Manual positioning
  lv_obj_set_pos(cont_select_room, 16, 80);                         // Position from screen top-left
  lv_obj_set_flex_flow(cont_select_room, LV_FLEX_FLOW_ROW_WRAP);    // Wrap buttons to next row
  lv_obj_clear_flag(cont_select_room, LV_OBJ_FLAG_SCROLLABLE);      // Disable scrolling
  lv_obj_set_style_border_width(cont_select_room, 0, LV_PART_MAIN); // No border
  lv_obj_set_style_bg_opa(cont_select_room, LV_OPA_TRANSP, LV_PART_MAIN);  // Transparent background
  lv_obj_set_style_pad_all(cont_select_room, 2, LV_PART_MAIN);             // No padding
  lv_obj_set_style_clip_corner(cont_select_room, false, LV_STATE_CHECKED); // Don't clip shadow

  // Create 6 room/unit selection buttons in a loop
  for (int i = 0; i < rm_num_btn_cnt; i++) {
    btn_rm_num[i] = lv_btn_create(cont_select_room);
    lv_obj_set_size(btn_rm_num[i], 219, 64); // Button size

    // Button styling - normal and selected states
    lv_obj_set_style_bg_color(btn_rm_num[i], lv_color_white(),
                              LV_PART_MAIN); // White background (normal)
    lv_obj_set_style_border_color(btn_rm_num[i], lv_color_hex(VST_COLOR_DARKBLUE),
                                  LV_PART_MAIN);                   // Charcoal border (normal)
    lv_obj_set_style_border_width(btn_rm_num[i], 3, LV_PART_MAIN); // 3px border width
    lv_obj_set_style_radius(btn_rm_num[i], 5, LV_PART_MAIN);

    lv_obj_set_style_bg_color(btn_rm_num[i], lv_color_hex(VST_COLOR_BTN_SELECTED_SKYBLUE),
                              LV_STATE_CHECKED); // Sky blue background (selected)
    lv_obj_set_style_border_color(btn_rm_num[i], lv_color_hex(VST_COLOR_BTN_SELECTED_SKYBLUE),
                                  LV_STATE_CHECKED); // Sky blue border (selected)
    lv_obj_set_style_shadow_color(btn_rm_num[i], lv_color_hex(0x74B4FF),
                                  LV_STATE_CHECKED);                         // Shadow color
    lv_obj_set_style_shadow_width(btn_rm_num[i], 2, LV_STATE_CHECKED);       // Shadow width
    lv_obj_set_style_shadow_opa(btn_rm_num[i], LV_OPA_50, LV_STATE_CHECKED); // Shadow opacity
    lv_obj_set_style_shadow_spread(btn_rm_num[i], 3, LV_STATE_CHECKED);      // Shadow spread
    lv_obj_set_style_shadow_offset_y(btn_rm_num[i], 0, LV_STATE_CHECKED);    // Shadow offset
    lv_obj_set_style_shadow_offset_x(btn_rm_num[i], 0, LV_STATE_CHECKED);    // Shadow offset

    // Create and style button label (displays room/unit number)
    lv_obj_t *lbl_room_num = lv_label_create(btn_rm_num[i]);
    lv_obj_center(lbl_room_num); // Center label in button
    lv_obj_set_style_text_color(lbl_room_num, lv_color_hex(VST_COLOR_DARKBLUE),
                                LV_PART_MAIN); // Charcoal text color
    lv_obj_set_style_text_font(lbl_room_num, &opensans_bold_18,
                               LV_PART_MAIN | LV_STATE_DEFAULT); // Bold 18px font

    // Attach click event handler for room/unit selection
    lv_obj_add_event_cb(btn_rm_num[i], rm_num_btn_click_cb, LV_EVENT_CLICKED, NULL);
  }

  /****************************** Page Navigation Container ******************************/
  // Create container for pagination controls (first/prev buttons, page info, next/last buttons)
  // Uses space-between layout to distribute navigation groups and page label
  lv_obj_t *cont_page_number_info = lv_obj_create(screen_select_room);
  lv_obj_set_size(cont_page_number_info, 448, 69.42);                    // Navigation bar size
  lv_obj_add_flag(cont_page_number_info, LV_OBJ_FLAG_IGNORE_LAYOUT);     // Manual positioning
  lv_obj_set_pos(cont_page_number_info, 16, 304);                        // Position below content
  lv_obj_set_style_border_width(cont_page_number_info, 0, LV_PART_MAIN); // No border
  lv_obj_set_style_pad_all(cont_page_number_info, 0, LV_PART_MAIN);      // No padding
  lv_obj_set_flex_flow(cont_page_number_info, LV_FLEX_FLOW_ROW);         // Horizontal layout
  lv_obj_set_flex_align(cont_page_number_info, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);                      // Distribute space
  lv_obj_clear_flag(cont_page_number_info, LV_OBJ_FLAG_SCROLLABLE); // Disable scrolling
  lv_obj_set_style_bg_color(cont_page_number_info, lv_color_hex(0xF6F6F6),
                            LV_PART_MAIN); // Light gray background

  // Local image variables for navigation button icons (not needed globally)
  lv_obj_t *img_left_page, *img_right_page, *img_fast_forward, *img_fast_backward;

  // Create left navigation group (first page + previous page buttons)
  lv_obj_t *left_nav_group = create_nav_group(cont_page_number_info);
  create_nav_button(left_nav_group, &go_to_first_page_btn, &img_fast_backward,
                    &icon_fast_backward); // Jump to first page
  create_nav_button(left_nav_group, &previous_page_btn, &img_left_page,
                    &icon_leftSide_btn); // Previous page

  // Page information label (center)
  lbl_Pageinfo = lv_label_create(cont_page_number_info);
  lv_obj_set_size(lbl_Pageinfo, LV_SIZE_CONTENT, LV_SIZE_CONTENT); // Auto-size to content
  lv_label_set_text_fmt(lbl_Pageinfo, "%d of %d", currPgCnt,
                        totPgCnt); // Display current and total pages
  lv_obj_set_style_text_font(lbl_Pageinfo, &opensans_regular_26,
                             LV_PART_MAIN | LV_STATE_DEFAULT); // Regular 26px font
  lv_obj_set_style_text_color(lbl_Pageinfo, lv_color_hex(VST_COLOR_INACTIVE_TEXT),
                              LV_PART_MAIN); // Charcoal text color

  // Create right navigation group (next page + last page buttons)
  lv_obj_t *right_nav_group = create_nav_group(cont_page_number_info);
  create_nav_button(right_nav_group, &next_page_btn, &img_right_page,
                    &icon_rightSide_btn); // Next page
  create_nav_button(right_nav_group, &go_to_last_page_btn, &img_fast_forward,
                    &icon_fast_forward); // Jump to last page

  // Create container for main action buttons (Back/Next)
  // Uses space-between layout for automatic button distribution
  lv_obj_t *cont_action_buttons = lv_obj_create(screen_select_room);
  lv_obj_set_size(cont_action_buttons, 448, 80);                       // Action bar size
  lv_obj_add_flag(cont_action_buttons, LV_OBJ_FLAG_IGNORE_LAYOUT);     // Manual positioning
  lv_obj_set_pos(cont_action_buttons, 16, 384);                        // Position at bottom
  lv_obj_set_style_border_width(cont_action_buttons, 0, LV_PART_MAIN); // No border
  lv_obj_set_style_bg_opa(cont_action_buttons, LV_OPA_TRANSP,
                          LV_PART_MAIN);                          // Transparent background
  lv_obj_set_style_pad_all(cont_action_buttons, 0, LV_PART_MAIN); // No padding
  lv_obj_set_flex_flow(cont_action_buttons, LV_FLEX_FLOW_ROW);    // Horizontal layout
  lv_obj_set_flex_align(cont_action_buttons, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);                    // Distribute buttons
  lv_obj_clear_flag(cont_action_buttons, LV_OBJ_FLAG_SCROLLABLE); // Disable scrolling

  // Create Back and Next action buttons
  lv_obj_t *btn_back_roomSelection = create_action_button(cont_action_buttons, "Back", false);
  btn_next_roomSelection = create_action_button(cont_action_buttons, "Next", true);
  set_btn_next_roomSelect_clickable(false); // Initially disabled until selection

  // Attach event handlers to all interactive elements
  lv_obj_add_event_cb(next_page_btn, img_right_page_click_cb, LV_EVENT_CLICKED,
                      NULL); // Next page navigation
  lv_obj_add_event_cb(previous_page_btn, img_left_page_click_cb, LV_EVENT_CLICKED,
                      NULL); // Previous page navigation
  lv_obj_add_event_cb(btn_next_roomSelection, select_room_next_btn_click_cb, LV_EVENT_CLICKED,
                      NULL); // Next with selection
  lv_obj_add_event_cb(go_to_last_page_btn, img_fast_forward_click_cb, LV_EVENT_CLICKED,
                      NULL); // Jump to last page
  lv_obj_add_event_cb(go_to_first_page_btn, img_fast_backward_click_cb, LV_EVENT_CLICKED,
                      NULL); // Jump to first page
  lv_obj_add_event_cb(btn_back_roomSelection, room_select_back_btn_click_cb, LV_EVENT_CLICKED,
                      NULL); // Back to previous screen
}

/******************************************************************************************/

lv_obj_t *screen_settings_select_room_get() {
  if (screen_select_room == NULL) {
    room_selection_cont_design(); // for room selection page design
  }
  return screen_select_room;
}

/**********************************Message box Common functions********************************/

lv_obj_t *create_opacity_screen(lv_obj_t *parent_obj) {
  lv_obj_t *opa_screen = lv_obj_create(parent_obj);
  lv_obj_set_size(opa_screen, lv_pct(100), lv_pct(100));
  lv_obj_set_style_bg_color(opa_screen, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(opa_screen, LV_OPA_40, LV_PART_MAIN);
  lv_obj_add_flag(opa_screen, LV_OBJ_FLAG_IGNORE_LAYOUT);
  return opa_screen;
}

/* Create pop up main container with padding and rounded corners */
/* parent_obj: parent object to which this container to be added */
/* cont_padding: array of 4 integers representing padding (top, bottom, left, right) */

lv_obj_t *create_mbox_container(lv_obj_t *parent_obj, const int *cont_padding) {
  lv_obj_t *cont = lv_obj_create(parent_obj);
  lv_obj_set_size(cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT); /// 1
  lv_obj_set_style_radius(cont, 12, LV_PART_MAIN);         // Rounded corners
  lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_border_width(cont, 1, LV_PART_MAIN);
  lv_obj_set_align(cont, LV_ALIGN_CENTER);
  lv_obj_add_flag(cont, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_set_style_pad_gap(cont, 32, LV_PART_MAIN);                 // Row gap
  lv_obj_set_style_pad_top(cont, cont_padding[0], LV_PART_MAIN);    // Top padding
  lv_obj_set_style_pad_bottom(cont, cont_padding[1], LV_PART_MAIN); // Bottom padding
  lv_obj_set_style_pad_left(cont, cont_padding[2], LV_PART_MAIN);   // Left padding
  lv_obj_set_style_pad_right(cont, cont_padding[3], LV_PART_MAIN);  // Right padding
  return cont;
}

void create_mbox_icon_obj(lv_obj_t *parent_obj, const lv_image_dsc_t *icon) {
  lv_obj_t *img_con = lv_img_create(parent_obj);
  lv_img_set_src(img_con, icon);
  lv_obj_set_size(img_con, LV_SIZE_CONTENT, LV_SIZE_CONTENT); /// 1
  lv_obj_set_align(img_con, LV_ALIGN_CENTER);
  lv_obj_clear_flag(img_con, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_clear_flag(img_con, LV_OBJ_FLAG_SCROLLABLE); /// Flags
}

void create_mbox_maintext_label(lv_obj_t *parent, const char *text) {
  lv_obj_t *lbl_mainTxt = lv_label_create(parent);
  lv_obj_set_size(lbl_mainTxt, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_label_set_text(lbl_mainTxt, text);
  lv_obj_set_style_text_font(lbl_mainTxt, &opensans_semibold_28, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_color(lbl_mainTxt, lv_color_hex(VST_COLOR_PRIMARY_TEXT), LV_PART_MAIN);
  lv_obj_set_align(lbl_mainTxt, LV_ALIGN_CENTER);
  lv_obj_set_style_text_align(lbl_mainTxt, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_line_space(lbl_mainTxt, 10, 0); // Line spacing
}

void create_mbox_subtext_label(lv_obj_t *parent, const char *text) {
  lv_obj_t *lbl_subTxt = lv_label_create(parent);
  lv_obj_set_size(lbl_subTxt, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_label_set_text(lbl_subTxt, text);
  lv_obj_set_style_text_font(lbl_subTxt, &opensans_regular_16, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_color(lbl_subTxt, lv_color_hex(VST_COLOR_PRIMARY_TEXT), LV_PART_MAIN);
  lv_obj_set_align(lbl_subTxt, LV_ALIGN_CENTER);
  lv_obj_set_style_text_align(lbl_subTxt, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_line_space(lbl_subTxt, 6, 0); // Line spacing
}

/**********************************Message box with one button********************************/
lv_obj_t *opacity_screen_oneBtn_msgBox;
lv_obj_t *opacity_screen_twoBtn_msgBox;

void close_one_btn_msg_box() {
  if (lv_obj_is_valid(opacity_screen_oneBtn_msgBox)) {
    lv_obj_add_flag(opacity_screen_oneBtn_msgBox, LV_OBJ_FLAG_HIDDEN);
  }
}

static void oneBtn_msgBox_btn_click_cb(lv_event_t *e) {
  lv_obj_t *btn = lv_event_get_current_target_obj(e);
  lv_obj_t *label = lv_obj_get_child(btn, 0);
  const char *btn_text = lv_label_get_text(label);
  if (strcmp(btn_text, "Cancel") == 0) {
    close_one_btn_msg_box();
    // invoke assign room cb(room_resp_timeout_cb) such that it will cancel the current request.
    room_resp_timeout_cb(assign_room_timeout_timer);
  }
}

// Create pop up box with one button
// parent_obj: parent object to which this popup box to be added
// icon: icon to be displayed in the popup box
// main_text: main text to be displayed in the popup box
// sub_text: sub text to be displayed in the popup box
// btn_text: text to be displayed on the button
// cont_padding: array of 4 integers representing popup box padding
// cont_padding[0]: top padding
// cont_padding[1]: bottom padding
// cont_padding[2]: left padding
// cont_padding[3]: right padding
// btn_size: array of 2 integers representing button size
// btn_size[0]: button width
// btn_size[1]: button height

void create_oneBtn_msgbox(lv_obj_t *parent_obj, const lv_image_dsc_t *icon, const char *main_text,
                          const char *sub_text, const char *btn_text, const int cont_padding[],
                          const int btn_size[]) {
  // if already showing then close it first.
  if (lv_obj_is_valid(opacity_screen_oneBtn_msgBox)) {
    lv_obj_delete(opacity_screen_oneBtn_msgBox);
  }
  if (lv_obj_is_valid(opacity_screen_twoBtn_msgBox)) {
    lv_obj_delete(opacity_screen_twoBtn_msgBox);
  }

  // create opacity screen
  opacity_screen_oneBtn_msgBox = create_opacity_screen(parent_obj);
  // create main container
  lv_obj_t *cont_oneBtn_msgbox = create_mbox_container(opacity_screen_oneBtn_msgBox, cont_padding);
  lv_obj_set_size(cont_oneBtn_msgbox, 416, 333); // Fixed size for one button message box
  // create icon
  create_mbox_icon_obj(cont_oneBtn_msgbox, icon);
  // create maintext label
  if (main_text != NULL) {
    create_mbox_maintext_label(cont_oneBtn_msgbox, main_text);
  }
  // create subtext label
  if (sub_text != NULL) {
    create_mbox_subtext_label(cont_oneBtn_msgbox, sub_text);
  }
  if (btn_text != NULL) {
    lv_obj_t *btn = lv_button_create(cont_oneBtn_msgbox);
    lv_obj_set_size(btn, btn_size[0], btn_size[1]); // Button size
    lv_obj_set_style_bg_color(btn, lv_color_hex(VST_COLOR_DARKBLUE), LV_PART_MAIN);
    lv_obj_set_style_radius(btn, 5, LV_PART_MAIN); // Rounded corners

    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, btn_text);
    lv_obj_align(lbl, LV_ALIGN_CENTER, 0, 3);
    lv_obj_set_size(lbl, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_text_font(lbl, &opensans_bold_24, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, oneBtn_msgBox_btn_click_cb, LV_EVENT_CLICKED, NULL);
  }
}

/**********************************Message box with two buttons********************************/

void close_two_btn_msg_box() {
  if (lv_obj_is_valid(opacity_screen_twoBtn_msgBox)) {
    lv_obj_add_flag(opacity_screen_twoBtn_msgBox, LV_OBJ_FLAG_HIDDEN);
  }
}

static void twoBtn_msgBox_btn1_click_cb(lv_event_t *e) {
  lv_obj_t *btn = lv_event_get_current_target_obj(e);
  lv_obj_t *label = lv_obj_get_child(btn, 0);
  const char *btn_text = lv_label_get_text(label);
  if ((strcmp(btn_text, "Cancel") == 0) || (strcmp(btn_text, "Reassign") == 0)) {
    close_two_btn_msg_box();
  }
}

static void twoBtn_msgBox_btn2_click_cb(lv_event_t *e) {
  lv_obj_t *btn = lv_event_get_current_target_obj(e);
  lv_obj_t *label = lv_obj_get_child(btn, 0);
  const char *btn_text = lv_label_get_text(label);
  if (strcmp(btn_text, "Try Again") == 0) {
    close_two_btn_msg_box();
    // send set_room command again with the same room number
    send_set_room_request();
  } else if (strcmp(btn_text, "Confirm") == 0) {
    close_two_btn_msg_box();
    // send set_room command again with the same room number
    send_set_room_request();
  } else if (strcmp(btn_text, "Continue") == 0) {
    close_two_btn_msg_box();
    // show bms configuration page
    lv_obj_t *screen = screen_settings_alerts_get();
    lv_scr_load(screen);
  }
}

// Create pop up box with two buttons
// parent_obj: parent object to which this popup box to be added
// icon: icon to be displayed in the popup
// main_text: main text to be displayed in the popup
// sub_text: sub text to be displayed in the popup
// btns_text: array of 2 strings representing text to be displayed on the buttons
// btns_text[0]: text for button 1 (left button)
// btns_text[1]: text for button 2 (right button)
// cont_padding: array of 4 integers representing popup box padding
// cont_padding[0]: top padding
// cont_padding[1]: bottom padding
// cont_padding[2]: left padding
// cont_padding[3]: right padding
// btns_size: array of 3 integers representing button size
// btns_size[0]: buttons container width (width of both buttons including gap between them)
// btns_size[1]: button width
// btns_size[2]: button height

void create_twoBtn_msgbox(lv_obj_t *parent_obj, const lv_image_dsc_t *icon, const char *main_text,
                          const char *sub_text, const char *btns_text[], const int cont_padding[],
                          const int btns_size[]) {
  // if one button messagebox is already showing then close it first.
  if (lv_obj_is_valid(opacity_screen_oneBtn_msgBox)) {
    lv_obj_delete(opacity_screen_oneBtn_msgBox);
  }
  if (lv_obj_is_valid(opacity_screen_twoBtn_msgBox)) {
    lv_obj_delete(opacity_screen_twoBtn_msgBox);
  }
  // create opacity screen
  opacity_screen_twoBtn_msgBox = create_opacity_screen(parent_obj);
  // create main container
  lv_obj_t *cont_twoBtn_msgbox = create_mbox_container(opacity_screen_twoBtn_msgBox, cont_padding);
  // create icon
  create_mbox_icon_obj(cont_twoBtn_msgbox, icon);
  // create maintext label
  if (main_text != NULL) {
    create_mbox_maintext_label(cont_twoBtn_msgbox, main_text);
  }
  // create subtext label
  if (sub_text != NULL) {
    create_mbox_subtext_label(cont_twoBtn_msgbox, sub_text);
  }

  lv_obj_t *cont_btns;
  if (btns_text[0] != NULL) {
    cont_btns = lv_obj_create(cont_twoBtn_msgbox);
    lv_obj_set_size(cont_btns, btns_size[0], btns_size[2]); // Container size
    lv_obj_set_flex_flow(cont_btns, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cont_btns, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(cont_btns, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_border_width(cont_btns, 0, LV_PART_MAIN);
    lv_obj_set_align(cont_btns, LV_ALIGN_BOTTOM_MID);      // Align to bottom center
    lv_obj_set_style_pad_gap(cont_btns, 16, LV_PART_MAIN); // Gap between buttons
    lv_obj_set_style_pad_all(cont_btns, 0, LV_PART_MAIN);  // No padding

    lv_obj_t *btn1 = lv_button_create(cont_btns);
    lv_obj_set_size(btn1, btns_size[1], btns_size[2]); // Button size
    lv_obj_set_style_bg_color(btn1, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_border_width(btn1, 3, LV_PART_MAIN);
    lv_obj_set_style_border_color(btn1, lv_color_hex(VST_COLOR_DARKBLUE), LV_PART_MAIN);
    lv_obj_set_style_radius(btn1, 5, LV_PART_MAIN); // Rounded corners

    lv_obj_t *lbl1 = lv_label_create(btn1);
    lv_label_set_text(lbl1, btns_text[0]);
    lv_obj_align(lbl1, LV_ALIGN_CENTER, 0, 3);
    lv_obj_set_size(lbl1, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_text_font(lbl1, &opensans_bold_24, LV_PART_MAIN); // Bold 24px font
    lv_obj_set_style_text_align(lbl1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(lbl1, lv_color_hex(VST_COLOR_DARKBLUE), LV_PART_MAIN);

    lv_obj_add_event_cb(btn1, twoBtn_msgBox_btn1_click_cb, LV_EVENT_CLICKED, NULL);
  }
  if (btns_text[1] != NULL) {
    lv_obj_t *btn2 = lv_button_create(cont_btns);
    lv_obj_set_size(btn2, btns_size[1], btns_size[2]); // Button size
    lv_obj_set_style_bg_color(btn2, lv_color_hex(VST_COLOR_DARKBLUE), LV_PART_MAIN);
    lv_obj_set_style_radius(btn2, 5, LV_PART_MAIN); // Rounded corners

    lv_obj_t *lbl2 = lv_label_create(btn2);
    lv_label_set_text(lbl2, btns_text[1]);
    lv_obj_align(lbl2, LV_ALIGN_CENTER, 0, 3);
    lv_obj_set_size(lbl2, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_text_font(lbl2, &opensans_bold_24, LV_PART_MAIN); // Bold 24px font
    lv_obj_set_style_text_align(lbl2, LV_TEXT_ALIGN_CENTER, 0);

    lv_obj_add_event_cb(btn2, twoBtn_msgBox_btn2_click_cb, LV_EVENT_CLICKED, NULL);
  }
}

/**************************show spinner screen****************************/

static lv_obj_t *opacity_spinner_screen;

void create_spinner_screen(lv_obj_t *parent_obj) {
  // if spinner screen is already showing then close it first.
  if (lv_obj_is_valid(opacity_spinner_screen)) {
    lv_obj_delete(opacity_spinner_screen);
  }
  // create opacity screen
  opacity_spinner_screen = create_opacity_screen(parent_obj);

  /*Create a spinner*/
  lv_obj_t *spinner = lv_spinner_create(opacity_spinner_screen);
  lv_obj_set_size(spinner, 100, 100);
  lv_obj_center(spinner);
  lv_spinner_set_anim_params(spinner, 10000, 200);
}

void close_spinner_screen() {
  if (lv_obj_is_valid(opacity_spinner_screen)) {
    lv_obj_add_flag(opacity_spinner_screen, LV_OBJ_FLAG_HIDDEN);
  }
}
