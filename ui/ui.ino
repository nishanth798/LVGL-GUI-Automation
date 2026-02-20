// clang-format off
#include "app.h"
#include "backlight.h"
#include "display.h"
#include "esp_task_wdt.h" // Watchdog timer functions
#include "jsonrpc2.h"
#include "lvgl.h"
#include "resp_state.h"
#include "touch.h"
#include "ui.h"
#include <Arduino_GFX_Library.h>
#include "esp_log.h"
#include "virc.h"
#include "device_info.h"
#include "disp_state.h"

// clang-format on
// Carry on formatting

#define IWDG_TIMEOUT_MS 10 // Watchdog timeout in seconds. Range(1-60)

#define SERIAL_TIMEOUT 1000

#if !SENSCAP_EN
#define BACKLIGHT_PIN 44
TaskHandle_t process_virc_task_handle = NULL;
#else
#define BACKLIGHT_PIN 45
#endif

// process request task handle
TaskHandle_t process_request_task_handle = NULL;

#define SERIAL_DATA_SIZE 1000

// char *serDataBuf;
char serDataBuf[SERIAL_DATA_SIZE]; // 1000

uint32_t alsLoopCnt = 0; // count variable used to process als data

void watchdog_init() {

  LV_LOG_USER("Watchdog timer inside initialisation");

  esp_err_t err;
  // Enable watchdog timer for task scheduling
  err = esp_task_wdt_init(IWDG_TIMEOUT_MS, true);
  if (err == ESP_OK) {
    LV_LOG_USER("Watchdog timer initialized with timeout %d sec", IWDG_TIMEOUT_MS);

    // Register the current thread as a watchdog timer task
    err = esp_task_wdt_add(NULL);
    if (err == ESP_OK) {
      LV_LOG_USER("Watchdog timer task registered");

    } else {
      LV_LOG_USER("Watchdog timer task registration failed with error = %x", err);
    }
  } else {
    LV_LOG_USER("Watchdog timer initialization failed with error = %x", err);
  }
}

void watchdog_feed() {
  esp_err_t err;

  // Reset the watchdog timer
  err = esp_task_wdt_reset();
  if (err != ESP_OK) {
    LV_LOG_USER("Watchdog timer reset failed with error = %x", err);
  }
}

void setup() {
  // Set log level to ESP_LOG_NONE to disable all logging
  esp_log_level_set("*", ESP_LOG_NONE);

  ESP_LOGD("Main", "begin");
  SERIAL_PORT.setRxBufferSize(1024);
  SERIAL_PORT.begin(115200);
  SERIAL_PORT.setTimeout(SERIAL_TIMEOUT);
  // delay(1000);

  //   while (!SERIAL_PORT) {
  //     LV_LOG_USER("Waiting for serial port to connect");
  //     delay(100);
  //   }

  // get the esp reatart reason and print it out
  // SERIAL_PORT.println("reason for esp restart()");
  // SERIAL_PORT.println(esp_reset_reason());

  lv_init();         // lvgl initialization
  my_lv_disp_init(); // display initialization

#if !SENSCAP_EN
  // make gpio 13 HW_VER_CNTRL_PIN as internal pullup and read the value
  pinMode(HW_VER_CNTRL_PIN, INPUT_PULLUP);
  int val = digitalRead(HW_VER_CNTRL_PIN); // pin will be high for hardware version v1.0.2 and
                                           // below, low for v1.0.3 and above

  // Set the correct INT pin based on hardware version
  if (val == HIGH) {
    TOUCH_GT911_INT = -1; // pin number for Hardware version v1.0.2 and below
  } else {
    TOUCH_GT911_INT = 43; // pin number for Hardware version v1.0.3
  }
#endif

  // scan i2c devices and print the i2c device slave addresses
  scan_i2c_devices(5000);
  // Initialize touch after setting the correct INT pin
  touch_init();

  // Initialize backlight and set brightness to 80%
  backlight_init(BACKLIGHT_PIN);
  disp_state_set_brightness(BRIGHTNESS_DEFAULT);

  watchdog_init(); // Initialize the watchdog timer

#if !SENSCAP_EN
  // printf("SENSCAP_EN is not defined\n");
  // Initialize the ALS (Ambient Light Sensor)
  // during als init, if als device not found, then flg_alert2d_display is set to true
  // which means the display is alert2d display, else it is nkd display
  als_init(); // als initialization

  // Disable Watchdog(NUC) and Virc for Alert2d display
  if (flg_alert2d_display == false) {
    // Initialize nuc watchdog reset gpio pin
    pinMode(NUC_RESET_PIN, OUTPUT);
    digitalWrite(NUC_RESET_PIN, LOW);

    // Initialize virc
    virc_setup();
  }
#endif

  app_init();
  ESP_LOGD("Main", "device ready");

  // allocate memory dynamically for buffer which is used to store received serial data
  // serDataBuf = (char *)malloc(SERIAL_DATA_SIZE);
  memset(serDataBuf, '\0', SERIAL_DATA_SIZE);

  // create serial data mutex
  serDataMutex = xSemaphoreCreateMutex();
  if (serDataMutex == NULL) {
    LV_LOG_USER("Failed to create serial data mutex");
  } else {
    LV_LOG_USER("Successfully created serial data mutex");
  }

  // create serial data queue
  xQueue = xQueueCreate(5, SERIAL_DATA_SIZE);

  if (xQueue == NULL) {
    LV_LOG_USER("Failed to create serial data queue");
  }

  // create task with 16k bytes size for receiving serial data and maintaining it in queue
  int myTaskHandle1 = xTaskCreate(&process_request_task, "process request task", 16384, NULL, 5,
                                  &process_request_task_handle);
  if (myTaskHandle1 != pdPASS) {
    LV_LOG_USER("Failed to create process request task");
  } else {
    LV_LOG_USER("process request task created");
  }

#if !SENSCAP_EN
  // Do not create virc task for IR remote functionality if flg_alert2d_display is true
  if (flg_alert2d_display == false) {
    // create task with 4k bytes size, for virc (IR remote) functionality
    int myTaskHandle2 = xTaskCreate(&process_virc_task, "process virc task", 4096, NULL, 4,
                                    &process_virc_task_handle);
    if (myTaskHandle2 != pdPASS) {
      LV_LOG_USER("Failed to create process virc task");
    } else {
      LV_LOG_USER("process virc task created");
    }
  }
#endif
}

