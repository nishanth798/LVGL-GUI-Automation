#include "ui.h"

// Create a global toast object and reassign it to the active screen,
// everytime we want to show a toast.

static lv_obj_t *firmware_toast;
static lv_timer_t *timer;

// hidden by default
static void firmware_toast_init(lv_obj_t *obj) {
  firmware_toast = lv_obj_create(obj);
  lv_obj_set_size(firmware_toast, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_clear_flag(firmware_toast, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(firmware_toast, LV_OBJ_FLAG_HIDDEN | LV_OBJ_FLAG_IGNORE_LAYOUT);

  lv_obj_set_style_bg_color(firmware_toast, lv_color_hex(0x322F35),
                            LV_PART_MAIN); //(0x9966ff)
  lv_obj_set_style_bg_opa(firmware_toast, LV_OPA_80, LV_PART_MAIN);
  lv_obj_set_style_radius(firmware_toast, 30, LV_PART_MAIN);
  lv_obj_set_style_border_width(firmware_toast, 0, LV_PART_MAIN);
  lv_obj_set_align(firmware_toast, LV_ALIGN_CENTER);

  lv_obj_set_y(firmware_toast, -70);

  lv_obj_t *label = lv_label_create(firmware_toast);
  lv_obj_set_style_text_font(label, &opensans_bold_24, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_color(label, lv_color_hex(0xe67300),
                              LV_PART_MAIN | LV_STATE_DEFAULT); // lv_color_white()(0xff8000)
  lv_obj_set_style_text_align(label, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_align(label, LV_ALIGN_CENTER);
}

void set_firmmware_toast_parent(lv_obj_t *obj) {
  if (firmware_toast == NULL) {
    firmware_toast_init(obj);
  }
  lv_obj_set_parent(firmware_toast, obj);
}

void label_firmware_toast_set_text(lv_obj_t *obj, const char *text) {
  // LV_LOG_USER("toast message: %s", text);
  lv_obj_t *label = lv_obj_get_child(obj, 0);
  lv_label_set_text(label, "");
  lv_label_set_text(label, text);
}

// NOTE: fade in/out animation is janky

void label_firmware_toast_show(lv_obj_t *obj) { lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN); }

void label_firmware_toast_hide(lv_obj_t *obj) { lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN); }

// This is the function that should be called from other files with respective
// screen as object, to show the toast message on that screen.
void firmware_status_toast_show(lv_obj_t *obj, const char *text) {
  // LV_LOG_USER("toast message: %s", text);
  set_firmmware_toast_parent(obj);
  label_firmware_toast_set_text(firmware_toast, text);
  label_firmware_toast_show(firmware_toast);
}

// This is the function that should be called from other files, to hide the
// toast message.
void firmware_status_toast_hide() { label_firmware_toast_hide(firmware_toast); }
