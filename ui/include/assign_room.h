#ifndef _ASSIGN_ROOMS_H
#define _ASSIGN_ROOMS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  ASSIGN_MODE_NONE,
  UNIT_SELECTION_MODE,
  ROOM_SELECTION_MODE,
} ASSIGN_MODE;

typedef struct {
  char rm_num_arr[100][11]; // 1 extra character for null termination.
  char selected_rm_num[11]; // 1 extra character for null termination.
  uint8_t rm_num_arr_idx;
  uint8_t rm_num_cnt;
} rm_num_info_t;

typedef struct {
  char unit_num_arr[100][11]; // 1 extra character for null termination.
  char selected_unit_num[11]; // 1 extra character for null termination.
  uint8_t unit_num_arr_idx;
  uint8_t unit_num_cnt;
} unit_num_info_t;

/*****************assign room*****************/
unit_num_info_t *unit_num_info_get();
void unit_num_info_clear(unit_num_info_t *);

rm_num_info_t *rm_num_info_get();
void rm_num_info_clear(rm_num_info_t *);

ASSIGN_MODE room_assign_get_mode();
void room_assign_set_mode(ASSIGN_MODE mode);

lv_obj_t *screen_settings_confirm_room_get();
lv_obj_t *screen_settings_assign_room_get();
lv_obj_t *screen_settings_select_room_get();

void close_one_btn_msg_box();
void create_oneBtn_msgbox(lv_obj_t *parent_obj, const lv_image_dsc_t *icon, const char *main_text,
                          const char *sub_text, const char *btn_text, const int cont_padding[],
                          const int btn_size[]);

void close_two_btn_msg_box();
void create_twoBtn_msgbox(lv_obj_t *parent_obj, const lv_image_dsc_t *icon, const char *main_text,
                          const char *sub_text, const char *btns_text[], const int cont_padding[],
                          const int btns_size[]);

void create_spinner_screen(lv_obj_t *parent_obj);
void close_spinner_screen();

/************Room view Check**********/
lv_obj_t *screen_settings_room_view_get();

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif