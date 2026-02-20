// clang-format off
#include "ui.h"
#include "jsonrpc2.h"
#include "images.h"
#include "sys_state.h"

// clang-format on
// Carry on formatting

static lv_obj_t *screen_read_only_disp;
static lv_obj_t *cont_read_only_disp;
static lv_obj_t *lbl_heading;
static lv_obj_t *img_icon;
static lv_obj_t *lbl_icon;
static lv_obj_t *pause_timer_panel;
static lv_obj_t *pause_timer;
static lv_obj_t *cont_sched_time = NULL;

// convert 24 hr time string "HHMM" to 12 hr format "H:MM AM/PM"
static int convert_time_to_12hr(const char *in_str, char *out_str, size_t out_size) {
  int hour, min;
  if (sscanf(in_str, "%2d%2d", &hour, &min) != 2) {
    return -1;
  }
  if (hour < 0 || hour > 23 || min < 0 || min > 59) {
    return -1;
  }

  const char *ampm = (hour < 12) ? "AM" : "PM";
  int hour_12 = (hour == 0) ? 12 : (hour <= 12 ? hour : hour - 12);

  if (out_size < 8) {
    return -1; // needs "H:MM AM\0"
  }
  snprintf(out_str, out_size, "%d:%02d %s", hour_12, min, ampm);
  return 0;
}

// update scheduled monitoring time label
static void update_sched_time(lv_obj_t *cont_sched_time) {
  // create label to show scheduled time
  lv_obj_t *lbl_scheduled_time = lv_obj_get_child(cont_sched_time, 0);

  sys_state_t *ss = sys_state_get();
  // set scheduled time text
  char scheduled_time[20], start_time[9], end_time[9];
  // Convert mon_start time to 12-hour format
  convert_time_to_12hr(ss->mon_start, start_time, sizeof(start_time));

  snprintf(scheduled_time, sizeof(start_time), "%s", start_time);
  strcat(scheduled_time, " - ");

  // Convert mon_end time to 12-hour format
  convert_time_to_12hr(ss->mon_end, end_time, sizeof(end_time));

  strcat(scheduled_time, end_time);

  // Set the label text
  lv_label_set_text(lbl_scheduled_time, scheduled_time);
  lv_obj_center(lbl_scheduled_time);
}

