#ifndef _WIDGETS_H
#define _WIDGETS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ui.h"

// Button: base template for buttons used in the app

#define BTN_LABELS_ALREADY_SET -2;
#define BTN_LABELS_MISSING_ACTIVE -1;

typedef enum {
  BTN_STATE_ACTIVE,     // clickable (default)
  BTN_STATE_INACTIVE,   // not clickable
  BTN_STATE_DISABLED,   // not clickable, greyed out
  BTN_STATE_SUBMITTING, // not clickable, show spinner
  BTN_STATE_SUBMITTED,  // not clickable
  BTN_STATE_COUNT,
} BTN_STATE;

void btn_styles_init(); // needs to be called before calling btn_create
lv_obj_t *btn_create(lv_obj_t *obj, const char *text);
int8_t btn_set_state_label_text_map(lv_obj_t *obj, const char *texts[BTN_STATE_COUNT]);
int8_t btn_set_state_icon_map(lv_obj_t *obj, const lv_img_dsc_t *icons[BTN_STATE_COUNT]);
int8_t btn_set_state(lv_obj_t *obj, BTN_STATE state);
lv_obj_t *btn_get_label(lv_obj_t *obj);

lv_obj_t *btn_save_create_1(lv_obj_t *obj);
lv_obj_t *btn_secondary_create_1(lv_obj_t *obj, const char *text);
lv_obj_t *btn_primary_create_1(lv_obj_t *obj, const char *text, uint32_t color);

// Message Box (aka popup) for confirmation like save and deactivation
lv_obj_t *msgbox_confirm_create(const char *btns[], const char *msg_text, uint32_t btn_col);
lv_obj_t *msgbox_confirm_shutdown_create(const char *btns[], const char *msg_text,
                                         uint32_t btn_col);

// Radio selector for volume control and BMS
#define RADIO_SELECTOR_BTN_NONE 0xFF;
lv_obj_t *radio_selector_create_1(lv_obj_t *obj, const char *labels[], const lv_img_dsc_t *icons[]);
bool radio_selector_set_selected_btn_1(lv_obj_t *obj, int8_t btn_id);
int8_t radio_selector_get_selected_btn_1(lv_obj_t *obj);

// toast
lv_obj_t *toast_get(lv_obj_t *obj);
void label_toast_set_text(lv_obj_t *obj, const char *text);
void label_toast_show(lv_obj_t *obj);
void label_toast_hide(lv_obj_t *obj);

PAUSE widget_btnm_pause_get_selected_btn(lv_obj_t *obj);
MONITOR_MODE btnm_mode_get(lv_obj_t *obj);

void widget_status_panel_set_unset();

void toast_show(lv_obj_t *obj, const char *text);

void radio_selector_set_clickable(lv_obj_t *obj, bool clickable);

//
void alert_toast_show(); // to show alert toast message on settings screen
bool is_alert_toast_hidden();
void set_alert_toast_parent(lv_obj_t *parent_obj);
void alert_toast_hide();
// to update the message of alert toast when the display is in active home screen
void alert_toast_message_update();

void show_save_toast();
// Show bed width toast message when values are out of range(<44 or >80)
void show_bed_width_toast(const char *text);

// reposition alert timer in status panel
void set_reposition_alert_state(bool state, uint16_t pr_inj_tmr);
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
