#include "images.h"
#include "screens.h"
#include "stdio.h"
#include "ui.h"

static const char *label_back_str = "Back";
static const char *label_cancel_str = "Cancel";
static lv_obj_t *img_left_arrow, *img_info_icon;
static lv_obj_t *img_close;
static lv_obj_t *label_title;
static lv_obj_t *cont_left_arrow, *cont_info_icon, *cont_close_icon;

static SETTINGS_MODE settings_mode;

static void img_left_arrow_click_cb(lv_event_t *e) {
  // settings_mode = screen_settings_get_mode();
  // ASSIGN_MODE assign_mode = room_assign_get_mode();
  // if ((settings_mode == SETTINGS_MODE_ACTIVATION) && (assign_mode == ASSIGN_MODE_NONE)){
  //   lv_obj_t *screen = scr_home_inactive_get();
  //   lv_screen_load(screen);
  //   lv_event_stop_bubbling(e);
  // }
}

static void img_close_click_cb(lv_event_t *e) {
  settings_mode = screen_settings_get_mode();
  // if time picker is active and in save mode, clear session data and go to active home screen
  if(is_current_page_time_picker() == true && settings_mode == SETTINGS_MODE_SAVE) {
    clear_exit_alert_schedule_sessiondata();
    lv_obj_t *screen = screen_home_active_get();
    lv_screen_load(screen);
    return;
  }  // if not in system info page, then go back to home/settings home screen,
  // else get the parent screen of sys info screen and load it
  else if (is_current_page_system_info() == false) {
    if (settings_mode == SETTINGS_MODE_ACTIVATION) {
      lv_obj_t *screen = scr_home_inactive_get();
      lv_screen_load(screen);
      //  lv_event_stop_bubbling(e);
    } else if (settings_mode == SETTINGS_MODE_SAVE) {
      lv_obj_t *screen = screen_home_active_get();
      lv_screen_load(screen);
    }
  }  else {
    lv_obj_t *screen = get_parent_of_system_info_screen();
    lv_screen_load(screen);
    return;
  }
}

