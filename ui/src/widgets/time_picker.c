/**
 * @file time_picker.c
 * @brief Template source file for LVGL objects
 */

/*********************
 * INCLUDES
 *********************/

#include "exit_alert_schedule.h"
#include <string.h>

/*********************
 * DEFINES
 *********************/

/**********************
 * TYPEDEFS
 **********************/

/***********************
 * STATIC VARIABLES
 **********************/

/***********************
 * STATIC PROTOTYPES
 **********************/

/**********************
 * GLOBAL FUNCTIONS
 **********************/

lv_obj_t *btn_am_up, *btn_pm_down;
lv_obj_t *label_time_picker_title;


// Function to update the time picker title based on context
void update_time_picker_title(const char *title)
{
    lv_label_set_text(label_time_picker_title, title);
    
}

lv_obj_t * column_create(lv_obj_t * parent)
{
    LV_TRACE_OBJ_CREATE("begin");

    static lv_style_t style_base;

    static bool style_inited = false;

    if (!style_inited) {
        lv_style_init(&style_base);
        lv_style_set_width(&style_base, LV_SIZE_CONTENT);
        lv_style_set_height(&style_base, LV_SIZE_CONTENT);
        lv_style_set_layout(&style_base, LV_LAYOUT_FLEX);
        lv_style_set_flex_flow(&style_base, LV_FLEX_FLOW_COLUMN);
        lv_style_set_border_width(&style_base, 2);

        style_inited = true;
    }

    lv_obj_t * lv_obj_0 = lv_obj_create(parent);

    lv_obj_remove_style_all(lv_obj_0);
    lv_obj_add_style(lv_obj_0, &style_base, 0);

    LV_TRACE_OBJ_CREATE("finished");

    lv_obj_set_name(lv_obj_0, "column_#");

    return lv_obj_0;
}


/**
 * @brief Observer to manage the disabled state of the AM/PM buttons based on the current time_half
 * subject value. If the current value is "AM", the AM button (btn_am_up) is disabled.
 */
static void am_pm_change_observer(lv_observer_t *observer, lv_subject_t *subject) {
  // Fetch the string directly from the subject
  const char *current_am_pm = lv_subject_get_string(subject);

  // If the current time is AM, disable the AM button (btn_am_up) and enable the PM button
  // (btn_pm_down)
  if (strcmp(current_am_pm, "AM") == 0) {
    lv_obj_add_state(btn_am_up, LV_STATE_DISABLED);
    lv_obj_clear_state(btn_pm_down, LV_STATE_DISABLED);
  }
  // If the current time is PM, enable the AM button (btn_am_up) and disable the PM button
  // (btn_pm_down)
  else {
    lv_obj_clear_state(btn_am_up, LV_STATE_DISABLED);
    lv_obj_add_state(btn_pm_down, LV_STATE_DISABLED);
  }
}

static void create_time_picker_column(lv_obj_t *parent, lv_subject_t *subject,
                                      const lv_img_dsc_t *up_img, const lv_img_dsc_t *down_img,
                                      lv_style_t *disp_style) {
  // UP Button
  lv_obj_t *btn_up = lv_button_create(parent);
  lv_obj_set_style_bg_color(btn_up, lv_color_hex(0xffffff), 0);
  lv_obj_set_style_pad_all(btn_up, 0, 0);
  lv_obj_t *img_up = lv_image_create(btn_up);
  lv_image_set_src(img_up, up_img);
  lv_subject_increment_dsc_t *desc_up =
      lv_obj_add_subject_increment_event(btn_up, subject, LV_EVENT_CLICKED, 1);
  lv_obj_set_subject_increment_event_rollover(btn_up, desc_up, true);

  // Display Button
  lv_obj_t *btn_disp = lv_button_create(parent);
  lv_obj_remove_style_all(btn_disp);
  lv_obj_add_style(btn_disp, disp_style, 0);
  lv_obj_t *label = lv_label_create(btn_disp);
  lv_label_bind_text(label, subject, "%02d"); // Hardcoded format
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 3);

  // DOWN Button
  lv_obj_t *btn_down = lv_button_create(parent);
  lv_obj_set_style_bg_color(btn_down, lv_color_hex(0xffffff), 0);
  lv_obj_set_style_pad_all(btn_down, 0, 0);
  lv_obj_t *img_down = lv_image_create(btn_down);
  lv_image_set_src(img_down, down_img);
  lv_subject_increment_dsc_t *desc_down =
      lv_obj_add_subject_increment_event(btn_down, subject, LV_EVENT_CLICKED, -1);
  lv_obj_set_subject_increment_event_rollover(btn_down, desc_down, true);
}

