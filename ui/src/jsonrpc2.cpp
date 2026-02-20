#include "jsonrpc2.h"
#include "json_handles.h"
#include "json_response.h"
// #include "api.h"
#include "app.h"
#include "backlight.h"

#include "disp_state.h"
#include "lvgl.h"
#include "resp_state.h"
#include "sys_state.h"
#include "ui.h"
#include "touch.h"

#include <cstdbool>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>

const char *no_response_str = "No Response!";
const char *invalid_response_str = "Invalid Response!";

uint32_t MY_EVENT_REQUEST_TIMEOUT, MY_EVENT_KEEPALIVE_TIMEOUT, MY_EVENT_IDLE_TIMEOUT;
// triggered after the respective method handlers return a success.
uint32_t MY_EVENT_SET_STATE_OK, MY_EVENT_SET_STATE_FAIL, MY_EVENT_SET_DISP;

uint32_t MY_EVENT_SENSOR_TOGGLE;

#define NUC_RESET_PIN 46 // NUC reset pin, GPIO 46

// maximum size of jsonrpc2 request, in bytes.
#define REQUEST_SIZE_MAX 1000
#define RESPONSE_SIZE_MAX 500
#define INPUT_DATA_SIZE 1000

// maximum size of json document for deserializing request
// todo: make an equivalent for response size?
static const size_t jsondoc_size = 1.5 * REQUEST_SIZE_MAX;

// todo: add keepalive timer
static lv_timer_t *read_timeout_timer, *keep_alive_timer;
static lv_timer_t *activating_timer;
static bool screen_activating_handled = false;
#if (SIMULATOR == 0) && (SENSCAP_EN == 0)
static lv_timer_t *nuc_rstpin_low_timer;
#endif
// static jsonrpc_event_cb_t jsonrpc_event_cb;

static sys_state_t *request_state_inflight;
static CMD request_method_inflight = CMD_NONE;

static bool sensor_toggle_inprogress = false;

// char serDataBuf[200];//200
// bool flgNewData = false;
#ifndef SIMULATOR
xSemaphoreHandle serDataMutex;
QueueHandle_t xQueue;
#endif

static bool flg_nuc_reset_enable = false;

// Stores the recently sent request session data
static resp_session_data_t *resp_session_data;
static lv_timer_t *room_resp_timeout_timer; // timer to check room response timeout
bool room_resp_timeout = false;             // true if timedout, false if not
bool flg_disp_sleep = false;                // during sleep, first touch event for screen wakeup

lv_timer_t *assign_room_timeout_timer; // timer to check assign room response timeout

static void jsonrpc_event_cb(lv_event_code_t code) {
  if (code == MY_EVENT_REQUEST_TIMEOUT) {
    // LV_LOG_USER("MY_EVENT_REQUEST_TIMEOUT");
    sensor_toggle_inprogress = false;
    lv_obj_send_event(lv_screen_active(), code, NULL);
    // LV_LOG_USER("jsonrpc dbg 1");
  } else if (code == MY_EVENT_SET_STATE_OK) {
    // LV_LOG_USER("MY_EVENT_SET_STATE_OK");

    const sys_state_t *ss = sys_state_get();
    lv_obj_t *screen = screen_get(ss->screen);
    lv_obj_t *screen_active = lv_screen_active();

    if (is_landing_screen(screen_active) || sensor_toggle_inprogress == true) {
      // LV_LOG_USER("jsonrpc dbg 2 and screen value is %d", ss->screen);
      lv_screen_load(screen);
      lv_obj_send_event(screen, code, NULL);
    } else {
      // notify active screen
      lv_obj_send_event(screen_active, code, NULL);
      // LV_LOG_USER("jsonrpc dbg 3");
    }
    sensor_toggle_inprogress = false;
  } else if (code == MY_EVENT_SET_STATE_FAIL) {
    // LV_LOG_USER("MY_EVENT_SET_STATE_FAIL");
    lv_obj_t *screen_active = lv_screen_active();
    lv_obj_send_event(screen_active, code, NULL);
    // LV_LOG_USER("jsonrpc dbg 4");
    sensor_toggle_inprogress = false;
  } else if (code == MY_EVENT_SET_DISP) {
    // LV_LOG_USER("MY_EVENT_SET_DISP");
  } else if (code == MY_EVENT_SENSOR_TOGGLE) {
    sensor_toggle_inprogress = true;
  } else if (code == MY_EVENT_KEEPALIVE_TIMEOUT) {
    lv_obj_t *screen_active = lv_screen_active();
    lv_obj_send_event(screen_active, code, NULL);
    // LV_LOG_USER("jsonrpc dbg 5");
  }
}

