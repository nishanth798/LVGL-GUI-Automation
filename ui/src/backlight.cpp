
#ifndef SIMULATOR

#include "backlight.h"
#include "Adafruit_VEML7700.h"
#include "jsonrpc2.h" //for debugging(myprint)
#include "stdio.h"    //for debugging
#include "stdlib.h"

#define NUM_SAMPLES 100 // Number of samples for the moving average

// TODO:
// improve this library.; stop logging at library level.
// Expose in API, full range of brightness (0-100)
// Fix ALS: accomodate gain change, light attenuation due to plain/tint glass
// etc https://www.vishay.com/docs/84323/designingveml7700.pdf

/* Setting PWM Properties */
const int PWMFreq = 2000; /* 2 KHz */
const int PWMChannel = 0;
const int PWMResolution = 10;
const int MAX_DUTY_CYCLE =
    (int)(pow(2, PWMResolution) - 1); // max duty cycle will be 1023 for 10 bit resolution

const int ALS_MIN_LUX = 0;   // min lux value that is considered from the als sensor readings
const int ALS_MAX_LUX = 200; // max lux value that is considered from the als sensor readings
// ESP32 PWM is inverted, so we need to invert the duty cycle.
// 0 means 100% duty cycle and 1023 means 0% duty cycle
const int PWM_MIN_DUTYCYCLE = 0;
const int PWM_MAX_DUTYCYCLE = 650; // Max duty cycle is 1023, but we are limiting it to 650 to
                                   //  avoid over-dimming of diaplay. 650 is approx 36% brightness

bool flg_als_Enable = false;
bool flg_disp_inactive = false;
bool flg_process_als = false; // flag control to call set_backlight_als from ui.ino loop

bool flg_alert2d_display =
    false; // flag to indicate if the hardware is rlated to alert2d display or nkd display

Adafruit_VEML7700 veml = Adafruit_VEML7700();

// static lv_timer_t *als_timer;

typedef struct {
  float als_val[NUM_SAMPLES];
  int arr_idx;
} als_t;

static als_t als_d;

void als_init() {
  if (!veml.begin()) { // requires a separate twowire object?
    // LV_LOG_USER("ambient light sensor initialization failed");
    // SERIAL_PORT.println("ambient light sensor initialization failed");
    flg_alert2d_display = true; // if als device not found then veml.begin fails, which means the
                                // display is 2da display
    return;                     // return if als device not found
  } else {
    // LV_LOG_USER("ambient light sensor initialized successfully");
    // SERIAL_PORT.println("ambient light sensor initialized successfully");
    flg_alert2d_display = false; // if als device found then veml.begin succeeds, which means the
                                 // display is nkd display
  }
  veml.setIntegrationTime(VEML7700_IT_100MS);
  veml.setGain(VEML7700_GAIN_2);
}

void als_on() { veml.enable(true); }

void als_off() { veml.enable(false); }

// initlize backlight
void backlight_init(uint8_t pin) {
  ledcSetup(PWMChannel, PWMFreq, PWMResolution);
  /* Attach the LED PWM Channel to the GPIO Pin */
  ledcAttachPin(pin, PWMChannel);
}

void set_backlight(uint32_t dutycycle) {
  // LV_LOG_USER("dutycycle %d", dutycycle);
  uint32_t val = 0;
#if !SENSCAP_EN
  val = (uint32_t)(MAX_DUTY_CYCLE * (1 - (dutycycle / 100.0)));
#else
  val = (uint32_t)(MAX_DUTY_CYCLE * (dutycycle / 100.0));
#endif
  ledcWrite(PWMChannel, val);
}

float push_als_data(float alsval) {
  // store latest als data in buffer in 0th index
  //  shift the array elements to discard the last array index value.
  for (int i = NUM_SAMPLES - 1; i > 0; i--) {
    als_d.als_val[i] = als_d.als_val[i - 1];
  }
  als_d.als_val[0] = alsval; // set the first element to latest als value

  // calculate the sum of all the samples present in the array
  float sum = 0.0;
  for (int i = 0; i < NUM_SAMPLES; i++) {
    sum += als_d.als_val[i];
  }
  return sum / NUM_SAMPLES; // return average
}

