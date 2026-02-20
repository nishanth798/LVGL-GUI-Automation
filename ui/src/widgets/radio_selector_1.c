#include "ui.h"
#include "widgets.h"

#define BTN_COUNT 3

// widget info
typedef struct {
  lv_obj_t *icons[BTN_COUNT];
  lv_obj_t *btns[BTN_COUNT];
  bool has_icons;
  int8_t btn_id;
} widget_info_t;

static widget_info_t *get_widget_info(lv_obj_t *obj) {
  return (widget_info_t *)lv_obj_get_user_data(obj);
}

static void clear_icons(widget_info_t *winfo) {
  uint8_t size = sizeof(winfo->btns) / sizeof(winfo->btns[0]);
  for (uint8_t i = 0; i < size; i++) {
    lv_obj_clear_state(winfo->icons[i], LV_STATE_CHECKED);
  }
}

static void set_icon(widget_info_t *winfo, uint8_t id) {
  lv_obj_add_state(winfo->icons[id], LV_STATE_CHECKED);
}

static void clear_btns(widget_info_t *winfo) {
  uint8_t size = sizeof(winfo->btns) / sizeof(winfo->btns[0]);
  for (uint8_t i = 0; i < size; i++) {
    lv_obj_clear_state(winfo->btns[i], LV_STATE_CHECKED);
    lv_obj_add_flag(winfo->btns[i], LV_OBJ_FLAG_CLICKABLE);
  }
}

static void set_btn(widget_info_t *winfo, uint8_t id) {
  lv_obj_add_state(winfo->btns[id], LV_STATE_CHECKED);
  lv_obj_clear_flag(winfo->btns[id], LV_OBJ_FLAG_CLICKABLE);
}

static void cb_click_cb(lv_event_t *e) {
  lv_obj_t *cb = lv_event_get_target_obj(e);
  lv_obj_t *cont_cb = lv_obj_get_parent(cb);
  uint8_t cb_id = lv_obj_get_index(cont_cb);
  int8_t btn_id = cb_id - 1; // offset by 1 to match ui index and array index

  // get widget info to access icons and btns
  lv_obj_t *widget = lv_obj_get_parent(cont_cb);
  radio_selector_set_selected_btn_1(widget, btn_id);
}

//
// Section: UI
//

static lv_obj_t *label_cb_create(lv_obj_t *obj, const char *text) {
  lv_obj_t *label = lv_label_create(obj);
  lv_obj_set_size(label, LV_SIZE_CONTENT, LV_SIZE_CONTENT); /// 1
  lv_label_set_text_static(label, text);
  lv_obj_set_style_text_font(label, &opensans_regular_15, LV_PART_MAIN);
  lv_obj_set_style_text_color(label, lv_color_hex(VST_COLOR_PRIMARY_TEXT), LV_PART_MAIN);
  return label;
}

