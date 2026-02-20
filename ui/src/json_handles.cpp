#include "json_handles.h"
#include "json_response.h"

#include "backlight.h"
#include "device_info.h"
#include "device_status.h"
#include "disp_state.h"
#include "resp_state.h"
#include "sys_state.h"
#include "ui.h"

// firmware variant
NKD_VARIANT firmware_variant = NKD_VARIANT_UNSET;

NKD_VARIANT get_firmware_variant() { return firmware_variant; }

JsonRpcError validate_firmware_variant(JsonObject parameter) {
  const char key_name[20] = "variant";
  if (!parameter.containsKey(key_name)) {
    return INVALID_REQUEST;
  }
  int val = parameter[key_name];

  if ((val >= NKD_INTERACTIVE) && (val <= NKD_READ_ONLY)) {
    return JSONRPC_ERROR_NONE;
  } else {
    return INVALID_PARAMS;
  }
}

/*Firmware variant handling.
 returns invalid params if variant is not 1/2/3.
returns no error if variant is 1/2/3.
 */
// {"jsonrpc":"2.0","method":"set_variant","params":{"variant":1, "fts_avail":127},"id":1}
JsonRpcError handle_set_variant_method(JsonObject parameter, uint32_t id) {

  JsonRpcError err = JSONRPC_ERROR_NONE;
  // validate variant parametr
  err = validate_firmware_variant(parameter);
  if (err != JSONRPC_ERROR_NONE) {
    return err;
  }
  // check if the "fts_avail" key is available or not and validate it first
  // extract and update sys_state structure if fts_avail param is present
  const char key_name[20] = "fts_avail";

  if (parameter.containsKey(key_name)) {
    // screen value is paased as zero because it is irrelevant here.
    if (!sys_state_validate(key_name, parameter[key_name], 0) == true) {
      return INVALID_PARAMS;
    }
  } else {
    return INVALID_REQUEST;
  }

  // get and clear the sys_state structure
  sys_state_t *ss = sys_state_get();
  sys_state_clear(ss);
// screen value is paased as zero because it is irrelevant here.
  bool ok = sys_state_set(key_name, parameter[key_name], 0);
  if (!ok) {
    return INVALID_PARAMS;
  }

  // close any message boxes left from previous variant session
  close_active_msgbox();

  lv_obj_t *tempScreen;
  if (parameter["variant"] == 1) {
    firmware_variant = NKD_INTERACTIVE;
    tempScreen = scr_home_inactive_get();
    // LV_LOG_USER("NKD_INTERACTIVE");
  } else if (parameter["variant"] == 2) {
    firmware_variant = NKD_GO;
    tempScreen = scr_home_inactive_get();
    // LV_LOG_USER("NKD_GO");
  } else if (parameter["variant"] == 3) {
    firmware_variant = NKD_READ_ONLY;
    tempScreen = screen_variant_notset_get();
    // LV_LOG_USER("NKD_READ_ONLY");
  } else {
    firmware_variant = NKD_VARIANT_UNSET;
    tempScreen = screen_variant_notset_get();
    // LV_LOG_USER("NKD_VARIANT_UNSET");
    return INVALID_PARAMS;
  }

#ifndef SIMULATOR
  disp_state_set_brightness(BRIGHTNESS_DEFAULT);          // to increse brightness during set variant
  turn_off_als(); // to turn off ALS during set variant
  lv_disp_trig_activity(NULL); // trigger a dummy activity on display, to avoid sleep
#endif
  // load the screen
  lv_screen_load(tempScreen);
  // if the screen is already inactive home then lvgl won't create a load event. so create a new
  // screen load event manually
  lv_obj_send_event(tempScreen, lv_event_code_t(LV_EVENT_SCREEN_LOAD_START), NULL);
  return JSONRPC_ERROR_NONE;
}