lv_obj_t *cont_schedule_time_create(lv_obj_t *parent, const char *title) {
  LV_TRACE_OBJ_CREATE("begin");

  static lv_style_t style_main_container;
  static lv_style_t style_time_disp_btn;

  static bool style_inited = false;

  if (!style_inited) {
    // --- Main Container Style Init ---
    lv_style_init(&style_main_container);
    lv_style_set_bg_color(&style_main_container, lv_color_hex(0xffffff));
    lv_style_set_border_color(&style_main_container, lv_color_hex(0xd9d9d9));
    lv_style_set_border_opa(&style_main_container, 255);
    lv_style_set_border_width(&style_main_container, 1);
    lv_style_set_width(&style_main_container, 448);
    lv_style_set_height(&style_main_container, 291);
    lv_style_set_radius(&style_main_container, 10);
    lv_style_set_shadow_width(&style_main_container, 6);
    lv_style_set_shadow_color(&style_main_container, lv_color_hex(0x000000));
    lv_style_set_shadow_opa(&style_main_container, 64);
    lv_style_set_shadow_offset_x(&style_main_container, 0);
    lv_style_set_shadow_offset_y(&style_main_container, 3);
    lv_style_set_pad_all(&style_main_container, 16);
    lv_style_set_pad_gap(&style_main_container, 16);

    // --- Time Display Button Style Init ---
    lv_style_init(&style_time_disp_btn);
    lv_style_set_bg_color(&style_time_disp_btn, lv_color_hex(0xfafafa));
    lv_style_set_border_color(&style_time_disp_btn, lv_color_hex(0xd1d1d1));
    lv_style_set_border_opa(&style_time_disp_btn, 255);
    lv_style_set_border_width(&style_time_disp_btn, 3);
    lv_style_set_width(&style_time_disp_btn, 120);
    lv_style_set_height(&style_time_disp_btn, 72);
    lv_style_set_radius(&style_time_disp_btn, 5);
    lv_style_set_pad_hor(&style_time_disp_btn, 41);
    lv_style_set_pad_ver(&style_time_disp_btn, 34);
    lv_style_set_text_font(&style_time_disp_btn, &opensans_semibold_36);

    style_inited = true;
  }

  lv_subject_add_observer(&hour, time_change_observer, NULL);
  lv_subject_add_observer(&minute, time_change_observer, NULL);
  lv_subject_add_observer(&time_half, time_change_observer, NULL);
  // --- Main Container (lv_obj_0) ---
  lv_obj_t *lv_obj_0 = lv_obj_create(parent);
  // lv_obj_set_flex_flow(lv_obj_0, LV_FLEX_FLOW_COLUMN);
  lv_obj_add_style(lv_obj_0, &style_main_container, 0);
  lv_obj_clear_flag(lv_obj_0, LV_OBJ_FLAG_SCROLLABLE);

  // Title Label
  label_time_picker_title = lv_label_create(lv_obj_0);
  lv_obj_set_y(label_time_picker_title, 4);
  lv_label_set_text(label_time_picker_title, title);
  lv_obj_set_style_text_font(label_time_picker_title, &opensans_semibold_18, 0);

  // --- Wrapper Container (lv_obj_1) ---
  lv_obj_t *lv_obj_1 = lv_obj_create(lv_obj_0);
  lv_obj_set_style_pad_all(lv_obj_1, 0, 0);
  lv_obj_set_height(lv_obj_1, 223);
  lv_obj_set_width(lv_obj_1, 416);
  lv_obj_set_y(lv_obj_1, 35);
  lv_obj_set_style_border_width(lv_obj_1, 0, 0);
  lv_obj_set_flex_flow(lv_obj_1, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flag(lv_obj_1, LV_OBJ_FLAG_SCROLLABLE, false);

  // --- Picker Row (cont_main) ---
  // lv_obj_t *cont_main = row_create(lv_obj_1);
  lv_obj_t *cont_main = lv_obj_create(lv_obj_1);
  lv_obj_set_style_pad_all(cont_main, 0, 0);
  lv_obj_set_height(cont_main, LV_SIZE_CONTENT);
  lv_obj_set_width(cont_main, lv_pct(100));
  lv_obj_set_style_border_width(cont_main, 0, 0);
  lv_obj_set_flex_flow(cont_main, LV_FLEX_FLOW_ROW);

  lv_obj_set_style_pad_column(cont_main, 27, 0);

  // --- HOUR Column (column_0) ---
  // --- HOUR Column (column_0) ---
  lv_obj_t *column_0 = column_create(cont_main);
  lv_obj_set_style_border_width(column_0, 0, 0);
  lv_obj_set_style_pad_row(column_0, 11, 0);
  create_time_picker_column(column_0, &hour, image_up_arrow, image_down_arrow,
                            &style_time_disp_btn);

  // Create the colon container
  lv_obj_t *cont_colon = lv_obj_create(lv_obj_0);
  lv_obj_set_size(cont_colon, 20, 40);
  lv_obj_remove_style_all(cont_colon); // Remove padding/background/border
  lv_obj_clear_flag(cont_colon, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *img_colon = lv_img_create(cont_colon);
  lv_img_set_src(img_colon, &icon_two_dots);
  lv_obj_add_flag(cont_colon, LV_OBJ_FLAG_FLOATING);
  lv_obj_set_pos(cont_colon, 130, 130);
  lv_obj_clear_flag(cont_colon, LV_OBJ_FLAG_CLICKABLE);

  // --- MINUTE Column (column_1) ---
  lv_obj_t *column_1 = column_create(cont_main);
  lv_obj_set_style_border_width(column_1, 0, 0);
  lv_obj_set_style_pad_row(column_1, 11, 0);
  create_time_picker_column(column_1, &minute, image_up_arrow, image_down_arrow,
                            &style_time_disp_btn);
  // --- AM/PM Column (column_2) ---
  lv_obj_t *column_2 = column_create(cont_main);
  lv_obj_set_style_border_width(column_2, 0, 0);
  lv_obj_set_style_pad_row(column_2, 11, 0);

  // AM UP Button (btn_am_up)
  btn_am_up = lv_button_create(column_2);
  lv_obj_set_style_bg_color(btn_am_up, lv_color_hex(0xffffff), 0);
  lv_obj_set_style_pad_all(btn_am_up, 0, 0);
  lv_obj_t *lv_image_4 = lv_image_create(btn_am_up);
  lv_image_set_src(lv_image_4, image_up_arrow);
  // Initial state set here, but updated by the observer upon screen load
  lv_obj_add_state(btn_am_up, LV_STATE_DISABLED);
  lv_obj_add_subject_set_string_event(btn_am_up, &time_half, LV_EVENT_CLICKED, "AM");

  // AM/PM Display (lv_button_6)
  lv_obj_t *lv_button_6 = lv_button_create(column_2);
  lv_obj_remove_style_all(lv_button_6);
  lv_obj_add_style(lv_button_6, &style_time_disp_btn, 0);
  lv_obj_t *lv_label_3 = lv_label_create(lv_button_6);
  lv_label_bind_text(lv_label_3, &time_half, NULL);
  lv_obj_align(lv_label_3, LV_ALIGN_CENTER, 0, 3);

  // PM DOWN Button (btn_pm_down)
  btn_pm_down = lv_button_create(column_2);
  lv_obj_set_style_bg_color(btn_pm_down, lv_color_hex(0xffffff), 0);
  lv_obj_set_style_pad_all(btn_pm_down, 0, 0);
  lv_obj_t *lv_image_5 = lv_image_create(btn_pm_down);
  lv_image_set_src(lv_image_5, image_down_arrow);
  // Initial state set here, but updated by the observer upon screen load
  lv_obj_add_state(btn_pm_down, LV_STATE_DISABLED);
  lv_obj_add_subject_set_string_event(btn_pm_down, &time_half, LV_EVENT_CLICKED, "PM");

  // --- Final Observer Registration ---
  lv_subject_add_observer(&time_half, am_pm_change_observer, NULL);

  LV_TRACE_OBJ_CREATE("finished");

  lv_obj_set_name(lv_obj_0, "cont_schedule_time");

  return lv_obj_0;
}

/**********************
 * STATIC FUNCTIONS
 **********************/