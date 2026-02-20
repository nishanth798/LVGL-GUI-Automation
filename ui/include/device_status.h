#ifndef _DEVICE_STATUS_H
#define _DEVICE_STATUS_H

#include <cstdint>

typedef struct {
  uint64_t uptime;
} dev_status_t;

dev_status_t *get_dev_status();

#endif