lv_obj_t *screen_get(SCREEN screen) {
  switch (screen) {
  case SCREEN_HOME_INACTIVE:
  case SCREEN_UNSET:
    return scr_home_inactive_get();
    break;
  case SCREEN_HOME_ACTIVE:
    return screen_home_active_get();
    break;
    /* case SCREEN_CALIBRATING:
       return scr_calibrating_get();
       break;
     case SCREEN_CALIBRATING_CHAIR:
       return scr_calibrating_chair_get();
       break;*/
  case SCREEN_DEACTIVATING:
    return screen_deactivating_get();
    break;
  case SCREEN_VARIANT_NOTSET:
    return screen_variant_notset_get();
    break;
  case SCREEN_ACTIVATING:
    return screen_activating_get();
    break;
  default:
    // LV_LOG_USER("unknown screen: %d", screen);
    return NULL;
    break;
  }
}

bool is_landing_screen(lv_obj_t *obj) {
  if (obj == screen_get(SCREEN_HOME_INACTIVE) || obj == screen_get(SCREEN_HOME_ACTIVE) ||
      obj == screen_get(SCREEN_DEACTIVATING) || obj == screen_get(SCREEN_VARIANT_NOTSET) ||
      obj == screen_get(SCREEN_ACTIVATING)) {
    // FIXME: Typically, we prevent screen changes while a user request is in progress — for
    // example, when on the `Settings` screen. However, we make an exception for the `Shutdown`
    // button on the `Activate Sensor` page. If the user clicks `Shutdown` and leaves the
    // confirmation popup open, we allow screen changes. This behavior was implemented for
    // simplicity, but can be revised if needed.
    close_active_msgbox();
    return true;
  }
  return false;
}

void request_state_inflight_populate(CMD method, JsonObject params, sys_state_t *req) {
  switch (method) {
  case CMD_NONE:
    break;
  case CMD_SAVE_BMS:
    req->bms = params["bms"];
    //   LV_LOG_USER("populate_bms_req:%d\n",req->bms);
    break;
  case CMD_SAVE_AUDIO: {
    req->audio = params["audio"];
    strcpy(req->lang, params["lang"] | ""); // default to empty string
    req->vol = params["vol"] | VOL_UNSET;
    break;
  }
  case CMD_SAVE_DISPLAY: {
    req->abc = params["abc"];
    break;
  }
  case CMD_SAVE_BED_WIDTH: {
    req->bed_wid = params["width"];
    break;
  }
  case CMD_SAVE_BED_POS: {
    req->bed_pos = params["pos"];
    break;
  }
  case CMD_SAVE_OCCUPANT_SIZE: {
    req->occ_size = params["size"];
    break;
  }
  case CMD_SAVE_ALERTS: {
    req->fts_state.dec_val = params["fts_state"];
    break;
  }
  case CMD_ACTIVATE_SENSOR: {
    req->bms = params["bms"];
    req->audio = params["audio"];
    strcpy(req->lang, params["lang"] | ""); // default to empty string
    req->vol = params["vol"] | VOL_UNSET;

    req->fts_state.dec_val = params["fts_state"];
    // Not sending these params to backend as of now
    req->bed_wid = params["bed_wid"];
    req->bed_pos = params["bed_pos"];
    req->occ_size = params["occ_size"];
    strcpy(req->mon_start, params["mon_start"]);
    strcpy(req->mon_end, params["mon_end"]);

    break;
  }
  case CMD_ACTIVATE_BED_MODE: {
    req->mode = MODE_BED;
    break;
  }
  case CMD_ACTIVATE_CHAIR_MODE: {
    req->mode = MODE_CHAIR;
    break;
  }
  case CMD_PAUSE_SENSOR: {
    break;
  }
  case CMD_CLEAR_ALERT: {
    break;
  }
  case CMD_RESUME_FALL_MONITORING: {
    break;
  }
  case CMD_DEACTIVATE_SENSOR: {
    break;
  }
  case CMD_SAVE_MON_SCH: {
    strcpy(req->mon_start, params["mon_start"]);
    strcpy(req->mon_end, params["mon_end"]);
    break;
  }
  default: {
    break;
  }
  }
}

