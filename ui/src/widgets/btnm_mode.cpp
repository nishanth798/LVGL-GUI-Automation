#include "sys_state.h"
#include "ui.h"
#include "widgets.h"

#define BLINK_DELAY 500 // in ms

// todo: switch to lvgl animation module for blinking instead of freertos task
// use outline to mark boundary of buttons instead of border

// this widget
lv_obj_t *widget_btnm_mode;

static lv_anim_t anim_blink;

// widget style
static const lv_coord_t border_width = 3;

static const char *btnm_map[] = {"Bed Mode", "Chair Mode", NULL};

static lv_style_t style_bg, style_btn;

static void style_bg_init(lv_style_t *style) {
  lv_style_init(style);
  lv_style_set_pad_left(style, 4);
  lv_style_set_pad_right(style, 4);
  lv_style_set_pad_top(style, 4);
  lv_style_set_pad_bottom(style, 4);
  lv_style_set_pad_gap(style, 0);
  lv_style_set_clip_corner(style, true);
  lv_style_set_border_width(style, border_width);
  lv_style_set_border_color(style, lv_color_hex(VST_COLOR_DARKBLUE));
}

static void style_btn_init(lv_style_t *style) {
  lv_style_init(style);
  lv_style_set_radius(style, 0);
}

static int get_checked_button(lv_obj_t *btnm)
{
   
    for (uint16_t i = 0; i < 2; i++) {
        if (lv_buttonmatrix_has_button_ctrl(btnm, i, LV_BUTTONMATRIX_CTRL_CHECKED)) {
            return i;  // This button is actually highlighted
        }
    }
 
    return LV_BUTTONMATRIX_BUTTON_NONE;  // No button highlighted
}
 
static void handle_invalid_response(lv_obj_t *obj)
{
    int32_t selected = get_checked_button(obj); // get real highlighted button
 
    if (selected == LV_BUTTONMATRIX_BUTTON_NONE) {
        // No button was previously highlighted → keep nothing highlighted
        lv_buttonmatrix_clear_button_ctrl_all(obj, LV_BUTTONMATRIX_CTRL_CHECKED);
        lv_buttonmatrix_set_selected_button(obj, LV_BUTTONMATRIX_BUTTON_NONE);
        return;
    }
 
    // One button was highlighted → keep it exactly as is
    lv_buttonmatrix_clear_button_ctrl_all(obj, LV_BUTTONMATRIX_CTRL_CHECKED);
    lv_buttonmatrix_set_button_ctrl(obj, selected, LV_BUTTONMATRIX_CTRL_CHECKED);
    lv_buttonmatrix_set_selected_button(obj, selected);
}

static void btnm_mode_toggle(lv_obj_t *obj, uint16_t btn_id) {
  bool checked = lv_buttonmatrix_has_button_ctrl(obj, btn_id, LV_BUTTONMATRIX_CTRL_CHECKED);
  if (checked) {
    lv_buttonmatrix_clear_button_ctrl(obj, btn_id, LV_BUTTONMATRIX_CTRL_CHECKED);
  } else {
    lv_buttonmatrix_set_button_ctrl(obj, btn_id, LV_BUTTONMATRIX_CTRL_CHECKED);
  }
}

static int16_t mode_to_btn_id(MONITOR_MODE mode) {
  switch (mode) {
  case MODE_BED:
    return 0;
  case MODE_CHAIR:
    return 1;
  default:
    return -1;
  }
}
static void mode_set(lv_obj_t *obj, MONITOR_MODE mode) {
  lv_buttonmatrix_set_selected_button(obj, LV_BUTTONMATRIX_BUTTON_NONE);
    lv_buttonmatrix_clear_button_ctrl_all(obj, LV_BUTTONMATRIX_CTRL_CHECKED);
 
    // If mode is valid, apply highlight to that button
    if (mode == MODE_BED || mode == MODE_CHAIR) {
        int btn_id = mode_to_btn_id(mode);
        lv_buttonmatrix_set_selected_button(obj, btn_id);
        lv_buttonmatrix_set_button_ctrl(obj, btn_id, LV_BUTTONMATRIX_CTRL_CHECKED);
    }
}


static MONITOR_MODE btnm_mode_btn_id_to_mode(uint16_t btn_id) {
  switch (btn_id) {
  case 0:
    return MODE_BED;
  case 1:
    return MODE_CHAIR;
  default:
    return MODE_UNSET;
  }
}

// Create a JSON-RPC request
// static void jsonrpc_create(uint16_t btn_id) {
//   CMD cmd;
//   MONITOR_MODE btn_clicked = btnm_mode_btn_id_to_mode(btn_id);
//   switch (btn_clicked) {
//   case MODE_BED:
//     cmd = CMD_ACTIVATE_BED_MODE;
//     break;
//   case MODE_CHAIR:
//     cmd = CMD_ACTIVATE_CHAIR_MODE;
//     break;
//   default:
//     return;
//     break;
//   }

//   JsonObject params;
//   serializeJsonRpcRequest(0, cmd_names[cmd], params);
// }

static void anim_blink_cb(void *var, int32_t v) {
  lv_obj_t *obj = (lv_obj_t *)var;
  uint16_t btn_id = lv_buttonmatrix_get_selected_button(obj);
  btnm_mode_toggle(obj, btn_id);
}

