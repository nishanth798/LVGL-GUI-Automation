#include "json_response.h"

// maximum size of jsonrpc2 request, in bytes.
#define REQUEST_SIZE_MAX 1000
#define RESPONSE_SIZE_MAX 500

// maximum size of json document for deserializing request
// todo: make an equivalent for response size?
static const size_t jsondoc_size = 1.5 * REQUEST_SIZE_MAX;

static const char *Jsonrpc_error_to_string(JsonRpcError error) {
  switch (error) {
  case JSONRPC_ERROR_NONE:
    return "JSONRPC_ERROR_NONE";
  case PARSE_ERROR:
    return "PARSE_ERROR";
  case INVALID_REQUEST:
    return "INVALID_REQUEST";
  case METHOD_NOT_FOUND:
    return "METHOD_NOT_FOUND";
  case INVALID_PARAMS:
    return "INVALID_PARAMS";
  case INTERNAL_ERROR:
    return "INTERNAL_ERROR";
  case MULTIPLE_PARAMS_SET:
    return "MULTIPLE_PARAMS_SET";
  case SCREEN_LOCKED_BY_USER:
    return "SCREEN_LOCKED_BY_USER";
  case INVALID_RESPONSE:
    return "INVALID_RESPONSE";
  case VARIANT_UNSET:
    return "VARIANT_UNSET";
  default:
    return "UNKNOWN_ERROR";
  }
}

// todo: mention in documentation that id does not strictly adhere to spec and
// is restricted to uint32_t.
void respond(const uint32_t *id_ptr, JsonRpcError code, const char *data) {
  // compose json doc

  JsonDocument doc;
  doc["jsonrpc"] = "2.0";

  if (code) {
    JsonObject error = doc["error"].to<JsonObject>();
    error["code"] = code;
    error["message"] = Jsonrpc_error_to_string(code);
    if (strcmp(data, "") != 0) {
      error["data"] = data;
    }
  } else {
    doc["result"] = nullptr;
  }
  if (id_ptr == NULL) {
    doc["id"] = nullptr;
  } else {
    doc["id"] = *id_ptr;
  }

  // serialize and send
  char response_buffer[RESPONSE_SIZE_MAX];
  serializeJson(doc, response_buffer);
  myprint(response_buffer);
}

void serialize_jsonrpc_response(uint32_t id, JsonObject result) {
  // compose json doc
  JsonDocument doc;
  doc["jsonrpc"] = "2.0";
  doc["result"] = result;
  if (id) {
    doc["id"] = id;
  }

  // serialize and send
  char resp_buf[jsondoc_size];
  serializeJson(doc, resp_buf);
  myprint(resp_buf);
}