// call this serialize_and_send()
void serializeJsonRpcRequest(int id, CMD method, JsonObject params) {
  JsonDocument doc;

  // Populate the JSON document with the request data
  doc["jsonrpc"] = "2.0";
  doc["method"] = cmd_str[method];
  if (!params.isNull()) { // fixme:
    doc["params"] = params;
  }
  if (id) {
    doc["id"] = id;
  }

  // serialize
  char request[jsondoc_size];
  serializeJson(doc, request);
  myprint(request);

  // If the json rpc command sent is IR remote value then return after sending out the command on
  // serial. And do nothing that is present below this if condition. Same for System event
  if ((method == CMD_VIRC) || (method == CMD_SYSTEM_EVENT)) {
    // LV_LOG_USER("method is virc and returning function");
    return;
  }

  request_state_inflight_populate(method, params, request_state_inflight);
  request_method_inflight = method;
  // LV_LOG_USER("jsonrpc, serializeJsonRpcRequest, request_method_inflight value is %d!",
  //             request_method_inflight);

  if (method == CMD_SAVE_BMS || method == CMD_SAVE_AUDIO || method == CMD_ACTIVATE_SENSOR ||
      method == CMD_DEACTIVATE_SENSOR || method == CMD_SHUTDOWN || method == CMD_SAVE_DISPLAY ||
      method == CMD_SAVE_MON_SCH || method == CMD_SAVE_BED_WIDTH || method == CMD_SAVE_BED_POS ||
      method == CMD_SAVE_OCCUPANT_SIZE || method == CMD_SAVE_ALERTS) {
    // disable touch
    lv_indev_t *indev_touch = lv_indev_get_act();
    lv_indev_enable(indev_touch, false);
  }

  if (method == CMD_GET_ROOM || method == CMD_GET_UNITS || method == CMD_GET_ROOMS ||
      method == CMD_SET_ROOM || method == CMD_GET_ROOM_VIEW || method == CMD_SET_ROOM_VIEW ||
      method == CMD_GET_SYS_INFO || method == CMD_GET_LANGUAGES) {

    // start timer here

    if (method == CMD_SET_ROOM) {
      lv_timer_reset(assign_room_timeout_timer);
      lv_timer_resume(assign_room_timeout_timer);
    } else {
      lv_timer_reset(room_resp_timeout_timer);
      lv_timer_resume(room_resp_timeout_timer);
      // disable touch
      lv_indev_t *indev_touch = lv_indev_get_act();
      lv_indev_enable(indev_touch, false);
    }

    room_resp_timeout = false;

  } else { // if request is not related to room response, then reset and resume read timeout
           // timer.
    // start read timeout timer here
    lv_timer_reset(read_timeout_timer);
    lv_timer_resume(read_timeout_timer);
  }

  if (method == CMD_ACTIVATE_SENSOR || method == CMD_DEACTIVATE_SENSOR || method == CMD_SHUTDOWN) {
    jsonrpc_event_cb((lv_event_code_t)MY_EVENT_SENSOR_TOGGLE); // request_sensor_toggle
    // LV_LOG_USER("jsonrpc dbg MY_EVENT_SENSOR_TOGGLE");
  }
}

