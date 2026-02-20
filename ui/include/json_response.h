#ifndef _JSON_RESPONSE_H
#define _JSON_RESPONSE_H

#ifdef SIMULATOR
#include <ArduinoJson.h>
#else
#include <ArduinoJson.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "jsonrpc2.h"
/*call serialize_jsonrpc_response to send response from display to NUC
input params: 1)id (json rpc request id)
              2)result (serialized json object which need to be sent out)
return: nothing
*/
void serialize_jsonrpc_response(uint32_t id, JsonObject result);

void respond(const uint32_t *id_ptr, JsonRpcError code, const char *data);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*_JSON_RESPONSE_H*/