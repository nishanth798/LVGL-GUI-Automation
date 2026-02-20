#ifndef SIMULATOR
// #include "Arduino.h"
#include "touch.h"
#include "jsonrpc2.h"
#include "display.h"
#include <Wire.h>

#if !SENSCAP_EN
#include <TAMC_GT911.h>
#else
#include "Arduino.h"
#include "TouchLib.h"
#endif

#if !SENSCAP_EN
TAMC_GT911 *ts;
static uint8_t active_gt911_addr = GT911_ADDR1;
#else
#define TOUCH_ROTATE_180 1 // define this macro if the touch coordinates need to be rotated by 180 degree
TouchLib *ts;
#endif

int TOUCH_GT911_INT = -1;  // Default to -1, will be set based on hardware version

bool was_touched = false;
int touch_last_x = 0, touch_last_y = 0;
bool idle_touch = false;         // flag to check if the touch event has happened during
                                 // idle time(sleep mode).
bool idle_touch_release = false; // flag to check if the touch event has been released.
static lv_indev_t *indev; // LVGL touch input device
// bool flag to control the touch reinit attempts to only once, after that asking NUC to reflash the firmware
bool flg_touch_reinit = false;
// bool flg_touch_reinit_success = false; //deprecate

uint8_t config[186];
bool set_liq_touch_filter = true; // flag to indicate whether to set liquid touch filter or not
bool debug_logs = false; // flag to enable/disable debug logs for liquid touch filter

// Clean up touch driver state and LVGL input device
void touch_cleanup() {
  // Delete LVGL input device if it exists
  if (indev != NULL) {
    lv_indev_delete(indev);
    indev = NULL;
  }
  // Reset touch state variables
  was_touched = false;
  touch_last_x = 0;
  touch_last_y = 0;
  idle_touch = false;
  idle_touch_release = false;
}

//scan i2c devices and print the i2c device slave addresses
//Now modify the below function to scan for i2c address 5d or 14 for timeout period and return if the 5d or 14 found or after timeout period
void scan_i2c_devices(int timeout) {  // timeout in milliseconds
  Wire.begin(TOUCH_SDA, TOUCH_SCL);
  // SERIAL_PORT.println("scan_i2c_devices: Scanning for GT911 I2C addresses (0x5D or 0x14)...");
  byte error;
  bool found = false;
  unsigned long startTime = millis();

  while ((millis() - startTime) < (unsigned long)timeout && !found) {
#if !SENSCAP_EN
    // Check for address 0x5D (93)
    Wire.beginTransmission(0x5D);
    error = Wire.endTransmission();

    if (error == 0) {
      // SERIAL_PORT.println("GT911 found at address 0x5D");
      found = true;
      break;
    }

    // Check for address 0x14 (20)
    Wire.beginTransmission(0x14);
    error = Wire.endTransmission();

    if (error == 0) {
      // SERIAL_PORT.println("GT911 found at address 0x14");
      found = true;
      break;
    }
#else
    // Check for address 0x48 (72) for senscap touch controller
    Wire.beginTransmission(0x48);
    error = Wire.endTransmission();
    if (error == 0) {
      printf("FT6X36 found at address 0x48");
      found = true;
      break;
    }
#endif
    
    delay(100); // Wait 100ms before next scan
  }

  if (!found) {
    // SERIAL_PORT.println("GT911 not found at either address (0x5D or 0x14) within timeout period");
  }
  Wire.end(); //end i2c
  // SERIAL_PORT.println("scan_i2c_devices: Done.");
}

