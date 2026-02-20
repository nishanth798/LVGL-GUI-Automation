#ifndef _DEVICE_INFO_H
#define _DEVICE_INFO_H

#ifdef _cplusplus
extern "C" {
#endif

#define HW_VER_CNTRL_PIN 13

typedef struct {
  char mfr[25];
  char model[25];
  char serialMac[30];
  char hwVer[25];
  char fwVer[25];
} dev_info_t;

void init_dev_info();
dev_info_t *get_dev_info();

#ifdef _cplusplus
}
#endif

#endif