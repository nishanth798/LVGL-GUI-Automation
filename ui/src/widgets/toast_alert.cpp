#include "sys_state.h"
#include "ui.h"

// Create a global toast object and reassign it to the active screen,
// everytime we want to show a toast.

static lv_obj_t *alert_toast;
static lv_obj_t *img_icon, *img_close, *label_msg;
const sys_state_t *sys_state;
static char prev_badge_text[50] = {'\0'}; // to hold the text of previous bage.

bool flg_alert_toast_hidden = true;

// This is the function that should be called from other files, to hide the
// toast message.
void alert_toast_hide() {
  if (alert_toast != NULL) {
    // LV_LOG_USER("alert toast hide called!");
    lv_obj_add_flag(alert_toast, LV_OBJ_FLAG_HIDDEN);
    flg_alert_toast_hidden = true;
  }
}

static void img_close_click_cb(lv_event_t *e) { alert_toast_hide(); }

static void alert_toast_click_cb(lv_event_t *e) {
  alert_toast_hide(); // hide alert toast and show the active home screen
  lv_obj_t *screen = screen_home_active_get();
  lv_screen_load(screen);
}

static void cont_img_close_click_cb(lv_event_t *e) { alert_toast_hide(); }

// hidden by default
static void alert_toast_init(lv_obj_t *obj) {
  alert_toast = lv_obj_create(obj);
  lv_obj_set_size(alert_toast, 459, 55);
  lv_obj_clear_flag(alert_toast, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(alert_toast, LV_OBJ_FLAG_FLOATING);
  lv_obj_set_style_radius(alert_toast, 10, LV_PART_MAIN);
  lv_obj_set_style_border_width(alert_toast, 2, LV_PART_MAIN);
  lv_obj_set_align(alert_toast, LV_ALIGN_TOP_MID);
  lv_obj_set_style_pad_right(alert_toast, 0, LV_PART_MAIN);
  lv_obj_add_flag(alert_toast, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_flag(alert_toast, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_style_border_opa(alert_toast, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(alert_toast, 255, LV_PART_MAIN);

  lv_obj_set_y(alert_toast, 12);

  // icon
  img_icon = lv_img_create(alert_toast);
  lv_obj_set_size(img_icon, LV_SIZE_CONTENT, LV_SIZE_CONTENT); /// 1
  lv_obj_set_align(img_icon, LV_ALIGN_LEFT_MID);
  lv_obj_clear_flag(img_icon, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_add_flag(img_icon, LV_OBJ_FLAG_EVENT_BUBBLE); /// Flags
  lv_obj_add_flag(img_icon, LV_OBJ_FLAG_CLICKABLE);

  // text label
  label_msg = lv_label_create(alert_toast);
  lv_obj_set_style_text_font(label_msg, &opensans_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_size(label_msg, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_add_flag(label_msg, LV_OBJ_FLAG_EVENT_BUBBLE); /// Flags
  lv_obj_add_flag(label_msg, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_style_text_opa(label_msg, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_t *cont_img_close = lv_obj_create(alert_toast);
  lv_obj_set_size(cont_img_close, 55, 55);
  // lv_obj_add_flag(cont_img_close, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_set_align(cont_img_close, LV_ALIGN_RIGHT_MID);
  lv_obj_clear_flag(cont_img_close, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_add_flag(cont_img_close, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_style_bg_opa(cont_img_close, 0, LV_PART_MAIN);
  lv_obj_set_style_border_opa(cont_img_close, 0, LV_PART_MAIN);

  // Add Close "X" button
  img_close = lv_img_create(cont_img_close);
  lv_img_set_src(img_close, &icon_badge_close);
  // lv_obj_set_size(img_close, LV_SIZE_CONTENT, LV_SIZE_CONTENT); /// 1
  //  lv_obj_clear_flag(img_close, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_align(img_close, LV_ALIGN_RIGHT_MID);
  // lv_obj_add_flag(img_close, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_flag(label_msg, LV_OBJ_FLAG_EVENT_BUBBLE); /// Flags
  // lv_obj_add_flag(img_close,LV_OBJ_FLAG_ADV_HITTEST);

  lv_obj_add_event_cb(alert_toast, alert_toast_click_cb, LV_EVENT_CLICKED, NULL);
  // lv_obj_add_event_cb(img_close, img_close_click_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(cont_img_close, cont_img_close_click_cb, LV_EVENT_CLICKED, NULL);
}

// void init_alert_toast() {
//   if (alert_toast == NULL) {
//     alert_toast_init();
//   }
// //   lv_obj_set_parent(alert_toast, obj);
// }

void alert_toast_set_text(const char *text) {
  // LV_LOG_USER("toast message: %s", text);
  lv_label_set_text(label_msg, "");
  lv_label_set_text(label_msg, text);
}

void badge_alert_show() {

  // if the badge text is not same as previous, then only show
  if (strcmp(prev_badge_text, lv_label_get_text(label_msg)) != 0) {
    // LV_LOG_USER("alert toast is showed!");
    strcpy(prev_badge_text, lv_label_get_text(label_msg));
    lv_obj_clear_flag(alert_toast, LV_OBJ_FLAG_HIDDEN);
    flg_alert_toast_hidden = false;
  }
}

void set_badge_style_alert() {
  char msg[50];
  if (sys_state->alert != ALERT_NONE) {
    switch (sys_state->alert) {
    case ALERT_BED_EXIT:
      strcpy(msg, "Bed Exit");
      lv_img_set_src(img_icon, &icon_badge_bed); // set icon
      break;
    case ALERT_CHAIR_EXIT:
      strcpy(msg, "Chair Exit");
      lv_img_set_src(img_icon, &icon_badge_chair); // set icon
      break;
    case ALERT_FALL_DETECTED:
      strcpy(msg, "Fall Detected");
      lv_img_set_src(img_icon, &icon_badge_fall_detected); // set icon
      break;
    case ALERT_REPOSITION:
      strcpy(msg, "Reposition");
      lv_img_set_src(img_icon, &icon_badge_reposition); // set icon
      break;
    default:
      break;
    }
  }
  if (strcmp(sys_state->alert_title, "") != 0) {
    strcpy(msg, sys_state->alert_title);
    lv_img_set_src(img_icon, &icon_badge_gen_alert); // set generic alert icon
  }

  // if alert is reposition alert and alert title is empty then apply purple theme
  if ((sys_state->alert == ALERT_REPOSITION) && (strcmp(sys_state->alert_title, "") == 0)) {
    // close `X` icon recolor
    lv_obj_set_style_img_recolor(img_close, lv_color_hex(VST_COLOR_ALERT_PURPLE), LV_PART_MAIN);
    // lv_obj_set_style_img_recolor_opa(img_close, 255, LV_PART_MAIN);

    // badge border color
    lv_obj_set_style_border_color(alert_toast, lv_color_hex(VST_COLOR_ALERT_PURPLE),
                                  LV_PART_MAIN | LV_STATE_DEFAULT);
    // badge bg color
    lv_obj_set_style_bg_color(alert_toast, lv_color_hex(0XEFD6FF), LV_PART_MAIN);
    // label text color and aligning
    lv_obj_set_style_text_color(label_msg, lv_color_hex(VST_COLOR_ALERT_PURPLE),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
  } else {
    // close `X` icon recolor
    lv_obj_set_style_img_recolor(img_close, lv_color_hex(0x650F16), LV_PART_MAIN);
    // lv_obj_set_style_img_recolor_opa(img_close, 255, LV_PART_MAIN);

    // badge border color
    lv_obj_set_style_border_color(alert_toast, lv_color_hex(0xAC211C),
                                  LV_PART_MAIN | LV_STATE_DEFAULT);
    // badge bg color
    lv_obj_set_style_bg_color(alert_toast, lv_color_hex(0xFFD0D0), LV_PART_MAIN);
    // label text color and aligning
    lv_obj_set_style_text_color(label_msg, lv_color_hex(0x650F16), LV_PART_MAIN | LV_STATE_DEFAULT);
  }
  lv_obj_set_style_img_recolor_opa(img_close, 255, LV_PART_MAIN);
  lv_obj_align_to(label_msg, img_icon, LV_ALIGN_OUT_RIGHT_MID, 15, 3);
  alert_toast_set_text(msg);
}

void set_badge_style_error() {
  char msg[50];
  if (sys_state->syserr != SYSERR_NONE) {
    switch (sys_state->syserr) {
    case SYSERR_INCORRECT_MODE:
      strcpy(msg, "Incorrect Mode");
      lv_img_set_src(img_icon, &icon_badge_incorrect_mode); // set icon
      break;
    case SYSERR_SUNLIGHT:
      strcpy(msg, "Sunlight");
      lv_img_set_src(img_icon, &icon_badge_sunlight); // set icon
      break;
    case SYSERR_OBSTRUCTED:
      strcpy(msg, "Obstructed");
      lv_img_set_src(img_icon, &icon_badge_obstruct); // set icon
      break;
    case SYSERR_NOT_MONITORING:
      strcpy(msg, "Not Monitoring");
      lv_img_set_src(img_icon, &icon_badge_sensor_disconnected); // set icon
      break;
    case SYSERR_SYSTEM_DISCONNECTED:
      strcpy(msg, "System Disconnected");
      lv_img_set_src(img_icon, &icon_badge_comp_disconnected); // set icon
      break;
    case SYSERR_UNASSIGNED:
      strcpy(msg, "Unassigned");
      lv_img_set_src(img_icon, &icon_badge_unassigned); // set icon
      break;
    default:
      break;
    }
  }
  if (strcmp(sys_state->syserr_title, "") != 0) {
    strcpy(msg, sys_state->syserr_title);
    lv_img_set_src(img_icon, &icon_badge_gen_error); // set generic error icon
  }

  // close `X` icon recolor
  lv_obj_set_style_img_recolor(img_close, lv_color_hex(0x31344E), LV_PART_MAIN);
  lv_obj_set_style_img_recolor_opa(img_close, 255, LV_PART_MAIN);

  // badge border color
  lv_obj_set_style_border_color(alert_toast, lv_color_hex(0xF78513),
                                LV_PART_MAIN | LV_STATE_DEFAULT);

  // badge bg color
  lv_obj_set_style_bg_color(alert_toast, lv_color_hex(0xFFF1BE), LV_PART_MAIN);

  // label text color and aligning
  lv_obj_set_style_text_color(label_msg, lv_color_hex(0x31344E), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_align_to(label_msg, img_icon, LV_ALIGN_OUT_RIGHT_MID, 15, 3);

  alert_toast_set_text(msg);
}

void set_badge_alert_style() {
  sys_state = sys_state_get();
  if (sys_state->syserr != SYSERR_NONE || strcmp(sys_state->syserr_title, "") != 0) {
    set_badge_style_error();
  } else if (sys_state->alert != ALERT_NONE || strcmp(sys_state->alert_title, "") != 0) {
    set_badge_style_alert();
  } else {
    // if the alert/error is cleared, then clear the previous badge text
    memset(prev_badge_text, '\0', sizeof(prev_badge_text));
    alert_toast_hide(); // if error is cleared, then hide the badge and return
    return;
  }
  badge_alert_show(); // if any alert or error exist, then show the badge
  // LV_LOG_USER("alert toast message is %s \n", prev_badge_text);
}

void alert_toast_message_update() {
  // if alert toast is not initialized, then initialize it
  lv_obj_t *screen_active = lv_screen_active();
  if (alert_toast == NULL) {
    alert_toast_init(screen_active);
  }

  sys_state = sys_state_get();
  if (sys_state->syserr != SYSERR_NONE || strcmp(sys_state->syserr_title, "") != 0) {
    set_badge_style_error();
  } else if (sys_state->alert != ALERT_NONE || strcmp(sys_state->alert_title, "") != 0) {
    set_badge_style_alert();
  } else {
    lv_label_set_text(label_msg, ""); // if no alert/error then clear label text
  }

  strcpy(prev_badge_text, lv_label_get_text(label_msg)); // update the message text into buffer
  lv_label_set_text(label_msg, "");                      // clear the message label
}

// This is the function that should be called from other files with respective
// screen as object, to show the toast message on that screen.
void alert_toast_show() {
  SETTINGS_MODE settings_mode = screen_settings_get_mode();
  if (settings_mode == SETTINGS_MODE_SAVE) {
    // LV_LOG_USER("alert toast show called!");
    lv_obj_t *screen_active = lv_screen_active();
    if (alert_toast == NULL) {
      alert_toast_init(screen_active);
    }
    lv_obj_set_parent(alert_toast, screen_active);
    // flg_alert_toast_hidden = false;
    set_badge_alert_style();
    return;
  }
}

// returns true, if alert toast is hidden and
// returns false, if alert toast is not hidden
bool is_alert_toast_hidden() { return flg_alert_toast_hidden; }

void set_alert_toast_parent(lv_obj_t *parent_obj) {
  // lv_obj_t *screen_active = lv_screen_active();
  lv_obj_set_parent(alert_toast, parent_obj);
  return;
}