#if !SENSCAP_EN
// Configure GT911 sensitivity
// Modifies: 0x8053 (Touch), 0x8054 (Leave), 0x804F (Debounce), 0x8052 (Noise_Reduction)
void configure_gt911_sensitivity(uint8_t gt911_addr, int touch_sensitivity, int leave_sensitivity, int debounce, int noise_reduction = 0) {
  // Step 1: Read current configuration from GT911
  Wire.beginTransmission(gt911_addr);
  Wire.write(0x80); // Config register high byte
  Wire.write(0x47); // Config register low byte (address 0x8047)
  uint8_t result = Wire.endTransmission();
  if (result != 0) {
    // SERIAL_PORT.println("Failed to start config read");
    return;
  }
  delay(10); // Small delay for GT911 to prepare data

  uint8_t bytesReceived = Wire.requestFrom(gt911_addr, (uint8_t)186);
  if (bytesReceived != 186) {
    // SERIAL_PORT.print("Config read failed, received only ");
    // SERIAL_PORT.print(bytesReceived);
    // SERIAL_PORT.println(" bytes");
    return;
  }

  // Read all 186 bytes into config array
  for (int i = 0; i < 186; i++) {
    if (Wire.available()) {
      config[i] = Wire.read();
    } else {
      // SERIAL_PORT.println("Unexpected end of config data");
      return;
    }
  }

  // Step 2: Store original values for comparison and logging
  uint8_t old_touch_sensitivity = config[0x0C];  // 0x8053
  uint8_t old_leave_sensitivity = config[0x0D];  // 0x8054
  uint8_t old_debounce = config[0x08];           // 0x804F
  uint8_t old_noise_reduction = config[0x0B];    // 0x8052

  // Check if config already matches
  if ((old_touch_sensitivity == touch_sensitivity) &&
      (old_leave_sensitivity == leave_sensitivity) &&
      (old_debounce == debounce) &&
      (old_noise_reduction == noise_reduction)) {
    // SERIAL_PORT.println("GT911 sensitivity already configured. No changes made.");
    return;
  }

  // Step 3: Bump config version
  config[0x00] = (uint8_t)(config[0x00] + 1);

  // Step 4: Modify the registers
  config[0x0C] = touch_sensitivity;  // 0x8053: Screen_Touch_Level
  config[0x0D] = leave_sensitivity;  // 0x8054: Screen_Leave_Level
  config[0x08] = debounce;           // 0x804F: Shake_Count (debounce)
  config[0x0B] = noise_reduction;    // 0x8052: Noise_Reduction (0-15 valid)

  if (debug_logs) {
        // Reset count after valid touch
    SERIAL_PORT.println("======= GT911 CONFIG CHANGE =======");

    SERIAL_PORT.printf("Config Version: %d -> %d\n", config[0x00] - 1, config[0x00]);
    SERIAL_PORT.printf("Touch Level (0x8053): %d -> %d\n", old_touch_sensitivity, config[0x0C]);
    SERIAL_PORT.printf("Leave Level (0x8054): %d -> %d\n", old_leave_sensitivity, config[0x0D]);
    SERIAL_PORT.printf("Debounce (0x804F): %d -> %d\n", old_debounce, config[0x08]);
    SERIAL_PORT.printf("Noise Reduction (0x8052): %d -> %d\n", old_noise_reduction, config[0x0B]);
    SERIAL_PORT.println("===================================");
  }

  // Step 5: Calculate checksum
  uint8_t checksum = 0;
  for (int i = 0; i < 184; i++) {
    checksum += config[i];
  }

  checksum = (uint8_t)(~checksum + 1);  // two's complement
  config[184] = checksum;               // 0x80FF
  config[185] = 0x01;                   // 0x8100: Config_Fresh

  // Step 6: Write config (184 bytes) to GT911
  Wire.beginTransmission(gt911_addr);
  Wire.write(0x80);
  Wire.write(0x47);
  for (int i = 0; i < 184; i++) {
    Wire.write(config[i]);
  }
  result = Wire.endTransmission();

  // Step 7: Write checksum to 0x80FF
  Wire.beginTransmission(gt911_addr);
  Wire.write(0x80);
  Wire.write(0xff);
  Wire.write(config[184]);
  result = Wire.endTransmission();

  // Step 8: Write config fresh flag to 0x8100
  Wire.beginTransmission(gt911_addr);
  Wire.write(0x81);
  Wire.write(0x00);
  Wire.write(config[185]);
  result = Wire.endTransmission();

  // Step 9: Verify by reading back
  Wire.beginTransmission(gt911_addr);
  Wire.write(0x80);
  Wire.write(0x47);
  result = Wire.endTransmission();
  if (result != 0) {
    return;
  }
  delay(10);
  bytesReceived = Wire.requestFrom(gt911_addr, (uint8_t)186);
  if (bytesReceived != 186) {
    return;
  }

  for (int i = 0; i < 186; i++) {
    if (Wire.available()) {
      config[i] = Wire.read();
    } else {
      return;
    }
  }
  if (debug_logs) {
    SERIAL_PORT.println("Verifying written config...");
    SERIAL_PORT.printf("Touch:%d, Leave:%d, Debounce:%d, Noise:%d\n", config[0x0C], config[0x0D], config[0x08], config[0x0B]);
  }
  delay(1000); // Give GT911 time to process new config

}
#endif