// int count = 0;

void loop() {
  // print free mem for every 10 seconds
  // count++;
  // if (count >= 2000) {
  //   count = 0;
  //   LV_LOG_USER("Free Heap: %d", ESP.getFreeHeap());
  //   LV_LOG_USER("Free PsRam: %d", ESP.getFreePsram());
  // }
  lv_timer_handler(); // let the GUI do its work
  lv_tick_inc(5);     // Tell LVGL that 5 msec have elapsed because of delay(5)

  process_request(100); // process incoming JSON-RPC request

  // Feed the watchdog timer periodically, if not the device will be reset
  watchdog_feed();

#if !SENSCAP_EN
  // Process ALS only for nkd display
  if (flg_alert2d_display == false) {
    // process als when condition satisfies
    if ((flg_als_Enable == true) && (flg_process_als == true)) {
      if ((alsLoopCnt % 20) == 0) { // call every 100msec once (20(loopcnt) *5(delay))
        set_backlight_als();        // set backlight value based on als value
        // In als reading, 100msec device integration time is required. so tell LVGL that 100 msec
        // have elapsed during execution of set_backlight_als()
        lv_tick_inc(100);
      }
      // reset count variable to zero before count reaches max value (4294967296)
      if (alsLoopCnt == 1000000) {
        alsLoopCnt = 0;
      }
      alsLoopCnt++; // increment loop count
    }
  }
#endif
  delay(5);
  taskYIELD();
}

void process_request_task(void *pvParams) {
  for (;;) {
    xSemaphoreTake(serDataMutex, portMAX_DELAY); // take mutex
    memset(serDataBuf, '\0', SERIAL_DATA_SIZE);
    int read_err = read_input_line(serDataBuf, SERIAL_DATA_SIZE,
                                   SERIAL_TIMEOUT); // read serial data into serDataBuf buffer
    // if no error while reading serial data, push read data into queue
    if (!read_err) {
      if (xQueueSendToBack(xQueue, serDataBuf, 0) != pdPASS) {
        LV_LOG_USER("xQueueSendToBack failed");
      } else {
      }
    }
    xSemaphoreGive(serDataMutex); // give mutex
    delay(10);
    taskYIELD();
  }
}

#if !SENSCAP_EN
void process_virc_task(void *pvParams) {
  for (;;) {
    // SERIAL_PORT.println("inside virc loop");
    read_virc_data();
    delay(3);
    taskYIELD();
  }
}
#endif