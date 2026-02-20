#ifndef _API_H
#define _API_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include <stdbool.h>

#define READ_TIMEOUT 3000         // in milliseconds
#define TITLE_TEXT_LEN_MAX 50     // characters
#define KEEP_ALIVE_TIMEOUT 300000 // in milliseconds (5 minutes)
// 2 min of inactivity (1000(1sec) * 60(1min) * 2(minutes) = 120000
#define INACTIVE_TIMEOUT 120000   // in milliseconds (2 minutes)
#define ASSIGN_ROOM_TIMEOUT 45000 // in millis (45 seconds)
#define ACTIVATING_TIMEOUT 60000  // in millis (60 seconds)

// Default states
#define SCREEN_DEFAULT SCREEN_HOME_INACTIVE
#define BMS_DEFAULT BMS_HIGH
#define AUDIO_DEFAULT AUDIO_ON
#define LANG_DEFAULT LANG_ENGLISH
#define VOL_DEFAULT VOL_MED
#define ABC_DEFAULT ABC_OFF
#define BRIGHTNESS_DEFAULT 80
#if !SENSCAP_EN
#define BRIGHTNESS_MIN 30
#else
#define BRIGHTNESS_MIN 1
#endif

// TODO: request to simplify command set to , get_state, set_state.
// repalce CMD with method
typedef enum {
  CMD_NONE, // comm check
  CMD_PAUSE_SENSOR,
  CMD_DEACTIVATE_SENSOR,
  CMD_SHUTDOWN,
  CMD_VIRC,
  CMD_SYSTEM_EVENT,
  CMD_CLEAR_ALERT,
  CMD_RESUME_FALL_MONITORING,

  CMD_SAVE_BMS,
  CMD_SAVE_AUDIO,
  CMD_SAVE_DISPLAY,
  CMD_SAVE_MON_SCH,
  CMD_SAVE_OCCUPANT_SIZE,
  CMD_SAVE_BED_POS,
  CMD_SAVE_BED_WIDTH,
  CMD_SAVE_ALERTS,

  CMD_GET_ROOM,
  CMD_GET_UNITS,
  CMD_GET_ROOMS,
  CMD_GET_ROOM_VIEW,
  CMD_GET_SYS_INFO,
  CMD_GET_LANGUAGES,

  CMD_SET_ROOM,
  CMD_SET_ROOM_VIEW,

  CMD_ACTIVATE_SENSOR, // custom name for multiple state change
  CMD_ACTIVATE_BED_MODE,
  CMD_ACTIVATE_CHAIR_MODE,
} CMD;

extern const char *cmd_str[];

typedef enum {
  BMS_LOW = 1,
  BMS_HIGH = 2,
  BMS_ULTRA_HIGH = 3,
  BMS_UNSET = -1,
} BMS;

typedef enum {
  SYSERR_NONE = 0,

  // monitoring errors
  SYSERR_INCORRECT_MODE = 11,
  SYSERR_SUNLIGHT = 12,
  SYSERR_OBSTRUCTED = 13,

  // connectivity errors
  SYSERR_NOT_MONITORING = 41,
  SYSERR_SYSTEM_DISCONNECTED = 42,

  // other errors
  SYSERR_UNASSIGNED = 61,

} SYSERR;

typedef enum {
  ALERT_NONE = 0,
  ALERT_BED_EXIT = 1,
  ALERT_CHAIR_EXIT = 2,
  ALERT_FALL_DETECTED = 101,
  ALERT_REPOSITION = 102,
} ALERT;

typedef enum {
  PAUSE_OFF = 0,
  PAUSE_SHORT = 1,
  PAUSE_LONG = 2,
} PAUSE;

typedef enum {
  CAL_OFF = 0,
  CAL_ON = 1,
  CAL_CHAIR = 2,
} CALIBRATION;

typedef enum {
  LANG_ENGLISH = 1,
  LANG_SPANISH = 2,
  LANG_UNSET = -1,
} LANG;

typedef enum {
  VOL_LOW = 1,
  VOL_MED = 2,
  VOL_HIGH = 3,
  VOL_UNSET = -1,
} VOLUME;

typedef enum {
  ABC_OFF = 0,
  ABC_ON = 1,
  ABC_UNSET = -1,
} ABC;

typedef enum {
  AUDIO_OFF = 0,
  AUDIO_ON = 1,
  AUDIO_UNSET = -1,
} AUDIO;

typedef enum {
  // MODE_OFF = 0,   // sensor off // no longer valid with the introduction of
  // screen variable
  MODE_BED = 1,             // sensor on
  MODE_CHAIR = 2,           // sensor on
  MODE_FALL_MON = 3,        // fall monitoring
  MODE_PRESSURE_INJURY = 4, // pressure injury/reposition alert
  MODE_SCHEDULED_MON = 5,   // scheduled monitoring
  MODE_UNSET = -1,
} MONITOR_MODE;

typedef struct {
  AUDIO audio;
  VOLUME vol;
  LANG lang;
} audio_t;

typedef enum {
  SCREEN_HOME_INACTIVE = 1,
  SCREEN_HOME_ACTIVE = 2,
  // SCREEN_CALIBRATING = 3,
  // SCREEN_CALIBRATING_CHAIR = 4,
  SCREEN_DEACTIVATING = 5,
  SCREEN_VARIANT_NOTSET = 6,
  SCREEN_ACTIVATING = 7,
  SCREEN_UNSET = -1,
} SCREEN;

typedef enum {
  NKD_INTERACTIVE = 1,
  NKD_GO = 2,
  NKD_READ_ONLY = 3,
  NKD_VARIANT_UNSET,
} NKD_VARIANT;

typedef enum {
  BED_POS_LEFT = 1,
  BED_POS_CENTER = 2,
  BED_POS_RIGHT = 3,
  BED_POS_UNSET = -1,
} BED_POS;

typedef enum {
  OCC_SIZE_SMALL = 1,
  OCC_SIZE_STANDARD = 2,
  OCC_SIZE_LARGE = 3,
  OCC_SIZE_UNSET = -1,
} OCC_SIZE;

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif // _API_H
