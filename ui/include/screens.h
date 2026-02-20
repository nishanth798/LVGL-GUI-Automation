#ifndef _SCREENS_H
#define _SCREENS_H

#ifdef __cplusplus
extern "C" {
#endif

// clang-format off
#include "ui.h"
#include "assign_room.h"
// clang-format on
// Carry on formatting

typedef enum {
  SETTINGS_MODE_ACTIVATION,
  SETTINGS_MODE_SAVE,
} SETTINGS_MODE;

lv_obj_t *tmpl_settings_create_1(lv_obj_t *obj, const char *mainTitle, bool leftIcon);

void tmpl_settings_show_label_cancel(lv_obj_t *obj, bool show);

lv_obj_t *screen_settings_home_get();

void screen_settings_audio_set_bms(BMS bms);

// function to send bed width session data to NUC
void send_bed_width_session_data(uint8_t bed_width);
// function to send bed placement session data
void send_bed_placement_session_data(BED_POS bed_position);
// function to send occupant size session data
void send_occupant_size_session_data(OCC_SIZE occ_size);

void screen_settings_set_alerts_state(bool bed_alt, bool fall_alt, bool rep_alt);

void settings_show_mode(lv_obj_t *screen, lv_obj_t *btn_save, lv_obj_t *cont_btns_nav,
                        SETTINGS_MODE mode);

void tmpl_settings_label_set_clickable(lv_obj_t *obj, bool clickable);
lv_obj_t *get_img_left_arrow_obj();
lv_obj_t *get_cont_left_arrow_obj();
lv_obj_t *get_header_label_obj();
lv_obj_t *get_img_info_icon_obj();   // returns settings header info icon image obj
lv_obj_t *get_cont_info_icon_obj();  // returns settings header info icon container obj
lv_obj_t *get_cont_close_icon_obj(); // returns settings header close icon container

void screen_settings_set_mode(SETTINGS_MODE mode);

SETTINGS_MODE screen_settings_get_mode();

void clear_bms_sessiondata();
void clear_audio_sessiondata();
void clear_alerts_sessiondata();
void clear_occupant_size_sessiondata();
void clear_bed_placement_sessiondata();
void clear_bed_width_sessiondata();

// return true if the current active screen is system info screen, else false
bool is_current_page_system_info();
bool is_current_page_time_picker();
lv_obj_t *get_parent_of_system_info_screen();

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