void handle_get_device_info_method(uint32_t id) {
  JsonDocument doc;
  JsonObject result = doc["result"].to<JsonObject>();

  dev_info_t *dev_info = get_dev_info(); // returns the device info structure pointer
  result["mfr"].set(dev_info->mfr);
  result["model"].set(dev_info->model);
  result["serial"].set(dev_info->serialMac);
  result["hw_ver"].set(dev_info->hwVer);
  result["fw_ver"].set(dev_info->fwVer);

  serialize_jsonrpc_response(id, result); // to compose json object and send out the data
}

void handle_get_device_status_method(uint32_t id) {
  JsonDocument doc;
  JsonObject result = doc["result"].to<JsonObject>();

  dev_status_t *dev_status = get_dev_status(); // returns the device status structure pointer
  result["uptime"].set(dev_status->uptime);

  serialize_jsonrpc_response(id, result); // to compose json object and send out the data
}

bool toast_message_validate_set(const char *key, const char *msg) {
  if (strcmp(key, "message") == 0) {
    // if msg length is in between 1 to 26 then display the message on the current screen.
    // save the message to a global variable, so that it can be used all over the project.
    if (msg != NULL) {
      if ((strlen(msg) > 0) && (strlen(msg) <= 26)) {
        // if the current screen is home inactive, home active, deactivating or variant notset
        //  then only show the toast message.
        lv_obj_t *screen_active = lv_screen_active();
        if (screen_active == screen_get(SCREEN_HOME_INACTIVE) ||
            screen_active == screen_get(SCREEN_HOME_ACTIVE) ||
            screen_active == screen_get(SCREEN_DEACTIVATING) ||
            screen_active == screen_get(SCREEN_VARIANT_NOTSET) ||
            screen_active == screen_get(SCREEN_ACTIVATING)) {
          firmware_status_toast_show(screen_active, msg);
        } else {
          // if the current screen is not one of the above mentioned screens then make the screen
          // value present in set_state as parent and show the toast message.
          const sys_state_t *ss_t = sys_state_get();
          screen_active = screen_get(ss_t->screen);
          firmware_status_toast_show(screen_active, msg);
        }

        return true;
      } else if (strcmp(msg, "") == 0) { // if the msg is empty then hide the toast.
        firmware_status_toast_hide();
        return true;
      } else {
        return false;
      }
    } else {
      return false;
    }
  } else {
    return false;
  }
}

JsonRpcError handle_set_toast_method(const JsonObject params, char *data, size_t datasize) {
  // validate the message, i.e., no.of characters in the message should be not
  // more than 26.
  for (JsonPair p : params) {
    const char *key = p.key().c_str();
    bool res = toast_message_validate_set(key, p.value());
    if (res == false) {
      strncpy(data, key, datasize - 1);
      return INVALID_PARAMS;
    }
  }
  return JSONRPC_ERROR_NONE;
}

JsonRpcError handle_set_disp_method(const JsonObject params, char *data, size_t datasize) {

  if (screen_settings_display_get() == lv_scr_act()) {
    return SCREEN_LOCKED_BY_USER;
  }

  // validate all params before updating any of the state vars.
  for (JsonPair p : params) {
    const char *key = p.key().c_str();
    bool ok = disp_state_validate(key, p.value());
    if (!ok) {
      strncpy(data, key, datasize - 1);
      //   LV_LOG_USER("invalid param: %s", key);
      return INVALID_PARAMS;
    }
  }

  // update system state if validation is successfull
  for (JsonPair p : params) {
    const char *key = p.key().c_str();
    bool ok = disp_state_set(key, p.value());
    if (!ok) {
      strncpy(data, key, datasize - 1);
      //   LV_LOG_USER("invalid param: %s", key);
      return INVALID_PARAMS;
    }
  }

  return JSONRPC_ERROR_NONE;
}

// todo: also check if pause_tmr is set without setting pause

