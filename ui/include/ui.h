#ifndef _UI_H
#define _UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "api.h"
#include "exit_alert_schedule.h"
#include "fonts.h"
#include "images.h"
#include "lvgl.h"
#include "palette.h"
#include "screens.h"
#include "widgets.h"

// todo: font aliases for title, subtitile, normal text etc
// typedef void (*notification_cb)();

#define ACTIVATING 1

// SCREEN: Active Home
lv_obj_t *screen_home_active_get();
void screen_home_active_set_state();
void hide_no_response_toast();
// SCREEN: Settings
// lv_obj_t *screen_settings_get();

// SCREEN: Transient screens
lv_obj_t *screen_deactivating_get();
// lv_obj_t *scr_calibrating_get();
// lv_obj_t *scr_calibrating_chair_get();

lv_obj_t *get_custom_bed_width_screen(void);

// SCREEN Template: for transient screens
lv_obj_t *scr_trans_create();
void scr_trans_set_color(lv_obj_t *screen, lv_color_t color);
void scr_trans_set_title(lv_obj_t *screen, const char *title);
lv_obj_t *scr_trans_get_panel_main(lv_obj_t *screen);

// may be reduce scope to another h file
lv_obj_t *dot_add(lv_obj_t *parent);
lv_obj_t *three_dots_add(lv_obj_t *parent);
lv_obj_t *sym_chair_add(lv_obj_t *parent);

lv_obj_t *scr_home_inactive_get();

// SCREEN Template: settings
lv_obj_t *screen_tmpl_settings_create(lv_obj_t *parent, const char *subtitle);
lv_obj_t *screen_tmpl_settings_get_container_subtitle(lv_obj_t *tmpl);
lv_obj_t *screen_tmpl_settings_get_label_back(lv_obj_t *tmpl);
lv_obj_t *screen_tmpl_settings_get_label_cancel(lv_obj_t *tmpl);

// lv_obj_t *btn_save_create(lv_obj_t *obj);
lv_obj_t *btn_next_create(lv_obj_t *obj);
lv_obj_t *btn_previous_create(lv_obj_t *obj);
lv_obj_t *btn_activate_create(lv_obj_t *obj);

// SCREEN: settings bms
lv_obj_t *screen_settings_bms_get();

// SCREEN: settings audio
lv_obj_t *screen_settings_audio_get();
// set langauge selection flag. Used to fix volume and language reset issue
void set_language_selection_flag(bool flag);
void set_bed_width_selection_flag(bool flag);

// SCREEN: settings display
lv_obj_t *screen_settings_display_get();
void update_display_brightness_settings();
void turn_off_als(); // Used to turn off ALS and reset all its flags
// SCREEN: settings occupant size, bed placement position and bed width
lv_obj_t *screen_settings_get_occupant_size();
lv_obj_t *screen_settings_get_bed_placement();
lv_obj_t *screen_settings_get_bed_width();

// WIDGET: Monitor Mode Switch
lv_obj_t *widget_btnm_mode_add(lv_obj_t *screen);
void btnm_mode_set(lv_obj_t *obj, MONITOR_MODE mode);

// WIDGET: Pause Buttons
lv_obj_t *widget_btnm_pause_add(lv_obj_t *screen);

// WIDGET: BMS Level indicator
lv_obj_t *widget_bms_level_indicator_add(lv_obj_t *screen);
lv_obj_t *widget_bms_level_indicator_get();
void widget_bms_level_indicator_set(lv_obj_t *obj, BMS level);

// WIDGET: Status card
lv_obj_t *widget_status_panel_add(lv_obj_t *screen);
void widget_status_panel_set_monitoring(MONITOR_MODE mode);
void widget_status_panel_set_alert(ALERT alert, const char *alert_title);
void widget_status_panel_set_error(SYSERR err, const char *syserr_title);
void widget_status_panel_set_paused(uint16_t pause_tmr);
void widget_status_panel_set_calib(CALIBRATION cal);
void set_bms_level(BMS level);

// WIDGET: Confirmation message box (popup)
lv_obj_t *msgbox_confirm_deactivate_create();
lv_obj_t *msgbox_confirm_save_create();
void close_active_msgbox();

// WIDGET: radio_selector, base widget for bms selector and volume
// selector
lv_obj_t *radio_selector_create(lv_obj_t *parent, const lv_img_dsc_t *icons[],
                                const char *labels[]);
uint16_t radio_selector_get_selected_btn(const lv_obj_t *radio_selector);
void radio_selector_set_selected_btn(lv_obj_t *radio_selector, uint16_t btn_id);

// WIDGET: wait spinner for button
lv_obj_t *spinner_add_to_btn(lv_obj_t *btn);

// WIDGET: toast
lv_obj_t *label_toast_create(lv_obj_t *obj);
void label_toast_set_text(lv_obj_t *obj, const char *text);
void label_toast_show(lv_obj_t *obj);
void label_toast_hide(lv_obj_t *obj);

// For firmware upgrade status displaying, WIDGET: toast_firmware
void firmware_status_toast_hide();
void firmware_status_toast_show(lv_obj_t *obj, const char *text);
void set_firmmware_toast_parent(lv_obj_t *obj);

void ui_init(); // create ui.cpp, define ui_init();

// Read-only-disply
lv_obj_t *screen_read_only_get();

lv_obj_t *screen_variant_notset_get();

// screen setting alert
lv_obj_t *screen_settings_alerts_get();

// screen language and change width selection
lv_obj_t *get_audio_language_settings_screen();
lv_obj_t *get_change_width_settings_screen();

// Screen for system information
//  lv_obj_t *screen_system_info_get();
lv_obj_t *screen_system_info_get();

// Show or hide system info screen
//  where home_scr_name is the name of the parent screen to return to
void show_system_info_screen(char *home_scr_name);

void settings_audio_set_lang(LANG lang);
void settings_set_bed_width(uint8_t bed_width);


LANG settings_audio_get_lang();
uint8_t settings_get_bed_width();

typedef struct {
  char available_languages[50][21]; // Array of available language names (null-terminated)
  uint8_t available_language_count; // Number of languages in the array
  char selected_language[21];       // Name of the currently selected language (null-terminated)
  uint8_t selected_language_index;  // Index of the selected language in the array
} lang_info_t;

// Get the current language information
lang_info_t *lang_info_get();

// Clear the language information
void lang_info_clear(lang_info_t *lang_info);

// Set the language selection flag
extern bool flag_lang_selection;

// Handle language change timeout, if no response from NUC for language change request
void handle_language_change_timeout();

// Get the currently selected language string
char *get_selected_language();

// SCREEN: Activating screen
lv_obj_t *screen_activating_get();
// Set activating timeout
void set_activating_timeout();

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
