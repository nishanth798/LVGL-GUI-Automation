#include "sys_state.h"
#include "api.h"
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// global system state
// read by all screens, written by jsonrpc2 set_state method from host.
static sys_state_t *sys_state;

void fts_avail_clear(fts_avail_t *fa) {
  fa->dec_val = 0;
  fa->bed_alt = false;
  fa->fall_alt = false;
  fa->rep_alt = false;
  fa->sch_mon = false;
  fa->bed_pos = false;
  fa->bed_wid = false;
  fa->occ_size = false;
  return;
}

void fts_state_clear(fts_state_t *fs) {
  fs->dec_val = 0;
  fs->bed_alt = false;
  fs->fall_alt = false;
  fs->rep_alt = false;
  return;
}

void sys_state_clear(sys_state_t *state) {
  state->screen = SCREEN_UNSET;
  state->bed_wid = 0;
  state->bed_pos = BED_POS_UNSET;
  state->occ_size = OCC_SIZE_UNSET;
  state->bms = BMS_UNSET;
  state->audio = AUDIO_UNSET;
  state->vol = VOL_UNSET;
  // state->lang = LANG_UNSET;
  state->mode = MODE_UNSET;
  state->pause_tmr = 0;
  state->syserr = SYSERR_NONE;
  state->alert = ALERT_NONE;
  state->cal = CAL_OFF;
  state->abc = ABC_UNSET;

  memset(state->lang, '\0', sizeof(state->lang));

  memset(state->mon_start, '\0', sizeof(state->mon_start));
  memset(state->mon_end, '\0', sizeof(state->mon_end));

  memset(state->syserr_title, '\0', sizeof(state->syserr_title));
  memset(state->alert_title, '\0', sizeof(state->alert_title));
  memset(state->room_number, '\0', sizeof(state->room_number));

  fts_avail_clear(&state->fts_avail);
  fts_state_clear(&state->fts_state);
  state->pr_inj_tmr = 0;
}

sys_state_t *sys_state_create() {
  sys_state_t *state = (sys_state_t *)malloc(sizeof(sys_state_t));
  sys_state_clear(state);
  return state;
}

static void sys_state_init() {
  if (sys_state == NULL) {
    sys_state = sys_state_create();
  }
}

sys_state_t *sys_state_get() {
  if (sys_state == NULL) {
    sys_state_init();
  }
  return sys_state;
}

static bool validate_screen(SCREEN val) {
  return (val == SCREEN_HOME_INACTIVE || val == SCREEN_HOME_ACTIVE || val == SCREEN_DEACTIVATING ||
          val == SCREEN_VARIANT_NOTSET || val == SCREEN_ACTIVATING);
}

static bool set_screen(SCREEN val) {
  if (validate_screen(val)) {
    sys_state->screen = SCREEN(val);
    return true;
  }
  return false;
}

static bool validate_bms(BMS val) { return (val >= BMS_LOW && val <= BMS_ULTRA_HIGH); }

static bool set_bms(BMS val) {
  if (validate_bms(val)) {
    sys_state->bms = BMS(val);
    return true;
  }
  return false;
}

static bool validate_audio(AUDIO val) { return (val >= AUDIO_OFF && val <= AUDIO_ON); }

static bool set_audio(AUDIO val) {
  if (validate_audio(val)) {
    sys_state->audio = AUDIO(val);
    return true;
  }
  return false;
}

static bool validate_volume(VOLUME val) { return (val >= VOL_LOW && val <= VOL_HIGH); }

static bool set_volume(VOLUME val) {
  if (validate_volume(val)) {
    sys_state->vol = VOLUME(val);
    return true;
  }
  return false;
}

// Validate that lang contains only alphabetic characters and is not empty
static bool validate_lang(const char *val, uint8_t size) {
  // Check if the lang string is not NULL
  if (val != NULL) {
    // Check if the lang string is not empty
    if (strcmp(val, "") != 0) {
      // Check if the lang string contains only alphabetic characters
      for(int i = 0; i < strlen(val); i++) {
        if(!isalpha(val[i]) && val[i] != ' ') {
          return false;
        }
      }
      return (strlen(val) < size);
    } else {
      return false;
    }
  }
  return false;
}

static bool set_lang(const char *val, uint8_t size) {
  if (validate_lang(val, size)) {
    strncpy(sys_state->lang, val, size - 1);
    sys_state->lang[size - 1] = '\0';
    return true;
  }
  return false;
}

