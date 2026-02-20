/**
 * @file bedwidth_value_settings_gen.c
 * @brief Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/

#include "fonts.h"
#include "ui.h"
#include <string.h> /* for strcmp */
#include <stdlib.h>

lv_obj_t *screen = NULL; /* root screen object */

/* Image assets for backspace and OK icons (white PNGs with alpha) */
LV_IMG_DECLARE(backspace_symbol);
LV_IMG_DECLARE(ok_symbol);

/***********************
 *  STATIC PROTOTYPES
 **********************/
static void bedwidth_keypad_event_cb(lv_event_t *e);
static void bedwidth_textarea_event_cb(lv_event_t *e);

/* helper to create one keypad button in the grid */
static lv_obj_t *create_keypad_button(lv_obj_t *parent, const char *txt, uint8_t col, uint8_t row,
                                      lv_obj_t *textarea, bool is_symbol, const void *img_src);

/**********************
 *   STATIC STATE
 **********************/

/* Shared styles for keypad buttons */
static lv_style_t btn_style;         /* normal: purple filled for digits */
static lv_style_t btn_style_pressed; /* pressed: darker purple (digits only) */
lv_obj_t *lv_textarea_0;             // textarea object

static lv_style_t style_container; /* card/container style */
static lv_style_t style_inch_obj;  /* "in" box style */
static lv_style_t style_textarea;  /* textarea base style */

static bool style_inited = false;

/* Symbol buttons + their images, so we can recolor them */
static lv_obj_t *g_backspace_btn = NULL;
static lv_obj_t *g_ok_btn = NULL;
static lv_obj_t *g_backspace_img = NULL;
static lv_obj_t *g_ok_img = NULL;

/**********************
 *   HELPERS
 **********************/
// Close icon click callback → just reset the selection flag
static void icon_close_click_cb(lv_event_t *e) {
  set_bed_width_selection_flag(false);
}

// Timer callback to reset textarea border after toast
static void toast_timer_cb(lv_timer_t *timer) {
  lv_obj_set_style_border_color(lv_textarea_0, lv_color_hex(0x707070), LV_PART_MAIN);
  lv_obj_set_style_border_width(lv_textarea_0, 1, LV_PART_MAIN);
  lv_obj_set_style_text_color(lv_textarea_0, lv_color_hex(VST_COLOR_ACTIVATE_SENSOR_BLUE), 0);
  lv_timer_del(timer);
}

