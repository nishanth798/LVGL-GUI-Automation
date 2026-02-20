#ifndef _JSON_HANDLES_H
#define _JSON_HANDLES_H

#ifdef SIMULATOR
#include <ArduinoJson.h>
#else
#include <ArduinoJson.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "api.h"
#include "jsonrpc2.h"
#include "sys_state.h"

// returns firmware variant
NKD_VARIANT get_firmware_variant();
// handles set variant method
// returns JSONRPC_ERROR_NONE if variant is set successfully
// returns INVALID_PARAMS if variant is not 1/2/3
JsonRpcError handle_set_variant_method(JsonObject parameter, uint32_t id);

void handle_get_device_info_method(uint32_t id);

void handle_get_device_status_method(uint32_t id);

JsonRpcError handle_set_toast_method(const JsonObject params, char *data, size_t datasize);

JsonRpcError handle_set_disp_method(const JsonObject params, char *data, size_t datasize);

JsonRpcError handle_set_state_method(const JsonObject params, char *data, size_t datasize);

bool check_response(const JsonObject params, sys_state_t *req_state_inflight,
                    CMD req_method_inflight);

void handle_sys_info_method(JsonDocument doc);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*_JSON_HANDLES_H*/