// JSON-RPC request handler
void process_request(int timeoutMillis) {

  // get firmware variant
  NKD_VARIANT firmware_variant = get_firmware_variant();

  // Disbale sleep mode for read only variant and enable it for interactive and traveller variant
  if ((firmware_variant == NKD_INTERACTIVE) || (firmware_variant == NKD_GO)) {
    SETTINGS_MODE setting_mode = screen_settings_get_mode();
    // Go to sleep mode and process inactivity only if the display is in save mode (display is
    // activated)
    if (setting_mode == SETTINGS_MODE_SAVE) {
      // get the current state session
      sys_state_t *ss_temp = sys_state_get();

      // check for inactive timeout and if the display is in save mode then only do the following
      if (lv_disp_get_inactive_time(NULL) > INACTIVE_TIMEOUT) {
        // LV_LOG_USER("sleep debug: process_request, inactive time: %d, setting mode: %d",
        //             lv_disp_get_inactive_time(NULL), setting_mode);
        // if any alert, error or pause timer is running, trigger dummy activity to come out of
        // sleep
        if ((ss_temp->alert != ALERT_NONE || strcmp(ss_temp->alert_title, "") != 0) ||
            (ss_temp->syserr != SYSERR_NONE || strcmp(ss_temp->syserr_title, "") != 0) ||
            (ss_temp->pause_tmr > 0)) {
#ifndef SIMULATOR
          lv_disp_trig_activity(NULL); // trigger dummy activity
          if (flg_alert2d_display == false) {
#if !SENSCAP_EN
            als_off(); // turn off als, if als is turned it will  be turned off
            flg_process_als = false;
#endif
          }
          // This flag is made false, so that first touch event is considered without neglecting
          flg_disp_sleep = false;
          disp_state_set_brightness(BRIGHTNESS_DEFAULT); // set display brightness to 80%
#endif
          process_inactivity(); // go to nearest home screen after inactive time of
                                // INACTIVE_TIMEOUT
        }
      }

      // Check for 2 min of inactivity
      if (lv_disp_get_inactive_time(NULL) > INACTIVE_TIMEOUT) {
#ifndef SIMULATOR
        if (flg_disp_inactive == false) {
          if (flg_alert2d_display == false) {
#if !SENSCAP_EN
            als_off();
            flg_process_als = false;
#endif
          }
          disp_state_set_brightness(BRIGHTNESS_MIN); // dim the backlight before going to sleep
        }
        flg_disp_sleep = true;
        flg_disp_inactive = true;
#endif
        process_inactivity(); // go to nearest home screen after inactive time of INACTIVE_TIMEOUT
      }
#ifndef SIMULATOR
      else {
        if (flg_disp_inactive == true) {
          flg_disp_inactive = false;
// Before activating sleep mode, if als was turned on, then after coming out of sleep
// mode turn on als again.
#if !SENSCAP_EN
          if (flg_alert2d_display == false) {
            if (flg_als_Enable == true) {
              disp_state_set_brightness(BRIGHTNESS_DEFAULT);
              als_on();
              flg_process_als = true;
            } else {
              disp_state_set_brightness(BRIGHTNESS_DEFAULT); // if als was not on before sleep mode, then set
                                                 // backlight to 80%
            }
          } else {
            disp_state_set_brightness(BRIGHTNESS_DEFAULT); // for alert2d display set backlight to 80%
          }
#else
          disp_state_set_brightness(BRIGHTNESS_DEFAULT); // for senscap set backlight to 80%
#endif
        }
      }
#endif
    } else if (setting_mode == SETTINGS_MODE_ACTIVATION) {
      // if the display is in activation mode, then don't go to sleep mode just process inactivity
      // check for 2 min of inactivity
      if (lv_disp_get_inactive_time(NULL) > INACTIVE_TIMEOUT) {
        process_inactivity(); // go to nearest home screen after inactive time of INACTIVE_TIMEOUT
      }
    }
  } // sleep mode disabled for read only variant

  JsonDocument json_in;
  char inputLine[INPUT_DATA_SIZE]; // serial data pull out from queue
  // if (inputLine == NULL) {
  //   inputLine = (char *)malloc(INPUT_DATA_SIZE);
  // }

  if (inputLine == NULL) {
    // LV_LOG_USER("Malloc failed:");
    return;
  }
  memset(inputLine, '\0', INPUT_DATA_SIZE);
  // LV_LOG_USER("Malloc Success in jsonrpc:");

#ifndef SIMULATOR
  xSemaphoreTake(serDataMutex, portMAX_DELAY); // take mutex
  // LV_LOG_USER("mutex locked in jsonrpc");

  int retVal = xQueueReceive(xQueue, inputLine, 0); // pull data from queue
  if (retVal != pdPASS) // if failed to receive data from queue, return from the function
  {
    // LV_LOG_USER("return value %d", retVal);
  }
  // LV_LOG_USER("completed receive data from queue");
  // LV_LOG_USER("In jsonrpc received data is %s, length: %d", inputLine, strlen(inputLine));

  xSemaphoreGive(serDataMutex); // give mutex
  // LV_LOG_USER("mutex unlocked in jsonrpc");

  if (retVal != pdPASS) // if failed to receive data from queue, return from the function
  {
    // LV_LOG_USER("failed to receive data from queue");
    return;
  }
#endif

#if SIMULATOR
  int read_err = read_input_line(inputLine, sizeof(inputLine), timeoutMillis);
  if (read_err) {
    return;
  }
#endif

  // LV_LOG_USER("received data is %s", inputLine);
  // parse input as json
  // create json doc to hold input data
  DeserializationError deser_err = deserializeJson(json_in, inputLine);
  // LV_LOG_USER("deserialisation completed");
  // flgNewData = false;

  uint32_t reqid;
  uint32_t *reqid_ptr = NULL;

  if (json_in.containsKey("id")) {
    reqid = json_in["id"];
    reqid_ptr = &reqid;
  }

  // Check for parsing errors
  if (deser_err) {
    // LV_LOG_USER("T1");
    // LV_LOG_USER("In jsonrpc received data is %s, length: %d", inputLine, strlen(inputLine));
    respond(reqid_ptr, PARSE_ERROR, deser_err.c_str());
    return;
  }

  // Now the get_sys_info response which has result in response also handled in this block.
  // So in both the variants it is handled in the same way.
  if (firmware_variant == NKD_GO || firmware_variant == NKD_INTERACTIVE) {
    // Check if it's a valid JSON-RPC 2.0 request
    if (json_in.containsKey("jsonrpc")) {
      if (json_in.containsKey("method")) { // if it is having method then go to label method.
        goto method;
      } else if (json_in.containsKey("result") || json_in.containsKey("error")) {
        if (room_resp_timeout == false) { // if response timeout not happend, then process the data
          lv_timer_pause(
              room_resp_timeout_timer); // pause the timer, so timer callback won't trigger
          lv_timer_pause(
              assign_room_timeout_timer); // pause the timer, so timer callback won't trigger
          // close the spinner screen, if it is visible
          close_spinner_screen();
          close_one_btn_msg_box();
          close_two_btn_msg_box();
          // LV_LOG_USER("Spinner closed");
          handle_jsonrpc_result(json_in); // call the response handling function
          // enable touch
          lv_indev_t *indev_touch = lv_indev_get_act();
          lv_indev_enable(indev_touch, true);
          request_method_inflight = CMD_NONE;
          // LV_LOG_USER("jsonrpc, room resp, request_method_inflight value is %d!",
          // request_method_inflight);
          return;
        }
      } else {
        respond(reqid_ptr, INVALID_REQUEST, "");
        return;
      }
    } else {
      respond(reqid_ptr, INVALID_REQUEST, "");
      return;
    }
  } else {
    goto method;
  }
method:

  // LV_LOG_USER("jsonrpc process method!");

  // Check if it's a valid JSON-RPC 2.0 request
  if (!json_in.containsKey("jsonrpc") || !json_in.containsKey("method")) {
    respond(reqid_ptr, INVALID_REQUEST, "");
    return;
  }
  // Extract the method and parameters from input request
  const char *method = json_in["method"];
  JsonObject params;
  // if params present, only then try to pull out the params
  if (json_in.containsKey("params")) {
    params = json_in["params"];
  }

  // variables to hold method processing errors.
  JsonRpcError err = JSONRPC_ERROR_NONE;
  char data[30] = "";

  // Handle the set_state method
  if (strcmp(method, "set_state") == 0) {
    // if variant is not set, then don't accept set_state command
    if (firmware_variant == NKD_VARIANT_UNSET) {
      err = VARIANT_UNSET;
      strcpy(data, "variant not set");
      respond(reqid_ptr, err, data);
      return;
    }
    // LV_LOG_USER("jsonrpc process set_state method!");
    lv_timer_reset(keep_alive_timer);
    lv_timer_resume(keep_alive_timer);

    // LV_LOG_USER("inside set_state");
    // err = handle_set_state_method(params, data, sizeof(data));
    lv_obj_t *screen_active = lv_screen_active();
    // LV_LOG_USER("jsonrpc, request_method_inflight value is %d!, error value is %d",
    // request_method_inflight, err);

    // If variant is read only display
    if (firmware_variant == NKD_READ_ONLY) {
      err = handle_set_state_method(params, data, sizeof(data));
      if (!err) {
        lv_obj_t *read_only_scr = screen_read_only_get();
        lv_screen_load(read_only_scr);
        lv_obj_send_event(read_only_scr, (lv_event_code_t)MY_EVENT_SET_STATE_OK, NULL);
      }
    } else if (request_method_inflight) {
      // Irrespective of valid or invalid response if the user not in landing screen show
      // alert/error toast
      SETTINGS_MODE settings_mode = screen_settings_get_mode();
      if (settings_mode == SETTINGS_MODE_SAVE) {
        if (!is_landing_screen(screen_active)) {
          // if not landing screen show alert toast if available
          err = handle_set_state_method(params, data, sizeof(data));
          // LV_LOG_USER("jsonrpc dbg not is_landing_screen");
          if (!err) {
            alert_toast_show();
            // LV_LOG_USER("alert toast show called form jsonrpc!");
            // return;
          }
        }
      }

      SCREEN screen_resp = params["screen"];
      if (screen_resp == SCREEN_ACTIVATING && request_method_inflight == CMD_ACTIVATE_SENSOR &&
          screen_activating_handled == false) {
        screen_activating_handled = true;
        lv_obj_t *screen = screen_activating_get();
        lv_screen_load(screen);
        return;
      } else {
        bool respOk = true;
        if (request_method_inflight != CMD_ACTIVATE_SENSOR) {
          respOk = check_response(params, request_state_inflight, request_method_inflight);
        }
        if (respOk) {
          err = handle_set_state_method(params, data, sizeof(data));
          if (!err) {
            jsonrpc_event_cb((lv_event_code_t)MY_EVENT_SET_STATE_OK);
            // LV_LOG_USER("jsonrpc dbg request_method_inflight not none");
            request_method_inflight = CMD_NONE;
            // pause read timeout timer and notify app/screens/widgets to update
            // themselves.
            if (screen_activating_handled) {
              lv_timer_pause(activating_timer);
            }
            screen_activating_handled = false;
            lv_timer_pause(read_timeout_timer);

            // enable touch
            lv_indev_t *indev_touch = lv_indev_get_act();
            lv_indev_enable(indev_touch, true);
            // check if user is in landing screen, if yes then update alert toast message if any
            if (is_landing_screen(screen_active)) {
              alert_toast_message_update(); // update the message of alert toast but don't show it
            }
          }
        }
      }
    } else if (is_landing_screen(screen_active)) {
      err = handle_set_state_method(params, data, sizeof(data));
      if (!err) {
        jsonrpc_event_cb((lv_event_code_t)MY_EVENT_SET_STATE_OK);
        // LV_LOG_USER("jsonrpc dbg is_landing_screen");
        // pause read timeout timer and notify app/screens/widgets to update
        // themselves.
        lv_timer_pause(read_timeout_timer);

        // enable touch
        lv_indev_t *indev_touch = lv_indev_get_act();
        lv_indev_enable(indev_touch, true);

        alert_toast_message_update(); // update the message of alert toast but don't show it
      }
    }
    // if not landing screen show alert toast if available
    else if (!is_landing_screen(screen_active)) {
      err = handle_set_state_method(params, data, sizeof(data));
      // LV_LOG_USER("jsonrpc dbg not is_landing_screen");
      if (!err) {
        alert_toast_show();
        // LV_LOG_USER("alert toast show called form jsonrpc!");
        // return;
      }
    }
  } // handle set_disp method
  else if (strcmp(method, "set_disp") == 0) {
    err = handle_set_disp_method(params, data, sizeof(data));
    if (!err) // if no error, then trigger a dummy activity, such that if the
              // display is in sleep mode it will come out of sleep mode else it
              // will discard the event.
    {
      lv_disp_trig_activity(NULL);
    }
  } else if (strcmp(method, "enable_watchdog") == 0) {
    JsonArray par = json_in["params"];
    // LV_LOG_USER("inside enable_watchdog");
    if (par[0] == 1) {
      flg_nuc_reset_enable = true;
      // LV_LOG_USER("Enable watchdog");

    } else if (par[0] == 0) // disable
    {
      flg_nuc_reset_enable = false;
      // LV_LOG_USER("Disable watchdog");
    }
  } else if (strcmp(method, "set_toast") == 0) {
    err = handle_set_toast_method(params, data, sizeof(data));
  } else if (strcmp(method, "get_device_info") == 0) {
    handle_get_device_info_method(reqid);
    return;
  } else if (strcmp(method, "get_device_status") == 0) {
    handle_get_device_status_method(reqid);
    return;
  } else if (strcmp(method, "set_variant") == 0) {
    // JsonArray par = json_in["params"];
    if (screen_activating_handled == true) {
      // pause activating timer
      lv_timer_pause(activating_timer);
      // make screen activating handled false
      screen_activating_handled = false;
    }
    hide_no_response_toast();
    request_method_inflight = CMD_NONE;
    err = handle_set_variant_method(params, reqid);
  } else if ((strcmp(method, "restart_nkd") == 0) || (strcmp(method, "restart_display") == 0)) {
// restart esp32 using esp.restart()
#ifndef SIMULATOR
    esp_restart();
#endif
  }
#if (SIMULATOR == 0) && (SENSCAP_EN == 0)
  else if(strcmp(method, "set_liquid_touch_filter") == 0){
    int enable = params["enable"];
    set_liquid_touch_filter(enable);
  }else if(strcmp(method, "set_liquid_touch_debug_logs") == 0){
    int enable = params["enable"];
    set_liquid_touch_filter_debug_logs(enable);
  } 
#endif
  else {
    err = METHOD_NOT_FOUND;
  }

  // respond only if request has an id
  if (reqid_ptr != NULL) {

    respond(reqid_ptr, err, data);
    // char printMsg[100];
    // sprintf(printMsg,"reqid_ptr err: %ld", err);
    // myprint(printMsg);
  }
}

