#include "device_status.h"
#ifndef SIMULATOR
#include "esp_timer.h"
#endif

static dev_status_t dev_status;

void clear_dev_status() { dev_status.uptime = 0; }

void get_uptime() {
#ifndef SIMULATOR
  dev_status.uptime = esp_timer_get_time();
#else
  dev_status.uptime = 0;
#endif
}

dev_status_t *get_dev_status() {
  clear_dev_status();
  get_uptime();
  return &dev_status;
}
