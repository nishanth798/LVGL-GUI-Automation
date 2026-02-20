#include "jsonrpc2.h"
#include "screens.h"
#include "sys_state.h"
#include "ui.h"

// --- Constants and Macros ---

#define WIDTH_CHOICE_COUNT 4
#define FULL_XL_BED_WIDTH 54
#define QUEEN_BED_WIDTH 60
#define CALI_KING_BED_WIDTH 72
#define KING_BED_WIDTH 76

static const char *width_labels[WIDTH_CHOICE_COUNT] = {"Full/Full XL (54 in.)", "Queen (60 in.)",
                                                       "California King\n        (72 in.)",
                                                       "King (76 in.)"};
static const char *custom_value_label = {"Enter Custom Value"};

// --- Static Variables ---

static lv_obj_t *screen;                        // main screen object
static lv_obj_t *btn_width[WIDTH_CHOICE_COUNT]; // width selection buttons
static bool is_left_arrow_clicked = false; 
static bool is_custom_bed_width_value_btn_clicked = false; 

// --- Static Function Prototypes ---

static void screen_init();
static void screen_unload_cb(lv_event_t *e);
static void screen_load_cb(lv_event_t *e);
static void left_arrow_click_cb(lv_event_t *e);
static void icon_close_click_cb(lv_event_t *e);
static void change_width_btn_click_cb(lv_event_t *e);
static void custom_bed_width_value_btn_cb(lv_event_t *e);

// --- Static Function Definitions (Callbacks) ---

/**
 *  Handles click event for the close icon.
 */
static void icon_close_click_cb(lv_event_t *e) {
  // Just reset the bed width selection flag
  set_bed_width_selection_flag(false);
}

// Called when the screen is unloaded.
static void screen_unload_cb(lv_event_t *e) {
  if( is_left_arrow_clicked  || is_custom_bed_width_value_btn_clicked) {
    // Reset the flags 
    is_left_arrow_clicked = false;
    is_custom_bed_width_value_btn_clicked = false;
    return;
  } else{
    // Clear the bed width selection flag if navigating without using left arrow or custom value button
    set_bed_width_selection_flag(false);
  }
  
}
/**
 * Called when the screen is loaded.
 * Handles setting the initial checked state based on saved width.
 */
static void screen_load_cb(lv_event_t *e) {
  uint8_t current_width = settings_get_bed_width(); // get  selected width
  uint8_t selected_index = 0;                       // 0 indicates custom/not standard

  // Map current width value to a button index (1-based)
  if (current_width == FULL_XL_BED_WIDTH) {
    selected_index = 1;
  } else if (current_width == QUEEN_BED_WIDTH) {
    selected_index = 2;
  } else if (current_width == CALI_KING_BED_WIDTH) {
    selected_index = 3;
  } else if (current_width == KING_BED_WIDTH) {
    selected_index = 4;
  }
  // If selected_index remains 0, no  button is checked

  for (int i = 0; i < WIDTH_CHOICE_COUNT; i++) {
    lv_obj_t *child_label = lv_obj_get_child(btn_width[i], 0); // get label from button

    if ((i + 1) == selected_index) {
      // If this button is the current width -> mark as checked and highlight text
      lv_obj_add_state(btn_width[i], LV_STATE_CHECKED);
      lv_obj_set_style_text_color(child_label, lv_color_white(), LV_PART_MAIN);
    } else {
      // Otherwise uncheck and set text to normal color (dark blue)
      lv_obj_remove_state(btn_width[i], LV_STATE_CHECKED);
      lv_obj_set_style_text_color(child_label, lv_color_hex(VST_COLOR_DARKBLUE), LV_PART_MAIN);
    }
  }
}

/**
 *  Left arrow click callback -> go back to bed width settings screen.
 */
static void left_arrow_click_cb(lv_event_t *e) {
  is_left_arrow_clicked = true;
  lv_scr_load(screen_settings_get_bed_width());
}

/**
 * Change bed width button click callback (for standard widths).
 * Updates the setting and the visual state of all buttons.
 */
static void change_width_btn_click_cb(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_target_obj(e); // button that was clicked
  lv_obj_t *parent = lv_obj_get_parent(obj);  // parent container

  // Loop through all bed width buttons in the container
  for (uint32_t i = 0; i < lv_obj_get_child_count(parent); i++) {
    lv_obj_t *child = lv_obj_get_child(parent, i);
    lv_obj_t *child_label = lv_obj_get_child(child, 0); // get button’s label

    // Uncheck/reset all buttons first
    lv_obj_remove_state(child, LV_STATE_CHECKED);
    lv_obj_set_style_text_color(child_label, lv_color_hex(VST_COLOR_DARKBLUE), LV_PART_MAIN);

    if (child == obj) {
      // Selected button: checked + white text
      lv_obj_add_state(child, LV_STATE_CHECKED);
      lv_obj_set_style_text_color(child_label, lv_color_white(), LV_PART_MAIN);

      const char *btn_text = lv_label_get_text(child_label);

      // Update the bed width setting based on the label text
      if (strcmp(btn_text, width_labels[0]) == 0) {
        settings_set_bed_width(FULL_XL_BED_WIDTH);
      } else if (strcmp(btn_text, width_labels[1]) == 0) {
        settings_set_bed_width(QUEEN_BED_WIDTH);
      } else if (strcmp(btn_text, width_labels[2]) == 0) {
        settings_set_bed_width(CALI_KING_BED_WIDTH);
      } else if (strcmp(btn_text, width_labels[3]) == 0) {
        settings_set_bed_width(KING_BED_WIDTH);
      }
    }
  }
}

