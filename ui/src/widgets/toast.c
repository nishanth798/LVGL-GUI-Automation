#include "ui.h"


// Create a global toast object and reassign it to the active screen,
// everytime we want to show a toast.

// static lv_obj_t *toast;
// static lv_timer_t *timer;

// static void timer_cb(lv_timer_t *timer) {
//   lv_timer_pause(timer);
//   label_toast_hide(toast);
// }

// // hidden by default
// static lv_obj_t *init(lv_obj_t *obj) {
//   toast = lv_obj_create(obj);
//   lv_obj_set_size(toast, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
//   lv_obj_clear_flag(toast, LV_OBJ_FLAG_SCROLLABLE);
//   lv_obj_add_flag(toast, LV_OBJ_FLAG_HIDDEN | LV_OBJ_FLAG_IGNORE_LAYOUT);

//   lv_obj_set_style_bg_color(toast, lv_color_hex(0x322F35), LV_PART_MAIN);
//   lv_obj_set_style_bg_opa(toast, LV_OPA_80, LV_PART_MAIN);
//   lv_obj_set_style_radius(toast, 30, LV_PART_MAIN);
//   // lv_obj_set_style_border_color(cont_toast, lv_color_black(), LV_PART_MAIN );
//   lv_obj_set_style_border_width(toast, 0, LV_PART_MAIN);
//   lv_obj_set_align(toast, LV_ALIGN_BOTTOM_MID);

//   lv_obj_set_y(toast, -80);

//   lv_obj_t *label = lv_label_create(toast);
//   lv_obj_set_style_text_font(label, &opensans_semibold_18, LV_PART_MAIN);
//   lv_label_set_text(label, "This is a toast");
//   lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
//   lv_obj_set_align(label, LV_ALIGN_CENTER);

//   // create timer
//   timer = lv_timer_create(timer_cb, READ_TIMEOUT, NULL);
//   lv_timer_pause(timer);

//   return toast;
// }

// lv_obj_t *toast_get(lv_obj_t *obj) {
//   if (toast == NULL) {
//     init(obj);
//   }
//   lv_obj_set_parent(toast, obj);
//   return toast;
// }

// void label_toast_set_text(lv_obj_t *obj, const char *text) {
//   lv_obj_t *label = lv_obj_get_child(obj, 0);
//   lv_label_set_text_static(label, text);
// }

// // NOTE: fade in/out animation is janky

// void label_toast_show(lv_obj_t *obj) {
//   lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);
//   // lv_obj_fade_in(obj, 1000, 0);

//   // start read timeout timer here
//   lv_timer_reset(timer);
//   lv_timer_resume(timer);
// }

// void label_toast_hide(lv_obj_t *obj) {
//   lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
//   // lv_obj_fade_out(obj, 1000, 0);
// }

// void toast_show(lv_obj_t *obj, const char *text) {
//   lv_obj_t *toast = toast_get(obj);
//   label_toast_set_text(toast, text);
//   label_toast_show(toast);
// }





static lv_obj_t *no_response_toast = NULL;
static lv_timer_t *no_response_timer = NULL;


static void hide_no_response_toast_timer_cb(lv_timer_t *timer){
  lv_obj_add_flag(no_response_toast, LV_OBJ_FLAG_HIDDEN);
  lv_timer_pause(timer);
  no_response_toast = NULL;
}

void hide_no_response_toast() {
  lv_obj_t *toast = no_response_toast;
  if(toast != NULL) {
    lv_obj_add_flag(toast, LV_OBJ_FLAG_HIDDEN);
  }
  if(no_response_timer != NULL) {
    lv_timer_pause(no_response_timer);
  }
  no_response_toast = NULL;
}

void toast_show(lv_obj_t *parent, const char *message){
    
      if(no_response_toast == NULL) {
        no_response_toast = lv_obj_create(parent);
        lv_obj_set_size(no_response_toast, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_clear_flag(no_response_toast, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(no_response_toast, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(no_response_toast, LV_OBJ_FLAG_IGNORE_LAYOUT);

        lv_obj_set_style_bg_color(no_response_toast, lv_color_hex(0x322F35), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(no_response_toast, LV_OPA_80, LV_PART_MAIN);;
        lv_obj_set_style_radius(no_response_toast, 30, LV_PART_MAIN);
        lv_obj_set_style_border_width(no_response_toast, 0, LV_PART_MAIN);
        lv_obj_set_align(no_response_toast, LV_ALIGN_BOTTOM_MID);
        lv_obj_set_y(no_response_toast, -80);

        lv_obj_t *label = lv_label_create(no_response_toast);
        lv_obj_set_style_text_font(label, &opensans_semibold_18, LV_PART_MAIN);
        lv_label_set_text(label, message);
        lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
        lv_obj_set_align(label, LV_ALIGN_CENTER);

        no_response_timer = lv_timer_create(hide_no_response_toast_timer_cb, 3000, NULL);
      }
      lv_obj_clear_flag(no_response_toast, LV_OBJ_FLAG_HIDDEN);
      lv_timer_reset(no_response_timer);
      lv_timer_resume(no_response_timer);
      
}


// --- Unified Static Variables ---
static lv_obj_t *common_toast = NULL;
static lv_obj_t *toast_label = NULL;
static lv_timer_t *toast_timer = NULL;
// New static reference for the close icon image object
static lv_obj_t *toast_close_img = NULL;

// --- Forward Declarations ---
void hide_toast();
static void init_common_toast(lv_obj_t *parent);

// --- Core Logic Functions ---

/**
 * @brief Hides the main toast object.
 */
void hide_toast() {
  if (common_toast != NULL) {
    lv_obj_add_flag(common_toast, LV_OBJ_FLAG_HIDDEN);
  }
}

/**
 * @brief Timer callback to hide the toast and delete the timer.
 * @param timer The timer instance (unused in logic but required by signature).
 */
static void hide_toast_timer_cb(lv_timer_t *timer) {
  hide_toast();
  if (timer) {
    lv_timer_del(timer);
    toast_timer = NULL; // Clear global reference
  }
}

/**
 * @brief Event callback for the close button.
 * @param e The LVGL event structure.
 */
static void toast_close_click_cb(lv_event_t *e) {
  hide_toast();
  // If a timer is running, stop it when the user manually closes the toast
  if (toast_timer != NULL) {
    lv_timer_del(toast_timer);
    toast_timer = NULL;
  }
}

/**
 * @brief Initializes the common toast object and its internal components once.
 * @param parent The parent object (usually the screen).
 */
static void init_common_toast(lv_obj_t *parent) {
  if (common_toast != NULL) {
    return; // Already created
  }

  // 1. Create the main container (toast)
  common_toast = lv_obj_create(parent);
  lv_obj_set_size(common_toast, 459, 55);
  lv_obj_clear_flag(common_toast, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_align(common_toast, LV_ALIGN_TOP_MID);
  lv_obj_set_y(common_toast, 11); // Offset from the top

  // Common Styles
  lv_obj_set_style_radius(common_toast, 10, LV_PART_MAIN);
  lv_obj_set_style_border_width(common_toast, 2, LV_PART_MAIN);
  lv_obj_set_style_pad_right(common_toast, 0, LV_PART_MAIN);
  lv_obj_set_style_border_opa(common_toast, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(common_toast, 255, LV_PART_MAIN);

  // Common Flags
  lv_obj_add_flag(common_toast, LV_OBJ_FLAG_CLICKABLE);
  // Hide by default, and ignore layout to allow fixed positioning
  lv_obj_add_flag(common_toast, LV_OBJ_FLAG_HIDDEN | LV_OBJ_FLAG_IGNORE_LAYOUT);

  // 2. Create Icon placeholder (will hold the check or cross circle)
  lv_obj_t *icon_obj = lv_img_create(common_toast);
  lv_obj_set_size(icon_obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_align(icon_obj, LV_ALIGN_LEFT_MID);
  lv_obj_clear_flag(icon_obj, LV_OBJ_FLAG_SCROLLABLE);
  // Store a reference to the status icon object in user data to update the source later
  lv_obj_set_user_data(common_toast, icon_obj);

  // 3. Create Text Label and save its reference globally
  toast_label = lv_label_create(common_toast);
  lv_obj_set_style_text_font(toast_label, &opensans_bold_20, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_size(toast_label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_style_text_opa(toast_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  // Align the label relative to the icon
  lv_obj_align_to(toast_label, icon_obj, LV_ALIGN_OUT_RIGHT_MID, 35, 3);

  // 4. Create Close Button Container
  lv_obj_t *cont_img_close = lv_obj_create(common_toast);
  lv_obj_set_size(cont_img_close, 55, 55);
  lv_obj_set_align(cont_img_close, LV_ALIGN_RIGHT_MID);
  lv_obj_clear_flag(cont_img_close, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(cont_img_close, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_style_bg_opa(cont_img_close, 0, LV_PART_MAIN);
  lv_obj_set_style_border_opa(cont_img_close, 0, LV_PART_MAIN);

  // 5. Add Close "X" image inside the container
  // Store the reference globally so we can update its source in show_toast
  toast_close_img = lv_img_create(cont_img_close);
  lv_img_set_src(toast_close_img, &icon_cross); // Set default source
  lv_obj_set_align(toast_close_img, LV_ALIGN_RIGHT_MID);

  // Set the event callback for manual closing
  lv_obj_add_event_cb(cont_img_close, toast_close_click_cb, LV_EVENT_CLICKED, NULL);
}

/**
 * @brief The core function to show a dynamically configured toast notification.
 * The duration is fixed internally to TOAST_DEFAULT_DURATION_MS (3000ms).
 * @param message The text message to display.
 * @param bg_color The background color in 0xRRGGBB format.
 * @param border_color The border color in 0xRRGGBB format.
 * @param status_icon_src The image source for the main status icon (e.g., &icon_check).
 * @param close_icon_src The image source for the close button icon (e.g., &icon_cross).
 */
void show_toast(const char *message, uint32_t bg_color, uint32_t border_color, const void *status_icon_src, const void *close_icon_src, uint32_t text_color) {
  lv_obj_t *screen_active = lv_screen_active();

  // 1. Initialization (only happens once)
  if (common_toast == NULL) {
    init_common_toast(screen_active);
  }

  // Ensure the toast is on the active screen
  lv_obj_set_parent(common_toast, screen_active);

  // 2. Stop any existing timer to prevent hiding prematurely
  if (toast_timer != NULL) {
    lv_timer_del(toast_timer);
    toast_timer = NULL;
  }

  // 3. Update dynamic content and styling
  if (toast_label) {
    lv_label_set_text(toast_label, message);
    // Set text color
    lv_obj_set_style_text_color(toast_label, lv_color_hex(text_color), LV_PART_MAIN | LV_STATE_DEFAULT);
  }

  // Set colors
  lv_obj_set_style_bg_color(common_toast, lv_color_hex(bg_color), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(common_toast, lv_color_hex(border_color), LV_PART_MAIN | LV_STATE_DEFAULT);

  // Set main status icon by retrieving the stored reference
  lv_obj_t *icon_obj = (lv_obj_t *)lv_obj_get_user_data(common_toast);
  if (icon_obj) {
      lv_img_set_src(icon_obj, status_icon_src);
  }

  // Set close button icon (New logic)
  if (toast_close_img && close_icon_src) {
      lv_img_set_src(toast_close_img, close_icon_src);
  }

  // 4. Show the toast
  lv_obj_clear_flag(common_toast, LV_OBJ_FLAG_HIDDEN);

  // 5. Start the new timer using the fixed duration
  toast_timer = lv_timer_create(hide_toast_timer_cb, 3000, NULL);
}

// --- Original Public API Functions (now wrappers for the unified system) ---

/**
 * @brief Public function to show the Success Toast.
 */
void show_save_toast() {
  // Original Success Colors: BG: 0xBBE4CB, Border: 0x218448
  // Now passing both the status icon (&icon_check) and the close icon (&icon_cross)
  show_toast("Setting changes have been saved!", VST_COLOR_MINT_GREEN, VST_COLOR_MONITOR_GREEN, &icon_check, &icon_cross, VST_COLOR_ACTIVATE_SENSOR_BLUE);
}

/**
 * @brief Public function to show the Alert/Error Toast.
 * @param text The dynamic error message to display.
 */
void show_bed_width_toast(const char *text) {
  // Original Error Colors: BG: 0xFFD0D0, Border: VST_COLOR_ALERT_RED (using placeholder 0xC82333)
  // Now passing both the status icon (&icon_cross_circle) and the close icon (&icon_badge_close)
  show_toast(text, VST_COLOR_ALERT_PINK, VST_COLOR_ALERT_RED, &icon_cross_circle, &icon_badge_close, VST_COLOR_CRITICAL_DARK_RED);
}