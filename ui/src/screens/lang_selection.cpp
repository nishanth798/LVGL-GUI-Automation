#include "jsonrpc2.h"
#include "screens.h"
#include "sys_state.h"
#include "ui.h"

// --- Static Variables ---
static lv_obj_t *screen = NULL;        // Declaration for the main LVGL screen object.
static lv_obj_t *lbl_page_info = NULL; // Declaration for the label showing "page x of y".
static lv_obj_t
    *btn_language[6]; // Array of button objects for language selection (max 6 per page).
static lv_obj_t *btn_page_prev = NULL; // Declaration for the Previous page button object.
static lv_obj_t *btn_page_next = NULL; // Declaration for the Next page button object.

static int curr_page = 1,
           total_pages = 1; // Tracks the current page number and total pages for pagination.
static const int LANG_BTN_COUNT =
    6; // Defines the fixed number of language buttons displayed per page.

lang_info_t *lang_info = NULL; // Global pointer to the structure holding language data.

static bool is_left_arrow_clicked =
    false; // Flag to indicate if the left arrow (back) button was clicked.

// --- Language Info Helpers ---
void lang_info_clear(lang_info_t *lang_info) { // Clears the content of the language info structure.
  if (lang_info != NULL) {                     // Checks if the structure pointer is valid.
    // Loop through the available_languages array.
    for (int i = 0;
         i < (sizeof(lang_info->available_languages) / sizeof(lang_info->available_languages[0]));
         i++) {
      // Null-terminate and clear the string buffer for each language name.
      memset(lang_info->available_languages[i], '\0', sizeof(lang_info->available_languages[i]));
    }
    // Clear the currently selected language string buffer.
    memset(lang_info->selected_language, '\0', sizeof(lang_info->selected_language));
    lang_info->selected_language_index = 0;  // Reset the index pointer for available languages.
    lang_info->available_language_count = 0; // Reset the total count of available languages.
  }
}

static void lang_info_init() { // Initializes the global language info structure.
  if (lang_info == NULL) {     // Checks if the structure has not been allocated yet.
    lang_info = (lang_info_t *)malloc(sizeof(lang_info_t)); // Allocates memory for the structure.
    lang_info_clear(lang_info); // Calls the helper to initialize and clear the structure fields.
  }
}

lang_info_t *lang_info_get() { // Getter function for the global language info structure.
  if (lang_info == NULL) {
    lang_info_init();
  }
  return lang_info; // Returns the pointer to the structure.
}

static void
unselect_btns() { // Manages the visual state (checked/unchecked) of the language buttons.
  for (int i = 0; i < LANG_BTN_COUNT;
       i++) { // Iterates through all 6 language buttons on the current page.
    // Compares the globally selected language string with the text of the current button's label.
    if (strcmp(lang_info->selected_language,
               lv_label_get_text(lv_obj_get_child(btn_language[i], 0))) == 0) {
      lv_obj_add_state(btn_language[i],
                       LV_STATE_CHECKED); // If matching, set the button to the 'checked' state.
      // Set the label text color to white for the checked state.
      lv_obj_set_style_text_color(lv_obj_get_child(btn_language[i], 0), lv_color_white(),
                                  LV_PART_MAIN);
    } else {
      lv_obj_remove_state(btn_language[i],
                          LV_STATE_CHECKED); // If not matching, remove the 'checked' state.
      // Set the label text color to the default dark blue for the unchecked state.
      lv_obj_set_style_text_color(lv_obj_get_child(btn_language[i], 0),
                                  lv_color_hex(VST_COLOR_DARKBLUE), LV_PART_MAIN);
    }
  }
}