static void update_symbol_appearance(bool enabled) {
  if (g_backspace_btn) {
    lv_obj_set_style_bg_opa(g_backspace_btn, 82, LV_PART_MAIN);
  }

  if (g_ok_btn) {
    lv_obj_set_style_bg_opa(g_ok_btn, 82, LV_PART_MAIN);
  }
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t *bedwidth_value_settings_create(void) {
  LV_TRACE_OBJ_CREATE("begin");

  if (!style_inited) {
    /******************************************
     * KEYPAD BUTTON STYLES (shared)
     ******************************************/
    lv_style_init(&btn_style);
    lv_style_set_width(&btn_style, 98);
    lv_style_set_height(&btn_style, 72);
    lv_style_set_radius(&btn_style, 8);

    /* Default digit look: purple fill, white text, purple border */
    lv_style_set_bg_color(&btn_style, lv_color_hex(0x1A1040));
    lv_style_set_bg_opa(&btn_style, LV_OPA_COVER);
    lv_style_set_border_color(&btn_style, lv_color_hex(0x1A1040));
    lv_style_set_border_opa(&btn_style, LV_OPA_COVER);
    lv_style_set_border_width(&btn_style, 4);
    lv_style_set_text_color(&btn_style, lv_color_hex(0xFFFFFF));
    lv_style_set_shadow_width(&btn_style, 0);
    lv_style_set_shadow_offset_x(&btn_style, 0);
    lv_style_set_shadow_offset_y(&btn_style, 0);

    /* Pressed look: slightly darker purple (digits only) */
    lv_style_init(&btn_style_pressed);
    lv_style_set_bg_color(&btn_style_pressed, lv_color_hex(0x150C36));
    lv_style_set_bg_opa(&btn_style_pressed, LV_OPA_COVER);

    /******************************************
     * Container card style
     ******************************************/
    lv_style_init(&style_container);
    lv_style_set_bg_color(&style_container, lv_color_hex(0xffffff));
    lv_style_set_border_color(&style_container, lv_color_hex(0xd9d9d9));
    lv_style_set_border_opa(&style_container, 255);
    lv_style_set_border_width(&style_container, 1);
    lv_style_set_width(&style_container, 448);
    lv_style_set_height(&style_container, 366);
    lv_style_set_radius(&style_container, 10);
    lv_style_set_shadow_width(&style_container, 6);
    lv_style_set_shadow_color(&style_container, lv_color_hex(0x000000));
    lv_style_set_shadow_opa(&style_container, 64);
    lv_style_set_shadow_offset_x(&style_container, 0);
    lv_style_set_shadow_offset_y(&style_container, 3);
    lv_style_set_pad_all(&style_container, 16);

    /******************************************
     * "in" box style
     ******************************************/
    lv_style_init(&style_inch_obj);
    lv_style_set_bg_color(&style_inch_obj, lv_color_hex(VST_COLOR_CONTAINER_BORDER));
    lv_style_set_bg_opa(&style_inch_obj, LV_OPA_COVER);
    lv_style_set_radius(&style_inch_obj, 0);
   
    lv_style_set_border_color(&style_inch_obj, lv_color_hex(0x707070));
    lv_style_set_border_opa(&style_inch_obj, 255);
    
    lv_style_set_width(&style_inch_obj, 30);
    lv_style_set_height(&style_inch_obj, 50);
    lv_style_set_pad_all(&style_inch_obj, 8);

    /******************************************
     * Text area base style
     ******************************************/
    lv_style_init(&style_textarea);
    lv_style_set_bg_color(&style_textarea, lv_color_hex(0xFFFFFF));
    lv_style_set_bg_opa(&style_textarea, LV_OPA_COVER);
    lv_style_set_radius(&style_textarea, 10);
    lv_style_set_border_color(&style_textarea, lv_color_hex(0xD9D9D9)); /* light grey */
    lv_style_set_border_opa(&style_textarea, LV_OPA_COVER);
    lv_style_set_border_width(&style_textarea, 1);

    style_inited = true;
  }

  /* Root object (screen) */
  // lv_obj_t *lv_obj_0 = lv_obj_create(NULL);
  // lv_obj_set_name_static(lv_obj_0, "bedwidth_value_settings_#");

  /* Container card */
  lv_obj_t *lv_obj_1 = lv_obj_create(screen);
  lv_obj_set_flag(lv_obj_1, LV_OBJ_FLAG_SCROLLABLE, false);
  lv_obj_set_align(lv_obj_1, LV_ALIGN_TOP_MID);
  lv_obj_set_y(lv_obj_1, 62);
  lv_obj_add_style(lv_obj_1, &style_container, 0);

  /* Title */
  lv_obj_t *lv_label_0 = lv_label_create(lv_obj_1);
  lv_label_set_text(lv_label_0, "Enter Custom Bed Width Value");
  lv_obj_set_style_text_color(lv_label_0, lv_color_hex(0x31344e), 0);
  lv_obj_set_width(lv_label_0, 416);
  lv_obj_set_height(lv_label_0, 20);
  lv_obj_set_style_text_font(lv_label_0, &opensans_semibold_18, 0);

  /******************************************************
   * TEXTAREA + CUSTOM PLACEHOLDER + "in" BOX
   ******************************************************/
  lv_textarea_0 = lv_textarea_create(lv_obj_1);
  lv_obj_set_y(lv_textarea_0, 34);
  lv_textarea_set_text(lv_textarea_0, "");

  /* Base style for both states; we'll tweak border color in VALUE_CHANGED */
  lv_obj_add_style(lv_textarea_0, &style_textarea, 0);
  lv_obj_set_style_bg_color(lv_textarea_0, lv_color_hex(VST_COLOR_SCREEN_BG_GREY), 0);

  lv_obj_set_width(lv_textarea_0, 416);
  lv_obj_set_height(lv_textarea_0, 50);
  lv_obj_set_style_radius(lv_textarea_0, 10, 0);
  lv_obj_set_flag(lv_textarea_0, LV_OBJ_FLAG_SCROLLABLE, false);

  /* Padding for vertical centering and left spacing */
  lv_obj_set_style_pad_left(lv_textarea_0, 16, 0);
  lv_obj_set_style_pad_right(lv_textarea_0, 0, 0); /* no right pad so "in" touches end */
  lv_obj_set_style_pad_top(lv_textarea_0, 15, 0);
  lv_obj_set_style_pad_bottom(lv_textarea_0, 15, 0);

  /* Text style */
  lv_obj_set_style_text_font(lv_textarea_0, &opensans_regular_16, 0);
  lv_obj_set_style_text_color(lv_textarea_0, lv_color_hex(VST_COLOR_ACTIVATE_SENSOR_BLUE), 0);

  /* Custom placeholder label (grey + italic, same position as text) */
  lv_obj_t *lv_label_1 = lv_label_create(lv_textarea_0);
  lv_obj_set_flag(lv_label_1, LV_OBJ_FLAG_SCROLLABLE, false);
  lv_label_set_text(lv_label_1, "Enter Bed Width");
  lv_obj_set_style_text_color(lv_label_1, lv_color_hex(0x787b91), 0);
  lv_obj_set_style_text_font(lv_label_1, &opensans_italic_16, 0);
  lv_obj_set_align(lv_label_1, LV_ALIGN_LEFT_MID);

  /* show/hide placeholder + toggle symbol buttons & textarea border based on text */
  lv_obj_add_event_cb(lv_textarea_0, bedwidth_textarea_event_cb, LV_EVENT_VALUE_CHANGED,
                      lv_label_1);

  /* "in" box inside textarea */
  
  lv_obj_t *lv_obj_2 = lv_obj_create(lv_textarea_0);
  lv_obj_remove_style_all(lv_obj_2);
  lv_obj_align(lv_obj_2, LV_ALIGN_RIGHT_MID, 0, 0);
  lv_obj_set_flag(lv_obj_2, LV_OBJ_FLAG_SCROLLABLE, false);
  lv_obj_add_style(lv_obj_2, &style_inch_obj, 0);
  lv_obj_set_style_border_width(lv_obj_2, 0, 0);
  lv_obj_set_style_radius(lv_obj_2, 7, 0);
  lv_obj_t *lv_label_2 = lv_label_create(lv_obj_2);
  lv_obj_set_flag(lv_label_2, LV_OBJ_FLAG_SCROLLABLE, false);
  lv_label_set_text(lv_label_2, "in");
  lv_obj_set_style_text_color(lv_label_2, lv_color_hex(VST_COLOR_INACTIVE_TEXT), 0);
  lv_obj_set_style_text_font(lv_label_2, &opensans_regular_16, LV_PART_MAIN);
  lv_obj_align(lv_label_2, LV_ALIGN_CENTER, 0, 1); /* slight down to optically center */

  /******************************************
   * Keypad: individual buttons in a grid
   ******************************************/

  static int32_t col_dsc[] = {98, 98, 98, 98, LV_GRID_TEMPLATE_LAST};
  static int32_t row_dsc[] = {72, 72, 72, LV_GRID_TEMPLATE_LAST};

  lv_obj_t *keypad = lv_obj_create(lv_obj_1);
  lv_obj_remove_style_all(keypad);
  lv_obj_set_size(keypad, 416, 232);
  lv_obj_set_align(keypad, LV_ALIGN_BOTTOM_MID);

  lv_obj_set_layout(keypad, LV_LAYOUT_GRID);
  lv_obj_set_grid_dsc_array(keypad, col_dsc, row_dsc);

  lv_obj_set_style_pad_row(keypad, 8, 0);
  lv_obj_set_style_pad_column(keypad, 8, 0);
  lv_obj_set_style_pad_all(keypad, 0, 0);

  /* Row 0 */
  create_keypad_button(keypad, "1", 0, 0, lv_textarea_0, false, NULL);
  create_keypad_button(keypad, "2", 1, 0, lv_textarea_0, false, NULL);
  create_keypad_button(keypad, "3", 2, 0, lv_textarea_0, false, NULL);
  create_keypad_button(keypad, "4", 3, 0, lv_textarea_0, false, NULL);

  /* Row 1 */
  create_keypad_button(keypad, "5", 0, 1, lv_textarea_0, false, NULL);
  create_keypad_button(keypad, "6", 1, 1, lv_textarea_0, false, NULL);
  create_keypad_button(keypad, "7", 2, 1, lv_textarea_0, false, NULL);
  create_keypad_button(keypad, "8", 3, 1, lv_textarea_0, false, NULL);

  /* Row 2 – symbols use images instead of text */
  g_backspace_btn = create_keypad_button(keypad, "", 0, 2, lv_textarea_0, true, &backspace_symbol);
  create_keypad_button(keypad, "9", 1, 2, lv_textarea_0, false, NULL);
  create_keypad_button(keypad, "0", 2, 2, lv_textarea_0, false, NULL);
  g_ok_btn = create_keypad_button(keypad, "", 3, 2, lv_textarea_0, true, &ok_symbol);

  /* Initially disabled (no text) -> semi-opa bg + grey icon, and ignore clicks */
  if (g_backspace_btn)
    lv_obj_add_state(g_backspace_btn, LV_STATE_DISABLED);
  if (g_ok_btn)
    lv_obj_add_state(g_ok_btn, LV_STATE_DISABLED);
  update_symbol_appearance(false);

  LV_TRACE_OBJ_CREATE("finished");
  return screen;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/* Show/hide placeholder label and enable/disable backspace/OK */
static void bedwidth_textarea_event_cb(lv_event_t *e) {
  lv_obj_t *ta = lv_event_get_target(e);
  lv_obj_t *ph = lv_event_get_user_data(e);

  const char *txt = lv_textarea_get_text(ta);
  bool has_text = (txt && txt[0] != '\0');

  /* Placeholder visibility */
  if (has_text) {
    lv_obj_add_flag(ph, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_clear_flag(ph, LV_OBJ_FLAG_HIDDEN);
  }

  /* Textarea border color should change only when text changes (i.e., after pressing digits) */
  if (has_text) {
    lv_obj_set_style_border_color(ta, lv_color_hex(0x31344E), 0); /* dark when there is text */
  } else {
    lv_obj_set_style_border_color(ta, lv_color_hex(0xD9D9D9), 0); /* light when empty */
  }

  /* Toggle backspace & OK states (for interaction) + appearance */
  if (g_backspace_btn) {
    if (has_text)
      lv_obj_clear_state(g_backspace_btn, LV_STATE_DISABLED);
    else
      lv_obj_add_state(g_backspace_btn, LV_STATE_DISABLED);
  }

  if (g_ok_btn) {
    if (has_text)
      lv_obj_clear_state(g_ok_btn, LV_STATE_DISABLED);
    else
      lv_obj_add_state(g_ok_btn, LV_STATE_DISABLED);
  }

  update_symbol_appearance(has_text);
}

/* Helper to create an individual keypad button */
static lv_obj_t *create_keypad_button(lv_obj_t *parent, const char *txt, uint8_t col, uint8_t row,
                                      lv_obj_t *textarea, bool is_symbol, const void *img_src) {
  lv_obj_t *btn = lv_btn_create(parent);

  /* Shared base style for size, radius, border, etc. */
  lv_obj_add_style(btn, &btn_style, 0);

  /* Pressed style only for digit buttons (symbol bg stays white) */
  if (!is_symbol) {
    lv_obj_add_style(btn, &btn_style_pressed, LV_STATE_PRESSED);
  }

  lv_obj_set_grid_cell(btn, LV_GRID_ALIGN_STRETCH, col, 1, LV_GRID_ALIGN_STRETCH, row, 1);

  lv_obj_add_event_cb(btn, bedwidth_keypad_event_cb, LV_EVENT_CLICKED, textarea);

  if (is_symbol && img_src) {
    /* Symbol buttons: white background ALWAYS; opa adjusted in update_symbol_appearance */
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xFFFFFF), 0);
    // lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);

    /* Create icon */
    lv_obj_t *img = lv_img_create(btn);
    lv_img_set_src(img, img_src);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);

    /* Store pointers so we can recolor later */
    if (img_src == &backspace_symbol) {
      g_backspace_img = img;
    } else if (img_src == &ok_symbol) {
      g_ok_img = img;
    }
  } else {
    /* Digit buttons: label with OpenSans */
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, txt);
    lv_obj_set_style_text_font(label, &opensans_bold_20, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
  }

  return btn;
}

