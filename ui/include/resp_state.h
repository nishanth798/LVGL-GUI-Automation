#ifndef _RESP_STATE_H
#define _RESP_STATE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  char response_method[100]; // recently sent method name
  int resp_id;               // response id to keep track on session data
  char assigned_room[11]; // 1 extra character for null termination. assigned room data, if room is
                          // already assigned
  char sensor_id[20];     // sensor id if room is already assigned to other sensor
  char compute_id[50];    // compute id of the device (NUC/alert2d)
  char sw_ver[50];        // software version of the device (NUC/alert2d)
} resp_session_data_t;

resp_session_data_t *resp_session_data_get();
void resp_session_data_clear(resp_session_data_t *);
void resp_method_clear(resp_session_data_t *resp_sData);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif