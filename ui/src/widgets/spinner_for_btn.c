#include "ui.h"

// adds a spinner next to the button label
lv_obj_t *spinner_add_to_btn(lv_obj_t *btn) {
  lv_obj_t *spinner = lv_spinner_create(btn);
  lv_spinner_set_anim_params(spinner, 1000, 90);
  lv_obj_set_width(spinner, 28);
  lv_obj_set_height(spinner, 28);
  lv_obj_add_flag(spinner, LV_OBJ_FLAG_HIDDEN); /// Flags
  lv_obj_set_style_arc_width(spinner, 4, LV_PART_MAIN);
  lv_obj_set_style_arc_width(spinner, 4, LV_PART_INDICATOR);
  //   lv_obj_set_style_arc_color(spinner, lv_color_hex(VST_COLOR_SPINNER_ARC),
  //                              LV_PART_INDICATOR | LV_STATE_DEFAULT);

  // locate button label inside the button
  lv_obj_t *label = lv_obj_get_child(btn, 0);
  lv_obj_align_to(spinner, label, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
  return spinner;
}