// Activating timeout callback function
static void activating_timeout_cb(lv_timer_t *timer) {
  // pause activating timer
  lv_timer_pause(timer);
  // set request method inflight to none
  request_method_inflight = CMD_NONE;
  // send timeout event
  jsonrpc_event_cb((lv_event_code_t)MY_EVENT_REQUEST_TIMEOUT);
  // make screen activating handled false
  screen_activating_handled = false;
  lv_indev_t *indev_touch = lv_indev_get_act();
  lv_indev_enable(indev_touch, true);
}

// Set activating timeout timer
void set_activating_timeout() {
  lv_indev_t *indev_touch = lv_indev_get_act();
  lv_indev_enable(indev_touch, true);
  // Check if screen is loaded by activate sensor method
  if (screen_activating_handled == true) {
    // set request method inflight to activate sensor
    request_method_inflight = CMD_ACTIVATE_SENSOR;
    // create activating timeout timer
    activating_timer = lv_timer_create(activating_timeout_cb, ACTIVATING_TIMEOUT, NULL);
    // pause read timeout timer
    lv_timer_pause(read_timeout_timer);
    // reset and resume activating timer
    lv_timer_reset(activating_timer);
    lv_timer_resume(activating_timer);
  }
}

static void read_timeout_cb(lv_timer_t *timer) {
  lv_timer_pause(timer);
  request_method_inflight = CMD_NONE;
  //  LV_LOG_USER("MY_EVENT_REQUEST_TIMEOUT");
  jsonrpc_event_cb((lv_event_code_t)MY_EVENT_REQUEST_TIMEOUT);
  // LV_LOG_USER("jsonrpc dbg MY_EVENT_REQUEST_TIMEOUT");

  // enable touch
  lv_indev_t *indev_touch = lv_indev_get_act();
  lv_indev_enable(indev_touch, true);
}

