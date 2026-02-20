#ifndef _SYS_STATE_H
#define _SYS_STATE_H

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
  uint16_t dec_val; // decimal equivalent of 16-bit hex value for availability
  bool bed_alt;
  bool fall_alt;
  bool rep_alt;
  bool sch_mon;
  bool bed_pos;
  bool bed_wid;
  bool occ_size;
} fts_avail_t;

typedef struct {
  uint16_t dec_val; // decimal equivalent of 16-bit hex value for state
  bool bed_alt;
  bool fall_alt;
  bool rep_alt;
} fts_state_t;

// sys_state_t to hold the system state, received from NUC
typedef struct {
  SCREEN screen;
  char mon_start[5]; // "2130"
  char mon_end[5];   // "0700"
  BMS bms;
  uint8_t bed_wid; // in inch
  BED_POS bed_pos; // 1 - 3
  OCC_SIZE occ_size;
  AUDIO audio;
  VOLUME vol;
  char lang[21]; // e.g. "English", "Spanish"
  MONITOR_MODE mode;
  uint16_t pause_tmr; // in seconds
  SYSERR syserr;
  CALIBRATION cal;
  char syserr_title[TITLE_TEXT_LEN_MAX]; // max 50 characters
  ALERT alert;
  char alert_title[TITLE_TEXT_LEN_MAX];
  char room_number[11];
  ABC abc;
  fts_avail_t fts_avail;
  fts_state_t fts_state;
  uint16_t pr_inj_tmr; // pressure injury timer
} sys_state_t;

sys_state_t *sys_state_create();
void sys_state_clear(sys_state_t *state);

// readonly global system state, to be read by screens
sys_state_t *sys_state_get();
// sys_state_t *sys_state_get_write();

// set individual state variables. Returns false if input validation fails, else
// returns true.

bool sys_state_validate(const char *key, JsonVariant val, int screen_val);
bool sys_state_set(const char *key, JsonVariant val, int screen_val);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif // _SYS_STATE_H