void set_disp_bright_als(int dutyCycle) {
  // Apply the duty cycle to the PWM such that the brightness is changed
  // accordingly
  if ((dutyCycle <= PWM_MAX_DUTYCYCLE) && (dutyCycle >= PWM_MIN_DUTYCYCLE)) {
    ledcWrite(PWMChannel, dutyCycle);
  } else if (dutyCycle > PWM_MAX_DUTYCYCLE) // if duty cycle is greater than
                                            // PWM_MAX_DUTYCYCLE, then set the
                                            // duty cycle to PWM_MAX_DUTYCYCLE
  {
    ledcWrite(PWMChannel, PWM_MAX_DUTYCYCLE);
    dutyCycle = PWM_MAX_DUTYCYCLE;
  } else if (dutyCycle < PWM_MIN_DUTYCYCLE) // if duty cycle is less than PWM_MIN_DUTYCYCLE,
                                            // then set the duty cycle to PWM_MIN_DUTYCYCLE
  {
    ledcWrite(PWMChannel, PWM_MIN_DUTYCYCLE);
    dutyCycle = PWM_MIN_DUTYCYCLE;
  }
  // Calculate the brightness percentage and print it (for debugging)
  float brightPer = (dutyCycle / (float)MAX_DUTY_CYCLE) * 100;
  // LV_LOG_USER("ALS Brightness: %f", (100.0 - brightPer));
}

static int prevDutyCycVal = 650; // default value for previous duty cycle

void set_backlight_als() {
  // SERIAL_PORT.println("Enter set_backlight_als");
  int integTime = veml.getIntegrationTimeValue();
  float gain = veml.getGainValue();
  float als_lux = veml.readLux(VEML_LUX_CORRECTED);

  //  LV_LOG_USER("als_val:%f, integTime:%d, gain:%f",als_lux,integTime,gain);
  // if read lux is greater than ALS_MAX_LUX, then consider the ALS_MAX_LUX
  // instead of actual read value
  if (als_lux > ALS_MAX_LUX) {
    als_lux = ALS_MAX_LUX;
  }
  float als_AvgVal = push_als_data(als_lux); // push the als value to the array and get the average
                                             // value calculate the duty cycle using map function
  int currDutyCycVal =
      map(als_AvgVal, ALS_MIN_LUX, ALS_MAX_LUX, PWM_MIN_DUTYCYCLE, PWM_MAX_DUTYCYCLE);
  currDutyCycVal = PWM_MAX_DUTYCYCLE - currDutyCycVal; // Because esp32 PWM is inverted, so we
                                                       // need to invert the duty cycle

  // take the difference between previous average value and current avg value and update the
  // brightness in for loop like in increments or decrements of 1
  char dbgMsg[200];

  int diffDutyCycVal = abs(prevDutyCycVal - currDutyCycVal);
  // sprintf(dbgMsg, "prevDutyCycVal:%d, currDutyCycVal:%d, diffDutyCycVal:%d\n", prevDutyCycVal,
  //         currDutyCycVal, diffDutyCycVal);
  // SERIAL_PORT.println(dbgMsg);
  // delay(1);

  for (int i = 1; i <= diffDutyCycVal; i++) {
    if (prevDutyCycVal > currDutyCycVal) {
      prevDutyCycVal -= 1;
    } else if (prevDutyCycVal < currDutyCycVal) {
      prevDutyCycVal += 1;
    }
    set_disp_bright_als(prevDutyCycVal);
    // sprintf(dbgMsg, "dutycycle:%d", prevDutyCycVal);
    // SERIAL_PORT.println(dbgMsg);
  }
  set_disp_bright_als(prevDutyCycVal);
  // SERIAL_PORT.println("Exit set_backlight_als");
}

#endif // #ifndef SIMULATOR