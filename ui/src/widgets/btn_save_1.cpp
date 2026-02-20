#include "ui.h"
#include "widgets.h"

static const char *label_texts[] = {"Save", NULL, NULL, "Saving", "Save"};

static const lv_img_dsc_t *icons[] = {NULL, NULL, NULL, NULL, &icon_saved_check};

lv_obj_t *btn_save_create_1(lv_obj_t *obj) {
  lv_obj_t *btn = btn_create(obj, "Save");
  lv_obj_set_style_bg_color(btn, lv_color_hex(VST_COLOR_DARKBLUE), LV_PART_MAIN);

  int8_t err = btn_set_state_label_text_map(btn, label_texts);
  if (err) {
    // LV_LOG_USER("btn_label_map_error: %d", err);
  }
  err = btn_set_state_icon_map(btn, icons);
  if (err) {
    // LV_LOG_USER("btn_label_icon_map_error: %d", err);
  }
  return btn;
}

lv_obj_t *btn_secondary_create_1(lv_obj_t *obj, const char *text) {
  lv_obj_t *btn = btn_create(obj, text);
  lv_obj_set_style_bg_color(btn, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_border_color(btn, lv_color_hex(VST_COLOR_DARKBLUE), LV_PART_MAIN);
  lv_obj_set_style_border_width(btn, 3, LV_PART_MAIN);
  lv_obj_set_width(btn, 216); // fixed width

  lv_obj_t *label = btn_get_label(btn);
  lv_obj_set_style_text_color(label, lv_color_hex(VST_COLOR_DARKBLUE), LV_PART_MAIN);

  return btn;
}

lv_obj_t *btn_primary_create_1(lv_obj_t *obj, const char *text, uint32_t color) {
  lv_obj_t *btn = btn_create(obj, text);
  lv_obj_set_style_bg_color(btn, lv_color_hex(color), LV_PART_MAIN);
  lv_obj_set_width(btn, 216); // fixed width
  return btn;
}