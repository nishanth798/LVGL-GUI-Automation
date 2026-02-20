#include "string.h"
#include "sys_state.h"
#include "ui.h"
#include "widgets.h"
#include <stdio.h>

// this widget
lv_obj_t *status_panel;
lv_obj_t *title_panel, *title, *title_pr_inj_tmr_panel;
lv_obj_t *sym_bms_panel, *symbol, *bms_indicator;
lv_obj_t *pause_timer_panel, *pause_timer;
lv_obj_t *three_dots, *three_dots_chair;

static lv_style_t style_status_panel;
static lv_style_t style_title_panel, style_title;
static lv_style_t style_sym_bms_panel;
static lv_obj_t *cont_sched_time = NULL;

void set_bms_level(BMS level) {
  if (level == 1)
    lv_label_set_text(bms_indicator, "Low");
  else if (level == 2)
    lv_label_set_text(bms_indicator, "High");
  else if (level == 3)
    lv_label_set_text(bms_indicator, "Ultra-High");
  lv_obj_set_y(bms_indicator, 3); // align text
}

// Convert the seconds to minutes and seconds.
static void seconds_to_mm_ss(uint16_t seconds, int *mm, int *ss) {
  *mm = seconds / 60;
  *ss = seconds % 60;
}

static void minutes_to_hh_mm(uint16_t total_minutes, int *hh, int *mm) {
  *hh = total_minutes / 60;
  *mm = total_minutes % 60;
}

// Set the pressure injury timer value in the status panel
static void set_pr_inj_tmr(uint16_t pr_inj_tmr) {
  // validate pr_inj_tmr value before formatting(5999 = 99 hours and 59 minutess)
  if (pr_inj_tmr > 0 && pr_inj_tmr <= 5999) {
    int hours, minutes;
    minutes_to_hh_mm(pr_inj_tmr, &hours, &minutes);
    char fmt_time[6];
    snprintf(fmt_time, 6, "%02d:%02d", hours, minutes);
    lv_obj_t *label = lv_obj_get_child(title_pr_inj_tmr_panel, 1); // second child is label
    lv_label_set_text(label, fmt_time);
    return;
  }
  else{
    lv_obj_t *label = lv_obj_get_child(title_pr_inj_tmr_panel, 1); // second child is label
    lv_label_set_text(label, "00:00");
    return;
  }
}

static void style_status_panel_init(lv_style_t *style) {
  lv_style_init(style);
  lv_style_set_align(style, LV_ALIGN_CENTER);
  lv_style_set_flex_flow(style, LV_FLEX_FLOW_COLUMN);
  lv_style_set_flex_main_place(style, LV_FLEX_ALIGN_CENTER);
  lv_style_set_flex_cross_place(style, LV_FLEX_ALIGN_CENTER);
  lv_style_set_flex_track_place(style, LV_FLEX_ALIGN_CENTER);
  // lv_style_set_bg_color(style, lv_color_hex(VST_COLOR_MONITOR_GREEN));
  lv_style_set_bg_opa(style, 255);
  lv_style_set_pad_left(style, 0);
  lv_style_set_pad_right(style, 0);
  lv_style_set_pad_top(style, 16);
  lv_style_set_pad_bottom(style, 0);
}

static void style_title_panel_init(lv_style_t *style) {
  lv_style_init(style);
  lv_style_set_align(style, LV_ALIGN_TOP_MID);
  // lv_style_set_border_color(style, lv_color_hex(VST_COLOR_MONITOR_GREEN));
  lv_style_set_border_opa(style, 255);
  lv_style_set_border_width(style, 3);
}

static void style_title_init(lv_style_t *style) {
  lv_style_init(style);
  lv_style_set_align(style, LV_ALIGN_CENTER);
  lv_style_set_text_font(style, &opensans_bold_24);
  lv_style_set_text_color(style, lv_color_hex(VST_COLOR_PRIMARY_TEXT));
}

static void style_sym_bms_panel_init(lv_style_t *style) {
  lv_style_init(style);
  lv_style_set_align(style, LV_ALIGN_CENTER);
  lv_style_set_flex_flow(style, LV_FLEX_FLOW_COLUMN);
  lv_style_set_flex_main_place(style, LV_FLEX_ALIGN_CENTER);
  lv_style_set_flex_cross_place(style, LV_FLEX_ALIGN_CENTER);
  lv_style_set_flex_track_place(style, LV_FLEX_ALIGN_CENTER);
  lv_style_set_bg_color(style, lv_color_white());
  lv_style_set_bg_opa(style, 0);
  lv_style_set_border_width(style, 0);
}