static bool validate_mode(MONITOR_MODE val) {
  return (val >= MODE_BED && val <= MODE_SCHEDULED_MON);
}

static bool set_mode(MONITOR_MODE val) {
  if (validate_mode(val)) {
    sys_state->mode = MONITOR_MODE(val);
    return true;
  }
  return false;
}

static bool validate_pause_timer(uint16_t val) { return true; }

static bool set_pause_timer(uint16_t val) {
  sys_state->pause_tmr = val;
  return true;
}

static bool validate_syserr(SYSERR val) {
  switch (val) {
  case SYSERR_NONE:
  case SYSERR_INCORRECT_MODE:
  case SYSERR_SUNLIGHT:
  case SYSERR_OBSTRUCTED:
  case SYSERR_NOT_MONITORING:
  case SYSERR_SYSTEM_DISCONNECTED:
  case SYSERR_UNASSIGNED:
    return true;
  default:
    return false;
  }
}

static bool set_syserr(SYSERR val) {
  if (validate_syserr(val)) {
    sys_state->syserr = SYSERR(val);
    return true;
  }
  return false;
}

static bool validate_title(const char *val, size_t size) {
  if (val != NULL) {
    return (strlen(val) <= size);
  } else {
    return false;
  }
}

static bool set_syserr_title(const char *val, size_t size) {
  if (validate_title(val, size)) {
    strncpy(sys_state->syserr_title, val, size - 1);
    return true;
  }
  return false;
}

static bool validate_alert(ALERT val) {
  switch (val) {
  case ALERT_NONE:
  case ALERT_BED_EXIT:
    return true;
  case ALERT_CHAIR_EXIT:
    return true;
  case ALERT_FALL_DETECTED:
    return true;
  case ALERT_REPOSITION:
    return true;
  default:
    return false;
  }
}

static bool set_alert(ALERT val) {
  if (validate_alert(val)) {
    sys_state->alert = ALERT(val);
    return true;
  }
  return false;
}

static bool set_alert_title(const char *val, size_t size) {
  if (validate_title(val, size)) {
    strncpy(sys_state->alert_title, val, size - 1);
    return true;
  }
  return false;
}

static bool validate_cal(CALIBRATION val) { return (val >= CAL_OFF && val <= CAL_CHAIR); }

static bool set_cal(CALIBRATION val) {
  if (validate_cal(val)) {
    sys_state->cal = CALIBRATION(val);
    return true;
  }
  return false;
}

static bool validate_room_number(const char *val, uint8_t size) {
  if (val != NULL) {
    return (strlen(val) < (size));
  } else {
    return false;
  }
}

static bool set_room_number(const char *val, size_t size) {
  if (validate_room_number(val, size)) {
    strncpy(sys_state->room_number, val, size - 1);
    return true;
  }
  return false;
}

static bool validate_abc(ABC val) { return (val >= ABC_OFF && val <= ABC_ON); }

static bool set_abc(ABC val) {
  if (validate_abc(val)) {
    sys_state->abc = ABC(val);
    return true;
  }
  return false;
}

// Validate monitoring time in "HHMM" 24-hour format
static bool validate_mon_time(const char *val, size_t size) {
  // If string is not empty, then it is scheduled monitoring
  if (strcmp(val, "") != 0) {
    if (strlen(val) != size - 1) {
      return false;
    }
    // Check if all characters are digits
    for (size_t i = 0; i < strlen(val); i++) {
      if (!isdigit(val[i])) {
        return false;
      }
    }
    // Convert to hours and minutes
    int hour, minutes;
    char hour_str[3] = {val[0], val[1], '\0'};
    char min_str[3] = {val[2], val[3], '\0'};
    hour = atoi(hour_str);
    minutes = atoi(min_str);
    if (hour < 0 || hour > 23 || minutes < 0 || minutes > 59) {
      return false;
    }
    return true;
  }
  // If string is empty, then it is continuous monitoring, return true
  return true;
}

static bool set_mon_start(const char *val) {
  // get the array size of mon_start
  uint8_t start_mon_str_len = sizeof(sys_state->mon_start);
  if (validate_mon_time(val, start_mon_str_len)) {
    strncpy(sys_state->mon_start, val, start_mon_str_len - 1);
    return true;
  }
  return false;
}