// create container for scheduled monitoring time
static lv_obj_t *create_cont_sched_time(lv_obj_t *parent) {
  // create container for scheduled monitoring
  lv_obj_t *cont_sched_time = lv_obj_create(parent);
  lv_obj_set_size(cont_sched_time, 235, 30);
  lv_obj_set_align(cont_sched_time, LV_ALIGN_CENTER);
  lv_obj_set_y(cont_sched_time, 100);
  lv_obj_clear_flag(cont_sched_time, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_border_width(cont_sched_time, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(cont_sched_time, lv_color_hex(VST_COLOR_CERULEAN),
                            LV_PART_MAIN | LV_STATE_DEFAULT);

  // create label to show scheduled time
  lv_obj_t *lbl_scheduled_time = lv_label_create(cont_sched_time);

  lv_obj_set_style_text_color(lbl_scheduled_time, lv_color_white(),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(lbl_scheduled_time, &opensans_bold_24,
                             LV_PART_MAIN | LV_STATE_DEFAULT);
  update_sched_time(cont_sched_time);
  lv_obj_center(lbl_scheduled_time);
  return cont_sched_time;
}

void read_only_disp_set_alert_theme() {
  lv_obj_set_style_bg_color(cont_read_only_disp, lv_color_hex(VST_COLOR_ALERT_RED),
                            LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(cont_read_only_disp, lv_color_hex(VST_COLOR_ALERT_RED),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_flag(pause_timer_panel, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(img_icon, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(lbl_icon, LV_OBJ_FLAG_HIDDEN);
}

void read_only_disp_set_extreme_alert_theme() {
  lv_obj_set_style_bg_color(cont_read_only_disp, lv_color_hex(VST_COLOR_EXTREME_ALERT),
                            LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(cont_read_only_disp, lv_color_hex(VST_COLOR_EXTREME_ALERT),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_flag(pause_timer_panel, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(img_icon, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(lbl_icon, LV_OBJ_FLAG_HIDDEN);
}

void read_only_disp_set_purple_alert_theme() {
  lv_obj_set_style_bg_color(cont_read_only_disp, lv_color_hex(VST_COLOR_ALERT_PURPLE),
                            LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(cont_read_only_disp, lv_color_hex(VST_COLOR_ALERT_PURPLE),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_flag(pause_timer_panel, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(img_icon, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(lbl_icon, LV_OBJ_FLAG_HIDDEN);
}

// if alert_title is non empty, set title to alert_title, else status to
// preconfigured alert
void read_only_screen_set_alert(ALERT alert, const char *alert_title, BMS bms) {
  if (strcmp(alert_title, "") != 0) {
    lv_label_set_text(lbl_heading, alert_title);
    lv_img_set_src(img_icon, &icon_alert_90);
    read_only_disp_set_alert_theme();
    lv_obj_add_flag(lbl_icon, LV_OBJ_FLAG_HIDDEN);
    return;
  }

  switch (alert) {
  case ALERT_BED_EXIT:
    lv_label_set_text(lbl_heading, "BED EXIT");
    lv_img_set_src(img_icon, &icon_bed);
    read_only_disp_set_alert_theme();
    break;
  case ALERT_CHAIR_EXIT:
    lv_label_set_text(lbl_heading, "CHAIR EXIT");
    lv_img_set_src(img_icon, &icon_chair);
    read_only_disp_set_alert_theme();
    lv_obj_add_flag(lbl_icon, LV_OBJ_FLAG_HIDDEN); // hide bms label for chair exit
    break;
  case ALERT_FALL_DETECTED:
    lv_label_set_text(lbl_heading, "FALL DETECTED");
    lv_img_set_src(img_icon, &icon_fall_detected);
    read_only_disp_set_extreme_alert_theme();
    break;
  case ALERT_REPOSITION:
    lv_label_set_text(lbl_heading, "REPOSITION");
    lv_img_set_src(img_icon, &icon_pressure_injury);
    read_only_disp_set_purple_alert_theme();
    break;
  default:
    // LV_LOG_USER("unknown alert: %d", alert);
    break;
  }
  if ((alert == ALERT_BED_EXIT) || (alert == ALERT_CHAIR_EXIT)) {
    if (bms == BMS_LOW) {
      lv_label_set_text(lbl_icon, "Low");
    } else if (bms == BMS_HIGH) {
      lv_label_set_text(lbl_icon, "High");
    } else if (bms == BMS_ULTRA_HIGH) {
      lv_label_set_text(lbl_icon, "Ultra High");
    }
  }
  if (alert == ALERT_BED_EXIT) {
    lv_obj_set_y(img_icon, 17);
  } else {
    lv_obj_set_y(img_icon, 40);
  }
}

void read_only_disp_set_error_theme() {
  lv_obj_set_style_bg_color(cont_read_only_disp, lv_color_hex(VST_COLOR_ERROR_YELLOW),
                            LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(cont_read_only_disp, lv_color_hex(VST_COLOR_ERROR_YELLOW),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_flag(pause_timer_panel, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(img_icon, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(lbl_icon, LV_OBJ_FLAG_HIDDEN);
}

void read_only_screen_set_error(SYSERR err, const char *syserr_title) {
  if (err == SYSERR_SYSTEM_DISCONNECTED) {
    lv_label_set_text(lbl_heading, "SYSTEM DISCONNECTED");
    lv_obj_set_style_text_font(lbl_heading, &opensans_bold_28, LV_PART_MAIN);
    lv_img_set_src(img_icon, &icon_computer_disconnected);
    read_only_disp_set_error_theme();
    return;
  }
  if (strcmp(syserr_title, "") != 0) {
    lv_label_set_text(lbl_heading, syserr_title);
    lv_img_set_src(img_icon, &icon_error_90);
    read_only_disp_set_error_theme();
    return;
  }

  switch (err) {
  case SYSERR_INCORRECT_MODE:
    lv_label_set_text(lbl_heading, "INCORRECT MODE");
    lv_img_set_src(img_icon, &icon_incorrect_mode);
    read_only_disp_set_error_theme();
    break;
  case SYSERR_SUNLIGHT:
    lv_label_set_text(lbl_heading, "SUNLIGHT");
    lv_img_set_src(img_icon, &icon_sunlight);
    read_only_disp_set_error_theme();
    break;
  case SYSERR_OBSTRUCTED:
    lv_label_set_text(lbl_heading, "OBSTRUCTED");
    lv_img_set_src(img_icon, &icon_obstructed);
    read_only_disp_set_error_theme();
    break;
  case SYSERR_NOT_MONITORING:
    lv_label_set_text(lbl_heading, "NOT MONITORING");
    lv_img_set_src(img_icon, &icon_sensor_disconnected);
    read_only_disp_set_error_theme();
    break;
  case SYSERR_SYSTEM_DISCONNECTED:
    lv_label_set_text(lbl_heading, "SYSTEM DISCONNECTED");
    lv_obj_set_style_text_font(lbl_heading, &opensans_bold_28, LV_PART_MAIN);
    lv_img_set_src(img_icon, &icon_computer_disconnected);
    read_only_disp_set_error_theme();
    break;
  case SYSERR_UNASSIGNED:
    lv_label_set_text(lbl_heading, "UNASSIGNED");
    lv_img_set_src(img_icon, &icon_unassigned);
    read_only_disp_set_error_theme();
    break;
  default:
    // LV_LOG_USER("unknown sys error: %d", err);
    break;
  }
  lv_obj_set_y(img_icon, 40);
}

// Convert the seconds to minutes and seconds.
void seconds_to_mm_ss(uint16_t seconds, int *mm, int *ss) {
  *mm = seconds / 60;
  *ss = seconds % 60;
}

void read_only_disp_set_paused_theme() {
  read_only_disp_set_error_theme();
  lv_obj_clear_flag(pause_timer_panel, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(img_icon, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(lbl_icon, LV_OBJ_FLAG_HIDDEN);
}

void read_only_screen_set_paused(uint16_t pause_tmr) {
  if (pause_tmr > 0) {
    lv_label_set_text(lbl_heading, "PAUSED");
    int mm, ss;
    seconds_to_mm_ss(pause_tmr, &mm, &ss);
    char fmt_time[9];
    snprintf(fmt_time, 9, "%02d:%02d", mm, ss);
    lv_label_set_text(pause_timer, fmt_time);
    read_only_disp_set_paused_theme();
    return;
  }
}

void read_only_disp_set_calib_theme() {
  lv_obj_set_style_bg_color(cont_read_only_disp, lv_color_hex(VST_COLOR_ERROR_YELLOW),
                            LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(cont_read_only_disp, lv_color_hex(VST_COLOR_ERROR_YELLOW),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_flag(pause_timer_panel, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(img_icon, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(lbl_icon, LV_OBJ_FLAG_HIDDEN);
}

void read_only_screen_set_calib(CALIBRATION cal) {
  switch (cal) {
  case CAL_ON:
    lv_label_set_text(lbl_heading, "CALIBRATING...");
    lv_img_set_src(img_icon, &icon_calib_3dots);
    read_only_disp_set_calib_theme();
    break;
  case CAL_CHAIR:
    lv_label_set_text(lbl_heading, "CALIBRATING CHAIR...");
    lv_img_set_src(img_icon, &icon_calib_chair_with_dots);
    read_only_disp_set_calib_theme();
  default:
    // LV_LOG_USER("CALIBRATION OFF: %d", cal);
    break;
  }
}

void read_only_disp_set_monitoring_theme() {
  lv_obj_set_style_bg_color(cont_read_only_disp, lv_color_hex(VST_COLOR_MONITOR_GREEN),
                            LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(cont_read_only_disp, lv_color_hex(VST_COLOR_MONITOR_GREEN),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_flag(pause_timer_panel, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(img_icon, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(lbl_icon, LV_OBJ_FLAG_HIDDEN);
}

// set read only display scheduled monitoring theme
void read_only_disp_set_scheduled_monitoring_theme() {
  lv_obj_set_style_bg_color(cont_read_only_disp, lv_color_hex(VST_COLOR_CERULEAN),
                            LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(cont_read_only_disp, lv_color_hex(VST_COLOR_CERULEAN),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_flag(pause_timer_panel, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(img_icon, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(lbl_icon, LV_OBJ_FLAG_HIDDEN);
}

void read_only_screen_set_monitoring(MONITOR_MODE mode, BMS bms) {
  switch (mode) {
  case MODE_BED:
    lv_label_set_text(lbl_heading, "BED MONITORING");
    lv_img_set_src(img_icon, &icon_bed);
    read_only_disp_set_monitoring_theme();
    lv_obj_clear_flag(lbl_icon, LV_OBJ_FLAG_HIDDEN); // hide bms label while monitoring chair
    break;
  case MODE_CHAIR:
    lv_label_set_text(lbl_heading, "CHAIR MONITORING");
    lv_img_set_src(img_icon, &icon_chair);
    read_only_disp_set_monitoring_theme();
    break;
  case MODE_FALL_MON:
    lv_label_set_text(lbl_heading, "FALL MONITORING");
    lv_img_set_src(img_icon, &icon_fall_detected);
    read_only_disp_set_monitoring_theme();
    break;
  case MODE_PRESSURE_INJURY:
    lv_label_set_text(lbl_heading, "PRESSURE INJURY");
    lv_img_set_src(img_icon, &icon_pressure_injury);
    read_only_disp_set_monitoring_theme();
    break;
  case MODE_SCHEDULED_MON:
    lv_label_set_text(lbl_heading, "SCHEDULED");
    lv_img_set_src(img_icon, &icon_scheduled_mon);
    if (cont_sched_time == NULL) {
      cont_sched_time = create_cont_sched_time(cont_read_only_disp);
    }
    lv_obj_clear_flag(cont_sched_time, LV_OBJ_FLAG_HIDDEN);
    update_sched_time(cont_sched_time);

    read_only_disp_set_scheduled_monitoring_theme();
    break;
  default:
    // LV_LOG_USER("unknown mode: %d", mode);
    break;
  }
  if ((mode == MODE_BED) || (mode == MODE_CHAIR)) {
    if (bms == BMS_LOW) {
      lv_label_set_text(lbl_icon, "Low");
    } else if (bms == BMS_HIGH) {
      lv_label_set_text(lbl_icon, "High");
    } else if (bms == BMS_ULTRA_HIGH) {
      lv_label_set_text(lbl_icon, "Ultra High");
    }
  }
  if (mode == MODE_BED || mode == MODE_SCHEDULED_MON) {
    lv_obj_set_y(img_icon, 17);
  } else {
    lv_obj_set_y(img_icon, 40);
  }
}

void read_only_screen_set_deactivating() {
  lv_label_set_text(lbl_heading, "DEACTIVATING...");
  lv_img_set_src(img_icon, &icon_deactivate);

  lv_obj_set_style_bg_color(cont_read_only_disp, lv_color_hex(VST_COLOR_GREY),
                            LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(cont_read_only_disp, lv_color_hex(VST_COLOR_GREY),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_flag(pause_timer_panel, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(img_icon, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(lbl_icon, LV_OBJ_FLAG_HIDDEN);
}

void read_only_screen_set_activating() {
  lv_label_set_text(lbl_heading, "ACTIVATING...");
  lv_img_set_src(img_icon, &icon_activating);

  lv_obj_set_style_bg_color(cont_read_only_disp, lv_color_hex(VST_COLOR_DARKBLUE),
                            LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(cont_read_only_disp, lv_color_hex(VST_COLOR_DARKBLUE),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_flag(pause_timer_panel, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(img_icon, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(lbl_icon, LV_OBJ_FLAG_HIDDEN);
}

void read_only_screen_set_unset() { read_only_screen_set_error(SYSERR_NONE, "NO DATA"); }

static void refresh_read_only_screen() {
  // refresh mode switcher
  const sys_state_t *ss = sys_state_get();
  lv_obj_set_style_text_font(lbl_heading, &opensans_bold_32, LV_PART_MAIN);

  // hide scheduled monitoring time container by default
  if (cont_sched_time != NULL) {
    lv_obj_add_flag(cont_sched_time, LV_OBJ_FLAG_HIDDEN);
  }

  // refresh status panel
  // set status panel
  // base panel: monitoring
  // overlay panels: alert, error, pause
  if (ss->screen == SCREEN_DEACTIVATING) {
    read_only_screen_set_deactivating();
  } else if (ss->screen == SCREEN_VARIANT_NOTSET) {
    lv_obj_t *screen = screen_variant_notset_get();
    lv_scr_load(screen);
  } else if (ss->screen == SCREEN_ACTIVATING) {
    read_only_screen_set_activating();
  } else if (ss->syserr == SYSERR_SYSTEM_DISCONNECTED) {
    read_only_screen_set_error(ss->syserr, ss->syserr_title);
  } else if (ss->alert != ALERT_NONE || strcmp(ss->alert_title, "") != 0) {
    read_only_screen_set_alert(ss->alert, ss->alert_title, ss->bms);
  } else if (ss->syserr != SYSERR_NONE || strcmp(ss->syserr_title, "") != 0) {
    read_only_screen_set_error(ss->syserr, ss->syserr_title);
  } else if (ss->pause_tmr > 0) {
    read_only_screen_set_paused(ss->pause_tmr);
  } else if (ss->cal == CAL_ON || ss->cal == CAL_CHAIR) {
    read_only_screen_set_calib(ss->cal);
  } else if (ss->mode >= MODE_BED || ss->mode <= MODE_PRESSURE_INJURY) {
    read_only_screen_set_monitoring(ss->mode, ss->bms);
  } else {
    read_only_screen_set_unset();
  }
}

void read_only_set_state_ok_cb(lv_event_t *e) {
  // to update the data on the screen
  refresh_read_only_screen();
}

void screen_read_only_disp_load_cb(lv_event_t *e) {
  // to update the data on the screen
  refresh_read_only_screen();
}

void read_only_screen_design() {

  screen_read_only_disp = lv_obj_create(NULL);
  lv_obj_set_size(screen_read_only_disp, lv_pct(100), lv_pct(100));
  lv_obj_set_align(screen_read_only_disp, LV_ALIGN_CENTER);

  lv_obj_add_event_cb(screen_read_only_disp, read_only_set_state_ok_cb,
                      (lv_event_code_t)MY_EVENT_SET_STATE_OK, NULL);
  lv_obj_add_event_cb(screen_read_only_disp, screen_read_only_disp_load_cb,
                      LV_EVENT_SCREEN_LOAD_START, NULL);
  lv_obj_add_event_cb(screen_read_only_disp, read_only_set_state_ok_cb,
                      (lv_event_code_t)MY_EVENT_KEEPALIVE_TIMEOUT, NULL);
  // add content container
  cont_read_only_disp = lv_obj_create(screen_read_only_disp);
  lv_obj_set_size(cont_read_only_disp, 448, 448);
  lv_obj_set_align(cont_read_only_disp, LV_ALIGN_CENTER);
  lv_obj_clear_flag(cont_read_only_disp, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_border_width(cont_read_only_disp, 3, LV_PART_MAIN);
  lv_obj_set_style_border_color(cont_read_only_disp, lv_color_hex(VST_COLOR_MONITOR_GREEN),
                                LV_PART_MAIN);
  lv_obj_set_style_pad_all(cont_read_only_disp, 0, LV_PART_MAIN);
  lv_obj_set_style_bg_color(cont_read_only_disp, lv_color_hex(VST_COLOR_MONITOR_GREEN),
                            LV_PART_MAIN);

  // Image container to hold the image
  lv_obj_t *cont_header = lv_obj_create(cont_read_only_disp);
  lv_obj_set_size(cont_header, 442, 78);
  lv_obj_set_style_border_width(cont_header, 0, LV_PART_MAIN);
  lv_obj_set_align(cont_header, LV_ALIGN_TOP_MID);
  lv_obj_clear_flag(cont_header, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_radius(cont_header, 0, LV_PART_MAIN);

  lbl_heading = lv_label_create(cont_header);
  lv_label_set_text(lbl_heading, "BED MONITORING");
  lv_obj_set_align(lbl_heading, LV_ALIGN_CENTER);
  lv_obj_set_size(lbl_heading, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_style_text_font(lbl_heading, &opensans_bold_32, LV_PART_MAIN);
  lv_obj_set_style_text_color(lbl_heading, lv_color_hex(VST_COLOR_PRIMARY_TEXT), LV_PART_MAIN);

  // Image object
  img_icon = lv_img_create(cont_read_only_disp);
  lv_img_set_src(img_icon, &icon_bed); // Todo: change it later
  lv_obj_set_size(img_icon, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_add_flag(img_icon, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_set_align(img_icon, LV_ALIGN_CENTER);
  // lv_obj_align_to(img_icon, cont_header, LV_ALIGN_OUT_BOTTOM_MID, 0, 120);
  lv_obj_set_y(img_icon, 40);

  // low/high/ulta-high label
  lbl_icon = lv_label_create(cont_read_only_disp);
  lv_obj_set_size(lbl_icon, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_label_set_text(lbl_icon, "Ultra High");
  lv_obj_set_style_text_font(lbl_icon, &opensans_bold_24, LV_PART_MAIN);
  lv_obj_set_style_text_color(lbl_icon, lv_color_hex(0xffffff), LV_PART_MAIN);
  lv_obj_add_flag(lbl_icon, LV_OBJ_FLAG_IGNORE_LAYOUT);
  // lv_obj_set_align(lbl_icon, LV_ALIGN_CENTER);
  lv_obj_set_style_text_align(lbl_icon, LV_TEXT_ALIGN_CENTER, 0);
  // lv_obj_align_to(lbl_icon, img_icon, LV_ALIGN_OUT_BOTTOM_MID, 0, 15);
  // lv_obj_set_y(lbl_icon, 100);
  lv_obj_align(lbl_icon, LV_ALIGN_TOP_MID, 0, 308);

  // add pause timer container panel
  pause_timer_panel = lv_obj_create(cont_read_only_disp);
  lv_obj_set_size(pause_timer_panel, LV_SIZE_CONTENT, LV_SIZE_CONTENT); /// 100
  lv_obj_set_flex_flow(pause_timer_panel, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(pause_timer_panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);
  lv_obj_add_flag(pause_timer_panel, LV_OBJ_FLAG_HIDDEN);       /// Flags
  lv_obj_clear_flag(pause_timer_panel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_bg_color(pause_timer_panel, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(pause_timer_panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(pause_timer_panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_all(pause_timer_panel, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_row(pause_timer_panel, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_column(pause_timer_panel, 26, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_align(pause_timer_panel, LV_ALIGN_CENTER);
  lv_obj_set_y(pause_timer_panel, 30);

  // add paused symbol
  lv_obj_t *sym_paused = lv_img_create(pause_timer_panel);
  lv_img_set_src(sym_paused, &icon_pause);
  lv_obj_set_width(sym_paused, LV_SIZE_CONTENT);  /// 139
  lv_obj_set_height(sym_paused, LV_SIZE_CONTENT); /// 94
  lv_obj_set_align(sym_paused, LV_ALIGN_LEFT_MID);
  lv_obj_clear_flag(sym_paused, LV_OBJ_FLAG_SCROLLABLE); /// Flags

  pause_timer = lv_label_create(pause_timer_panel);
  lv_obj_set_width(pause_timer, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_height(pause_timer, LV_SIZE_CONTENT); /// 1
  lv_obj_set_align(pause_timer, LV_ALIGN_RIGHT_MID);
  lv_label_set_text(pause_timer, "--:--");
  lv_obj_set_style_text_font(pause_timer, &opensans_bold_70,
                             LV_PART_MAIN |
                                 LV_STATE_DEFAULT); // requires font size 70, opensans bold
}

lv_obj_t *screen_read_only_get() {
  if (screen_read_only_disp == NULL) {
    read_only_screen_design(); // for read-only page design
  }
  return screen_read_only_disp;
}