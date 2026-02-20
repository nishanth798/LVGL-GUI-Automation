#ifndef _DISP_STATE_H
#define _DISP_STATE_H

#ifdef SIMULATOR
#include <ArduinoJson.h>
#else
#include <ArduinoJson.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "api.h"
// #include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  uint8_t brightness;
  ABC abc;
} disp_state_t;

// readonly global system state, to be read by screens
const disp_state_t *disp_state_get();

bool disp_state_set_brightness(uint8_t val);
bool disp_state_set_abc(ABC val);

// set individual state variables. Returns false if input validation fails, else
// returns true.

bool disp_state_validate(const char *key, JsonVariant val);
bool disp_state_set(const char *key, JsonVariant val);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif // _SYS_STATE_H