static lv_obj_t *cb_create(lv_obj_t *obj, lv_event_cb_t event_cb) {
  lv_obj_t *btn = lv_checkbox_create(obj);
  lv_checkbox_set_text_static(btn, "");
  lv_obj_set_size(btn, 32, 32); // size of the radio button
  // increase radio btn size
  lv_obj_set_style_pad_all(btn, 0, LV_PART_MAIN); // outer padding
  lv_obj_set_style_pad_all(btn, 7, LV_PART_INDICATOR); // inner padding
  // remove spacing between tickbox and label
  lv_obj_set_style_pad_column(btn, 0, LV_PART_MAIN);

  lv_obj_set_style_bg_color(btn, lv_color_hex(0x828282), LV_PART_INDICATOR);
  lv_obj_set_style_border_color(btn, lv_color_hex(0xEFEFEF), LV_PART_INDICATOR);
  lv_obj_set_style_border_width(btn, 6, LV_PART_INDICATOR); // thickness of the circle

  lv_obj_set_style_bg_color(btn, lv_color_hex(0x31344E), LV_PART_INDICATOR | LV_STATE_CHECKED);
  lv_obj_set_style_border_color(btn, lv_color_hex(0xEFEFEF), LV_PART_INDICATOR | LV_STATE_CHECKED);
  lv_obj_set_style_border_width(btn, 5, LV_PART_INDICATOR | LV_STATE_CHECKED); // thickness of the circle

  lv_obj_set_style_outline_color(btn, lv_color_hex(0x31344E), LV_PART_INDICATOR | LV_STATE_CHECKED);
  lv_obj_set_style_outline_width(btn, 2, LV_PART_INDICATOR | LV_STATE_CHECKED);
  lv_obj_set_style_outline_pad(btn, 0, LV_PART_INDICATOR | LV_STATE_CHECKED);

  // remove check icon
  lv_obj_set_style_bg_image_src(btn, NULL, LV_PART_INDICATOR | LV_STATE_CHECKED);

  lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, LV_PART_INDICATOR);
  // lv_obj_set_style_pad_top(btn, 0, LV_PART_MAIN);

  // add event handler
  lv_obj_add_event_cb(btn, event_cb, LV_EVENT_VALUE_CHANGED, NULL);
  // bubble up  event to parent
  lv_obj_add_flag(btn, LV_OBJ_FLAG_EVENT_BUBBLE); /// Flags

  return btn;
}

static lv_obj_t *img_cb_create(lv_obj_t *obj, const lv_img_dsc_t *img) {
  lv_obj_t *icon = lv_img_create(obj);
  lv_img_set_src(icon, img);
  lv_obj_set_size(icon, LV_SIZE_CONTENT, LV_SIZE_CONTENT); /// 64
  lv_obj_clear_flag(icon, LV_OBJ_FLAG_SCROLLABLE);         /// Flags
  lv_obj_set_style_img_recolor(icon, lv_color_hex(VST_COLOR_DARKBLUE),
                               LV_PART_MAIN | LV_STATE_CHECKED);
  lv_obj_set_style_img_recolor_opa(icon, 255, LV_PART_MAIN | LV_STATE_CHECKED);

  return icon;
}