static bool set_mon_end(const char *val) {
  // get the array size of mon_end
  uint8_t start_mon_str_len = sizeof(sys_state->mon_end);
  if (validate_mon_time(val, start_mon_str_len)) {
    strncpy(sys_state->mon_end, val, start_mon_str_len - 1);
    return true;
  }
  return false;
}

static bool validate_bed_wid(uint16_t val) {
  if(val == 38 || val == 42) {
    return true;
  }
  if (val >= 44 && val <= 80) {
    return true;
  } else {
    return false;
  }
}

static bool set_bed_wid(uint16_t val) {
  if (validate_bed_wid(val) == true) {
    sys_state->bed_wid = val;
    return true;
  }
  return false;
}

static bool validate_bed_pos(BED_POS val) { return (val >= BED_POS_LEFT && val <= BED_POS_RIGHT); }

static bool set_bed_pos(BED_POS val) {
  if (validate_bed_pos(val)) {
    sys_state->bed_pos = BED_POS(val);
    return true;
  }
  return false;
}

static bool validate_occ_size(OCC_SIZE val) {
  return (val >= OCC_SIZE_SMALL && val <= OCC_SIZE_LARGE);
}

static bool set_occ_size(OCC_SIZE val) {
  if (validate_occ_size(val)) {
    sys_state->occ_size = OCC_SIZE(val);
    return true;
  }
  return false;
}

static bool validate_fts_avail(uint16_t val) {
  // value should not be greater than 127 which is equivalent to when all the features are available
  // check if the 0th bit is always set to 1 or not. By default Bed/chair alert will be always
  // enabled in cloud.
  if ((val & 0x0001) == 0) {
    return false;
  }
  // value should not be less than 1 (bed/chair alert enabled) and greater than 127 which is
  // equivalent to when all the features are available
  return (val >= 1 && val <= 127);
}

// here input argument hex_val is decimal equivalent of the hex value
static bool set_fts_avail(uint16_t dec_val) {
  if (validate_fts_avail(dec_val) == true) {
    sys_state->fts_avail.dec_val = dec_val;
    sys_state->fts_avail.bed_alt = (dec_val & 0x0001) != 0;
    sys_state->fts_avail.fall_alt = (dec_val & 0x0002) != 0;
    sys_state->fts_avail.rep_alt = (dec_val & 0x0004) != 0;
    sys_state->fts_avail.sch_mon = (dec_val & 0x0008) != 0;
    sys_state->fts_avail.bed_pos = (dec_val & 0x0010) != 0;
    sys_state->fts_avail.bed_wid = (dec_val & 0x0020) != 0;
    sys_state->fts_avail.occ_size = (dec_val & 0x0040) != 0;
    return true;
  } else {
    return false;
  }
}

static bool validate_fts_state(int16_t val, int screen_val) {

  // if screen is not home_active then accept fts_state as zero as well
  if(screen_val != SCREEN_HOME_ACTIVE) {
    if(val == 0) {
      return true;
    }
  }

  // value should not be less thn 1 which means atleast one alert should be enabled and also should
  // not be greater than 7 which is equivalent to when all the alert features are enabled by the
  // user
  return (val >= 1 && val <= 7);
}

// here input argument hex_val is decimal equivalent of the hex value
static bool set_fts_state(uint16_t dec_val, int screen_val) {
  if (validate_fts_state(dec_val, screen_val) == true) {
    sys_state->fts_state.dec_val = dec_val;
    sys_state->fts_state.bed_alt = (dec_val & 0x0001) != 0;
    sys_state->fts_state.fall_alt = (dec_val & 0x0002) != 0;
    sys_state->fts_state.rep_alt = (dec_val & 0x0004) != 0;
    return true;
  } else {
    return false;
  }
}

static bool validate_pressure_injury_timer(int val) {
  if(val>=0 && val <= 5999) { // max 99 hours and 59 minutes
    return true;
  }
  return false;

}

static bool set_pressure_injury_timer(uint16_t val) {
  if (validate_pressure_injury_timer(val)) {
    sys_state->pr_inj_tmr = val;
    return true;
  }
  return false;
}