static void keep_alive_timeout_cb(lv_timer_t *timer) {
  lv_timer_pause(timer);
  sys_state_t *ss = sys_state_get();
  ss->syserr = SYSERR_SYSTEM_DISCONNECTED;

  // get firmware variant
  NKD_VARIANT firmware_variant = get_firmware_variant();

  // If variant is read only display
  if (firmware_variant == NKD_READ_ONLY) {
    lv_obj_t *read_only_scr = screen_read_only_get();
    lv_screen_load(read_only_scr);
    lv_obj_send_event(read_only_scr, (lv_event_code_t)MY_EVENT_KEEPALIVE_TIMEOUT, NULL);
  } else {
    jsonrpc_event_cb((lv_event_code_t)MY_EVENT_KEEPALIVE_TIMEOUT);
    // LV_LOG_USER("jsonrpc dbg MY_EVENT_KEEPALIVE_TIMEOUT");
  }
// if not simulator or senscap
#if (SIMULATOR == 0) && (SENSCAP_EN == 0)
  // disable nuc watchdog control for alert variant
  if (flg_alert2d_display == false) {
    if (flg_nuc_reset_enable == true) {
      flg_nuc_reset_enable = false; // disable this feature once we issued the reset signal. If user
                                    // wants again they will enable it explicitly by using API
      digitalWrite(NUC_RESET_PIN, HIGH); // NUC RESET
      // LV_LOG_USER("nuc watchdog pin made high");
      // start timer here such that after 3 seconds the respective callback
      // function will make IO46 pin low.
      lv_timer_reset(nuc_rstpin_low_timer);
      lv_timer_resume(nuc_rstpin_low_timer);
    }
  }
#endif

  // reset keep alive timer
  lv_timer_reset(keep_alive_timer);
  lv_timer_resume(keep_alive_timer);
}

