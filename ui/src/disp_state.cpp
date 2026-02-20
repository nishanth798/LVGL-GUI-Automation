#include "disp_state.h"
#include "api.h"
#include "backlight.h"
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// global system state
// read by all screens, written by jsonrpc2 set_state method from host.

static disp_state_t disp_state = {
    .brightness = BRIGHTNESS_DEFAULT,
    .abc = ABC_DEFAULT,
};

const disp_state_t *disp_state_get() { return &disp_state; }

static bool validate_brightness(uint8_t val) { return (val <= 100); }

bool disp_state_set_brightness(uint8_t val) {
  if (validate_brightness(val)) {
    disp_state.brightness = val;
#ifndef SIMULATOR
    // LV_LOG_USER("set brightness %d", val);
    set_backlight(val);
#endif
    return true;
  }
  return false;
}

static bool validate_abc(ABC val) { return (val >= ABC_OFF && val <= ABC_ON); }

bool disp_state_set_abc(ABC val) {
  if (validate_abc(val)) {
    disp_state.abc = ABC(val);
// If not simulator and not senscap, then only process ABC(als) setting
#if (SIMULATOR == 0) && (SENSCAP_EN == 0)
    // For Alert display do not process ABC(als) setting
    if (flg_alert2d_display == false) {
      if (disp_state.abc == ABC_ON) {
        if (!flg_als_Enable) {
          //  LV_LOG_USER("ALS ON");
          als_on();
          flg_als_Enable = true;  // turn on ALS
          flg_process_als = true; // process als from ui.ino
        }
      } else if (disp_state.abc == ABC_OFF) {
        //  LV_LOG_USER("ALS OFF");
        als_off();
        flg_als_Enable = false;  // turn on ALS
        flg_process_als = false; // don't process als from ui.ino
      }
    }
#endif
    return true;
  }
  return false;
}

bool disp_state_validate(const char *key, JsonVariant val) {
  if (strcmp(key, "brightness") == 0) {
    return validate_brightness(val);
  } else if (strcmp(key, "abc") == 0) {
    return validate_abc(val);
  } else {
    return false;
  }
}

bool disp_state_set(const char *key, JsonVariant val) {
  if (strcmp(key, "brightness") == 0) {
    return disp_state_set_brightness(val);
  } else if (strcmp(key, "abc") == 0) {
    // return disp_state_set_abc(val);
    return true; // ignore abc setting in set_disp method
  } else {
    return false;
  }
}