// --- UI Update Helpers ---
// Updates the language buttons with content from the available languages array based on the current
// page index.
static void update_language_buttons(uint8_t *arr_idx, char arr[50][21], int total_count) {
  // Calculates the total number of pages required for all available languages.
  total_pages = (total_count + LANG_BTN_COUNT - 1) / LANG_BTN_COUNT;

  // Loop through the 6 language buttons.
  for (int i = 0; i < LANG_BTN_COUNT; i++) {
    lv_obj_t *label =
        lv_obj_get_child(btn_language[i], 0); // Get the label object inside the current button.
    // Check if the current index is within the total count AND the language string is not empty.
    if ((*arr_idx) < total_count && strcmp(arr[*arr_idx], "") != 0) {
      // Set the button's label text to the language name from the array.
      lv_label_set_text_fmt(label, "%s", arr[*arr_idx]);
      lv_obj_clear_flag(btn_language[i], LV_OBJ_FLAG_HIDDEN); // Make the button visible.
    } else {
      lv_obj_add_flag(
          btn_language[i],
          LV_OBJ_FLAG_HIDDEN); // Hide the button if no language is available for this slot.
    }
    (*arr_idx)++; // Increment the array index pointer to move to the next language for the next
                  // button.
  }
  // Calculate the current page number based on the updated array index.
  curr_page = ((*arr_idx) + LANG_BTN_COUNT - 1) / LANG_BTN_COUNT;
  // Update the page info label text ("X of Y").
  lv_label_set_text_fmt(lbl_page_info, "%d of %d", curr_page, total_pages);

  // Manage navigation button states based on current page position
  if (curr_page == 1) {                                 // If on the first page.
    lv_obj_add_state(btn_page_prev, LV_STATE_DISABLED); // Disable the previous button.
  } else {
    lv_obj_clear_state(btn_page_prev, LV_STATE_DISABLED); // Enable the previous button.
  }
  if (curr_page == total_pages) {                       // If on the last page.
    lv_obj_add_state(btn_page_next, LV_STATE_DISABLED); // Disable the next button.
  } else {
    lv_obj_clear_state(btn_page_next, LV_STATE_DISABLED); // Enable the next button.
  }

  unselect_btns(); // Re-apply the checked state based on the selected language.
}

// --- Event Callbacks ---
static void screen_load_cb(lv_event_t *e) { // Event fired when the screen is loaded.
  lang_info->selected_language_index = 0;   // Reset pagination index to the start (page 1).
  strcpy(lang_info->selected_language, ""); // Clear the selected language.
  // Update the UI buttons to display the languages starting from the beginning.
  update_language_buttons(&lang_info->selected_language_index, lang_info->available_languages,
                          lang_info->available_language_count);
}

static void screen_unload_cb(lv_event_t *e) { // Event fired when the screen starts unloading.
  // If the left arrow (back) button was clicked, do not clear the language selection flag.
  if (is_left_arrow_clicked) {
    is_left_arrow_clicked = false;
  }
  // If the left arrow was not clicked, clear the language selection flag.
  else {
    flag_lang_selection = false;
  }
}

static void left_arrow_click_cb(lv_event_t *e) { // Callback for the left arrow (back) button.
  is_left_arrow_clicked = true; // Sets a flag indicating the left arrow was clicked.
  if (strcmp(lang_info->selected_language, "") == 0) {
    char *selected_lang =
        get_selected_language(); // Get the currently selected language from session data.
    strcpy(lang_info->selected_language, selected_lang); // Copy the stored language.
  }
  lv_scr_load(screen_settings_audio_get()); // Loads the previous screen (audio settings screen).
}

static void lang_btn_click_cb(lv_event_t *e) { // Callback for when a language button is clicked.
  lv_obj_t *obj = lv_event_get_target_obj(e);  // Get the button object that triggered the event.

  for (uint32_t i = 0; i < LANG_BTN_COUNT; i++) { // Loop through all 6 buttons.
    lv_obj_t *child_label =
        lv_obj_get_child(btn_language[i], 0); // Get the label of the current button in the loop.

    if (btn_language[i] == obj) { // If the current button in the loop is the one that was clicked.
      lv_obj_add_state(btn_language[i],
                       LV_STATE_CHECKED); // Set the clicked button to the checked state.
      lv_obj_set_style_text_color(child_label, lv_color_white(),
                                  LV_PART_MAIN); // Set label color to white.
      // Update the selected language in the structure with the text from the clicked button.
      strcpy(lang_info->selected_language, lv_label_get_text(child_label));
    } else {
      lv_obj_remove_state(btn_language[i], LV_STATE_CHECKED); // Uncheck all other buttons.
      // Reset the label color to dark blue for unchecked buttons.
      lv_obj_set_style_text_color(child_label, lv_color_hex(VST_COLOR_DARKBLUE), LV_PART_MAIN);
    }
  }
}

