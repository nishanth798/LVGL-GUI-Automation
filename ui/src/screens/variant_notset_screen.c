#include "ui.h"

// this screen
static lv_obj_t *screen;
static lv_obj_t *img_icon;

static void screen_load_cb(lv_event_t *e) {
  // set this screen as parent for firmware upgrade status toast message.
  set_firmmware_toast_parent(screen);
}

static void screen_init() {
  screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screen, lv_color_hex(VST_COLOR_DARKBLUE), LV_PART_MAIN);

  // Image object
  img_icon = lv_img_create(screen);
  lv_img_set_src(img_icon, &icon_virtusense_logo);
  lv_obj_set_size(img_icon, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_align(img_icon, LV_ALIGN_CENTER);
  lv_obj_add_event_cb(screen, screen_load_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
}

lv_obj_t *screen_variant_notset_get() {
  if (screen == NULL) {
    screen_init();
  }
  return screen;
}