static void status_card_init(lv_obj_t *parent) {
  // create status card; the main container
  status_panel = lv_obj_create(parent);
  style_status_panel_init(&style_status_panel);
  lv_obj_add_style(status_panel, &style_status_panel, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_clear_flag(status_panel, LV_OBJ_FLAG_SCROLLABLE); /// Flags

  lv_obj_set_size(status_panel, 448, 221);
  lv_obj_set_flex_grow(status_panel, 1); // Let it expand if space available
                                         // lv_obj_set_flex_flow(status_panel, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_border_width(status_panel, 0, LV_PART_MAIN);

  // create title card; holds title
  title_panel = lv_obj_create(status_panel);
  style_title_panel_init(&style_title_panel);
  lv_obj_add_style(title_panel, &style_title_panel, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_flag(title_panel, LV_OBJ_FLAG_IGNORE_LAYOUT); /// Flags
  lv_obj_clear_flag(title_panel, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_size(title_panel, 446, 56);
  lv_obj_set_pos(title_panel, 0, -16);

  // create title label
  title = lv_label_create(title_panel);
  style_title_init(&style_title);
  lv_obj_align(title, LV_ALIGN_CENTER, 3, 3);
  lv_obj_add_style(title, &style_title, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_size(title, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_label_set_text(title, "");
  lv_obj_set_style_text_color(title, lv_color_black(), LV_PART_MAIN);
  // Add Pressure injury timer with icon

  // Add container with no borders and transparent bg to hold icon and label
  title_pr_inj_tmr_panel = lv_obj_create(title_panel);
  lv_obj_set_size(title_pr_inj_tmr_panel, 94, 28);
  lv_obj_set_align(title_pr_inj_tmr_panel, LV_ALIGN_RIGHT_MID);
  lv_obj_clear_flag(title_pr_inj_tmr_panel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_flex_flow(title_pr_inj_tmr_panel, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(title_pr_inj_tmr_panel, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_bg_opa(title_pr_inj_tmr_panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(title_pr_inj_tmr_panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  // add row gap between each child as 8px
  lv_obj_set_style_pad_gap(title_pr_inj_tmr_panel, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

  // Add icon
  lv_obj_t *icon_timer = lv_img_create(title_pr_inj_tmr_panel);
  lv_img_set_src(icon_timer, &icon_pressure_inj_tmr);
  lv_obj_set_size(icon_timer, LV_SIZE_CONTENT, LV_SIZE_CONTENT); /// 1

  // Add label
  lv_obj_t *pr_inj_timer = lv_label_create(title_pr_inj_tmr_panel);
  lv_obj_set_style_text_font(pr_inj_timer, &opensans_bold_24, LV_PART_MAIN);
  lv_obj_set_style_text_color(pr_inj_timer, lv_color_hex(VST_COLOR_PRIMARY_TEXT), LV_PART_MAIN);
  lv_obj_set_size(pr_inj_timer, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_label_set_text(pr_inj_timer, "00:00");
  
  // Nudge the text up slightly so it lines up optically with the other label
  lv_obj_set_style_pad_top(pr_inj_timer, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_bottom(pr_inj_timer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

  // create container panel to hold status symbol and bms level indicator
  sym_bms_panel = lv_obj_create(status_panel);
  lv_obj_set_size(sym_bms_panel, 448, 165);
  style_sym_bms_panel_init(&style_sym_bms_panel);
  lv_obj_add_style(sym_bms_panel, &style_sym_bms_panel, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_flag(sym_bms_panel, LV_OBJ_FLAG_IGNORE_LAYOUT); /// Flags
  lv_obj_clear_flag(sym_bms_panel, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_pos(sym_bms_panel, 0, 16);

  three_dots = three_dots_add(sym_bms_panel);
  three_dots_chair = sym_chair_add(sym_bms_panel);

  // add symbol
  symbol = lv_img_create(sym_bms_panel);
  lv_obj_set_width(symbol, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_height(symbol, LV_SIZE_CONTENT); /// 1
  lv_obj_set_align(symbol, LV_ALIGN_CENTER);
  lv_obj_clear_flag(symbol, LV_OBJ_FLAG_SCROLLABLE); /// Flags

  // add pause timer container panel
  pause_timer_panel = lv_obj_create(sym_bms_panel);
  lv_obj_set_width(pause_timer_panel, LV_SIZE_CONTENT);  /// 100
  lv_obj_set_height(pause_timer_panel, LV_SIZE_CONTENT); /// 50
  lv_obj_set_align(pause_timer_panel, LV_ALIGN_CENTER);
  lv_obj_set_flex_flow(pause_timer_panel, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(pause_timer_panel, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);
  lv_obj_add_flag(pause_timer_panel, LV_OBJ_FLAG_HIDDEN);       /// Flags
  lv_obj_clear_flag(pause_timer_panel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_bg_color(pause_timer_panel, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(pause_timer_panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(pause_timer_panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_row(pause_timer_panel, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_column(pause_timer_panel, 26, LV_PART_MAIN | LV_STATE_DEFAULT);
  // add paused symbol
  lv_obj_t *sym_paused = lv_img_create(pause_timer_panel);
  lv_img_set_src(sym_paused, &icon_pause);
  lv_obj_set_width(sym_paused, LV_SIZE_CONTENT);  /// 139
  lv_obj_set_height(sym_paused, LV_SIZE_CONTENT); /// 94
  lv_obj_set_align(sym_paused, LV_ALIGN_CENTER);
  lv_obj_clear_flag(sym_paused, LV_OBJ_FLAG_SCROLLABLE); /// Flags

  pause_timer = lv_label_create(pause_timer_panel);
  lv_obj_set_width(pause_timer, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_height(pause_timer, LV_SIZE_CONTENT); /// 1
  lv_obj_set_align(pause_timer, LV_ALIGN_CENTER);
  lv_label_set_text(pause_timer, "--:--");
  lv_obj_set_style_text_font(pause_timer, &opensans_bold_70,
                             LV_PART_MAIN |
                                 LV_STATE_DEFAULT); // requires font size 70, opensans bold

  // add bms indicator
  // bms_indicator = widget_bms_level_indicator_add(sym_bms_panel);
  // add bms level label

  bms_indicator = lv_label_create(sym_bms_panel);
  lv_obj_set_align(bms_indicator, LV_ALIGN_BOTTOM_MID);
  lv_obj_set_style_text_color(bms_indicator, lv_color_hex(0xffffff), LV_PART_MAIN);
  lv_obj_set_style_text_font(bms_indicator, &opensans_bold_24,
                             LV_PART_MAIN |
                                 LV_STATE_DEFAULT); // requires font size 70, opensans bold
}

lv_obj_t *widget_status_panel_add(lv_obj_t *screen) {
  if (status_panel == NULL) {
    status_card_init(screen);
  }
  return status_panel;
}

void set_monitoring_theme() {
  lv_obj_set_style_bg_color(status_panel, lv_color_hex(VST_COLOR_MONITOR_GREEN),
                            LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(title_panel, lv_color_hex(VST_COLOR_MONITOR_GREEN),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_flag(pause_timer_panel, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(three_dots, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(three_dots_chair, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(symbol, LV_OBJ_FLAG_HIDDEN);
  if (cont_sched_time != NULL) {
    lv_obj_add_flag(cont_sched_time, LV_OBJ_FLAG_HIDDEN);
  }
}

static void set_scheduled_monitoring_theme() {
  lv_obj_set_style_bg_color(status_panel, lv_color_hex(VST_COLOR_CERULEAN),
                            LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(title_panel, lv_color_hex(VST_COLOR_CERULEAN),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_flag(pause_timer_panel, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(three_dots, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(three_dots_chair, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(symbol, LV_OBJ_FLAG_HIDDEN);
}

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

// create container to show scheduled monitoring time.
static void create_cont_sched_time(lv_obj_t *parent) {
  // create container for scheduled monitoring
  cont_sched_time = lv_obj_create(parent);
  lv_obj_set_size(cont_sched_time, 235, 40);
  lv_obj_set_align(cont_sched_time, LV_ALIGN_CENTER);
  lv_obj_set_y(cont_sched_time, 76);
  lv_obj_clear_flag(cont_sched_time, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_border_width(cont_sched_time, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(cont_sched_time, lv_color_hex(VST_COLOR_CERULEAN),
                            LV_PART_MAIN | LV_STATE_DEFAULT);

  // create label to show scheduled time
  lv_obj_t *lbl_scheduled_time = lv_label_create(cont_sched_time);

  lv_obj_set_style_text_color(lbl_scheduled_time, lv_color_white(),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(lbl_scheduled_time, &opensans_semibold_24,
                             LV_PART_MAIN | LV_STATE_DEFAULT);
  update_sched_time(cont_sched_time);
  lv_obj_center(lbl_scheduled_time);
}

void widget_status_panel_set_monitoring(MONITOR_MODE mode) {

  // adjust symbol position for bed mode
  
  switch (mode) {
  case MODE_BED:
    lv_label_set_text(title, "BED MONITORING");
    lv_img_set_src(symbol, &icon_bed);
    lv_obj_set_align(symbol, LV_ALIGN_TOP_MID);
    lv_obj_clear_flag(bms_indicator, LV_OBJ_FLAG_HIDDEN);
    set_monitoring_theme();
    break;
  case MODE_CHAIR:
    lv_label_set_text(title, "CHAIR MONITORING");
    lv_img_set_src(symbol, &icon_chair);
    lv_obj_set_align(symbol, LV_ALIGN_CENTER);
    lv_obj_add_flag(bms_indicator, LV_OBJ_FLAG_HIDDEN);
    set_monitoring_theme();
    break;
  case MODE_FALL_MON:
    lv_label_set_text(title, "FALL MONITORING");
    lv_img_set_src(symbol, &icon_fall_detected);
    lv_obj_set_align(symbol, LV_ALIGN_CENTER);
    lv_obj_add_flag(bms_indicator, LV_OBJ_FLAG_HIDDEN);
    set_monitoring_theme();
    break;
  case MODE_PRESSURE_INJURY:
    lv_label_set_text(title, "PRESSURE INJURY");
    lv_img_set_src(symbol, &icon_pressure_injury);
    lv_obj_set_align(symbol, LV_ALIGN_CENTER);
    lv_obj_add_flag(bms_indicator, LV_OBJ_FLAG_HIDDEN);
    set_monitoring_theme();
    break;
  case MODE_SCHEDULED_MON:
    lv_label_set_text(title, "SCHEDULED");
    lv_img_set_src(symbol, &icon_scheduled_mon);
    lv_obj_set_align(symbol, LV_ALIGN_CENTER);
    lv_obj_add_flag(bms_indicator, LV_OBJ_FLAG_HIDDEN);
    if (cont_sched_time == NULL) {
      create_cont_sched_time(status_panel);
    }
    lv_obj_clear_flag(cont_sched_time, LV_OBJ_FLAG_HIDDEN);
    update_sched_time(cont_sched_time);

    set_scheduled_monitoring_theme();
    break;

  default:
    // LV_LOG_USER("unknown mode: %d", mode);
    break;
  }
  if (mode == MODE_BED ) {
    lv_obj_set_y(symbol, 3);
  } else if( mode == MODE_SCHEDULED_MON) {
    lv_obj_set_y(symbol, -18);
  }
  else {
    lv_obj_set_y(symbol, 8); // reset for other modes
  }
}

static void set_flags_alert_theme() {
  lv_obj_add_flag(bms_indicator, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(pause_timer_panel, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(three_dots, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(three_dots_chair, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(symbol, LV_OBJ_FLAG_HIDDEN);
  if (cont_sched_time != NULL) {
    lv_obj_add_flag(cont_sched_time, LV_OBJ_FLAG_HIDDEN);
  }
}

void set_alert_theme() {
  lv_obj_set_style_bg_color(status_panel, lv_color_hex(VST_COLOR_ALERT_RED),
                            LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(title_panel, lv_color_hex(VST_COLOR_ALERT_RED),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
  set_flags_alert_theme();
}

void set_extreme_alert_theme() {
  lv_obj_set_style_bg_color(status_panel, lv_color_hex(VST_COLOR_EXTREME_ALERT),
                            LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(title_panel, lv_color_hex(VST_COLOR_EXTREME_ALERT),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
  set_flags_alert_theme();
}

static void set_purple_alert_theme() {
  lv_obj_set_style_bg_color(status_panel, lv_color_hex(VST_COLOR_ALERT_PURPLE),
                            LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(title_panel, lv_color_hex(VST_COLOR_ALERT_PURPLE),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
  set_flags_alert_theme();
}

// if alert_title is non empty, set title to alert_title, else status to
// preconfigured alert
void widget_status_panel_set_alert(ALERT alert, const char *alert_title) {
  // adjust symbol position for bed exit alert
  if (alert == ALERT_BED_EXIT) {
    lv_obj_set_y(symbol, 3);
  } else {
    lv_obj_set_y(symbol, 8); // reset for other alerts
  }

  if (strcmp(alert_title, "") != 0) {
    lv_label_set_text(title, alert_title);
    lv_img_set_src(symbol, &icon_alert_90);
    lv_obj_set_align(symbol, LV_ALIGN_CENTER);
    set_alert_theme();
    return;
  }

  switch (alert) {
  case ALERT_BED_EXIT:
    lv_label_set_text(title, "BED EXIT");
    lv_img_set_src(symbol, &icon_bed);
    lv_obj_set_align(symbol, LV_ALIGN_TOP_MID);
    set_alert_theme();
    lv_obj_clear_flag(bms_indicator, LV_OBJ_FLAG_HIDDEN);
    break;
  case ALERT_CHAIR_EXIT:
    lv_label_set_text(title, "CHAIR EXIT");
    lv_img_set_src(symbol, &icon_chair);
    lv_obj_set_align(symbol, LV_ALIGN_CENTER);
    set_alert_theme();
    break;
  case ALERT_FALL_DETECTED:
    lv_label_set_text(title, "FALL DETECTED");
    lv_img_set_src(symbol, &icon_fall_detected);
    lv_obj_set_align(symbol, LV_ALIGN_CENTER); // center align
    set_extreme_alert_theme();
    break;
  case ALERT_REPOSITION:
    lv_label_set_text(title, "REPOSITION");
    lv_img_set_src(symbol, &icon_pressure_injury);
    lv_obj_set_align(symbol, LV_ALIGN_CENTER); // center align
    set_purple_alert_theme();
    break;
  default:
    // LV_LOG_USER("unknown alert: %d", alert);
    break;
  }
}

void set_error_theme() {
  lv_obj_set_style_bg_color(status_panel, lv_color_hex(VST_COLOR_ERROR_YELLOW),
                            LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(title_panel, lv_color_hex(VST_COLOR_ERROR_YELLOW),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_flag(bms_indicator, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(pause_timer_panel, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(three_dots, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(three_dots_chair, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(symbol, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_align(symbol, LV_ALIGN_CENTER);
  if (cont_sched_time != NULL) {
    lv_obj_add_flag(cont_sched_time, LV_OBJ_FLAG_HIDDEN);
  }
}

void widget_status_panel_set_error(SYSERR err, const char *syserr_title) {

  if (err == SYSERR_SYSTEM_DISCONNECTED) {
    lv_label_set_text(title, "SYSTEM DISCONNECTED");
    lv_img_set_src(symbol, &icon_computer_disconnected);
    set_error_theme();
    return;
  }

  if (strcmp(syserr_title, "") != 0) {
    lv_label_set_text(title, syserr_title);
    lv_img_set_src(symbol, &icon_error_90);
    set_error_theme();
    return;
  }

  switch (err) {
  case SYSERR_INCORRECT_MODE:
    lv_label_set_text(title, "INCORRECT MODE");
    lv_img_set_src(symbol, &icon_incorrect_mode);
    set_error_theme();
    break;
  case SYSERR_SUNLIGHT:
    lv_label_set_text(title, "SUNLIGHT");
    lv_img_set_src(symbol, &icon_sunlight);
    set_error_theme();
    break;
  case SYSERR_OBSTRUCTED:
    lv_label_set_text(title, "OBSTRUCTED");
    lv_img_set_src(symbol, &icon_obstructed);
    set_error_theme();
    break;
  case SYSERR_NOT_MONITORING:
    lv_label_set_text(title, "NOT MONITORING");
    lv_img_set_src(symbol, &icon_sensor_disconnected);
    set_error_theme();
    break;
  case SYSERR_SYSTEM_DISCONNECTED:
    lv_label_set_text(title, "SYSTEM DISCONNECTED");
    lv_img_set_src(symbol, &icon_computer_disconnected);
    set_error_theme();
    break;
  case SYSERR_UNASSIGNED:
    lv_label_set_text(title, "UNASSIGNED");
    lv_img_set_src(symbol, &icon_unassigned);
    set_error_theme();
    break;
  default:
    // LV_LOG_USER("unknown sys error: %d", err);
    break;
  }
  lv_obj_set_align(symbol, LV_ALIGN_CENTER); // center align
  lv_obj_set_y(symbol, 8);
}

void set_paused_theme() {
  set_error_theme();
  lv_obj_clear_flag(pause_timer_panel, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(symbol, LV_OBJ_FLAG_HIDDEN);
}

void widget_status_panel_set_paused(uint16_t pause_tmr) {
  if (pause_tmr > 0) {
    lv_label_set_text(title, "PAUSED");
    int mm, ss;
    seconds_to_mm_ss(pause_tmr, &mm, &ss);
    char fmt_time[9];
    snprintf(fmt_time, 9, "%02d:%02d", mm, ss);
    lv_label_set_text(pause_timer, fmt_time);
    set_paused_theme();
    return;
  }
}

void widget_status_panel_set_unset() { widget_status_panel_set_error(SYSERR_NONE, "NO DATA"); }

void set_calib_theme() {
  lv_obj_set_style_bg_color(status_panel, lv_color_hex(VST_COLOR_ERROR_YELLOW),
                            LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(title_panel, lv_color_hex(VST_COLOR_ERROR_YELLOW),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_flag(bms_indicator, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(pause_timer_panel, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(symbol, LV_OBJ_FLAG_HIDDEN);
  if (cont_sched_time != NULL) {
    lv_obj_add_flag(cont_sched_time, LV_OBJ_FLAG_HIDDEN);
  }
}

void widget_status_panel_set_calib(CALIBRATION cal) {
  switch (cal) {
  case CAL_ON:
    lv_label_set_text(title, "CALIBRATING...");
    lv_obj_add_flag(three_dots_chair, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(three_dots, LV_OBJ_FLAG_HIDDEN);
    set_calib_theme();
    break;
  case CAL_CHAIR:
    lv_label_set_text(title, "CALIBRATING CHAIR...");
    lv_obj_add_flag(three_dots, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(three_dots_chair, LV_OBJ_FLAG_HIDDEN);
    set_calib_theme();
  default:
    // LV_LOG_USER("CALIBRATION OFF: %d", cal);
    break;
  }
}

lv_obj_t *dot_add(lv_obj_t *parent) {
  lv_obj_t *dot = lv_img_create(parent);
  lv_img_set_src(dot, &icon_calibration_dot);
  lv_obj_set_size(parent, LV_SIZE_CONTENT, LV_SIZE_CONTENT); /// 1
  return dot;
}

lv_obj_t *three_dots_add(lv_obj_t *parent) {
  lv_obj_t *cont = lv_obj_create(parent);
  lv_obj_set_align(cont, LV_ALIGN_CENTER);
  lv_obj_set_size(cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT); /// 1
  lv_obj_set_style_bg_opa(cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_column(cont, 23, LV_PART_MAIN | LV_STATE_DEFAULT);

  // add 3 dots
  for (size_t i = 0; i < 3; i++) {
    dot_add(cont);
  }
  return cont;
}

lv_obj_t *sym_chair_add(lv_obj_t *parent) {
  lv_obj_t *panel_main = lv_obj_create(parent);
  lv_obj_set_flex_flow(panel_main, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(panel_main, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);
  lv_obj_clear_flag(panel_main, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_bg_opa(panel_main, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_align(panel_main, LV_ALIGN_CENTER, 0, 4);
  lv_obj_set_style_border_width(panel_main, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_pad_row(panel_main, 14,
                           LV_PART_MAIN | LV_STATE_DEFAULT); // gap between chair and dots

  lv_obj_t *img = lv_img_create(panel_main);
  lv_img_set_src(img, &icon_calibrating_chair);
  lv_obj_set_width(img, LV_SIZE_CONTENT);  /// 64
  lv_obj_set_height(img, LV_SIZE_CONTENT); /// 94
  lv_obj_set_align(img, LV_ALIGN_CENTER);
  lv_obj_clear_flag(img, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  // lv_obj_set_style_pad_top(img, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
  // lv_obj_set_style_pad_bottom(img, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_t *panel_dots = three_dots_add(panel_main);
  lv_obj_set_style_pad_top(panel_dots, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

  return panel_main;
}

// Reposition alert shows timer in title bar; adjust visibility and alignment
void set_reposition_alert_state(bool state, uint16_t pr_inj_tmr) {
  if (state) {
    lv_obj_clear_flag(title_pr_inj_tmr_panel, LV_OBJ_FLAG_HIDDEN);       /// Flags
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 3, 3);
    set_pr_inj_tmr(pr_inj_tmr);
  }else{
    lv_obj_add_flag(title_pr_inj_tmr_panel, LV_OBJ_FLAG_HIDDEN);       /// Flags
    lv_obj_align(title, LV_ALIGN_CENTER, 3, 3);
  }
}
