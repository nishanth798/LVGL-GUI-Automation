#include "ui.h"

// todo: remove gui creation cruft
// this widget
lv_obj_t *bms_level_indicator;

static void bms_level_indicator_init(lv_obj_t *parent) {
  // create container
  bms_level_indicator = lv_obj_create(parent);
  lv_obj_set_width(bms_level_indicator, 448);
  lv_obj_set_height(bms_level_indicator, 50);
  lv_obj_set_x(bms_level_indicator, 0);
  lv_obj_set_y(bms_level_indicator, 12);
  lv_obj_set_align(bms_level_indicator, LV_ALIGN_BOTTOM_MID);
  lv_obj_set_flex_flow(bms_level_indicator, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(bms_level_indicator, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);
  lv_obj_clear_flag(bms_level_indicator, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_bg_color(bms_level_indicator, lv_color_hex(0xFFFFFF),
                            LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(bms_level_indicator, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(bms_level_indicator, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

  // create level panels for the following 3 levels
  const char *labels[] = {"Low", "High", "Ultra-High", NULL};
  for (int i = 0; labels[i] != NULL; i++) {
    lv_obj_t *panel = lv_obj_create(bms_level_indicator);
    lv_obj_set_width(panel, 136);
    lv_obj_set_height(panel, 36);
    lv_obj_set_align(panel, LV_ALIGN_CENTER);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_radius(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    //  lv_obj_set_style_bg_color(panel, lv_color_hex(0x43BA73), LV_PART_MAIN |
    //  LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(panel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(panel, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(panel, 255, LV_PART_MAIN | LV_STATE_CHECKED);

    lv_obj_t *label = lv_label_create(panel);
    lv_obj_set_width(label, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(label, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(label, LV_ALIGN_CENTER);
    lv_label_set_text(label, labels[i]);

    //  lv_color_t color = lv_obj_get_style_bg_color(panel, LV_PART_MAIN |
    //  LV_STATE_DEFAULT);

    //    printf("color:%x\n",lv_color_to32(color));
    //   lv_obj_set_style_text_color(label, color, LV_PART_MAIN |
    //   LV_STATE_DEFAULT); lv_obj_set_style_text_color(label,
    //   lv_color_hex(0x43BA73), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(label, &opensans_bold_18, LV_PART_MAIN | LV_STATE_DEFAULT);
  }
}

void widget_bms_level_indicator_set(lv_obj_t *obj, BMS level) {

  lv_color_t color = lv_obj_get_style_bg_color(lv_obj_get_parent(lv_obj_get_parent(obj)),
                                               LV_PART_MAIN | LV_STATE_DEFAULT);

  int lvl = level - 1; // offset level by 1 in order to map to panel index
  for (int i = 0; i < lv_obj_get_child_cnt(obj); i++) {
    lv_obj_t *panel = lv_obj_get_child(obj, i);
    lv_obj_t *label = lv_obj_get_child(panel, 0);

    if ((i < lvl) || (i > lvl)) {
      lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_clear_state(panel, LV_STATE_CHECKED);
      lv_obj_clear_flag(label, LV_OBJ_FLAG_HIDDEN);
    } else if (i == lvl) {
      lv_obj_set_style_text_color(label, color, LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_add_state(panel, LV_STATE_CHECKED);
      lv_obj_clear_flag(label, LV_OBJ_FLAG_HIDDEN);
    }
  }
}

lv_obj_t *widget_bms_level_indicator_add(lv_obj_t *screen) {
  if (bms_level_indicator == NULL) {
    bms_level_indicator_init(screen);
  }
  return bms_level_indicator;
}

lv_obj_t *widget_bms_level_indicator_get() { return bms_level_indicator; }