// mainTitle : Main heading
// leftIcon : left side icon. true for left arrow and i icons, false for no icon at all
// returns: screen object
lv_obj_t *tmpl_settings_create_1(lv_obj_t *obj, const char *mainTitle, bool leftIcon) {
  lv_obj_t *screen = lv_obj_create(obj);
  lv_obj_set_size(screen, lv_pct(100), lv_pct(100));
  lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_align(screen, LV_ALIGN_CENTER);
  lv_obj_set_flex_flow(screen, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(screen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_row(screen, 16, LV_PART_MAIN);
  lv_obj_set_style_pad_top(screen, 0, LV_PART_MAIN);
  lv_obj_set_style_bg_color(screen, lv_color_hex(0xF6F6F6), LV_PART_MAIN);
  // lv_obj_clear_flag(screen, LV_OBJ_FLAG_CLICKABLE);

  int header_container_height = 64;
  // add container for mode switcher and settings gear
  lv_obj_t *cont_header = lv_obj_create(screen);
  lv_obj_clear_flag(cont_header, LV_OBJ_FLAG_SCROLLABLE);             /// Flags
  lv_obj_set_size(cont_header, lv_pct(100), header_container_height); /// 1
  lv_obj_set_style_border_width(cont_header, 1, LV_PART_MAIN);
  lv_obj_set_style_border_color(cont_header, lv_color_hex(VST_COLOR_CONTAINER_BORDER),
                                LV_PART_MAIN);
  lv_obj_set_style_border_side(cont_header, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN);
  lv_obj_clear_flag(cont_header, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_style_pad_all(cont_header, 0, LV_PART_MAIN);

  if (leftIcon == true) {
    // add info icon container
    cont_info_icon = lv_obj_create(cont_header);
    lv_obj_add_flag(cont_info_icon, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_clear_flag(cont_info_icon, LV_OBJ_FLAG_SCROLLABLE);            /// Flags
    lv_obj_set_size(cont_info_icon, lv_pct(15), header_container_height); /// 1
    lv_obj_set_style_border_width(cont_info_icon, 0, LV_PART_MAIN);
    lv_obj_align(cont_info_icon, LV_ALIGN_LEFT_MID, 0, 2);
    // lv_obj_set_style_pad_all(cont_left_arrow, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_top(cont_info_icon, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_left(cont_info_icon, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_right(cont_info_icon, 0, LV_PART_MAIN);

    // add info icon symbol
    img_info_icon = lv_img_create(cont_info_icon);
    lv_img_set_src(img_info_icon, &icon_info);
    lv_obj_set_size(img_info_icon, LV_SIZE_CONTENT, LV_SIZE_CONTENT); /// 1
    lv_obj_add_flag(img_info_icon, LV_OBJ_FLAG_EVENT_BUBBLE);         /// Flags
    lv_obj_clear_flag(img_info_icon, LV_OBJ_FLAG_SCROLLABLE);         /// Flags
    lv_obj_set_style_img_recolor(img_info_icon, lv_color_hex(0x999999),
                                 LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_img_recolor_opa(img_info_icon, 255, LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_align(img_info_icon, LV_ALIGN_BOTTOM_MID);
    lv_obj_add_flag(img_info_icon, LV_OBJ_FLAG_CLICKABLE);

    // add left arrow container
    cont_left_arrow = lv_obj_create(cont_header);
    lv_obj_add_flag(cont_left_arrow, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_clear_flag(cont_left_arrow, LV_OBJ_FLAG_SCROLLABLE);            /// Flags
    lv_obj_set_size(cont_left_arrow, lv_pct(15), header_container_height); /// 1
    lv_obj_set_style_border_width(cont_left_arrow, 0, LV_PART_MAIN);
    lv_obj_set_align(cont_left_arrow, LV_ALIGN_LEFT_MID);
    // lv_obj_set_style_pad_all(cont_left_arrow, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_top(cont_left_arrow, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_left(cont_left_arrow, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_right(cont_left_arrow, 0, LV_PART_MAIN);
    lv_obj_add_flag(cont_left_arrow, LV_OBJ_FLAG_HIDDEN);

    // Replace back label with left arrow
    img_left_arrow = lv_img_create(cont_left_arrow);
    lv_img_set_src(img_left_arrow, &icon_left_arrow);
    lv_obj_set_size(img_left_arrow, LV_SIZE_CONTENT, LV_SIZE_CONTENT); /// 1
    lv_obj_add_flag(img_left_arrow, LV_OBJ_FLAG_EVENT_BUBBLE);         /// Flags
    lv_obj_clear_flag(img_left_arrow, LV_OBJ_FLAG_SCROLLABLE);         /// Flags
    lv_obj_set_style_img_recolor(img_left_arrow, lv_color_hex(0x999999),
                                 LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_img_recolor_opa(img_left_arrow, 255, LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_align(img_left_arrow, LV_ALIGN_BOTTOM_MID, 0, -2);
    lv_obj_add_flag(img_left_arrow, LV_OBJ_FLAG_CLICKABLE);
  }
  // add title container
  // lv_obj_t *cont_title = lv_obj_create(cont_header);
  // lv_obj_clear_flag(cont_title, LV_OBJ_FLAG_SCROLLABLE);         /// Flags
  // lv_obj_set_size(cont_title, LV_SIZE_CONTENT, LV_SIZE_CONTENT); /// 1
  // lv_obj_set_style_border_width(cont_title, 0, LV_PART_MAIN);
  // lv_obj_set_align(cont_title, LV_ALIGN_CENTER);
  //  lv_obj_clear_flag(cont_title, LV_OBJ_FLAG_CLICKABLE);
  // lv_obj_set_style_pad_left(cont_title, 0, LV_PART_MAIN);

  // add settings title
  label_title = lv_label_create(cont_header);
  lv_obj_set_size(label_title, LV_SIZE_CONTENT, LV_SIZE_CONTENT); /// 1
  lv_label_set_text(label_title, mainTitle);
  lv_obj_align(label_title, LV_ALIGN_CENTER, 0,
               3); // pushed down a bit to optically center it with the icons
  lv_obj_set_style_text_font(label_title, &opensans_bold_20, LV_PART_MAIN);
  lv_obj_set_style_text_color(label_title, lv_color_hex(VST_COLOR_PRIMARY_TEXT),
                              LV_PART_MAIN); // text color

  // lv_obj_set_ext_click_area(img_left_arrow, 30);
  // lv_image_set_scale(img_left_arrow, 510);

  // add left arrow container
  cont_close_icon = lv_obj_create(cont_header);
  lv_obj_add_flag(cont_close_icon, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_clear_flag(cont_close_icon, LV_OBJ_FLAG_SCROLLABLE);            /// Flags
  lv_obj_set_size(cont_close_icon, lv_pct(15), header_container_height); /// 1
  lv_obj_set_style_border_width(cont_close_icon, 0, LV_PART_MAIN);
  lv_obj_set_align(cont_close_icon, LV_ALIGN_RIGHT_MID);
  // lv_obj_set_style_pad_all(cont_close_icon, 0, LV_PART_MAIN);
  // lv_obj_set_style_pad_top(cont_close_icon, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_top(cont_close_icon, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_left(cont_close_icon, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_right(cont_close_icon, 0, LV_PART_MAIN);

  // Add Close "X" button
  img_close = lv_img_create(cont_close_icon);
  lv_img_set_src(img_close, &icon_close);
  lv_obj_set_size(img_close, LV_SIZE_CONTENT, LV_SIZE_CONTENT); /// 1
  lv_obj_add_flag(img_close, LV_OBJ_FLAG_EVENT_BUBBLE);         /// Flags
  lv_obj_clear_flag(img_close, LV_OBJ_FLAG_SCROLLABLE);         /// Flags
  lv_obj_set_style_img_recolor(img_close, lv_color_hex(0x999999), LV_PART_MAIN | LV_STATE_DISABLED);
  lv_obj_set_style_img_recolor_opa(img_close, 255, LV_PART_MAIN | LV_STATE_DISABLED);
  lv_obj_align(img_close, LV_ALIGN_CENTER, 0, 10);
  lv_obj_add_flag(img_close, LV_OBJ_FLAG_CLICKABLE);
  // lv_obj_set_ext_click_area(img_close, 30);
  // lv_image_set_scale(img_close, 510);

  // if (subTitle != NULL) {
  //   // add sub title container
  //   lv_obj_t *cont_subtitle = lv_obj_create(screen);
  //   lv_obj_set_size(cont_subtitle, lv_pct(100), LV_SIZE_CONTENT);
  //   lv_obj_set_style_radius(cont_subtitle, 0, LV_PART_MAIN);
  //   lv_obj_clear_flag(cont_subtitle, LV_OBJ_FLAG_SCROLLABLE); /// Flags

  //   // add sub title
  //   lv_obj_t *label_subtitle = lv_label_create(cont_subtitle);
  //   lv_obj_set_size(label_subtitle, LV_SIZE_CONTENT, LV_SIZE_CONTENT); /// 1
  //   lv_obj_set_align(label_subtitle, LV_ALIGN_CENTER);
  //   lv_label_set_text_static(label_subtitle, subTitle);
  //   lv_obj_set_style_text_font(label_subtitle, &opensans_semibold_20, LV_PART_MAIN);
  // }

  // lv_obj_add_event_cb(cont_close_icon, img_close_click_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(cont_close_icon, img_close_click_cb, LV_EVENT_CLICKED, NULL);
  if (leftIcon == true) {
    lv_obj_add_event_cb(cont_left_arrow, img_left_arrow_click_cb, LV_EVENT_CLICKED, NULL);
  }

  return screen;
}

void tmpl_settings_show_label_cancel(lv_obj_t *obj, bool show) {
  // lv_obj_t *label_back = (lv_obj_t *)lv_obj_get_user_data(obj);
  // if (show) {
  //   lv_label_set_text_static(label_back, label_cancel_str);
  // } else {
  //   lv_label_set_text_static(label_back, label_back_str);
  // }
}

void tmpl_settings_label_set_clickable(lv_obj_t *obj, bool clickable) {
  // if (clickable) {
  //   lv_obj_add_flag(img_close, LV_OBJ_FLAG_CLICKABLE);
  //   lv_obj_add_flag(img_left_arrow, LV_OBJ_FLAG_CLICKABLE);
  // } else {
  //   lv_obj_clear_flag(img_close, LV_OBJ_FLAG_CLICKABLE);
  //   lv_obj_clear_flag(img_left_arrow, LV_OBJ_FLAG_CLICKABLE);
  // }
}

lv_obj_t *get_img_left_arrow_obj() { return img_left_arrow; }

lv_obj_t *get_cont_left_arrow_obj() { return cont_left_arrow; }

lv_obj_t *get_img_info_icon_obj() { return img_info_icon; }

lv_obj_t *get_cont_info_icon_obj() { return cont_info_icon; }

lv_obj_t *get_cont_close_icon_obj() { return cont_close_icon; }

lv_obj_t *get_header_label_obj() { return label_title; }

// common functions
// todo: move somewhere appropriate