/* Keypad event: digits → textarea, handle backspace / OK */
static void bedwidth_keypad_event_cb(lv_event_t *e) {
  if (lv_event_get_code(e) != LV_EVENT_CLICKED)
    return;

  lv_obj_t *btn = lv_event_get_target(e);

  /* Ignore presses when button is disabled (no value yet) */
  if (lv_obj_has_state(btn, LV_STATE_DISABLED))
    return;

  lv_obj_t *ta = lv_event_get_user_data(e);

  /* Symbol buttons: identified by pointer */
  if (btn == g_backspace_btn) {
    lv_textarea_delete_char(ta);
    return;
  }

  if (btn == g_ok_btn) {
    /* TODO: handle save/confirm */
    // Validate value range: 44-80 inches
    char custom_width[10] = {0};
    strncpy(custom_width, lv_textarea_get_text(ta), sizeof(custom_width) - 1);
    custom_width[sizeof(custom_width) - 1] = '\0';
    int width = atoi(custom_width);

    if (width < 44 || width > 80) {
      if (width < 44) {
        show_bed_width_toast("Minimum Width: 44 in.");
      } else if (width > 80) {
        show_bed_width_toast("Maximum Width: 80 in.");
      }
      lv_obj_set_style_border_width(ta, 2, LV_PART_MAIN);
      lv_obj_set_style_border_color(ta, lv_color_hex(VST_COLOR_ALERT_RED),
                                    LV_PART_MAIN); /* reset border color */
      lv_obj_set_style_text_color(ta, lv_color_hex(VST_COLOR_EXTREME_ALERT), LV_PART_MAIN);
      lv_timer_create(toast_timer_cb, 3000, NULL); // reset after 3s
      return;
    }
    // Save valid width to settings and go back to bed width screen
    settings_set_bed_width(width);
    lv_scr_load(screen_settings_get_bed_width());
  }

  /* Digit buttons: read text from first child label */
  lv_obj_t *label = lv_obj_get_child(btn, 0);
  if (!label)
    return;

  const char *txt = lv_label_get_text(label);
  if (!txt || txt[0] == '\0')
    return;

  lv_textarea_add_text(ta, txt);
}

