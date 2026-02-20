#include "screens.h"
#include "stdio.h"

static SETTINGS_MODE settings_mode = SETTINGS_MODE_SAVE;

void settings_show_mode(lv_obj_t *screen, lv_obj_t *btn_save, lv_obj_t *cont_btns_nav,
                        SETTINGS_MODE mode) {
  if (mode == SETTINGS_MODE_ACTIVATION) {
    tmpl_settings_show_label_cancel(screen, true);
    lv_obj_add_flag(btn_save, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(cont_btns_nav, LV_OBJ_FLAG_HIDDEN);

  } else if (mode == SETTINGS_MODE_SAVE) {
    tmpl_settings_show_label_cancel(screen, false);
    lv_obj_add_flag(cont_btns_nav, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(btn_save, LV_OBJ_FLAG_HIDDEN);
  }
}

void screen_settings_set_mode(SETTINGS_MODE mode) {
  settings_mode = mode;
}

SETTINGS_MODE screen_settings_get_mode() {
  return settings_mode;
}