void touch_init() {
  // Clean up existing touch state first
  touch_cleanup();
  // SERIAL_PORT.println("touch_init: Setting address to ADDR1");
#if !SENSCAP_EN
  ts = new TAMC_GT911(TOUCH_SDA, TOUCH_SCL, TOUCH_GT911_INT, TOUCH_RST,
                           max(TOUCH_MAP_X1, TOUCH_MAP_X2), max(TOUCH_MAP_Y1, TOUCH_MAP_Y2));
  ts->begin(GT911_ADDR1);
  // Need to solve floating INT pin so on reset changes between the 2 addresses
  Wire.beginTransmission(GT911_ADDR1);
  Wire.write(highByte(GT911_PRODUCT_ID));
  Wire.write(lowByte(GT911_PRODUCT_ID));
  Wire.endTransmission();
  // returns rxLength - if 0 we have a problem
  uint8_t returnSize = Wire.requestFrom(GT911_ADDR1, (uint8_t)1);
// if(returnSize == 0){
//   SERIAL_PORT.println("touch_init: Failed to Connect to GT911 with ADDR1");
// }else{
//   SERIAL_PORT.println("touch_init: Connected to GT911 with ADDR1");
// }
  active_gt911_addr = GT911_ADDR1; // Track which address worked (using global variable)
  if (returnSize == 0) {
    // SERIAL_PORT.println("touch_init: Setting address to ADDR2");
    // restart with other address
    ts->begin(GT911_ADDR2);
    Wire.beginTransmission(GT911_ADDR2);
    Wire.write(highByte(GT911_PRODUCT_ID));
    Wire.write(lowByte(GT911_PRODUCT_ID));
    Wire.endTransmission();
    int returnSize = Wire.requestFrom(GT911_ADDR2, (uint8_t)1);
    if (returnSize == 0) {
      // SERIAL_PORT.println("touch_init: Failed to connect to GT911 with ADDR2");
    } else {
      // SERIAL_PORT.println("touch_init: Connected to GT911 with ADDR2");
      active_gt911_addr = GT911_ADDR2; // Update active address (global)
    }
  } else {
      // SERIAL_PORT.println("touch_init: Connected to GT911 with ADDR1");
  }

  // Now do your normal job
  ts->setRotation(TOUCH_GT911_ROTATION);
  configure_gt911_sensitivity(active_gt911_addr, 120, 110, 1, 5);  // noise_reduction = 5

#else
  // SERIAL_PORT.println("touch_init: Setting address to 0x48");
  ts = new TouchLib(Wire, TOUCH_SDA, TOUCH_SCL, TOUCH_FT6X36_ADDR);
  ts->init();

  // Need to solve floating INT pin so on reset changes between the 2 addresses
  Wire.beginTransmission(TOUCH_FT6X36_ADDR);
  // Wire.write(highByte(GT911_PRODUCT_ID));
  // Wire.write(lowByte(GT911_PRODUCT_ID));
  Wire.endTransmission();
  // returns rxLength - if 0 we have a problem
  uint8_t returnSize = Wire.requestFrom(TOUCH_FT6X36_ADDR, (uint8_t)1);
// if(returnSize == 0){
//   SERIAL_PORT.println("touch_init: Failed to Connect to GT911 with ADDR1");
// }else{
//   SERIAL_PORT.println("touch_init: Connected to GT911 with ADDR1");
// }
#endif
  my_lv_drv_init();
}

bool touch_has_signal() {
#if !SENSCAP_EN
#if defined(TOUCH_GT911)
  return true;
#else
  return false;
#endif

#else
#if defined(TOUCH_FT6X36)
  return true;
#else
  return false;
#endif
#endif
}

