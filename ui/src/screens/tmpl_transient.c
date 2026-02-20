#include "ui.h"

// Template screen for transient screens such as Deactivating, Calibrating,
// Calibrating Chair

#define PANEL_TITLE_HEIGHT 14 // in px (64px in design, 13% is 62px)

lv_obj_t *scr_trans_create() {
  lv_obj_t *screen = lv_obj_create(NULL); // main screen object

  lv_obj_t *cont_deactivate_scr = lv_obj_create(screen); // main container
  lv_obj_set_size(cont_deactivate_scr, 448, 448); // Set screen size to 448x448
  lv_obj_set_align(cont_deactivate_scr, LV_ALIGN_CENTER);
  lv_obj_clear_flag(cont_deactivate_scr, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_flex_flow(cont_deactivate_scr, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(cont_deactivate_scr, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
 
  //  child0 of screen
  lv_obj_t *panel_title = lv_obj_create(cont_deactivate_scr);
  lv_obj_set_width(panel_title, 442);
  lv_obj_set_height(panel_title, 78);
  lv_obj_clear_flag(panel_title, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_align(panel_title, LV_ALIGN_TOP_MID);
  lv_obj_set_style_border_width(panel_title, 3, LV_PART_MAIN | LV_STATE_DEFAULT);

  //  child1 of screen
  lv_obj_t *panel_main = lv_obj_create(cont_deactivate_scr);
  lv_obj_set_width(panel_main, lv_pct(100));
  lv_obj_set_height(panel_main, lv_pct(100 - PANEL_TITLE_HEIGHT));
  lv_obj_clear_flag(panel_main, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_style_border_width(panel_main, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(panel_main, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

  // child0 of, panel_title
  lv_obj_t *label_title = lv_label_create(panel_title);
  lv_obj_set_width(label_title, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_height(label_title, LV_SIZE_CONTENT); /// 1
  lv_obj_set_align(label_title, LV_ALIGN_CENTER);
  lv_obj_set_style_text_font(label_title, &opensans_bold_32, LV_PART_MAIN | LV_STATE_DEFAULT); 

  return cont_deactivate_scr;
}

static lv_obj_t *panel_title_get(lv_obj_t *screen) { return lv_obj_get_child(screen, 0); }

static lv_obj_t *label_title_get(lv_obj_t *screen) {
  lv_obj_t *panel_title = panel_title_get(screen);
  return lv_obj_get_child(panel_title, 0);
}

void scr_trans_set_color(lv_obj_t *screen, lv_color_t color) {
  lv_obj_set_style_bg_color(screen, color, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_t *panel_title = panel_title_get(screen);
  lv_obj_set_style_border_color(panel_title, color, LV_PART_MAIN | LV_STATE_DEFAULT);
}

void scr_trans_set_title(lv_obj_t *screen, const char *title) {
  lv_obj_t *label_title = label_title_get(screen);
  lv_label_set_text(label_title, title);
}

lv_obj_t *scr_trans_get_panel_main(lv_obj_t *screen) { return lv_obj_get_child(screen, 1); }