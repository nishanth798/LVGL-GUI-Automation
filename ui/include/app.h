#ifndef _APP_H
#define _APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ui.h"

void app_init();
// void notify_set_state();
// void notify_timeout();
void process_inactivity();

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
