#include "jsonrpc2.h"
#include "ui.h"

// this screen
static lv_obj_t *screen;
static lv_obj_t *img_icon;

static void request_timeout_cb(lv_event_t *e) {
  // pause activating timer
  lv_obj_t *screen = screen_settings_audio_get();
  lv_screen_load(screen);
  toast_show(screen, no_response_str);
}

static void screen_load_cb(lv_event_t *e) {
  // set this screen as parent for firmware upgrade status toast message.
  set_firmmware_toast_parent(screen);
  set_activating_timeout();
}

static void screen_init() {
  lv_obj_t *cont_activating = scr_trans_create(); // main container
  scr_trans_set_color(cont_activating, lv_color_hex(VST_COLOR_DARKBLUE));
  scr_trans_set_title(cont_activating, "ACTIVATING...");

  // Image object
  img_icon = lv_img_create(cont_activating);
  lv_img_set_src(img_icon, &icon_activating);
  lv_obj_set_size(img_icon, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_add_flag(img_icon, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_set_align(img_icon, LV_ALIGN_CENTER);
  lv_obj_set_y(img_icon, 35);
  // lv_obj_align_to(img_icon, cont_header, LV_ALIGN_OUT_BOTTOM_MID, 0, 120);

  screen = lv_obj_get_parent(cont_activating); // get the parent of this screen
  lv_obj_add_event_cb(screen, screen_load_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
  lv_obj_add_event_cb(screen, request_timeout_cb, (lv_event_code_t)MY_EVENT_REQUEST_TIMEOUT, NULL);
}

lv_obj_t *screen_activating_get() {
  if (screen == NULL) {
    screen_init();
  }
  return screen;
}
