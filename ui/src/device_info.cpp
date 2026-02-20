#include "device_info.h"
#include "touch.h"
#include <stdlib.h>
#include <string.h>

#ifndef SIMULATOR
#include "Arduino.h"
#include "esp_system.h"
#endif

#define MANUFACTURER "Virtusense"
#if !SENSCAP_EN
#define MODEL "NA"
#else
#define MODEL "Senscap"
#endif

static dev_info_t dev_info;

void clear_dev_info() {
  memset(dev_info.mfr, '\0', sizeof(dev_info.mfr));
  memset(dev_info.model, '\0', sizeof(dev_info.model));
  memset(dev_info.serialMac, '\0', sizeof(dev_info.serialMac));
  memset(dev_info.hwVer, '\0', sizeof(dev_info.hwVer));
  memset(dev_info.fwVer, '\0', sizeof(dev_info.fwVer));
}

#ifndef SIMULATOR
void get_serialMac() {
  uint8_t mac_address[6];
  esp_read_mac(mac_address, ESP_MAC_WIFI_STA); // MAC Address w.r.t WIFI STATION
  sprintf(dev_info.serialMac, "%02X:%02X:%02X:%02X:%02X:%02X", mac_address[0], mac_address[1],
          mac_address[2], mac_address[3], mac_address[4], mac_address[5]);
}
#endif

void init_dev_info() {
  clear_dev_info();
  strcpy(dev_info.mfr, MANUFACTURER); // manufacturer details
  strcpy(dev_info.model, MODEL);      // model details
#ifndef SIMULATOR
  get_serialMac(); // Mac Address
#else
  strcpy(dev_info.serialMac, "NA");
#endif
// If not simulator and not senscap, then only set the hw version by reading pin status
#if (SIMULATOR == 0) && (SENSCAP_EN == 0)
  pinMode(HW_VER_CNTRL_PIN, INPUT_PULLUP);
  int val = digitalRead(HW_VER_CNTRL_PIN);
  // Set the correct hardware version based on HW_VER_CNTRL_PIN status
  if (val == HIGH) {
    strcpy(dev_info.hwVer, "v1.0.2");
  } else {
    strcpy(dev_info.hwVer, "v1.0.3");
  }
#else
  strcpy(dev_info.hwVer, "NA");
#endif
  strcpy(dev_info.fwVer, FW_VER);
}

dev_info_t *get_dev_info() { return &dev_info; }
