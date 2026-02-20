#include "jsonrpc2.h"
#include "widgets.h"

// todo:
// - add saved icon
// - make primary (next, save etc) and secondary (previous) themes

// widget info
typedef struct {
  const char **texts;
  const lv_img_dsc_t **icons;
  lv_obj_t *label, *spinner, *img_icon;
  BTN_STATE state;
} widget_info_t;

static widget_info_t *get_widget_info(lv_obj_t *obj) {
  return (widget_info_t *)lv_obj_get_user_data(obj);
}

lv_obj_t *btn_get_label(lv_obj_t *obj) {
  widget_info_t *winfo = (widget_info_t *)lv_obj_get_user_data(obj);
  return winfo->label;
}

static bool has_texts(widget_info_t *winfo) {
  if (winfo->texts && winfo->texts[0] != NULL) {
    return true;
  }
  return false;
}

static bool has_icons(widget_info_t *winfo) {
  if (winfo->icons && winfo->icons[0] != NULL) {
    return true;
  }
  return false;
}
static bool has_spinner(widget_info_t *winfo) { return winfo->spinner != NULL; }

static bool has_img_icon(widget_info_t *winfo) { return winfo->img_icon != NULL; }

lv_style_t style_btn, style_btn_label;

static void style_btn_init(lv_style_t *style) {
  lv_style_init(style);
  lv_style_set_width(style, lv_pct(100));
  lv_style_set_height(style, 80);
  lv_style_set_radius(style, 5);
  lv_style_set_align(style, LV_ALIGN_BOTTOM_MID);
}

static void style_btn_label_init(lv_style_t *style) {
  lv_style_init(style);
  lv_style_set_width(style, LV_SIZE_CONTENT);
  lv_style_set_height(style, LV_SIZE_CONTENT);
  // lv_style_set_align(style, LV_ALIGN_CENTER);
  lv_style_set_text_font(style, &opensans_bold_24); // Updated font size from 20 to 24
}

// adds a icon image next to the button label
static lv_obj_t *img_icon_add_to_btn(lv_obj_t *btn, const lv_img_dsc_t *img) {
  lv_obj_t *icon = lv_img_create(btn);
  lv_img_set_src(icon, img);
  lv_obj_set_size(icon, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_add_flag(icon, LV_OBJ_FLAG_HIDDEN);
  // locate button label inside the button
  lv_obj_t *label = lv_obj_get_child(btn, 0);
  lv_obj_align_to(icon, label, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
  return icon;
}

lv_obj_t *btn_create(lv_obj_t *obj, const char *text) {
  lv_obj_t *btn = lv_btn_create(obj);

  // add default btn style
  lv_obj_add_style(btn, &style_btn, LV_PART_MAIN);

  lv_obj_t *label = lv_label_create(btn);
  lv_obj_add_style(label, &style_btn_label, LV_PART_MAIN);
  lv_label_set_text_static(label, text);
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 4); // Center the label

  // add widget's internal structure as userdata
  widget_info_t *winfo = (widget_info_t *)lv_malloc(sizeof(widget_info_t));
  winfo->label = label;
  winfo->spinner = NULL;
  winfo->img_icon = NULL;
  winfo->texts = NULL;
  winfo->icons = NULL;
  lv_obj_set_user_data(btn, winfo);

  return btn;
}

int8_t btn_set_state(lv_obj_t *obj, BTN_STATE state) {
  widget_info_t *w = get_widget_info(obj);

  if (has_texts(w)) {
    if (w->texts[state] != NULL) {
      lv_label_set_text_static(w->label, w->texts[state]);
    } else {
      lv_label_set_text_static(w->label, w->texts[BTN_STATE_ACTIVE]);
    }
  }

  // reset state
  lv_obj_clear_state(obj, LV_STATE_DISABLED);
  lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);

  if (has_spinner(w)) {
    lv_obj_add_flag(w->spinner, LV_OBJ_FLAG_HIDDEN);
  }

  if (has_img_icon(w)) {
    lv_obj_add_flag(w->img_icon, LV_OBJ_FLAG_HIDDEN);
  }

  switch (state) {
  case BTN_STATE_ACTIVE:
    break;
  case BTN_STATE_INACTIVE:
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    break;
  case BTN_STATE_DISABLED:
    lv_obj_add_state(obj, LV_STATE_DISABLED);
    break;
  case BTN_STATE_SUBMITTING:
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    if (!has_spinner(w)) {
      w->spinner = spinner_add_to_btn(obj);
    }
    lv_obj_clear_flag(w->spinner, LV_OBJ_FLAG_HIDDEN);
    break;
  case BTN_STATE_SUBMITTED:
    // lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    // if (!has_img_icon(w)) {
    //   w->img_icon = img_icon_add_to_btn(obj, w->icons[state]);
    // }
    // lv_obj_clear_flag(w->img_icon, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_state(obj, LV_STATE_DISABLED);
    show_save_toast();
    break;
  default:
    // unknown state
    return -1;
  }
  return 0;
}

void btn_styles_init() {
  style_btn_init(&style_btn);
  style_btn_label_init(&style_btn_label);
}

// BTN_STATE_ACTIVE (aka default state) cannot be null
// can be set only once
int8_t btn_set_state_label_text_map(lv_obj_t *obj, const char *texts[BTN_STATE_COUNT]) {
  widget_info_t *winfo = get_widget_info(obj);

  if (has_texts(winfo)) {
    return BTN_LABELS_ALREADY_SET;
  }

  if (texts == NULL || texts[BTN_STATE_ACTIVE] == NULL) {
    return BTN_LABELS_MISSING_ACTIVE;
  }

  winfo->texts = texts;

  return 0;
}

int8_t btn_set_state_icon_map(lv_obj_t *obj, const lv_img_dsc_t *icons[BTN_STATE_COUNT]) {
  widget_info_t *winfo = get_widget_info(obj);

  if (has_icons(winfo)) {
    return BTN_LABELS_ALREADY_SET;
  }

  winfo->icons = icons;

  return 0;
}

// void btn_set_state_icons(lv_obj_t *obj, const lv_img_dsc_t *icons[]) {
// lv_obj_set_user_data(obj, icons); }