// Helper to create the navigation (previous/next page) buttons.
static void create_nav_button(lv_obj_t *parent, lv_obj_t **btn, lv_obj_t **img,
                              const lv_image_dsc_t *icon) {
  *btn = lv_btn_create(parent);   // Create a button object on the parent container.
  lv_obj_set_size(*btn, 138, 64); // Set the fixed size of the button.
  lv_obj_set_style_bg_color(*btn, lv_color_hex(0x1A1040), LV_PART_MAIN); // Set background color.
  lv_obj_set_style_radius(*btn, 5.42, LV_PART_MAIN);                     // Set rounded corners.

  *img = lv_img_create(*btn); // Create an image object inside the button.
  lv_img_set_src(*img, icon); // Set the image source (e.g., arrow icon).
  lv_obj_center(*img);        // Center the image within the button.
}

static void close_icon_click_cb(lv_event_t *e) { // Callback for the close (X) icon button.
  set_language_selection_flag(false); // Clears a flag indicating language selection is in progress.
}

static void prev_page_btn_click_cb(lv_event_t *e) { // Callback for the Previous Page button.
  // Check if moving back would result in a negative index (i.e., we are already on the first page's
  // block).
  if (lang_info->selected_language_index <= LANG_BTN_COUNT)
    return;
  // Move the index back by two full pages to position it at the start of the previous page's data.
  lang_info->selected_language_index -= (LANG_BTN_COUNT * 2);
  // Update buttons using the new, earlier starting index.
  update_language_buttons(&lang_info->selected_language_index, lang_info->available_languages,
                          lang_info->available_language_count);
}

static void next_page_btn_click_cb(lv_event_t *e) { // Callback for the Next Page button.
  // Check if the current index pointer is already at the last available language.
  if (lang_info->selected_language_index > lang_info->available_language_count - 1)
    return;
  update_language_buttons(&lang_info->selected_language_index, lang_info->available_languages,
                          lang_info->available_language_count);
}