#if (SIMULATOR == 0) && (SENSCAP_EN == 0)
static void nuc_rstpin_low_cb(lv_timer_t *timer) {
  // disable nuc watchdog control for alert variant
  if (flg_alert2d_display == false) {
    lv_timer_pause(nuc_rstpin_low_timer);
    digitalWrite(NUC_RESET_PIN, LOW); // NUC RESET
    // LV_LOG_USER("nuc watchdog pin made low");
  }
}
#endif

/* This callback function triggers when the response from NUC is not received for more than
 * READ_TIMEOUT time.
 */
void room_resp_timeout_cb(lv_timer_t *timer) {
  // room response not came within timeout, so ignore the response that comes after timeout
  room_resp_timeout = true;
  lv_timer_pause(timer); // pause the timer

  // enable touch
  lv_indev_t *indev_touch = lv_indev_get_act();
  lv_indev_enable(indev_touch, true);

  // close the spinner screen, if it is visible
  close_spinner_screen();
  close_one_btn_msg_box();
  close_two_btn_msg_box();
  request_method_inflight = CMD_NONE;

  lv_obj_t *scr = lv_scr_act();
  if (flag_lang_selection && scr == screen_settings_audio_get()) {
    handle_language_change_timeout();
  } else {
    flag_lang_selection = false;
    toast_show(scr, no_response_str); // show no response toast message
  }
}