// check if multiple parameters are set
static JsonRpcError has_multiple_params_set(const JsonObject params, char *data, size_t datasize) {
  uint8_t set_count = 0;

  ALERT alert = params["alert"];
  const char *alert_title = params["alert_title"] | "";
  if (alert != ALERT_NONE || strcmp(alert_title, "") != 0) {
    set_count++;
    strncat(data, "alert ", datasize - strlen(data) - 1);
  }

  SYSERR syserr = params["syserr"];
  const char *syserr_title = params["syserr_title"] | "";
  if (syserr != SYSERR_NONE || strcmp(syserr_title, "") != 0) {
    set_count++;
    strncat(data, "syserr ", datasize - strlen(data) - 1);
  }

  CALIBRATION cal = params["cal"];
  if (cal > 0) {
    set_count++;
    strncat(data, "cal ", datasize - strlen(data) - 1);
  }

  uint16_t pause_tmr = params["pause_tmr"];
  if (pause_tmr > 0) {
    set_count++;
    strncat(data, "pause_tmr ", datasize - strlen(data) - 1);
  }

  if (set_count > 1) {
    return MULTIPLE_PARAMS_SET; // Only one parameter is set
  }
  return JSONRPC_ERROR_NONE;
}

static bool check_response_bms(const JsonObject params, sys_state_t *req_state_inflight) {
  BMS bms_req = req_state_inflight->bms;
  BMS bms_resp = params["bms"];
  // LV_LOG_USER("bms_req:%d   bms_resp:%d\n", bms_req, bms_resp);
  if (bms_resp != bms_req) {
    // strncpy(data, "bms", datasize - 1);
    return false;
  }
  return true;
}

// check bed width response
static bool check_response_bed_width(const JsonObject params, sys_state_t *req_state_inflight) {
  uint8_t bed_wid_req = req_state_inflight->bed_wid;
  uint8_t bed_wid_resp = params["bed_wid"];
  if (bed_wid_resp != bed_wid_req) {
    return false;
  }
  return true;
}

// check bed position response
static bool check_response_bed_position(const JsonObject params, sys_state_t *req_state_inflight) {
  BED_POS bed_pos_req = req_state_inflight->bed_pos;
  BED_POS bed_pos_resp = params["bed_pos"];
  if (bed_pos_resp != bed_pos_req) {
    return false;
  }
  return true;
}

// check occupant size response
static bool check_response_occupant_size(const JsonObject params, sys_state_t *req_state_inflight) {
  OCC_SIZE occ_size_req = req_state_inflight->occ_size;
  OCC_SIZE occ_size_resp = params["occ_size"];
  if (occ_size_resp != occ_size_req) {
    return false;
  }
  return true;
}

static bool check_response_display(const JsonObject params, sys_state_t *req_state_inflight) {
  ABC abc_req = req_state_inflight->abc;
  ABC abc_resp = params["abc"];
  if (abc_resp != abc_req) {
    // strncpy(data, "disp", datasize - 1);
    return false;
  }
  return true;
}

static bool check_response_deactivate_shutdown(const JsonObject params,
                                               sys_state_t *req_state_inflight) {
  SCREEN screen_resp = params["screen"];
  if (screen_resp != SCREEN_DEACTIVATING && screen_resp != SCREEN_VARIANT_NOTSET) {
    //  strncpy(data, "screen", datasize - 1);
    return false;
  }
  return true;
}

static bool check_response_audio(const JsonObject params, sys_state_t *req_state_inflight) {
  AUDIO audio_req = req_state_inflight->audio;
  AUDIO audio_resp = params["audio"];
  if (audio_resp != audio_req) {
    //  strncpy(data, "audio", datasize - 1);
    return false;
  }
  if (audio_req == AUDIO_OFF) {
    return true;
  }

  const char *lang_req = req_state_inflight->lang;
  const char *lang_resp = params["lang"];

  if (lang_resp != NULL && lang_req != NULL) {
    if (strcmp(lang_resp, lang_req) != 0) {
      //   strncpy(data, "lang", datasize - 1);
      return false;
    }
  } else {
    //   strncpy(data, "lang", datasize - 1);
    return false;
  }

  VOLUME vol_req = req_state_inflight->vol;
  VOLUME vol_resp = params["vol"];

  if (vol_resp != vol_req) {
    // strncpy(data, "vol", datasize - 1);
    return false;
  }

  return true;
}