static lv_obj_t *cont_cb_create(lv_obj_t *obj, uint8_t index, bool has_icons) {
  lv_obj_t *cont = lv_obj_create(obj);
  widget_info_t *winfo = get_widget_info(obj);
  if (has_icons) {
    lv_obj_set_size(cont, 75, 103);
  } else {
    lv_obj_set_size(cont, 75, 60); 
  }

  lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
  if(index == 0) {
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  }else if(index == 1) {
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  }else{
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END);
  }
  lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_pad_row(cont, 8, LV_PART_MAIN); // Row gap
  lv_obj_set_style_border_width(cont, 0, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(cont, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(cont, 2, LV_PART_MAIN); // outer padding
  

  lv_obj_add_flag(cont, LV_OBJ_FLAG_EVENT_BUBBLE);

  return cont;
}

//
// Section: API
//

lv_obj_t *radio_selector_create_1(lv_obj_t *obj, const char *labels[],
                                  const lv_img_dsc_t *icons[]) {
  lv_obj_t *widget = lv_obj_create(obj);
  lv_obj_set_size(widget, 416, 103); // size of the radio selector
  lv_obj_set_pos(widget, 0, 35);
  lv_obj_set_flex_flow(widget, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(widget, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_END,
                        LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_bg_opa(widget, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(widget, 0, LV_PART_MAIN); 
  lv_obj_set_style_border_width(widget, 0, LV_PART_MAIN);
  lv_obj_clear_flag(widget, LV_OBJ_FLAG_SCROLLABLE); /// Flags

  // add widget's internal structure as userdata
  widget_info_t  *widget_info = (widget_info_t *)lv_malloc(sizeof(widget_info_t));
  widget_info->has_icons = true;
  if (icons == NULL) {
    widget_info->has_icons = false;
  }
  
  // add radio buttons with icon and label
  for (int i = 0; i < BTN_COUNT; i++) {
    lv_obj_t *cont = cont_cb_create(widget, i, widget_info->has_icons);
   
    if (icons != NULL) {
      lv_obj_t *img = img_cb_create(cont, icons[i]);
      widget_info->icons[i] = img;
    }
    lv_obj_t *cb = cb_create(cont, cb_click_cb);
    widget_info->btns[i] = cb;
    if (labels != NULL) {
      label_cb_create(cont, labels[i]);
    }
    // special handling for 3rd container when icons are present
    //  if(i==2 && icons != NULL){
    //   lv_obj_set_size(cont, 80, 103); // size of the container
    //   lv_obj_set_layout(cont, LV_LAYOUT_NONE); // free form layout
    //   lv_obj_t *img = widget_info->icons[i]; // get icon
    //   lv_obj_set_pos(img, 36, 12);  // position of icon
    //   lv_obj_t *cb = widget_info->btns[i]; // get checkbox
    //   lv_obj_set_pos(cb, 42, 46); // position of checkbox
    //   lv_obj_t *label = lv_obj_get_child(cont, 2); // get label
    //   lv_obj_set_pos(label, 2, 85); // position of label
    //  }
  }
  
  widget_info->btn_id = RADIO_SELECTOR_BTN_NONE;
  lv_obj_set_user_data(widget, widget_info);

  // embellish radio buttons with a link running through all 3 buttons
  lv_obj_t *cont_link = lv_obj_create(widget);
  lv_obj_set_size(cont_link, 356, 16); // size of the link
  lv_obj_set_style_border_width(cont_link, 0, LV_PART_MAIN);
  if (icons == NULL) {
    lv_obj_set_y(cont_link, -13);
  } else {
    lv_obj_set_y(cont_link, 10);
  }
  lv_obj_set_align(cont_link, LV_ALIGN_CENTER);
  lv_obj_add_flag(cont_link, LV_OBJ_FLAG_IGNORE_LAYOUT); /// Flags
  lv_obj_clear_flag(cont_link, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_bg_color(cont_link, lv_color_hex(VST_COLOR_SCREEN_BG_GREY), LV_PART_MAIN);

  // move connector to the back so radio buttons show up on top
  lv_obj_move_background(cont_link);

  // draw slim line on top
  lv_obj_t *link = lv_obj_create(cont_link);
  lv_obj_set_size(link, 352, 1); // size of the slim link
  lv_obj_align(link, LV_ALIGN_CENTER, -6, 0);
  lv_obj_clear_flag(link, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_radius(link, 0, LV_PART_MAIN);
  lv_obj_set_style_border_color(link, lv_color_hex(0x707070), LV_PART_MAIN);
  lv_obj_set_style_border_width(link, 1, LV_PART_MAIN);

  return widget;
}

bool radio_selector_set_selected_btn_1(lv_obj_t *obj, int8_t btn_id) {
  if (btn_id > BTN_COUNT) {
    return false;
  }

  widget_info_t *winfo = get_widget_info(obj);
  // store btn id to serve external requests
  winfo->btn_id = btn_id;

  if (btn_id == BMS_UNSET) {
    if (winfo->has_icons) {
      clear_icons(winfo);
    }
    clear_btns(winfo);
    return true;
  }

  if (winfo->has_icons) {
    clear_icons(winfo);
    set_icon(winfo, btn_id);
  }

  clear_btns(winfo);
  set_btn(winfo, btn_id);

  return true;
}

// Get the recently clicked radio button by the user
int8_t radio_selector_get_selected_btn_1(lv_obj_t *obj) {
  widget_info_t *winfo = get_widget_info(obj);
  return winfo->btn_id;
}

void radio_selector_set_clickable(lv_obj_t *obj, bool clickable) {
  widget_info_t *winfo = get_widget_info(obj);
  for (int i = 0; i < BTN_COUNT; i++) {
    lv_obj_t *obj = winfo->btns[i];
    if (clickable) {
      lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    } else {
      lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    }
  }
}
