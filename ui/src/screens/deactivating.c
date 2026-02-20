#include "ui.h"

// this screen
static lv_obj_t *screen;
static lv_obj_t *img_icon;

static void screen_load_cb(lv_event_t *e) {
  // set this screen as parent for firmware upgrade status toast message.
  set_firmmware_toast_parent(screen);
}

static void screen_init() {
  lv_obj_t *cont_deactivate = scr_trans_create(); // main container
  scr_trans_set_color(cont_deactivate, lv_color_hex(VST_COLOR_GREY));
  scr_trans_set_title(cont_deactivate, "DEACTIVATING...");

  // Image object
  img_icon = lv_img_create(cont_deactivate);
  lv_img_set_src(img_icon, &icon_deactivate);
  lv_obj_set_size(img_icon, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_add_flag(img_icon, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_set_align(img_icon, LV_ALIGN_CENTER);
  lv_obj_set_y(img_icon, 35);
  // lv_obj_align_to(img_icon, cont_header, LV_ALIGN_OUT_BOTTOM_MID, 0, 120);

  screen = lv_obj_get_parent(cont_deactivate); // get the parent of this screen
  lv_obj_add_event_cb(screen, screen_load_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
}

lv_obj_t *screen_deactivating_get() {
  if (screen == NULL) {
    screen_init();
  }
  return screen;
}