// check exit alert schedule response
static bool check_response_exit_alert_schedule(const JsonObject params,
                                               sys_state_t *req_state_inflight) {
  char mon_start_req[5], mon_end_req[5], mon_start_resp[5], mon_end_resp[5];
  // Copy request state values
  strncpy(mon_start_req, req_state_inflight->mon_start, sizeof(mon_start_req));
  strncpy(mon_end_req, req_state_inflight->mon_end, sizeof(mon_end_req));

  // Copy response state values
  strncpy(mon_start_resp, params["mon_start"], sizeof(mon_start_resp));
  strncpy(mon_end_resp, params["mon_end"], sizeof(mon_end_resp));

  // compare request and response values
  if (strcmp(mon_start_resp, mon_start_req) != 0) {
    return false;
  }
  if (strcmp(mon_end_resp, mon_end_req) != 0) {
    return false;
  }
  return true;
}

static bool check_response_alerts(const JsonObject params, sys_state_t *req_state_inflight) {
  // extract fts_state struct from request sys_state
  uint16_t fts_state_req_dec_val = req_state_inflight->fts_state.dec_val;

  // extarct fts_state from params which is decimal value.
  int fts_state_resp_dec_val = params["fts_state"];

  // compare the request and response fts_state decimal values
  if (fts_state_resp_dec_val != fts_state_req_dec_val) {
    // strncpy(data, "fts_state", datasize - 1);
    return false;
  } else {
    return true;
  }
}

bool check_response(const JsonObject params, sys_state_t *req_state_inflight,
                    CMD req_method_inflight) {

  switch (req_method_inflight) {
  case CMD_NONE:
    return true;
    break;
  case CMD_SAVE_BMS:
    return check_response_bms(params, req_state_inflight);
    break;
  case CMD_SAVE_AUDIO:
    return check_response_audio(params, req_state_inflight);
    break;
  case CMD_SAVE_DISPLAY:
    return check_response_display(params, req_state_inflight);
    break;
  case CMD_SAVE_BED_WIDTH:
    return check_response_bed_width(params, req_state_inflight);
    break;
  case CMD_SAVE_BED_POS:
    return check_response_bed_position(params, req_state_inflight);
    break;
  case CMD_SAVE_OCCUPANT_SIZE:
    return check_response_occupant_size(params, req_state_inflight);
    break;
  case CMD_SAVE_ALERTS:
    return check_response_alerts(params, req_state_inflight);
    break;
  case CMD_ACTIVATE_SENSOR: {
    bool err = check_response_bms(params, req_state_inflight);
    if (err == false) {
      return false;
    }
    err = check_response_audio(params, req_state_inflight);
    if (err == false) {
      return false;
    }
    err = check_response_alerts(params, req_state_inflight);
    if (err == false) {
      return false;
    }
    // check only those parameters which are available in fts_avail
    sys_state_t *ss = sys_state_get();

    // Check scheduled monitoring response
    if (ss->fts_avail.sch_mon) {
      err = check_response_exit_alert_schedule(params, req_state_inflight);
      if (err == false) {
        return false;
      }
    }

    // Check bed width response
    if (ss->fts_avail.bed_wid) {
      err = check_response_bed_width(params, req_state_inflight);
      if (err == false) {
        return false;
      }
    }

    // Check bed position response
    if (ss->fts_avail.bed_pos) {
      err = check_response_bed_position(params, req_state_inflight);
      if (err == false) {
        return false;
      }
    }

    // Check occupant size response
    if (ss->fts_avail.occ_size) {
      err = check_response_occupant_size(params, req_state_inflight);
      if (err == false) {
        return false;
      }
    }

    return true;
    break;
  }
  case CMD_ACTIVATE_BED_MODE:
  case CMD_ACTIVATE_CHAIR_MODE: {
    // MONITOR_MODE mode_req = req_state_inflight->mode;
    // MONITOR_MODE mode_resp = params["mode"];
    // if (mode_resp != mode_req) {
    //   LV_LOG_USER("mode_resp:%d != mode_req:%d", mode_resp, mode_req);
    //   strncpy(data, "mode", datasize - 1);
    //   return INVALID_RESPONSE;
    // }
    // return JSONRPC_ERROR_NONE;
    break;
  }
  case CMD_PAUSE_SENSOR: {
    //   uint16_t pause_tmr = params["pause_tmr"];
    //   if (pause_tmr <= 0) {
    //     strncpy(data, "pause_tmr", datasize - 1);
    //     return INVALID_RESPONSE;
    //   }
    //   return JSONRPC_ERROR_NONE;
    break;
  }
  case CMD_CLEAR_ALERT: {
    break;
  }
  case CMD_RESUME_FALL_MONITORING: {
    break;
  }
  case CMD_DEACTIVATE_SENSOR:
  case CMD_SHUTDOWN:
    return check_response_deactivate_shutdown(params, req_state_inflight);
    break;
    //{
    //   SCREEN screen = params["screen"];
    //   if (screen != SCREEN_DEACTIVATING) {
    //     strncpy(data, "screen", datasize - 1);
    //     return INVALID_RESPONSE;
    //   }
    //   return JSONRPC_ERROR_NONE;
    // break;
    // }

  case CMD_SAVE_MON_SCH:
    return check_response_exit_alert_schedule(params, req_state_inflight);
    break;
  default: {
    return false;
    break;
  }
  }
  return true;
}

