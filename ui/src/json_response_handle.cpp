// clang-format off
#include "jsonrpc2.h"
#include "assign_room.h"
#include "resp_state.h"
#include "json_handles.h"

// clang-format on
// Carry on formatting

int32_t room_resp_error_code;
char room_resp_error_msg[100];

static resp_session_data_t *resp_session_data;

void handle_get_room_method(JsonDocument doc) {
  lv_obj_t *screen; // screen object to hold the screen
  const char *room_num = doc["result"];

  if (room_num == nullptr) {
    // show assign room screen
    screen = screen_settings_assign_room_get();
  } else {
    // show confirm room screen
    if (strlen(room_num) < sizeof(resp_session_data->assigned_room)) {
      strncpy(resp_session_data->assigned_room, room_num,
              sizeof(resp_session_data->assigned_room) - 1); // saved assigned room number
      screen = screen_settings_confirm_room_get();
    } else {
      return;
    }
  }
  room_assign_set_mode(ASSIGN_MODE_NONE);
  lv_screen_load(screen);
}

void handle_unit_number_method(JsonDocument doc) {
  static unit_num_info_t *unit_num_info = unit_num_info_get();
  unit_num_info_clear(unit_num_info);

  JsonArray unitNumArr = doc["result"];
  bool valid = true; // assume response is valid
  int item_cnt = 0;
  // First, check all items are strings or not
  for (JsonVariant item : unitNumArr) {
    if (!item.is<const char *>()) {
      valid = false; // found a non-string item
      break;
    }
  }
  if (valid) {
    for (JsonVariant item : unitNumArr) {
      if (item_cnt < 100) {
        const char *arr = item;
        if(strcmp(arr, "") == 0) {
          continue; // Skip empty strings
        }
        strncpy(unit_num_info->unit_num_arr[item_cnt], arr,
                sizeof(unit_num_info->unit_num_arr[item_cnt]) - 1);
        // myprint(unit_num_info->unit_num_arr[i]);
        item_cnt++;
      } else {
        break;
      }
    }
  }

  unit_num_info->unit_num_cnt = item_cnt;

  // show the unit number page only if the count is greater than 0, i.e., if atleast one unit is
  //  present
  if (unit_num_info->unit_num_cnt > 0) {
    room_assign_set_mode(UNIT_SELECTION_MODE);
    lv_obj_t *scr = screen_settings_select_room_get();
    // lv_obj_send_event(scr, LV_EVENT_SCREEN_LOAD_START, NULL);
    lv_screen_load(scr);
  }
}

void handle_room_number_method(JsonDocument doc) {
  static rm_num_info_t *rm_num_info = rm_num_info_get();
  rm_num_info_clear(rm_num_info);

  JsonArray roomNumArr = doc["result"];
  bool valid = true; // assume response is valid
  int item_cnt = 0;
  // First, check all items are strings or not
  for (JsonVariant item : roomNumArr) {
    if (!item.is<const char *>()) {
      valid = false; // found a non-string item
      break;
    }
  }
  if (valid) {
    for (JsonVariant item : roomNumArr) {
      if (item_cnt < 100) {
        const char *arr = item;
        if(strcmp(arr, "") == 0) {
          continue; // Skip empty strings
        }
        strncpy(rm_num_info->rm_num_arr[item_cnt], arr,
                sizeof(rm_num_info->rm_num_arr[item_cnt]) - 1);
        // myprint(rm_num_info->rm_num_arr[item_cnt]);
        item_cnt++;
      } else {
        break;
      }
    }
  }

  rm_num_info->rm_num_cnt = item_cnt;
  // show the room number page only if the count is greater than 0, i.e., if atleast one room is
  // present
  if (rm_num_info->rm_num_cnt > 0) {
    room_assign_set_mode(ROOM_SELECTION_MODE);
    lv_obj_t *scr = screen_settings_select_room_get();
    lv_obj_send_event(scr, LV_EVENT_SCREEN_LOAD_START, NULL);
    // lv_screen_load(scr);
  }
}