// --- Screen Initialization ---
void screen_init() {
  // Create the main screen using a settings template function.
  screen = tmpl_settings_create_1(NULL, "Settings", true);
  // Attach an event callback to run when the screen starts loading.
  lv_obj_add_event_cb(screen, screen_load_cb, LV_EVENT_SCREEN_LOAD_START, NULL);
  lv_obj_add_event_cb(screen, screen_unload_cb, LV_EVENT_SCREEN_UNLOAD_START, NULL);

  // Left arrow (back) setup
  lv_obj_t *lef_arrow_obj =
      get_img_left_arrow_obj(); // Get the left arrow image object (assumed template function).
  lv_obj_add_flag(lef_arrow_obj,
                  LV_OBJ_FLAG_EVENT_BUBBLE); // Allow events to bubble up to the parent container.
  lv_obj_t *title_cont_left_arrow_obj =
      get_cont_left_arrow_obj(); // Get the container for the left arrow.
  lv_obj_clear_flag(title_cont_left_arrow_obj,
                    LV_OBJ_FLAG_HIDDEN); // Ensure the container is visible.
  // Attach the click callback.
  lv_obj_add_event_cb(title_cont_left_arrow_obj, left_arrow_click_cb, LV_EVENT_CLICKED, NULL);

  // Hide info icon
  lv_obj_t *title_cont_info_icon_obj = get_cont_info_icon_obj(); // Get the info icon container.
  lv_obj_add_flag(title_cont_info_icon_obj, LV_OBJ_FLAG_HIDDEN); // Hide the info icon.

  // Close icon setup
  lv_obj_t *cont_close_icon = get_cont_close_icon_obj(); // Get the close icon container.
  // Attach the close icon click callback.
  lv_obj_add_event_cb(cont_close_icon, close_icon_click_cb, LV_EVENT_CLICKED, NULL);

  // Main content tile (Container for all language options and navigation)
  lv_obj_t *tile = lv_obj_create(screen); // Create the main tile object on the screen.
  lv_obj_add_flag(tile, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_set_pos(tile, 16, 80);
  lv_obj_clear_flag(tile, LV_OBJ_FLAG_SCROLLABLE);      // Prevent the tile from being scrollable.
  lv_obj_set_size(tile, 448, 360);                      // Set fixed size.
  lv_obj_set_style_radius(tile, 10, LV_PART_MAIN);      // Set rounded corners.
  lv_obj_set_style_pad_all(tile, 16, LV_PART_MAIN);     // Set padding inside the tile.
  lv_obj_set_style_pad_gap(tile, 16, LV_PART_MAIN);     // Set spacing between flex items.
  lv_obj_set_style_border_width(tile, 1, LV_PART_MAIN); // Set border width.
  lv_obj_set_style_border_color(tile, lv_color_hex(0xD9D9D9), LV_PART_MAIN); // Set border color.
  lv_obj_set_flex_flow(tile, LV_FLEX_FLOW_ROW_WRAP); // Set layout to flow rows and wrap content.

  // Drop shadow styling
  lv_obj_set_style_shadow_ofs_y(tile, 3, LV_PART_MAIN); // Shadow vertical offset.
  lv_obj_set_style_shadow_color(tile, lv_color_hex(0x000000),
                                LV_PART_MAIN);          // Shadow color (black).
  lv_obj_set_style_shadow_width(tile, 8, LV_PART_MAIN); // Shadow width/blur.
  lv_obj_set_style_shadow_opa(tile, 64, LV_PART_MAIN);  // Shadow opacity (25%).

  // Header: "Languages" label container
  lv_obj_t *label_container = lv_obj_create(tile); // Create a container for the title label.
  lv_obj_clear_flag(label_container, LV_OBJ_FLAG_SCROLLABLE); // Prevent scrolling.
  lv_obj_set_size(label_container, 416, 20); // Set fixed size (width of the tile content area).
  lv_obj_set_style_border_width(label_container, 0, LV_PART_MAIN); // Remove border.
  lv_obj_set_style_pad_all(label_container, 0, LV_PART_MAIN);      // Remove internal padding.

  lv_obj_t *label = lv_label_create(label_container); // Create the "Languages" label.
  lv_label_set_text(label, "Languages");              // Set the label text.
  lv_obj_set_style_text_font(label, &opensans_semibold_18, LV_PART_MAIN);   // Set the font style.
  lv_obj_set_style_text_color(label, lv_color_hex(0x31344E), LV_PART_MAIN); // Set the text color.

  // Language buttons container
  lv_obj_t *language_opt_container =
      lv_obj_create(tile); // Create a container for the 6 language buttons.
  lv_obj_clear_flag(language_opt_container, LV_OBJ_FLAG_SCROLLABLE); // Prevent scrolling.
  lv_obj_set_size(language_opt_container, 416, 212); // Set fixed size for the button area.
  lv_obj_set_style_border_width(language_opt_container, 0, LV_PART_MAIN); // Remove border.
  lv_obj_set_flex_flow(language_opt_container, LV_FLEX_FLOW_ROW_WRAP);    // Set layout to row wrap.
  lv_obj_set_style_pad_all(language_opt_container, 0, LV_PART_MAIN); // Remove internal padding.

  // Loop to create the 6 language buttons.
  for (int i = 0; i < LANG_BTN_COUNT; i++) {
    btn_language[i] = lv_btn_create(language_opt_container); // Create the button.
    lv_obj_set_size(btn_language[i], 203, 64);               // Set button size .
    lv_obj_set_style_bg_color(btn_language[i], lv_color_white(),
                              LV_PART_MAIN); // Default background color.
    lv_obj_set_style_border_color(btn_language[i], lv_color_hex(VST_COLOR_DARKBLUE),
                                  LV_PART_MAIN);                     // Default border color.
    lv_obj_set_style_border_width(btn_language[i], 3, LV_PART_MAIN); // Border width.
    lv_obj_set_style_radius(btn_language[i], 5, LV_PART_MAIN);       // Border radius.

    // Styling for the CHECKED state (when selected)
    lv_obj_set_style_bg_color(btn_language[i], lv_color_hex(VST_COLOR_BTN_SELECTED_SKYBLUE),
                              LV_STATE_CHECKED); // Selected background color.
    lv_obj_set_style_border_color(btn_language[i], lv_color_hex(VST_COLOR_BTN_SELECTED_SKYBLUE),
                                  LV_STATE_CHECKED); // Selected border color.
    // Shadow styling for checked state
    lv_obj_set_style_shadow_color(btn_language[i], lv_color_hex(0x74B4FF), LV_STATE_CHECKED);
    lv_obj_set_style_shadow_width(btn_language[i], 2, LV_STATE_CHECKED);
    lv_obj_set_style_shadow_opa(btn_language[i], LV_OPA_50, LV_STATE_CHECKED);
    lv_obj_set_style_shadow_spread(btn_language[i], 3, LV_STATE_CHECKED);
    lv_obj_set_style_shadow_offset_y(btn_language[i], 0, LV_STATE_CHECKED);
    lv_obj_set_style_shadow_offset_x(btn_language[i], 0, LV_STATE_CHECKED);

    lv_obj_t *lbl = lv_label_create(btn_language[i]); // Create the label inside the button.
    lv_obj_center(lbl);                               // Center the label text.
    lv_obj_set_style_text_color(lbl, lv_color_hex(VST_COLOR_DARKBLUE),
                                LV_PART_MAIN); // Default text color.
    lv_obj_set_style_text_font(lbl, &opensans_bold_18,
                               LV_PART_MAIN | LV_STATE_DEFAULT); // Set the font.

    lv_obj_add_event_cb(btn_language[i], lang_btn_click_cb, LV_EVENT_CLICKED,
                        NULL); // Attach click handler.
  }

  // Footer: navigation and page info container
  lv_obj_t *cont_page_number_info =
      lv_obj_create(tile);                         // Create a container for navigation elements.
  lv_obj_set_size(cont_page_number_info, 413, 64); // Set size.
  lv_obj_set_style_border_width(cont_page_number_info, 0, LV_PART_MAIN); // Remove border.
  lv_obj_set_style_pad_all(cont_page_number_info, 0, LV_PART_MAIN);      // Remove padding.
  lv_obj_set_flex_flow(cont_page_number_info, LV_FLEX_FLOW_ROW); // Set layout to a single row.
  lv_obj_set_flex_align(cont_page_number_info, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);
  lv_obj_clear_flag(cont_page_number_info, LV_OBJ_FLAG_SCROLLABLE); // Prevent scrolling.

  // Navigation buttons and page info label setup
  lv_obj_t *img_left_page, *img_right_page; // Declarations for images inside nav buttons.
  // Create the Previous Page button.
  create_nav_button(cont_page_number_info, &btn_page_prev, &img_left_page, &icon_leftSide_btn);
  // Attach the previous page click callback.
  lv_obj_add_event_cb(btn_page_prev, prev_page_btn_click_cb, LV_EVENT_CLICKED, NULL);

  lbl_page_info = lv_label_create(cont_page_number_info); // Create the "X of Y" page info label.
  lv_obj_set_size(lbl_page_info, LV_SIZE_CONTENT, LV_SIZE_CONTENT); // Size content automatically.
  lv_obj_set_style_text_font(lbl_page_info, &opensans_regular_26,
                             LV_PART_MAIN | LV_STATE_DEFAULT); // Set font.
  lv_obj_set_style_text_color(lbl_page_info, lv_color_hex(VST_COLOR_INACTIVE_TEXT),
                              LV_PART_MAIN); // Set text color.

  // Create the Next Page button.
  create_nav_button(cont_page_number_info, &btn_page_next, &img_right_page, &icon_rightSide_btn);
  // Attach the next page click callback.
  lv_obj_add_event_cb(btn_page_next, next_page_btn_click_cb, LV_EVENT_CLICKED, NULL);
}

// --- Public Getter ---
lv_obj_t *
get_audio_language_settings_screen() { // Public function to get or create the screen object.
  if (screen == NULL) {                // Check if the screen has been initialized.
    screen_init();                     // Initialize the screen if it hasn't been.
  }
  return screen; // Return the screen object pointer.
}