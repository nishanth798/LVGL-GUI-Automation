#include "ui.h"
#include <string.h>

#define MSG_COLOR 0xD9D9D9

// todo: deactivate message needs to be in rich text form.
// The text "DEACTIVATE" needs to be in bold while the rest of
// the message in regular font.
lv_obj_t *mbox;
lv_obj_t *stdwn_box;

lv_obj_t *msgbox_confirm_create(const char *btns[], const char *msg_text, uint32_t btn_col) {
  // create message box
  mbox = lv_msgbox_create(NULL);
  lv_msgbox_add_title(mbox, "");
  lv_msgbox_add_text(mbox, msg_text);
  // lv_obj_t *close_btn = lv_msgbox_add_close_button(mbox);

  lv_obj_t *mbtn1 = lv_msgbox_add_footer_button(mbox, btns[0]); //"Don't Save" button
  lv_obj_t *mbtn2 = lv_msgbox_add_footer_button(mbox, btns[1]); //"Save" button

  lv_obj_set_width(mbox, 360);
  lv_obj_set_height(mbox, LV_SIZE_CONTENT);
  //   lv_obj_set_style_pad_top(mbox,-5,LV_PART_MAIN);
  lv_obj_set_style_pad_bottom(mbox, 16, LV_PART_MAIN);
  lv_obj_set_style_radius(mbox, 11, LV_PART_MAIN);
  lv_obj_center(mbox);
  lv_obj_clear_flag(mbox, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_flex_flow(mbox, LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_flex_align(mbox, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  lv_obj_set_style_pad_row(mbox, 5, LV_PART_MAIN);

  // buttons styling
  lv_obj_set_size(mbtn1, 150, 50);
  lv_obj_set_style_text_font(mbtn1, &opensans_bold_18, LV_PART_MAIN);
  lv_obj_set_style_bg_color(mbtn1, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_border_width(mbtn1, 3, LV_PART_MAIN);
  lv_obj_set_style_border_color(mbtn1, lv_color_hex(VST_COLOR_DARKBLUE), LV_PART_MAIN);
  lv_obj_set_style_text_color(mbtn1, lv_color_hex(VST_COLOR_DARKBLUE), LV_PART_MAIN);

  lv_obj_set_size(mbtn2, 150, 50);
  lv_obj_set_style_text_font(mbtn2, &opensans_bold_18, LV_PART_MAIN);
  lv_obj_set_style_bg_color(mbtn2, lv_color_hex(btn_col), LV_PART_MAIN);
  lv_obj_set_style_border_width(mbtn2, 3, LV_PART_MAIN);
  lv_obj_set_style_border_color(mbtn2, lv_color_hex(btn_col), LV_PART_MAIN);
  lv_obj_set_style_text_color(mbtn2, lv_color_white(), LV_PART_MAIN);

  lv_obj_t *msg_header = lv_msgbox_get_header(mbox);
  lv_obj_set_style_bg_color(msg_header, lv_color_white(), LV_PART_MAIN);

  lv_obj_t *msg_footer = lv_msgbox_get_footer(mbox);
  lv_obj_set_height(msg_footer, LV_SIZE_CONTENT);

  // style message text
  lv_obj_t *msg = lv_msgbox_get_content(mbox);
  lv_obj_set_size(msg, lv_pct(80), LV_SIZE_CONTENT);
  lv_obj_set_style_text_align(msg, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_set_style_text_font(msg, &opensans_regular_20, LV_PART_MAIN);

  // style close button
  //  lv_obj_set_style_bg_color(close_btn, lv_color_white(), LV_PART_MAIN);
  //  lv_obj_set_style_text_color(close_btn, lv_color_hex(MSG_COLOR), LV_PART_MAIN);
  //  lv_obj_set_style_shadow_width(close_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  //  lv_obj_set_style_text_font(close_btn, &lv_font_montserrat_30, LV_PART_MAIN);

  return mbox;
}

lv_obj_t *msgbox_confirm_save_create() {
  static const char *btns[] = {"Don't Save", "Save", ""};
  static const char *msg = "Go back without saving changes?";
  return msgbox_confirm_create(btns, msg, VST_COLOR_DARKBLUE);
}

void close_active_msgbox() {
  // Close regular message box if it's valid and active
  if (mbox && lv_obj_is_valid(mbox)) {
    lv_msgbox_close(mbox);
    mbox = NULL;
  }

  // Close shutdown message box if it's valid and active
  if (stdwn_box && lv_obj_is_valid(stdwn_box)) {
    lv_msgbox_close(stdwn_box);
    stdwn_box = NULL;
  }
}

lv_obj_t *msgbox_confirm_shutdown_create(const char *btns[], const char *msg_text,
                                         uint32_t btn_col) {
  stdwn_box = lv_msgbox_create(NULL);
  lv_msgbox_add_title(stdwn_box, "");

  // Check if message contains "DEACTIVATE" or "SHUT DOWN" for special formatting (uppercase only)
  const char *deactivate_pos = strstr(msg_text, "DEACTIVATE");
  const char *shutdown_pos = strstr(msg_text, "SHUT DOWN");

  lv_obj_t *mbtn1 = lv_msgbox_add_footer_button(stdwn_box, btns[0]); //"Don't Save" button
  lv_obj_t *mbtn2 = lv_msgbox_add_footer_button(stdwn_box, btns[1]); //"Save" button

  lv_obj_set_size(stdwn_box, 416, 290);
  lv_obj_center(stdwn_box);

  lv_obj_set_style_radius(stdwn_box, 11, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(stdwn_box, 1, LV_PART_MAIN);
  lv_obj_set_style_border_color(stdwn_box, lv_color_hex(VST_COLOR_CONTAINER_BORDER), LV_PART_MAIN);
  lv_obj_set_style_opa(stdwn_box, LV_OPA_100, LV_PART_MAIN);
  lv_obj_set_style_pad_all(stdwn_box, 0, LV_PART_MAIN);

  // Add shadow styling
  lv_obj_set_style_shadow_width(stdwn_box, 4, LV_PART_MAIN);
  lv_obj_set_style_shadow_opa(stdwn_box, 64, LV_PART_MAIN);
  lv_obj_set_style_shadow_offset_x(stdwn_box, 2, LV_PART_MAIN);
  lv_obj_set_style_shadow_offset_y(stdwn_box, 4, LV_PART_MAIN);
  lv_obj_set_style_shadow_color(stdwn_box, lv_color_black(), LV_PART_MAIN);

  lv_obj_clear_flag(stdwn_box, LV_OBJ_FLAG_SCROLLABLE); /// Flags
  lv_obj_set_flex_flow(stdwn_box, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(stdwn_box, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  // Get msgbox parts and apply padding/gap
  lv_obj_t *msg_header = lv_msgbox_get_header(stdwn_box);
  lv_obj_set_style_radius(msg_header, 11, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(msg_header, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_pad_all(msg_header, 0, LV_PART_MAIN);

  lv_obj_t *msg_content = lv_msgbox_get_content(stdwn_box);
  lv_obj_set_style_pad_top(msg_content, -3, LV_PART_MAIN);
  lv_obj_set_style_pad_bottom(msg_content, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_left(msg_content, 32, LV_PART_MAIN);
  lv_obj_set_style_pad_right(msg_content, 32, LV_PART_MAIN);

  lv_obj_t *msg_footer = lv_msgbox_get_footer(stdwn_box);
  lv_obj_set_height(msg_footer, LV_SIZE_CONTENT);
  lv_obj_set_style_pad_all(msg_footer, 32, LV_PART_MAIN);
  lv_obj_set_style_pad_gap(msg_footer, 10, LV_PART_MAIN);

  // buttons styling

  lv_obj_set_size(mbtn1, 168, 64);
  lv_obj_set_style_radius(mbtn1, 5, LV_PART_MAIN);
  lv_obj_set_style_bg_color(mbtn1, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_border_width(mbtn1, 3, LV_PART_MAIN);
  lv_obj_set_style_border_color(mbtn1, lv_color_hex(VST_COLOR_DARKBLUE), LV_PART_MAIN);
  lv_obj_set_style_text_color(mbtn1, lv_color_hex(VST_COLOR_DARKBLUE), LV_PART_MAIN);
  lv_obj_set_style_text_font(mbtn1, &opensans_bold_24, LV_PART_MAIN);

  lv_obj_set_size(mbtn2, 168, 64);
  lv_obj_set_style_radius(mbtn2, 5, LV_PART_MAIN);
  lv_obj_set_style_bg_color(mbtn2, lv_color_hex(btn_col), LV_PART_MAIN);
  lv_obj_set_style_text_color(mbtn2, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_text_font(mbtn2, &opensans_bold_24, LV_PART_MAIN);

  if (deactivate_pos || shutdown_pos) {
    // Create custom container for mixed font formatting - keep existing dimensions and styling
    lv_obj_clear_flag(msg_content, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_size(msg_content, lv_pct(100), 114);
    lv_obj_set_style_border_width(msg_content, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(msg_content, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(msg_content, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_text_align(msg_content, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(msg_content, 8, LV_PART_MAIN);
    lv_obj_set_style_text_line_space(msg_content, 8, LV_PART_MAIN);

    // Parse and create labels for different parts - only change font for special words
    const char *current_pos = msg_text;
    const char *end_pos = msg_text + strlen(msg_text);

    while (current_pos < end_pos) {
      // Find the next special word (only uppercase versions)
      const char *next_deactivate = strstr(current_pos, "DEACTIVATE");
      const char *next_shutdown = strstr(current_pos, "SHUT DOWN");

      // Choose the closest special word
      const char *next_special = NULL;
      size_t special_len = 0;
      bool is_shutdown = false;

      if (next_deactivate && (!next_special || next_deactivate < next_special)) {
        next_special = next_deactivate;
        special_len = 10; // "DEACTIVATE"
        is_shutdown = false;
      }
      if (next_shutdown && (!next_special || next_shutdown < next_special)) {
        next_special = next_shutdown;
        special_len = 9; // "SHUT DOWN"
        is_shutdown = true;
      }

      if (next_special) {
        // Add text before the special word
        if (next_special > current_pos) {
          lv_obj_t *lbl_before = lv_label_create(msg_content);
          size_t before_len = next_special - current_pos;
          char before_text[before_len + 1];
          strncpy(before_text, current_pos, before_len);
          before_text[before_len] = '\0';
          lv_label_set_text(lbl_before, before_text);
          lv_obj_set_style_text_font(lbl_before, &opensans_regular_28, LV_PART_MAIN);
          lv_obj_set_style_text_color(lbl_before, lv_color_black(), LV_PART_MAIN);
        }

        if (is_shutdown) {
          // Handle "SHUT DOWN" as two separate labels with line break
          // Add "SHUT" on second line
          lv_obj_t *lbl_shut = lv_label_create(msg_content);
          lv_label_set_text(lbl_shut, "SHUT");
          lv_obj_set_style_text_font(lbl_shut, &opensans_bold_28, LV_PART_MAIN);
          lv_obj_set_style_text_color(lbl_shut, lv_color_black(), LV_PART_MAIN);

          // Add "DOWN" on third line
          lv_obj_t *lbl_down = lv_label_create(msg_content);
          lv_label_set_text(lbl_down, "DOWN");
          lv_obj_set_style_text_font(lbl_down, &opensans_bold_28, LV_PART_MAIN);
          lv_obj_set_style_text_color(lbl_down, lv_color_black(), LV_PART_MAIN);
        } else {
          // Handle "DEACTIVATE" as single word with bold font
          lv_obj_t *lbl_special = lv_label_create(msg_content);
          lv_label_set_text(lbl_special, "DEACTIVATE");
          lv_obj_set_style_text_font(lbl_special, &opensans_bold_28, LV_PART_MAIN);
          lv_obj_set_style_text_color(lbl_special, lv_color_black(), LV_PART_MAIN);
        }

        current_pos = next_special + special_len;
      } else {
        // Add remaining text
        if (current_pos < end_pos) {
          lv_obj_t *lbl_remaining = lv_label_create(msg_content);
          lv_label_set_text(lbl_remaining, current_pos);
          lv_obj_set_style_text_font(lbl_remaining, &opensans_regular_28, LV_PART_MAIN);
          lv_obj_set_style_text_color(lbl_remaining, lv_color_black(), LV_PART_MAIN);
        }
        break;
      }
    }
  } else {
    // No special words found, use regular message box
    lv_msgbox_add_text(stdwn_box, msg_text);

    // Apply regular styling to the message content
    lv_obj_set_size(msg_content, lv_pct(100), 114);
    lv_obj_set_style_text_align(msg_content, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(msg_content, 20, LV_PART_MAIN);
    lv_obj_set_style_text_line_space(msg_content, 8, LV_PART_MAIN);
    lv_obj_set_style_text_color(msg_content, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_font(msg_content, &opensans_regular_28, LV_PART_MAIN);
  }
  return stdwn_box;
}