void jsonrpc_init() {
  sys_state_get(); // initalize

  request_state_inflight = sys_state_create();

  read_timeout_timer = lv_timer_create(read_timeout_cb, READ_TIMEOUT, NULL);
  lv_timer_pause(read_timeout_timer);

  keep_alive_timer = lv_timer_create(keep_alive_timeout_cb, KEEP_ALIVE_TIMEOUT, NULL);

  room_resp_timeout_timer = lv_timer_create(room_resp_timeout_cb, READ_TIMEOUT, NULL);
  lv_timer_pause(room_resp_timeout_timer); // pause the timer, so timer callback won't trigger

  assign_room_timeout_timer = lv_timer_create(room_resp_timeout_cb, ASSIGN_ROOM_TIMEOUT, NULL);
  lv_timer_pause(assign_room_timeout_timer); // pause the timer, so timer callback won't trigger

#if (SIMULATOR == 0) && (SENSCAP_EN == 0)
  // disable nuc watchdog control for alert variant
  if (flg_alert2d_display == false) {
    // Timer to make the nuc reset pin (IO46) low after making it high and keeping
    // it high for 3 seconds
    nuc_rstpin_low_timer = lv_timer_create(nuc_rstpin_low_cb, 3000, NULL);
    lv_timer_pause(nuc_rstpin_low_timer);
  }
#endif

  MY_EVENT_SET_STATE_OK = lv_event_register_id();
  MY_EVENT_SET_STATE_FAIL = lv_event_register_id();
  MY_EVENT_SET_DISP = lv_event_register_id();
  MY_EVENT_REQUEST_TIMEOUT = lv_event_register_id();
  MY_EVENT_KEEPALIVE_TIMEOUT = lv_event_register_id();
  MY_EVENT_IDLE_TIMEOUT = lv_event_register_id();
  MY_EVENT_SENSOR_TOGGLE = lv_event_register_id();
}

/*********************************************************************************/
void resp_session_data_clear(resp_session_data_t *resp_sData) {
  // clear data in the arrays
  if (resp_session_data != NULL) {
    memset(resp_sData->response_method, '\0', sizeof(resp_sData->response_method));
    memset(resp_sData->sensor_id, '\0', sizeof(resp_sData->sensor_id));
    memset(resp_sData->assigned_room, '\0', sizeof(resp_sData->assigned_room));
    resp_sData->resp_id = 0; // reset the array index to zero
  }
}

void resp_method_clear(resp_session_data_t *resp_sData) {
  // clear data in the array
  if (resp_session_data != NULL) {
    memset(resp_sData->response_method, '\0', sizeof(resp_sData->response_method));
  }
}

static void resp_session_data_init() {
  if (resp_session_data == NULL) {
    resp_session_data = (resp_session_data_t *)malloc(sizeof(resp_session_data_t));
    resp_session_data_clear(resp_session_data);
  }
}

resp_session_data_t *resp_session_data_get() {
  if (resp_session_data == NULL) {
    resp_session_data_init();
  }
  return resp_session_data;
}