JsonRpcError handle_set_state_method(const JsonObject params, char *data, size_t datasize) {
  JsonRpcError err;

  // if (request_method_inflight) {
  //   err = check_response(params, data, datasize, request_state_inflight,
  //   request_method_inflight); if (err) {
  //     return err;
  //   }
  // }

  // Check for "fts_avail" and "fts_state" key presence. If not available then reject set_state
  // command
  if (!(params.containsKey("fts_avail"))) {
    strcpy(data, "fts_avail"); // update fts_avail in error response data parameter
    return INVALID_REQUEST;
  } else if (!(params.containsKey("fts_state"))) {
    strcpy(data, "fts_state"); // update fts_avail in error response data parameter
    return INVALID_REQUEST;
  }

  // check if multiple parameters are set
  err = has_multiple_params_set(params, data, datasize);
  if (err) {
    return err;
  }

  SCREEN screen_t;
  MONITOR_MODE mode_t;
  BMS bms_t;
  char room_number_t[11] = "";
  // validate screen, mode, bms and room number parameters before updating the set_state structure
  if (params.containsKey("screen")) {
    screen_t = params["screen"];
  } else {
    strcpy(data, "screen"); // in error response data parameter is updated with screen
    return INVALID_REQUEST;
  }
  if (params.containsKey("mode")) {
    mode_t = params["mode"];
  } else {
    strcpy(data, "mode"); // in error response data parameter is updated with mode
    return INVALID_REQUEST;
  }
  // look for bms only if the mode is bed mode
  if (mode_t == MODE_BED) {
    if (params.containsKey("bms")) {
      bms_t = params["bms"];
    } else {
      strcpy(data, "bms"); // in error response data parameter is updated with bms
      return INVALID_REQUEST;
    }
  }
  if (params.containsKey("room_number")) {
    strncpy(room_number_t, params["room_number"], sizeof(room_number_t) - 1);
    // LV_LOG_USER("room_number_t: %s", room_number_t);
  } else {
    strcpy(data, "room_number"); // in error response data parameter is updated with room_number
    return INVALID_REQUEST;
  }

  if (screen_t == SCREEN_HOME_INACTIVE || screen_t == SCREEN_HOME_ACTIVE ||
      screen_t == SCREEN_DEACTIVATING || screen_t == SCREEN_VARIANT_NOTSET ||
      screen_t == SCREEN_ACTIVATING) {
    if (mode_t == MODE_BED) {
      // Reject mode=1 (bed mode) without bms
      if (bms_t == BMS_LOW || bms_t == BMS_HIGH || bms_t == BMS_ULTRA_HIGH) {
        err = JSONRPC_ERROR_NONE;
      } else {
        strcpy(data, "bms"); // in error response data parameter is updated with room_number
        return INVALID_PARAMS;
      }
    }
  } else {
    strcpy(data, "screen"); // in error response data parameter is updated with screen
    return INVALID_PARAMS;
  }
  // Reject if room number is empty in set_state command only if screen is active screen
  if (screen_t == SCREEN_HOME_ACTIVE) {
    if (strcmp(room_number_t, "") == 0) {
      strcpy(data, "room_number"); // in error response data parameter is updated with room_number
      return INVALID_PARAMS;
    } else {
      err = JSONRPC_ERROR_NONE;
    }
  }

  // extract screen value and use it for fts_state validation.
  //  Since fts_state validation depends on the screen value.
  int screen_value = -1; // Initialize screen_value to something invalid

  // Step 1: First, get the screen value if it exists
  for (JsonPair p : params) {
    const char *key = p.key().c_str();
    if (key != NULL) {
      if (strcmp(key, "screen") == 0) {
        // Track "screen" value here
        screen_value = p.value().as<int>();
        break;  // Once we find screen, no need to continue the loop
      }
    } else {
      return INVALID_PARAMS;
    }
  }

  // validate all params before updating any of the state vars.
  for (JsonPair p : params) {
    const char *key = p.key().c_str();
    if (key != NULL) {
      bool ok = sys_state_validate(key, p.value(), screen_value);
      if (!ok) {
        strncpy(data, key, datasize - 1);
        // LV_LOG_USER("S2 invalid param: %s", key);
        return INVALID_PARAMS;
      }
    } else {
      return INVALID_PARAMS;
    }
  }

  sys_state_t *ss = sys_state_get();
  sys_state_clear(ss);

  // update system state if validation is successfull
  for (JsonPair p : params) {
    const char *key = p.key().c_str();
    //  LV_LOG_USER("key: %s", key);
    if (key != NULL) {
      bool ok = sys_state_set(key, p.value(), screen_value);
      if (!ok) {
        strncpy(data, key, datasize - 1);
        return INVALID_PARAMS;
      }
    } else {
      return INVALID_PARAMS;
    }
  }

#if !SENSCAP_EN // abc is not available in senscap firmware
  // update display settings if abc is available in the params and the user is not in settings
  // disply screen
  NKD_VARIANT firmware_variant = get_firmware_variant();
  if (firmware_variant == NKD_INTERACTIVE || firmware_variant == NKD_GO) {
    if (screen_settings_display_get() != lv_scr_act()) {
      if (params.containsKey("abc")) {
        update_display_brightness_settings();
      }
    }
  }
#endif

  return err;
}

void handle_sys_info_method(JsonDocument doc) {
  // extract the comp_ID and sw_ver from params and push to resp_session_data struct
  JsonObject result;
  result = doc["result"];
  // if either "comp_id" or "sw_ver" is missing from the result then return without updating the
  // resp_session_data struct
  if (!result.containsKey("comp_id") || !result.containsKey("sw_ver")) {
    return;
  } else {
    const char *comp_id = result["comp_id"] | "";
    const char *sw_ver = result["sw_ver"] | "";

    static resp_session_data_t *resp_session_data = resp_session_data_get();
    strncpy(resp_session_data->compute_id, comp_id, sizeof(resp_session_data->compute_id) - 1);
    resp_session_data->compute_id[sizeof(resp_session_data->compute_id) - 1] = '\0';
    strncpy(resp_session_data->sw_ver, sw_ver, sizeof(resp_session_data->sw_ver) - 1);
    resp_session_data->sw_ver[sizeof(resp_session_data->sw_ver) - 1] = '\0';
    // load the system info screen if it is waiting for the response
    lv_scr_load(screen_system_info_get());
  }
}
