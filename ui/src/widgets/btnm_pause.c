#include "ui.h"
#include "widgets.h"

// this widget
lv_obj_t *widget_btnm_pause;

static const char *btnm_map[] = {"Short Pause", "Long Pause", NULL};

static lv_style_t style_bg, style_btn;

static void style_bg_init(lv_style_t *style) {
  lv_style_init(style);
  lv_style_set_pad_all(style, 0);
  lv_style_set_pad_gap(style, 16);
  lv_style_set_border_width(style, 0);
  lv_style_set_align(style, LV_ALIGN_BOTTOM_MID);
}

static void style_btn_init(lv_style_t *style) {
  lv_style_init(style);
  lv_style_set_bg_color(style, lv_color_hex(VST_COLOR_DARKBLUE));
  lv_style_set_text_color(style, lv_color_white());
  lv_style_set_text_font(style, &opensans_bold_20);
  lv_style_set_radius(style, 5);
}

PAUSE btnm_pause_btn_id_to_pause(uint16_t btn_id) {
  switch (btn_id) {
  case 0:
    return PAUSE_SHORT;
  case 1:
    return PAUSE_LONG;
  default:
    return -1;
  }
}

static void btnm_pause_init(lv_obj_t *parent) {
  // create and configure button matrix
  widget_btnm_pause = lv_buttonmatrix_create(parent);
  lv_buttonmatrix_set_map(widget_btnm_pause, btnm_map);
  lv_buttonmatrix_set_button_ctrl_all(widget_btnm_pause, LV_BUTTONMATRIX_CTRL_NO_REPEAT);

  // apply styles
  style_bg_init(&style_bg);
  lv_obj_add_style(widget_btnm_pause, &style_bg, LV_PART_MAIN);
  style_btn_init(&style_btn);
  lv_obj_add_style(widget_btnm_pause, &style_btn, LV_PART_ITEMS);

  // placement
  lv_obj_set_size(widget_btnm_pause, 448, 80);
  lv_obj_set_pos(widget_btnm_pause, 0, -16);
}

// fixme: *_create()
// todo: add spinners
// todo: widen container to we can see the button expand during press.
lv_obj_t *widget_btnm_pause_add(lv_obj_t *screen) {
  if (widget_btnm_pause == NULL) {
    btnm_pause_init(screen);
  }
  return widget_btnm_pause;
}

PAUSE widget_btnm_pause_get_selected_btn(lv_obj_t *obj) {
  uint32_t id = lv_buttonmatrix_get_selected_button(obj);
  return btnm_pause_btn_id_to_pause(id);
}
