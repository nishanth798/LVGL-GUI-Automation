#include "app.h"
#include "api.h"
#include "device_info.h"
#include "jsonrpc2.h"
#include "lvgl.h"
#include "screens.h"
#include "sys_state.h"
#include "ui.h"

#include <cstddef>

static lv_timer_t *user_workflow_idle_timer;

/*
 todo:
 RESTRUCTURE for better organization.

- adopt lvgl way of creating widgets.
- add toast for not implemented message, update received from nuc etc.
- name all magic numbers

Flow:
6. render/refresh display
    1.select screen/view
    2. view renders itself picking data from model, it morphs itself
    depending on model data

arduino-cli based build possible for CI.
run with warnings to look for bugs, static analysis etc
tests possible?
gui tool to test
configurable logging and read timeout
error toast
make toast, app wide global, so it can be attached to any active screen and
shown, or should we just create and destroy one?

7. wait for usb serial connection to start the display, show a waiting for host
screen or computer disconnected screen.
- can we distinguish between usb serial not opened by host, vs port opened but
no response.
- instead of loading screen, we can say waiting for host/nuc to connect


- utilize implementation related error code and add additional context using
data field in the error object.

- arduino compile, see if its building unwanted code files.

- set display config separately
set_disp: brightness, abc

make timeout configurable.

set_conf: timeout,

spinner, fix color

check if members of memallocated structs are initialized

account for unset values, how widgets would display themselves.

arduino-cli compile --output-dir build/ -e --optimize-for-debug --warnings
"none" --build-property "compiler.cpp.extra_flags=-DLV_CONF_INCLUDE_SIMPLE=1
-I./" --build-property "compiler.c.extra_flags=-DLV_CONF_INCLUDE_SIMPLE=1 -I./"

arduino-cli upload --input-dir build/ --discovery-timeout 5s

*/

void process_inactivity() {
  lv_obj_t *screen;

  screen = lv_screen_active();

  /* if active screen is already a home screen, then do nothing */
  if (screen == screen_get(SCREEN_HOME_ACTIVE) || screen == screen_get(SCREEN_DEACTIVATING) ||
      screen == screen_get(SCREEN_VARIANT_NOTSET) || screen == screen_get(SCREEN_ACTIVATING)) {
    return;
  } else {
    if (screen_settings_get_mode() == SETTINGS_MODE_ACTIVATION ||
        screen == screen_get(SCREEN_HOME_INACTIVE)) {
      screen = scr_home_inactive_get();

    } else if (screen_settings_get_mode() == SETTINGS_MODE_SAVE) {
      screen = screen_home_active_get();
    }

    // Close any active message boxes before going to home screen
    close_active_msgbox();

    lv_screen_load(screen); // load home screen
  }
}

void app_init() {
  jsonrpc_init();

  init_dev_info(); // initializes the device info structure with appropriate data
  // todo: move backlight initialization from ui.ino to app.cpp
  //  initlize backlight
  //  backlight_init(BACKLIGHT_PIN);
  //  disp_state_set_brightness(BRIGHTNESS_DEFAULT);

  lv_obj_t *screen;
  // screen = lv_obj_create(NULL);
  // lv_obj_set_size(screen, lv_pct(100), lv_pct(100));
  // lv_obj_set_style_pad_top(screen, 0, LV_PART_MAIN);

  // screen = screen_get(SCREEN_HOME_INACTIVE);
  //  screen = screen_get(SCREEN_HOME_ACTIVE);

  // screen = screen_home_active_get();
  // lv_obj_t label_toast_create();
  // screen = screen_settings_bms_get();
  // screen = screen_settings_audio_get();
  // screen = scr_calibrating_chair_get();
  // lv_obj_t *btn_save = btn_save_create(screen);
  // btn_save_set_state(btn_save, BTN_SAVE_STATE_SAVED);
  // screen = screen_settings_display_get();
  btn_styles_init();
  // screen = screen_settings_home_get();

  // screen = screen_settings_room_view_get();
  // screen = screen_read_only_get();
  screen = screen_variant_notset_get();
  lv_screen_load(screen);
}