#if !SENSCAP_EN
bool touch_touched() {
  //variable to store count of touch detected cycles before no touch is detected
  static int touch_detected_cycles = 0;
  static int last_touch_x = -1;
  static int last_touch_y = -1;
  const uint8_t  MAX_ALLOWED_CYCLES = 5;   // sanitizer "too perfect" threshold
  static bool is_touched_prev = false;

  ts->read(); // read registers

  if(set_liq_touch_filter == true){
    if (ts->isTouched) {
      if(touch_detected_cycles > 10000){
        touch_detected_cycles = 0; //reset to avoid overflow
      }
      touch_detected_cycles++;
      last_touch_x = ts->points[0].x;
      last_touch_y = ts->points[0].y;
      if(debug_logs){
        SERIAL_PORT.printf("liquid filter: Touch detected cycle count: %d, X:%d, y:%d\n", touch_detected_cycles, last_touch_x, last_touch_y);
      }
      is_touched_prev = true;
      return false;
    } else {
      if((touch_detected_cycles <= MAX_ALLOWED_CYCLES) && (is_touched_prev == true)) {
        is_touched_prev = false;
      // ---- At this point, we know it's a valid finger touch ----
        was_touched = true;
        if(debug_logs){
          SERIAL_PORT.printf("***liquid filter: Touch accepted cycle count: %d, X:%d, y:%d *****\n", touch_detected_cycles, last_touch_x, last_touch_y);
          Wire.beginTransmission(active_gt911_addr);
          Wire.write(0x80); // Config register high byte
          Wire.write(0x47); // Config register low byte (address 0x8047)
          uint8_t result = Wire.endTransmission();
          if (result != 0) {
            SERIAL_PORT.println("Failed to start config read");
            // return;
          }
          delay(10); // Small delay for GT911 to prepare data
          
          uint8_t bytesReceived = Wire.requestFrom(active_gt911_addr, (uint8_t)15);
          if (bytesReceived != 15) {
            // SERIAL_PORT.print("Config read failed, received only ");
            // SERIAL_PORT.print(bytesReceived);
            // SERIAL_PORT.println(" bytes");
            // return;
          }
          // Read all 186 bytes into config array
          for (int i = 0; i < 15; i++) {
            if (Wire.available()) {
              config[i] = Wire.read();
            } else {
              SERIAL_PORT.println("Unexpected end of config data");
              // return;
            }
          }
          SERIAL_PORT.printf("Touch Sens:%d, Leave Sens:%d, Debounce:%d\n", config[0x0C], config[0x0D], config[0x08]);
        }
  #if defined(TOUCH_SWAP_XY)
        touch_last_x = map(last_touch_y, TOUCH_MAP_X1, TOUCH_MAP_X2, 0, MY_DISP_HOR_RES - 1);
        touch_last_y = map(last_touch_x, TOUCH_MAP_Y1, TOUCH_MAP_Y2, 0, MY_DISP_VER_RES - 1);
    #else
        touch_last_x = map(last_touch_x, TOUCH_MAP_X1, TOUCH_MAP_X2, 0, MY_DISP_HOR_RES - 1);
        touch_last_y = map(last_touch_y, TOUCH_MAP_Y1, TOUCH_MAP_Y2, 0, MY_DISP_VER_RES - 1);
    #endif
    if(debug_logs){
          // Debug: Print raw and mapped coordinates
          SERIAL_PORT.printf("Touch RAW: x=%d, y=%d | MAPPED: x=%d, y=%d\n",
                        last_touch_x, last_touch_y, touch_last_x, touch_last_y);
        }
        // Reset count after valid touch
        touch_detected_cycles = 0;
        return true;
      }
      // Reset count when no touch detected
      touch_detected_cycles = 0;
      is_touched_prev = false;
      return false;
    }
  }else{
    if (ts->isTouched) {
      last_touch_x = ts->points[0].x;
      last_touch_y = ts->points[0].y;
      was_touched = true;
      if(debug_logs){
        SERIAL_PORT.printf("***Touch accepted X:%d, y:%d *****\n", last_touch_x, last_touch_y);
        Wire.beginTransmission(active_gt911_addr);
        Wire.write(0x80); // Config register high byte
        Wire.write(0x47); // Config register low byte (address 0x8047)
        uint8_t result = Wire.endTransmission();
        if (result != 0) {
          SERIAL_PORT.println("Failed to start config read");
          // return;
        }
        delay(10); // Small delay for GT911 to prepare data
        uint8_t bytesReceived = Wire.requestFrom(active_gt911_addr, (uint8_t)15);
          if (bytesReceived != 15) {
            // SERIAL_PORT.print("Config read failed, received only ");
            // SERIAL_PORT.print(bytesReceived);
            // SERIAL_PORT.println(" bytes");
            // return;
          }
          // Read all 186 bytes into config array
          for (int i = 0; i < 15; i++) {
            if (Wire.available()) {
              config[i] = Wire.read();
            } else {
              if(debug_logs){
                SERIAL_PORT.println("Unexpected end of config data");
              }
              // return;
            }
          }
          SERIAL_PORT.printf("Touch Sens:%d, Leave Sens:%d, Debounce:%d\n", config[0x0C], config[0x0D], config[0x08]);
        }
  #if defined(TOUCH_SWAP_XY)
      touch_last_x = map(ts->points[0].y, TOUCH_MAP_X1, TOUCH_MAP_X2, 0, MY_DISP_HOR_RES - 1);
      touch_last_y = map(ts->points[0].x, TOUCH_MAP_Y1, TOUCH_MAP_Y2, 0, MY_DISP_VER_RES - 1);
  #else
      touch_last_x = map(ts->points[0].x, TOUCH_MAP_X1, TOUCH_MAP_X2, 0, MY_DISP_HOR_RES - 1);
      touch_last_y = map(ts->points[0].y, TOUCH_MAP_Y1, TOUCH_MAP_Y2, 0, MY_DISP_VER_RES - 1);
  #endif
  if(debug_logs){
      // Debug: Print raw and mapped coordinates
        SERIAL_PORT.printf("Touch RAW: x=%d, y=%d | MAPPED: x=%d, y=%d\n",
                    ts->points[0].x, ts->points[0].y, touch_last_x, touch_last_y);
      }
      return true;
    } else {
      return false;
    }
  }
}
#else
bool touch_touched() {
  if (ts->read()) {
    was_touched = true;
    TP_Point p = ts->getPoint(0);

#if defined(TOUCH_ROTATE_180)
    touch_last_x = map((MY_DISP_HOR_RES - p.x), TOUCH_MAP_X1, TOUCH_MAP_X2, 0, MY_DISP_HOR_RES - 1);
    touch_last_y = map((MY_DISP_VER_RES - p.y), TOUCH_MAP_Y1, TOUCH_MAP_Y2, 0, MY_DISP_VER_RES - 1);
#else
    touch_last_x = map(p.x, TOUCH_MAP_X1, TOUCH_MAP_X2, 0, MY_DISP_HOR_RES - 1);
    touch_last_y = map(p.y, TOUCH_MAP_Y1, TOUCH_MAP_Y2, 0, MY_DISP_VER_RES - 1);
#endif
    // Serial0.println(touch_last_x, touch_last_y);
    // Serial0.printf("[TOUCH] x=%d y=%d\n", touch_last_x, touch_last_y); // ADDED: clean coords
    return true;
  }
  return false;
}
#endif

bool touch_released() {
#if !SENSCAP_EN
  ts->read();
  if (!ts->isTouched && was_touched) {
    was_touched = false;
    return true;
  } else {
    return false;
  }
#else
  bool down = ts->read();
  if (!down && was_touched) {
    was_touched = false;
    return true;
  }
  return false;
#endif
}

void send_touch_notification(JsonRpcError error, const char *message) {
  JsonDocument doc;
  JsonObject params = doc["params"].to<JsonObject>();
  params["code"] = error;
  params["message"] = message;
  serializeJsonRpcRequest(0, CMD_SYSTEM_EVENT, params);
}

void process_touch_fail(){
  // char buf[100];
  // snprintf(buf, sizeof(buf), "my_touchpad_read: touch co-ordiantes x(%d), y(%d) out of range", touch_last_x, touch_last_y);
  // SERIAL_PORT.println(buf);
  // Only attempt reinit once to avoid infinite loops
  if (flg_touch_reinit == false) {
    send_touch_notification(TOUCH_FIRST_FAIL, "TOUCH_FIRST_FAIL"); // Send touch first fail notification
    // Attempt to reinitialize the touch controller
    delay(500); // 500msec delay before reinitialization
    touch_init();
    delay(500);//Wait before reading touch again after reinitialization
    // Check if reinitialization fixed the issue by reading touch again
    touch_touched(); // this resulting in points values as 0,0. So reading once again
    delay(500);//Wait before reading touch again
    touch_touched();// reading touch second time as first time we are getting x,y as 0,0
    if ((touch_last_x >= 0 && touch_last_x < TOUCH_MAP_X1) &&
    (touch_last_y >= 0 && touch_last_y < TOUCH_MAP_Y1)){
      send_touch_notification(TOUCH_REINIT_SUCCESS, "TOUCH_REINIT_SUCCESS");
      flg_touch_reinit = false; //If reinit is success, then reset the flag and try again if the issue arises
      // flg_touch_reinit_success = true; //deprecate //says touch reinit was successful
    } else {
      send_touch_notification(TOUCH_REINIT_FAIL, "TOUCH_REINIT_FAIL");
      flg_touch_reinit = true; //indicates touch reinitialize failed, indicating NUC to take necessary actions
      // flg_touch_reinit_success = false;//deprecate //says touch reinit was not successful
    }
  } else{
    //sendout a errorcode to NUC with 1 second delay
    delay(1000);
    send_touch_notification(TOUCH_REINIT_FAIL, "TOUCH_REINIT_FAIL");
  }
  // US team does not require the second fail code, so commenting it out
  //   else{ //if (flg_touch_reinit == false)
  //   //sendout a errorcode to NUC, indicating NUC to reflash the display
  //   if(flg_touch_reinit_success == false){
  //     send_touch_notification(TOUCH_REINIT_FAIL, "TOUCH_REINIT_FAIL"); // Send touch reinit fail notification
  //   }else{
  //    send_touch_notification(TOUCH_SECOND_FAIL, "TOUCH_SECOND_FAIL"); // Send touch second fail notification
  //   }
  //    delay(1000); //sendout a errorcode to NUC with 1 second delay
  // }
}