/**
 * Custom bed width value button callback.
 * Loads the screen for entering a custom value.
 */
static void custom_bed_width_value_btn_cb(lv_event_t *e) {
  is_custom_bed_width_value_btn_clicked = true;
  // Load button matrix screen for entering custom bed width value and then set it's value
  lv_scr_load(get_custom_bed_width_screen());
}

// --- Screen Initialization ---

/**
 * Initializes the custom bed width settings screen.
 */
static void screen_init() {
  // create header and base screen
  screen = tmpl_settings_create_1(NULL, "Settings", true);

  // Run `screen_load_cb` once screen is loaded to set initial state
  lv_obj_add_event_cb(screen, screen_load_cb, LV_EVENT_SCREEN_LOADED, NULL);
  lv_obj_add_event_cb(screen, screen_unload_cb, LV_EVENT_SCREEN_UNLOAD_START, NULL);

  // Setup header controls (left arrow and close icon)
  lv_obj_t *lef_arrow_obj = get_img_left_arrow_obj();
  lv_obj_add_flag(lef_arrow_obj, LV_OBJ_FLAG_EVENT_BUBBLE);
  lv_obj_t *title_cont_left_arrow_obj = get_cont_left_arrow_obj();
  lv_obj_clear_flag(title_cont_left_arrow_obj, LV_OBJ_FLAG_HIDDEN); // show left arrow
  lv_obj_add_event_cb(title_cont_left_arrow_obj, left_arrow_click_cb, LV_EVENT_CLICKED, NULL);

  // Hide info icon container
  lv_obj_t *title_cont_info_icon_obj = get_cont_info_icon_obj();
  lv_obj_add_flag(title_cont_info_icon_obj, LV_OBJ_FLAG_HIDDEN);

  // Get close icon container and add event callback
  lv_obj_t *close_icon_cont = get_cont_close_icon_obj();
  lv_obj_add_event_cb(close_icon_cont, icon_close_click_cb, LV_EVENT_CLICKED, NULL);

  // Create main content tile/card
  lv_obj_t *tile = lv_obj_create(screen);
  lv_obj_clear_flag(tile, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_size(tile, 448, 302);
  lv_obj_set_style_radius(tile, 10, LV_PART_MAIN);
  lv_obj_set_style_pad_all(tile, 16, LV_PART_MAIN);
  lv_obj_set_style_pad_gap(tile, 16, LV_PART_MAIN);
  lv_obj_set_style_border_width(tile, 1, LV_PART_MAIN);
  lv_obj_set_style_border_color(tile, lv_color_hex(0xD9D9D9), LV_PART_MAIN);
  lv_obj_set_flex_flow(tile, LV_FLEX_FLOW_ROW_WRAP);

  // Drop shadow styling for the tile
  lv_obj_set_style_shadow_ofs_y(tile, 3, LV_PART_MAIN);
  lv_obj_set_style_shadow_color(tile, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_set_style_shadow_width(tile, 6, LV_PART_MAIN);
  lv_obj_set_style_shadow_opa(tile, 64, LV_PART_MAIN);

  // 1. Container for "Custom Bed Width" label
  lv_obj_t *label_container = lv_obj_create(tile);
  lv_obj_clear_flag(label_container, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(label_container, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_set_size(label_container, 448, LV_SIZE_CONTENT);
  lv_obj_set_style_border_width(label_container, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(label_container, 0, LV_PART_MAIN);
  lv_obj_align(label_container, LV_ALIGN_TOP_LEFT, 0, 3);

  // label
  lv_obj_t *label = lv_label_create(label_container);
  lv_label_set_text(label, "Custom Bed Width");
  // Assuming opensans_semibold_18 is available globally/in a header
  lv_obj_set_style_text_font(label, &opensans_semibold_18, LV_PART_MAIN);
  lv_obj_set_style_text_color(label, lv_color_hex(0x31344E), LV_PART_MAIN);

  // 2. Container for standard bed width buttons
  lv_obj_t *custom_bed_width_container = lv_obj_create(tile);
  lv_obj_add_flag(custom_bed_width_container, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_clear_flag(custom_bed_width_container, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_size(custom_bed_width_container, 428, 150);
  lv_obj_set_pos(custom_bed_width_container, -3, 31);
  lv_obj_set_style_border_width(custom_bed_width_container, 0, LV_PART_MAIN);
  lv_obj_set_flex_flow(custom_bed_width_container, LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_style_pad_all(custom_bed_width_container, 4, LV_PART_MAIN);
  lv_obj_set_style_pad_gap(custom_bed_width_container, 10, LV_PART_MAIN);
  
  // Create bed width selection buttons
  for (int i = 0; i < WIDTH_CHOICE_COUNT; i++) {
    btn_width[i] = lv_btn_create(custom_bed_width_container);
    lv_obj_remove_style(btn_width[i], NULL, LV_STYLE_SHADOW_COLOR | LV_STYLE_SHADOW_WIDTH | LV_STYLE_SHADOW_SPREAD | LV_STYLE_SHADOW_OFFSET_X | LV_STYLE_SHADOW_OFFSET_Y); // clear default styles
    lv_obj_set_size(btn_width[i], 203, 64);
    lv_obj_add_flag(btn_width[i], LV_OBJ_FLAG_CHECKABLE);

    // Default (unselected) style
    lv_obj_set_style_bg_color(btn_width[i], lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(btn_width[i], lv_color_hex(VST_COLOR_DARKBLUE),
                                  LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(btn_width[i], 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(btn_width[i], 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_shadow_color(btn_width[i], lv_color_hex(0x000000), LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(btn_width[i], 4, LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(btn_width[i], 64, LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(btn_width[i], 0, LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_offset_y(btn_width[i], 2, LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_offset_x(btn_width[i], 0, LV_STATE_DEFAULT);

    // Add button label (width name)
    lv_obj_t *label_width = lv_label_create(btn_width[i]);
    lv_obj_align(label_width, LV_ALIGN_CENTER, 0, 2);
    lv_label_set_text(label_width, width_labels[i]);
    lv_obj_set_style_text_color(label_width, lv_color_hex(VST_COLOR_DARKBLUE),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(label_width, &opensans_bold_18, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Checked (selected) state style
    lv_obj_set_style_bg_color(btn_width[i], lv_color_hex(VST_COLOR_BTN_SELECTED_SKYBLUE),
                              LV_STATE_CHECKED);
    lv_obj_set_style_border_color(btn_width[i], lv_color_hex(VST_COLOR_BTN_SELECTED_SKYBLUE),
                                  LV_STATE_CHECKED);
    // Apply standard shadow for checked state
    lv_obj_set_style_shadow_color(btn_width[i], lv_color_hex(0x74B4FF), LV_STATE_CHECKED);
    lv_obj_set_style_shadow_width(btn_width[i], 2, LV_STATE_CHECKED);
    lv_obj_set_style_shadow_opa(btn_width[i], LV_OPA_50, LV_STATE_CHECKED);
    lv_obj_set_style_shadow_spread(btn_width[i], 3, LV_STATE_CHECKED);
    lv_obj_set_style_shadow_offset_y(btn_width[i], 0, LV_STATE_CHECKED);
    lv_obj_set_style_shadow_offset_x(btn_width[i], 0, LV_STATE_CHECKED);
    lv_obj_set_style_text_color(btn_width[i], lv_color_white(), LV_STATE_CHECKED);

    // Click handler for width change
    lv_obj_add_event_cb(btn_width[i], change_width_btn_click_cb, LV_EVENT_CLICKED, NULL);
  }

  // 3. Line divider
  lv_obj_t *line_divider;
  line_divider = lv_img_create(tile);
  lv_img_set_src(line_divider, &icon_line_divider);
  lv_obj_add_flag(line_divider, LV_OBJ_FLAG_IGNORE_LAYOUT);
  // lv_obj_set_size(line_divider, 416, 1);
  // lv_obj_set_style_bg_color(line_divider, lv_color_hex(VST_COLOR_CONTAINER_BORDER), LV_PART_MAIN);
  lv_obj_set_y(line_divider, 188);
  // lv_obj_set_style_border_width(line_divider, 0, LV_PART_MAIN);

  // 4. "Enter Custom Value" button
  lv_obj_t *custom_value_btn;

  custom_value_btn = lv_btn_create(tile);
  lv_obj_add_flag(custom_value_btn, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_set_size(custom_value_btn, 416, 64);
  lv_obj_align(custom_value_btn, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_add_flag(custom_value_btn, LV_OBJ_FLAG_CLICKABLE);

  // Style
  lv_obj_set_style_bg_color(custom_value_btn, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(custom_value_btn, lv_color_hex(VST_COLOR_DARKBLUE),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(custom_value_btn, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(custom_value_btn, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

  // Label for custom value button
  lv_obj_t *label_custom_value = lv_label_create(custom_value_btn);
  lv_obj_center(label_custom_value);
  lv_label_set_text(label_custom_value, custom_value_label);
  lv_obj_set_style_text_color(label_custom_value, lv_color_hex(VST_COLOR_DARKBLUE),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(label_custom_value, &opensans_bold_18,
                             LV_PART_MAIN | LV_STATE_DEFAULT);

  // Click handler for custom value entry
  lv_obj_add_event_cb(custom_value_btn, custom_bed_width_value_btn_cb, LV_EVENT_CLICKED, NULL);
}

// --- Public Function Definition ---

lv_obj_t *get_change_width_settings_screen() {
  if (screen == NULL) {
    screen_init();
  }
  return screen;
}