void handle_set_room_method(JsonDocument doc) {
  // Accept only null or a valid string as result for sensor_id
  const char *sensor_id = nullptr;
  if (doc["result"].isNull()) {
    // If result is null, treat as no sensor assigned
    sensor_id = nullptr;
  } else if (doc["result"].is<const char *>()) {
    // If result is a string, assign it to sensor_id
    sensor_id = doc["result"];
  } else {
    // If result is not null and not a string, treat as invalid and do nothing
    return;
  }
  if (strcmp(resp_session_data->sensor_id, "") == 0) {
    if (sensor_id ==
        nullptr) { // if sensor id is nullptr, means no sensor id, i.e., memset the buffer
      memset(resp_session_data->sensor_id, '\0', sizeof(resp_session_data->sensor_id));
    } else {
      // save the sensor id, to which the selected room is assigned
      if (strlen(sensor_id) < sizeof(resp_session_data->sensor_id)) {
        strncpy(resp_session_data->sensor_id, sensor_id, sizeof(resp_session_data->sensor_id) - 1);
      } else {
        return;
      }
    }
  } else {
    memset(resp_session_data->sensor_id, '\0',
           sizeof(resp_session_data
                      ->sensor_id)); // save the sensor id, to which the selected room is assigned
  }

  lv_obj_t *scr = screen_settings_select_room_get();

  if (strcmp(resp_session_data->sensor_id, "") == 0) {
    // bug fix: assign the sensor id though it is nullptr or any value. so that the previous
    // sensor id will be cleared, if next time nullptr comes
    memset(resp_session_data->sensor_id, '\0',
           sizeof(resp_session_data
                      ->sensor_id)); // save the sensor id, to which the selected room is assigned
    // show success popup
    static rm_num_info_t *rm_num_info = rm_num_info_get(); // get the room number info
    static const char *btns[] = {"Reassign", "Continue"};
    char msg[50] = "Room ";
    strcat(msg, rm_num_info->selected_rm_num);
    strcat(msg, "\nAssigned Successfully!");
    static const int btns_padding[4] = {32, 32, 32,
                                        32}; // popup container padding (top, bottom, left, right)
    static const int btns_size[3] = {352, 168,
                                     64}; // buttons container width, button width and button height
    create_twoBtn_msgbox(scr, &icon_room_assign_success, msg, NULL, btns, btns_padding, btns_size);
  } else {
    // show room already assigned screen
    static rm_num_info_t *rm_num_info = rm_num_info_get();
    // formulate message 1 (Room 0000000 is already associated with Sensor 123456789.)
    char msg1[100] = "Room ";                   // default text
    strcat(msg1, rm_num_info->selected_rm_num); // concatanate the room number selected
    strcat(msg1, " is already\n associated with System\n ");
    strcat(msg1, sensor_id); // concatanate the sensor id
    strcat(msg1, ".");       // end the sentence

    // formulate message 2 (Please confirm that you would like to use the current sensor for Room
    // 0000000.)
    char msg2[100] =
        "Please confirm that you would like to use the \ncurrent system for Room "; // default
                                                                                    // text
    strcat(msg2, rm_num_info->selected_rm_num); // concatanate the room number selected
    strcat(msg2, ".");                          // end the sentence
    static const char *btns[] = {"Cancel", "Confirm"};
    static const int btns_padding[4] = {32, 16, 16,
                                        16}; // popup container padding (top, bottom, left, right)
    static const int btns_size[3] = {384, 184,
                                     64}; // buttons container width, button width and button height
    create_twoBtn_msgbox(scr, &icon_room_assign_generic, msg1, msg2, btns, btns_padding, btns_size);
  }
}

void handle_get_room_view_method(JsonDocument doc) {

  // show room view page
  lv_scr_load(screen_settings_room_view_get());

  display_image_room_view_check(doc);
}

void handle_set_room_view_method(JsonDocument doc) {
  SETTINGS_MODE settings_mode = screen_settings_get_mode();
  if (settings_mode == SETTINGS_MODE_SAVE) {
    // show success popup
    lv_obj_t *scr = screen_settings_room_view_get();
    static const char *btns[] = {"Settings", "Back to Home"};
    static const int btns_padding[4] = {32, 32, 32,
                                        32}; // popup container padding (top, bottom, left, right)
    static const int btns_size[3] = {352, 168,
                                     64}; // buttons container width, button width and button height
    create_twoBtn_msgbox(scr, &icon_room_assign_success, "Room View Updated!", NULL, btns,
                         btns_padding, btns_size);
  } else if (settings_mode == SETTINGS_MODE_ACTIVATION) {
    // show bms configuration page
    lv_scr_load(screen_settings_bms_get());
  }
}

void handle_get_languages_method(JsonDocument doc) {
  // Get the language info structure
  static lang_info_t *lang_info = lang_info_get();
  lang_info->available_language_count = 0; // Reset language count

  // Extract the array of languages from the JSON response
  JsonArray langArr = doc["result"];
  bool valid = true; // Assume the response is valid
  int item_cnt = 0;

  // First, check that all items in the array are strings
  for (JsonVariant item : langArr) {
    if (!item.is<const char *>()) {
      valid = false; // Found a non-string item, mark as invalid
      break;
    }
    const char* str = item.as<const char*>();
    for (int i = 0; str[i] != '\0'; ++i) {
        if (!isalpha((unsigned char)str[i]) && str[i] != ' ') {
            valid = false; // Found a non-alphabet character
            break;
        }
    }
    if (!valid) break;
  }

  // If all items are valid strings, copy them into the lang_info struct
  if (valid) {
    for (JsonVariant item : langArr) {
      if (item_cnt < 50) { // Limit to 50 languages
        const char *arr = item;
        if(strcmp(arr, "") == 0) {
          continue; // Skip empty strings
        }
        // Copy the language string into the struct, ensuring null-termination
        strncpy(lang_info->available_languages[item_cnt], arr,
                sizeof(lang_info->available_languages[item_cnt]) - 1);
        item_cnt++;
      } else {
        break; // Stop if maximum count is reached
      }
    }
  }

  // Update the total number of valid languages found
  if (item_cnt > 0) {
    lang_info->available_language_count = item_cnt;
    lv_obj_t *screen = get_audio_language_settings_screen();
    lv_scr_load(screen);
  }
  else if(flag_lang_selection) {
    handle_language_change_timeout();
  }
}