void anim_blink_init(lv_obj_t *obj, lv_anim_t *a) {
  lv_anim_init(a);
  lv_anim_set_var(a, obj);
  lv_anim_set_values(a, 0, 1);
  lv_anim_set_time(a, BLINK_DELAY);
  lv_anim_set_exec_cb(a, anim_blink_cb);
  lv_anim_set_repeat_count(a, LV_ANIM_REPEAT_INFINITE);
  lv_anim_set_repeat_delay(a, BLINK_DELAY);
}

void start_blinking() { lv_anim_start(&anim_blink); }

bool stop_blinking()
{
    // Stop blinking animation
    lv_anim_del(widget_btnm_mode, anim_blink_cb);
 
    sys_state_t *ss = sys_state_get();
 
    if (ss->mode == MODE_BED) {
        mode_set(widget_btnm_mode, MODE_BED);
    }
    else if (ss->mode == MODE_CHAIR) {
        mode_set(widget_btnm_mode, MODE_CHAIR);
    }
    else {
        handle_invalid_response(widget_btnm_mode);
    }
 
    return true;
}
 

// monitor mode button matrix event handler, event bubbled up,
//  to this home screen from the mode button matrix widget
void btnm_mode_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target_obj(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    uint32_t id = lv_buttonmatrix_get_selected_button(obj);
    const char *txt = lv_buttonmatrix_get_button_text(obj, id);
    // LV_LOG_USER("%s was pressed\n", txt);
    // jsonrpc_create(id);
    start_blinking();
  }
}

static void widget_btnm_mode_init(lv_obj_t *parent) {
  // create and configure button matrix
  widget_btnm_mode = lv_buttonmatrix_create(parent);
  lv_buttonmatrix_set_map(widget_btnm_mode, btnm_map);
  lv_buttonmatrix_set_button_ctrl_all(widget_btnm_mode, LV_BUTTONMATRIX_CTRL_CHECKABLE);
  // bubble click events to parent screen
  lv_obj_add_flag(widget_btnm_mode, LV_OBJ_FLAG_EVENT_BUBBLE);

  // apply styles
  style_bg_init(&style_bg);
  lv_obj_add_style(widget_btnm_mode, &style_bg, LV_PART_MAIN | LV_STATE_DEFAULT);
  style_btn_init(&style_btn);
  lv_obj_add_style(widget_btnm_mode, &style_btn, LV_PART_ITEMS | LV_STATE_DEFAULT);

  lv_obj_set_style_shadow_width(widget_btnm_mode, 0, LV_PART_ITEMS | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(widget_btnm_mode, 2, LV_PART_ITEMS | LV_STATE_DEFAULT);

  lv_obj_set_style_bg_color(widget_btnm_mode, lv_color_hex(VST_COLOR_DARKBLUE),
                            LV_PART_ITEMS | LV_STATE_CHECKED);
  lv_obj_set_style_bg_opa(widget_btnm_mode, 255, LV_PART_ITEMS | LV_STATE_CHECKED);
  lv_obj_set_style_bg_color(widget_btnm_mode, lv_color_white(), LV_PART_ITEMS | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(widget_btnm_mode, 255, LV_PART_ITEMS | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(widget_btnm_mode, &opensans_semibold_24,
                             LV_PART_ITEMS | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(widget_btnm_mode, &opensans_bold_24, LV_PART_ITEMS | LV_STATE_CHECKED);
  lv_obj_set_style_text_color(widget_btnm_mode, lv_color_hex(VST_COLOR_DARKBLUE),
                              LV_PART_ITEMS | LV_STATE_DEFAULT);
  lv_obj_set_style_text_color(widget_btnm_mode, lv_color_white(), LV_PART_ITEMS | LV_STATE_CHECKED);

  lv_obj_set_align(widget_btnm_mode, LV_ALIGN_CENTER);

  // placement
  lv_obj_set_size(widget_btnm_mode, 352, 80);
  // lv_obj_set_pos(widget_btnm_mode, 16, 16);

  lv_obj_add_event_cb(widget_btnm_mode, btnm_mode_cb, LV_EVENT_VALUE_CHANGED, NULL);

  anim_blink_init(widget_btnm_mode, &anim_blink);
}

lv_obj_t *widget_btnm_mode_add(lv_obj_t *screen) {
  if (widget_btnm_mode == NULL) {
    widget_btnm_mode_init(screen);
  }
  return widget_btnm_mode;
}

void btnm_mode_set(lv_obj_t *obj, MONITOR_MODE mode) {
  if (mode == MODE_BED || mode == MODE_CHAIR || mode == MODE_UNSET || mode == MODE_SCHEDULED_MON || mode == MODE_PRESSURE_INJURY || mode == MODE_FALL_MON) {
    if (stop_blinking()) {
      // LV_LOG_USER("stopped animation");
    }
  }
  if (mode == MODE_SCHEDULED_MON || mode == MODE_PRESSURE_INJURY || mode == MODE_FALL_MON) {
    lv_buttonmatrix_clear_button_ctrl_all(widget_btnm_mode, LV_BUTTONMATRIX_CTRL_CHECKABLE);  
  }
}

MONITOR_MODE btnm_mode_get(lv_obj_t *obj) {
  uint32_t id = lv_buttonmatrix_get_selected_button(obj);
  return (MONITOR_MODE)(id + 1);
}
