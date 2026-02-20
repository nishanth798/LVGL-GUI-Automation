#ifndef _JSONRPC2_H
#define _JSONRPC2_H

#ifdef SIMULATOR
#include <ArduinoJson.h>
#else
#include <ArduinoJson.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "api.h"
#include "ui.h"

typedef enum {
  JSONRPC_ERROR_NONE = 0,

  PARSE_ERROR = -32700,
  INVALID_REQUEST = -32600,
  METHOD_NOT_FOUND = -32601,
  INVALID_PARAMS = -32602,
  INTERNAL_ERROR = -32603,
  MULTIPLE_PARAMS_SET = -32000,
  SCREEN_LOCKED_BY_USER = -32001,
  INVALID_RESPONSE = -32002,
  UNKNOWN_ERROR = -32003,
  VARIANT_UNSET = -32004,

  TOUCH_FIRST_FAIL = -32301,    // Touch failure on first attempt
  TOUCH_REINIT_FAIL = -32302,   // Touch failed even after reinitialization
  TOUCH_REINIT_SUCCESS = 32303, // Touch successfully reinitialized
  // TOUCH_SECOND_FAIL = -32304,  //Deprecated  //Runtime fail message

} JsonRpcError;

#if !SENSCAP_EN
#define SERIAL_PORT Serial
#else
#define SERIAL_PORT Serial0
#endif

// extern char serDataBuf[200]; //1000
// extern bool flgNewData;
#ifndef SIMULATOR
extern xSemaphoreHandle serDataMutex;
extern QueueHandle_t xQueue;
#endif
extern bool flg_disp_sleep;

#define NUC_RESET_PIN 46 // NUC reset pin, GPIO 46

extern const char *no_response_str;
extern const char *invalid_response_str;

extern uint32_t MY_EVENT_REQUEST_TIMEOUT, MY_EVENT_KEEPALIVE_TIMEOUT, MY_EVENT_IDLE_TIMEOUT;
// triggered after the respective method handlers return a success.
extern uint32_t MY_EVENT_SET_STATE_OK, MY_EVENT_SET_STATE_FAIL, MY_EVENT_SET_DISP;

extern uint32_t MY_EVENT_SENSOR_TOGGLE;

typedef void (*jsonrpc_event_cb_t)(lv_event_code_t code);

void process_request(int timeoutMillis);
void serializeJsonRpcRequest(int id, CMD method, JsonObject params);

void jsonrpc_init();
// void jsonrpc_add_event_cb(jsonrpc_event_cb_t cb);

// os related function
int read_input_line(char *buffer, int bufferSize, int timeoutMillis);
void myprint(const char *str);

lv_obj_t *screen_get(SCREEN screen);
bool is_landing_screen(lv_obj_t *obj);

// room assign, response, check view, get language, sys info related
void handle_jsonrpc_result(JsonDocument json_in);
void display_image_room_view_check(JsonDocument doc);
void display_image_room_view_check_error();

// call this function inorder to trigger assign_room timeout or room_response timoeout
extern lv_timer_t *assign_room_timeout_timer; // timer to check assign room response timeout
void room_resp_timeout_cb(lv_timer_t *timer);

// extern NKD_VARIANT firmware_variant;

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