void my_touchpad_read(lv_indev_t *indev_drv, lv_indev_data_t *data) {
  if (touch_has_signal()) {
    if (touch_touched()) {
      if (flg_disp_sleep == true) { // if display is in sleep mode, wake it up
                                    // on the first touch event.
// set display sleep flag to false once the display is woken up.
        flg_disp_sleep = false;
// set the idle touch flag to true, such that untill the release event happens the events wont be processed.
        idle_touch = true;
// set the idle touch release flag to false, such that untill the release event happens the events wont be processed.
        idle_touch_release = false;
// trigger a dummy activity, such that (lv_disp_get_inactive_time(NULL) > INACTIVE_TIMEOUT)
// will return false and the display comes out of sleep mode
        lv_disp_trig_activity(NULL);
        return;
      } else if (idle_touch && !idle_touch_release) {
        // During sleep mode, if the first touch event happened and release event not
        // happened then return without processing the event.
        return;
      }
      if ((touch_last_x >= 0 && touch_last_x < TOUCH_MAP_X1) &&
          (touch_last_y >= 0 && touch_last_y < TOUCH_MAP_Y1)) {
        data->state = LV_INDEV_STATE_PRESSED;
        /*Set the coordinates*/
        data->point.x = touch_last_x;
        data->point.y = touch_last_y;
      } else { //if touch points out of bounds, process the touch failure
        process_touch_fail();
        return;
      }
    } else if (touch_released()) {
      data->state = LV_INDEV_STATE_RELEASED;
      idle_touch_release = true; // set the flag to true, when the release event happenes
      idle_touch = false;        // set the flag to false, once the display is woken up.
    }
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

void my_lv_drv_init() {
  indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_touchpad_read);
}

#if !SENSCAP_EN
void set_liquid_touch_filter(int enable){
  if(enable){
    if(debug_logs){
      SERIAL_PORT.println("Enabling liquid touch filter");
    }
    set_liq_touch_filter = true;
    configure_gt911_sensitivity(active_gt911_addr, 120, 110, 1, 5);  // noise_reduction = 5
  }else{
    if(debug_logs){
      SERIAL_PORT.printf("Disabling liquid touch filter\n");
    }
    set_liq_touch_filter = false;
    configure_gt911_sensitivity(active_gt911_addr, 80, 50, 1, 0);   // noise_reduction = 0
  }
}

void set_liquid_touch_filter_debug_logs(int enable){
  if(enable){
    debug_logs = true;
    SERIAL_PORT.println("Enabling liquid touch filter debug logs");
  }else{
    debug_logs = false;
    SERIAL_PORT.println("Disabling liquid touch filter debug logs");
  }
}
#endif
#endif // #ifndef SIMULATOR