static void left_arrow_click_cb(lv_event_t *e) { lv_scr_load(get_change_width_settings_screen()); }

static void screen_load_cb(lv_event_t *e) {
  lv_obj_t *screen = lv_event_get_target(e);
  // Clear textarea on screen load
  lv_textarea_set_text(lv_textarea_0, "");
}

static void screen_init() {
  screen = tmpl_settings_create_1(NULL, "Settings", true);

  lv_obj_add_event_cb(screen, screen_load_cb, LV_EVENT_SCREEN_LOADED, NULL);

  lv_obj_t *left_arrow_obj = get_cont_left_arrow_obj();
  lv_obj_add_event_cb(left_arrow_obj, left_arrow_click_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_clear_flag(left_arrow_obj, LV_OBJ_FLAG_HIDDEN);

  lv_obj_t *info_icon_obj = get_cont_info_icon_obj();
  lv_obj_add_flag(info_icon_obj, LV_OBJ_FLAG_HIDDEN);

  lv_obj_t *close_icon_cont = get_cont_close_icon_obj();
  lv_obj_add_event_cb(close_icon_cont, icon_close_click_cb, LV_EVENT_CLICKED, NULL);

  bedwidth_value_settings_create();
}

// Initialize the bed width selection screen
lv_obj_t *get_custom_bed_width_screen(void) {
  if (screen == NULL) {
    screen_init();
  }

  return screen;
}