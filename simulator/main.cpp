
/**
 * @file main
 *
 */
/*********************
 *      INCLUDES
 *********************/
#define _DEFAULT_SOURCE /* needed for usleep() */
#include "app.h"
#include "jsonrpc2.h"
#include "lv_drv_conf.h"
#include "lvgl.h"
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

/*********************
 *      DEFINES
 *********************/
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void hal_init(int32_t w, int32_t h);
static void hal_deinit(void);
static void *tick_thread(void *data);

/**********************
 *  STATIC VARIABLES
 **********************/
static pthread_t thr_tick;    /* thread */
static bool end_tick = false; /* flag to terminate thread */

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *      VARIABLES
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

int main(int argc, char **argv) {
  (void)argc; /*Unused*/
  (void)argv; /*Unused*/

  /*Initialize LVGL*/
  lv_init();

  /*Initialize the HAL (display, input devices, tick) for LVGL*/
  hal_init(480, 480);

  //  lv_example_switch_1();
  //  lv_example_calendar_1();
  //  lv_example_btnmatrix_2();
  //  lv_example_checkbox_1();
  //  lv_example_colorwheel_1();
  //  lv_example_chart_6();
  //  lv_example_table_2();
  //  lv_example_scroll_2();
  //  lv_example_textarea_1();
  //  lv_example_msgbox_1();
  //   lv_example_dropdown_2();
  //  lv_example_btn_1();
  //  lv_example_scroll_1();
  //  lv_example_tabview_1();
  //  lv_example_tabview_1();
  //  lv_example_flex_3();
  //  lv_example_label_1();

  // lv_demo_widgets();
  //  lv_demo_keypad_encoder();
  //  lv_demo_benchmark();
  //  lv_demo_stress();
  //  lv_demo_music();

  //  app_init();

  // lv_obj_t * src1 = lv_scr_act(); // Get the active screen
  // cap(src1,"A:/home/mahesh/snapshot5.png");
  // capture_widget_snap();

  app_init();

  while (1) {
    /* Periodically call the lv_task handler.
     * It could be done in a timer interrupt or an OS task too.*/
    lv_timer_handler();
    process_request(100);
    usleep(5 * 1000);
  }
  hal_deinit();
  return 0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Initialize the Hardware Abstraction Layer (HAL) for the LVGL graphics
 * library
 */
static void hal_init(int32_t w, int32_t h) {
  lv_group_set_default(lv_group_create());

  lv_display_t *disp = lv_sdl_window_create(w, h);

  lv_indev_t *mouse = lv_sdl_mouse_create();
  lv_indev_set_group(mouse, lv_group_get_default());
  lv_indev_set_display(mouse, disp);
  lv_display_set_default(disp);

  LV_IMAGE_DECLARE(mouse_cursor_icon); /*Declare the image file.*/
  lv_obj_t *cursor_obj;
  cursor_obj = lv_image_create(lv_screen_active()); /*Create an image object for the cursor */
  lv_image_set_src(cursor_obj, &mouse_cursor_icon); /*Set the image source*/
  lv_indev_set_cursor(mouse, cursor_obj);           /*Connect the image  object to the driver*/

  lv_indev_t *mousewheel = lv_sdl_mousewheel_create();
  lv_indev_set_display(mousewheel, disp);

  lv_indev_t *keyboard = lv_sdl_keyboard_create();
  lv_indev_set_display(keyboard, disp);
  lv_indev_set_group(keyboard, lv_group_get_default());

  // return disp;
}

/**
 * Releases the Hardware Abstraction Layer (HAL) for the LVGL graphics library
 */
static void hal_deinit(void) {
  end_tick = true;
  pthread_join(thr_tick, NULL);

#if USE_SDL
  // nop
#elif USE_X11
  lv_x11_deinit();
#endif
}

/**
 * A task to measure the elapsed time for LVGL
 * @param data unused
 * @return never return
 */
static void *tick_thread(void *data) {
  (void)data;

  while (!end_tick) {
    usleep(5000);
    lv_tick_inc(5); /*Tell LittelvGL that 5 milliseconds were elapsed*/
  }

  return NULL;
}