bool sys_state_validate(const char *key, JsonVariant val, int screen_value) {
  if (strcmp(key, "screen") == 0) {
    return validate_screen(val);
  } else if (strcmp(key, "bms") == 0) {
    return validate_bms(val);
  } else if (strcmp(key, "audio") == 0) {
    return validate_audio(val);
  } else if (strcmp(key, "vol") == 0) {
    return validate_volume(val);
  } else if (strcmp(key, "lang") == 0) {
    return validate_lang(val, sizeof(sys_state->lang));
  } else if (strcmp(key, "mode") == 0) {
    return validate_mode(val);
  } else if (strcmp(key, "pause_tmr") == 0) {
    return validate_pause_timer(val);
  } else if (strcmp(key, "syserr") == 0) {
    return validate_syserr(val);
  } else if (strcmp(key, "alert") == 0) {
    return validate_alert(val);
  } else if (strcmp(key, "syserr_title") == 0 || strcmp(key, "alert_title") == 0) {
    return validate_title(val, TITLE_TEXT_LEN_MAX);
  } else if (strcmp(key, "cal") == 0) {
    return validate_cal(val);
  } else if (strcmp(key, "room_number") == 0) {
    return validate_room_number(val, sizeof(sys_state->room_number));
  } else if (strcmp(key, "abc") == 0) {
    return validate_abc(val);
  } else if (strcmp(key, "mon_start") == 0) {
    return validate_mon_time(val, sizeof(sys_state->mon_start));
  } else if (strcmp(key, "mon_end") == 0) {
    return validate_mon_time(val, sizeof(sys_state->mon_end));
  } else if (strcmp(key, "bed_wid") == 0) {
    return validate_bed_wid(val);
  } else if (strcmp(key, "bed_pos") == 0) {
    return validate_bed_pos(val);
  } else if (strcmp(key, "occ_size") == 0) {
    return validate_occ_size(val);
  } else if (strcmp(key, "fts_avail") == 0) {
    return validate_fts_avail(val);
  } else if (strcmp(key, "fts_state") == 0) {
    return validate_fts_state(val, screen_value);
  } else if(strcmp(key, "pr_inj_tmr") == 0) {
    return validate_pressure_injury_timer(val);
  } else {
    // printf("Unknown key: %s\n", key);
    return 0;
  }
}

bool sys_state_set(const char *key, JsonVariant val, int screen_value) {
  if (strcmp(key, "screen") == 0) {
    return set_screen(val);
  } else if (strcmp(key, "bms") == 0) {
    return set_bms(val);
  } else if (strcmp(key, "audio") == 0) {
    return set_audio(val);
  } else if (strcmp(key, "vol") == 0) {
    return set_volume(val);
  } else if (strcmp(key, "lang") == 0) {
    return set_lang(val, sizeof(sys_state->lang));
  } else if (strcmp(key, "mode") == 0) {
    return set_mode(val);
  } else if (strcmp(key, "pause_tmr") == 0) {
    return set_pause_timer(val);
  } else if (strcmp(key, "syserr") == 0) {
    return set_syserr(val);
  } else if (strcmp(key, "alert") == 0) {
    return set_alert(val);
  } else if (strcmp(key, "syserr_title") == 0) {
    return set_syserr_title(val, TITLE_TEXT_LEN_MAX);
  } else if (strcmp(key, "alert_title") == 0) {
    return set_alert_title(val, TITLE_TEXT_LEN_MAX);
  } else if (strcmp(key, "cal") == 0) {
    return set_cal(val);
  } else if (strcmp(key, "room_number") == 0) {
    return set_room_number(val, sizeof(sys_state->room_number));
  } else if (strcmp(key, "abc") == 0) {
    return set_abc(val);
  } else if (strcmp(key, "mon_start") == 0) {
    return set_mon_start(val);
  } else if (strcmp(key, "mon_end") == 0) {
    return set_mon_end(val);
  } else if (strcmp(key, "bed_wid") == 0) {
    return set_bed_wid(val);
  } else if (strcmp(key, "bed_pos") == 0) {
    return set_bed_pos(val);
  } else if (strcmp(key, "occ_size") == 0) {
    return set_occ_size(val);
  } else if (strcmp(key, "fts_avail") == 0) {
    return set_fts_avail(val);
  } else if (strcmp(key, "fts_state") == 0) {
    return set_fts_state(val, screen_value);
  } else if(strcmp(key, "pr_inj_tmr") == 0) {
    return set_pressure_injury_timer(val);
  } else {
    // printf("Unknown key: %s\n", key);
    return 0;
  }
}