void handle_json_resp_result(JsonDocument doc) {
  uint32_t respId;
  if (doc.containsKey("id")) {
    respId = doc["id"];
  }

  resp_session_data = resp_session_data_get();

  // If the response ID matches the previously sent request ID then process the data accordingly
  if (respId == resp_session_data->resp_id) {
    if (strcmp(resp_session_data->response_method, cmd_str[CMD_GET_ROOM]) == 0) {
      handle_get_room_method(doc);
    } else if (strcmp(resp_session_data->response_method, cmd_str[CMD_GET_UNITS]) == 0) {
      handle_unit_number_method(doc);
    } else if (strcmp(resp_session_data->response_method, cmd_str[CMD_GET_ROOMS]) == 0) {
      handle_room_number_method(doc);
    } else if (strcmp(resp_session_data->response_method, cmd_str[CMD_SET_ROOM]) == 0) {
      handle_set_room_method(doc);
    } else if (strcmp(resp_session_data->response_method, cmd_str[CMD_GET_ROOM_VIEW]) == 0) {
      handle_get_room_view_method(doc);
    } else if (strcmp(resp_session_data->response_method, cmd_str[CMD_SET_ROOM_VIEW]) == 0) {
      handle_set_room_view_method(doc);
    } else if (strcmp(resp_session_data->response_method, cmd_str[CMD_GET_SYS_INFO]) == 0) {
      handle_sys_info_method(doc);
    } else if (strcmp(resp_session_data->response_method, cmd_str[CMD_GET_LANGUAGES]) == 0) {
      handle_get_languages_method(doc);
    } else {
      // ToDo: Decide what to do if the response method not matches
    }
    resp_method_clear(resp_session_data);
  } else {
    // ToDo: Decide what to do if the id not matches
    if(flag_lang_selection && (strcmp(resp_session_data->response_method, cmd_str[CMD_GET_LANGUAGES]) == 0)) {
      handle_language_change_timeout();
    }
  }
}

void handle_json_resp_error(JsonDocument doc) {
  // Get error code and error message
  JsonObject error;
  error = doc["error"];

  for (JsonPair p : error) {
    const char *key = p.key().c_str();
    if (strcmp(key, "code") == 0) {
      room_resp_error_code = p.value();
    } else if (strcmp(key, "message") == 0) {
      if (p.value() != nullptr) {
        strncpy(room_resp_error_msg, (const char *)(p.value()), sizeof(room_resp_error_msg) - 1);
      }
    }
  }
  // Get response id
  uint32_t respId;
  if (doc.containsKey("id")) {
    respId = doc["id"];
  }

  resp_session_data = resp_session_data_get();       // get the session data
  lv_obj_t *scr = screen_settings_select_room_get(); // parent screen to show popup message

  // If the response ID matches the previously sent request ID then process the data accordingly
  if (respId == resp_session_data->resp_id) {
    if (strcmp(resp_session_data->response_method, cmd_str[CMD_SET_ROOM]) == 0) {
      // show room assign failed message
      static const char *btns[] = {"Cancel", "Try Again"};
      // formulate message 1 (Room 0000000 Assignment Failed)
      static rm_num_info_t *rm_num_info = rm_num_info_get();
      char msg1[100] = "Room ";                   // default text
      strcat(msg1, rm_num_info->selected_rm_num); // concatanate the room number selected
      strcat(msg1, "\nAssignment Failed");
      static const int btns_padding[4] = {32, 32, 32,
                                          32}; // popup container padding (top, bottom, left, right)
      static const int btns_size[3] = {
          352, 168, 64}; // buttons container width, button width and button height
      create_twoBtn_msgbox(scr, &icon_room_assign_fail, msg1,
                           "Having issues? Contact help desk at\n (844) 878-2255.", btns,
                           btns_padding, btns_size);
    } else if ((strcmp(resp_session_data->response_method, cmd_str[CMD_GET_ROOM_VIEW]) == 0)) {
      // show room view error
      lv_scr_load(screen_settings_room_view_get());
      display_image_room_view_check_error();
    }
    resp_method_clear(resp_session_data);
  }
}

void handle_jsonrpc_result(JsonDocument json_in) {
  if (json_in.containsKey("result")) {
    // resultArray = json_in["result"];
    handle_json_resp_result(json_in);
  } else if (json_in.containsKey("error")) {
    handle_json_resp_error(json_in);
  }
  resp_session_data = resp_session_data_get();
}