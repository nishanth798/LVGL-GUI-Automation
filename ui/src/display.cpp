#ifndef SIMULATOR

#include "display.h"
#include "stdint.h"

#if SENSCAP_EN
#include <PCA95x5.h>
#include <Wire.h>

// --- Panel I/O expander on SenseCAP D1 ---
#define PANEL_I2C_SDA 39
#define PANEL_I2C_SCL 40
#define PANEL_IOEX_ADDR 0x20

static PCA9535 panel_ioex;
#endif

#if !SENSCAP_EN
Arduino_DataBus *bus = new Arduino_SWSPI(GFX_NOT_DEFINED /* DC */, 1 /* CS */, 12 /* SCK */,
                                         11 /* MOSI */, GFX_NOT_DEFINED /* MISO */);

Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
    45 /* DE */, 4 /* VSYNC */, 5 /* HSYNC */, 21 /* PCLK */, 39 /* R0 */, 40 /* R1 */, 41 /* R2 */,
    42 /* R3 */, 2 /* R4 */, 0 /* G0/P22 */, 9 /* G1/P23 */, 14 /* G2/P24 */, 47 /* G3/P25*/,
    48 /* G4/P26 */, 3 /* G5 */, 6 /* B0 */, 7 /* B1 */, 15 /* B2 */, 16 /* B3 */, 8 /* B4 */,
    1 /* hsync_polarity */, 10 /* hsync_front_porch */, 8 /* hsync_pulse_width */,
    50 /* hsync_back_porch */, 1 /* vsync_polarity */, 10 /* vsync_front_porch */,
    8 /* vsync_pulse_width */, 20 /* vsync_back_porch */);

Arduino_RGB_Display *gfx = new Arduino_RGB_Display(
    480 /* width */, 480 /* height */, rgbpanel, 0 /* rotation */, true /* auto_flush */, bus,
    GFX_NOT_DEFINED /* RST */, st7701_type1_init_operations, sizeof(st7701_type1_init_operations));

#else

// To avoid the display getting stuck in reset state, we need to do a panel reset sequence before initializing the display.
static void panel_reset_pre_init() {
  Wire.begin(PANEL_I2C_SDA, PANEL_I2C_SCL);
  panel_ioex.attach(Wire, PANEL_IOEX_ADDR);
  // Make CS (P04) and RST (P05) outputs
  panel_ioex.direction(PCA95x5::Port::P04, PCA95x5::Direction::OUT);
  panel_ioex.direction(PCA95x5::Port::P05, PCA95x5::Direction::OUT);

  panel_ioex.write(PCA95x5::Port::P04, PCA95x5::Level::H);
  panel_ioex.write(PCA95x5::Port::P05, PCA95x5::Level::L);
  delay(80);
  panel_ioex.write(PCA95x5::Port::P05, PCA95x5::Level::H);
  delay(320);
  panel_ioex.write(PCA95x5::Port::P04, PCA95x5::Level::L);
}

// panel_release_cs_post_init` will be called in `my_lv_disp_init` after `gfx->begin()` to release CS back to idle state,
// so that display can receive commands/data from the driver.
static void panel_release_cs_post_init() {
  // After gfx->begin() finishes, release CS back to idle
  panel_ioex.write(PCA95x5::Port::P04, PCA95x5::Level::H);
}

Arduino_DataBus *bus = new Arduino_SWSPI(GFX_NOT_DEFINED /* DC */, PCA95x5::Port::P04 /* CS */,
                                         41 /* SCK */, 48 /* MOSI */, GFX_NOT_DEFINED /* MISO */);

Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
    18 /* DE */, 17 /* VSYNC */, 16 /* HSYNC */, 21 /* PCLK */, 4 /* R0 */, 3 /* R1 */, 2 /* R2 */,
    1 /* R3 */, 0 /* R4 */, 10 /* G0/P22 */, 9 /* G1/P23 */, 8 /* G2/P24 */, 7 /* G3/P25*/,
    6 /* G4/P26 */, 5 /* G5 */, 15 /* B0 */, 14 /* B1 */, 13 /* B2 */, 12 /* B3 */, 11 /* B4 */,
    1
    /* hsync_polarity */,
    10 /* hsync_front_porch */, 8 /* hsync_pulse_width */, 50 /* hsync_back_porch */,
    1 /* vsync_polarity */, 10 /* vsync_front_porch */, 8 /* vsync_pulse_width */,
    20 /* vsync_back_porch */);

Arduino_RGB_Display *gfx =
    new Arduino_RGB_Display(480 /* width */, 480 /* height */, rgbpanel, 0 /* rotation */,
                            true /* auto_flush */, bus, PCA95x5::Port::P05 /* RST */,
                            st7701_type1_init_operations, sizeof(st7701_type1_init_operations));

#endif // #if !SENSCAP_EN

lv_display_t *display;

/* Display flushing */
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

#if (LV_COLOR_16_SWAP != 0)
  gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)(px_map), w, h);
#else
  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)(px_map), w, h);
#endif

  lv_disp_flush_ready(disp);
}

#define BYTE_PER_PIXEL                                                                             \
  (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565)) /*will be 2 for RGB565                        \
                                                      */

void my_lv_disp_init() {
#if SENSCAP_EN
  panel_reset_pre_init();
#endif

  display = lv_display_create(MY_DISP_HOR_RES, MY_DISP_VER_RES);

  // Allocate display buffers in PSRAM to save internal RAM
  // Each buffer is large enough for 480 lines which is equal to the full height of the display
  int32_t buf_lines = 480;
  int32_t buf_size = MY_DISP_HOR_RES * buf_lines * BYTE_PER_PIXEL;

  uint8_t *buf1 =
      (uint8_t *)heap_caps_aligned_alloc(4, buf_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if (buf1 == NULL) {
    printf("buf1 allocation failed\n");
  }

  uint8_t *buf2 =
      (uint8_t *)heap_caps_aligned_alloc(4, buf_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if (buf2 == NULL) {
    printf("buf2 allocation failed\n");
  }

  // printf("Internal Heap info after lvgl display buffers allocation:\n");
  // heap_caps_print_heap_info(MALLOC_CAP_INTERNAL);
  // printf("External Heap info after lvgl display buffers allocation:\n");
  // heap_caps_print_heap_info(MALLOC_CAP_SPIRAM);

  lv_display_set_buffers(display, buf1, buf2, buf_size, LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_flush_cb(display, my_disp_flush);

  // Init Display
  gfx->begin();
  gfx->fillScreen(WHITE);

#if SENSCAP_EN
  panel_release_cs_post_init();
#endif
}

#endif // #ifndef SIMULATOR