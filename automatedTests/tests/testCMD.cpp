#include "app.h"
#include "jsonrpc2.h"
#include "lv_test_helpers.h"
#include "lv_test_init.h"
#include "lvgl.h"
#include "unity_fixture.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

extern int g_bail;
extern int WR_FD;
extern ssize_t write_response(int fd, const char *data, size_t len);

extern int RD_FD, UI_WR_FD, UI_RD_FD, G_OUT, G_A0;
extern void wait_ms(uint32_t ms);
extern void drain_pipe_now(void);
extern ssize_t write_response(int fd, const char *data, size_t len);
extern int read_line_nb(int fd, char *b, size_t cap);

static void setv_variant_cmd(int variant, int fts_avail) {
  char msg[256];

  int n = snprintf(msg, sizeof(msg),
                   "{\"jsonrpc\":\"2.0\",\"method\":\"set_variant\","
                   "\"params\":{\"variant\":%d,\"fts_avail\":%d},\"id\":1}\n",
                   variant, fts_avail);
  write_response(WR_FD, msg, (size_t)n);
  process_request(50);
}

static void test_setv_command(void) {
  setv_variant_cmd(1, 127);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-01-NKD_CMD_001-Activate screen_1.png"),
      "Screenshot comparison failed for CMD Activate screen");
  lv_test_wait(300);
  setv_variant_cmd(2, 127);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-02-NKD_CMD_001-Activate screen_2.png"),
      "Screenshot comparison failed CMD Activate screen");
  lv_test_wait(300);
  setv_variant_cmd(3, 127);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-03-NKD_CMD_001-Activate screen_3.png"),
      "Screenshot comparison failed CMD Activate screen");
}

static int wait_for_error_line(uint32_t timeout_ms) {
  char line[1024];
  uint32_t waited = 0;

  while (waited < timeout_ms) {
    while (read_line_nb(RD_FD, line, sizeof(line))) {
      if (strstr(line, "\"error\"") || strstr(line, "INVALID_PARAMS") ||
          strstr(line, "Invalid params") || strstr(line, "PARSE_ERROR")) {
        return 1;
      }
    }
    wait_ms(10);
    waited += 10;
  }

  return 0;
}

static void test_invalid_setv_cmd(void) {
  setv_variant_cmd(-1, 127);
  TEST_ASSERT_TRUE_MESSAGE(wait_for_error_line(1000),
                           "Expected error response for invalid variant but none received");
  setv_variant_cmd(0, 127);
  TEST_ASSERT_TRUE_MESSAGE(wait_for_error_line(1000),
                           "Expected error response for invalid variant but none received");
  setv_variant_cmd(4, 127);
  TEST_ASSERT_TRUE_MESSAGE(wait_for_error_line(1000),
                           "Expected error response for invalid variant but none received");
}

static void test_edge_cases_fts_avail(void) {
  setv_variant_cmd(1, 7);
  lv_test_mouse_click_at(240, 240); // Click the "Activate" button
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-04-fts_avail_7.png"),
                           "Screenshot comparison failed CMD fts_avail_7");
  lv_test_wait(300);
  lv_test_mouse_click_at(445, 30);
  setv_variant_cmd(1, 1);
  lv_test_mouse_click_at(240, 240); // Click the "Activate" button
  lv_test_wait(350);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-05-fts_avail_1.png"),
                           "Screenshot comparison failed CMD fts_avail_1");
  lv_test_wait(300);
  lv_test_mouse_click_at(445, 30);
  setv_variant_cmd(1, 0);
  TEST_ASSERT_TRUE_MESSAGE(wait_for_error_line(1000),
                           "Expected error response for invalid variant but none received");
  setv_variant_cmd(1, 255);
  TEST_ASSERT_TRUE_MESSAGE(wait_for_error_line(1000),
                           "Expected error response for invalid variant but none received");
  setv_variant_cmd(1, -255);
  TEST_ASSERT_TRUE_MESSAGE(wait_for_error_line(1000),
                           "Expected error response for invalid variant but none received");
  char msg[256];
  int n = snprintf(msg, sizeof(msg),
                   "{\"jsonrpc\":\"2.0\",\"method\":\"set_variant\","
                   "\"params\":{\"variant\":1},\"id\":1}\n");
  write_response(WR_FD, msg, (size_t)n);
  process_request(50);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(wait_for_error_line(1000),
                           "Expected error response for invalid variant but none received");
}

static void send_set_state_skip_index(int skip_idx, int id) {
  const char *fields[] = {"\"screen\":2",    "\"bms\":1",
                          "\"vol\":1",       "\"lang\":\"English\"",
                          "\"audio\":1",     "\"mode\":1",
                          "\"pause_tmr\":0", "\"alert\":0",
                          "\"syserr\":0",    "\"room_number\":\"1001-a\"",
                          "\"cal\":0",       "\"fts_avail\":121",
                          "\"fts_state\":1"};

  char msg[1024];
  int n = 0;

  n += snprintf(msg + n, sizeof(msg) - (size_t)n,
                "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{");

  for (int i = 0; i < (int)(sizeof(fields) / sizeof(fields[0])); i++) {
    if (i == skip_idx)
      continue;
    n += snprintf(msg + n, sizeof(msg) - (size_t)n, "%s,", fields[i]);
  }

  // remove last comma
  if (n > 0 && msg[n - 1] == ',')
    n--;

  n += snprintf(msg + n, sizeof(msg) - (size_t)n, "},\"id\":%d}\n", id);

  write_response(WR_FD, msg, (size_t)n);
  process_request(50);
}

static void test_missing_params(void) {
  setv_variant_cmd(1, 127);
  const int mandatory_idx[] = {0, 5, 9, 11, 12};

  for (int k = 0; k < (int)(sizeof(mandatory_idx) / sizeof(mandatory_idx[0])); k++) {
    drain_pipe_now();
    send_set_state_skip_index(mandatory_idx[k], 200 + k);

    TEST_ASSERT_TRUE_MESSAGE(wait_for_error_line(1000),
                             "Expected error when mandatory param missing, but no error observed");
  }
}

static void send_set_state_with_fts(int fts_state, int fts_avail) {
  char resp[512];
  int len =
      snprintf(resp, sizeof(resp),
               "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
               "{\"screen\":2,\"fts_avail\":%d,\"fts_state\":%d,\"bed_pos\":1,"
               "\"bed_wid\":60,\"occ_size\":2,\"mon_start\":\"2130\",\"mon_end\":\"0700\","
               "\"bms\":2,\"vol\":2,\"audio\":1,\"lang\":\"English\",\"mode\":1,\"pause_tmr\":0,"
               "\"alert\":0,\"syserr\":0,\"syserr_title\":\"\",\"abc\":0,"
               "\"room_number\":\"1001-a\"},\"id\":1}\n",
               fts_state, fts_avail);
  write_response(WR_FD, resp, (size_t)len);
  process_request(50);
}

static void test_fts_options_edge_cases(void) {
  setv_variant_cmd(1, 127);
  send_set_state_with_fts(127, 7);
  lv_test_wait(300);
  send_set_state_with_fts(33, 1);
  lv_test_wait(300);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-06-fts_avail_33.png"),
      "Screenshot comparison failed CMD fts_avail_33");
  lv_test_mouse_click_at(60, 185); // Click the Alerts settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-07-fts_state_1.png"),
                           "Screenshot comparison failed CMD fts_state_1");
  lv_test_mouse_click_at(445, 30);
  send_set_state_with_fts(127, 5);
  lv_test_wait(300);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(300);
  lv_test_mouse_click_at(60, 185); // Click the Alerts settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-08-fts_state_5.png"),
                           "Screenshot comparison failed CMD fts_state_5");
  lv_test_mouse_click_at(445, 30);
  send_set_state_with_fts(255, 1);
  TEST_ASSERT_TRUE_MESSAGE(wait_for_error_line(1000),
                           "Expected error response for invalid variant but none received");
  send_set_state_with_fts(-2551, 1);
  TEST_ASSERT_TRUE_MESSAGE(wait_for_error_line(1000),
                           "Expected error response for invalid variant but none received");
}

static void send_set_state_with_screen(int screen) {
  char msg[1024];
  int n = snprintf(msg, sizeof(msg),
                   "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{"
                   "\"screen\":%d,"
                   "\"audio\":0,"
                   "\"mode\":2,"
                   "\"room_number\":\"1001-a\","
                   "\"cal\":0,"
                   "\"fts_avail\":121,"
                   "\"fts_state\":1"
                   "}}\n",
                   screen);

  write_response(WR_FD, msg, (size_t)n);
  process_request(50);
}

static void test_screen_param(void) {
  setv_variant_cmd(1, 127);
  send_set_state_with_screen(1);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-01-Activate screen_1.png"),
      "Screenshot comparison failed for CMD Activate screen");
  send_set_state_with_screen(2);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-09-screen_2.png"),
                           "Screenshot comparison failed for screen 2");
  send_set_state_with_screen(5);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-10-screen_5.png"),
                           "Screenshot comparison failed for screen 5");
  send_set_state_with_screen(6);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-11-screen_6.png"),
                           "Screenshot comparison failed for screen 6");
  send_set_state_with_screen(7);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-12-screen_7.png"),
                           "Screenshot comparison failed for screen 7");
}

static void send_set_state_with_bms_and_optional_mode(int bms, int omit_bms) {
  char msg[1024];
  int n = 0;
  n += snprintf(msg + n, sizeof(msg) - (size_t)n,
                "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{"
                "\"screen\":2,");
  if (!omit_bms) {
    n += snprintf(msg + n, sizeof(msg) - (size_t)n, "\"bms\":%d,", bms);
  }
  n += snprintf(msg + n, sizeof(msg) - (size_t)n,
                "\"mode\":1,"
                "\"room_number\":\"1001-a\","
                "\"fts_avail\":121,"
                "\"fts_state\":1"
                "},\"id\":4}\n");
  write_response(WR_FD, msg, (size_t)n);
  process_request(50);
}

static void test_bms_param(void) {
  setv_variant_cmd(1, 127);
  send_set_state_with_bms_and_optional_mode(1, 0);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-13-bms_1.png"),
                           "Screenshot comparison failed for bms 1 ");
  lv_test_wait(300);
  send_set_state_with_bms_and_optional_mode(2, 0);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-14-bms_2.png"),
                           "Screenshot comparison failed for bms 2 ");
  send_set_state_with_bms_and_optional_mode(3, 0);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-15-bms_3.png"),
                           "Screenshot comparison failed for bms 3 ");
  send_set_state_with_bms_and_optional_mode(5, 0);
  TEST_ASSERT_TRUE_MESSAGE(wait_for_error_line(1000),
                           "Expected error response for invalid variant but none received");
  send_set_state_with_bms_and_optional_mode(0, 1);
  TEST_ASSERT_TRUE_MESSAGE(wait_for_error_line(1000),
                           "Expected error response for invalid variant but none received");
}

static void send_set_state_with_bed_pos(int bed_pos) {
  char msg[1024];

  int n = snprintf(msg, sizeof(msg),
                   "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{"
                   "\"screen\":2,"
                   "\"fts_avail\":127,"
                   "\"fts_state\":7,"
                   "\"bed_pos\":%d,"
                   "\"bed_wid\":60,"
                   "\"occ_size\":2,"
                   "\"mon_start\":\"2130\","
                   "\"mon_end\":\"0700\","
                   "\"bms\":2,"
                   "\"vol\":2,"
                   "\"lang\":\"English\","
                   "\"mode\":4,"
                   "\"room_number\":\"1001-a\""
                   "},\"id\":1}\n",
                   bed_pos);
  write_response(WR_FD, msg, (size_t)n);
  process_request(50);
}

static void test_bed_pos_param(void) {
  setv_variant_cmd(1, 127);
  send_set_state_with_bed_pos(1);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(300);
  lv_test_mouse_click_at(110, 330); // Click the Bed Placement settings button
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-16-bp_1.png"),
                           "Screenshot comparison failed for bp 1 ");
  lv_test_mouse_click_at(445, 30);
  send_set_state_with_bed_pos(2);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(300);
  lv_test_mouse_click_at(110, 330); // Click the Bed Placement settings button
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-17-bp_2.png"),
                           "Screenshot comparison failed for bp 2 ");
  lv_test_mouse_click_at(445, 30);
  send_set_state_with_bed_pos(3);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(300);
  lv_test_mouse_click_at(110, 330); // Click the Bed Placement settings button
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-18-bp_3.png"),
                           "Screenshot comparison failed for bp 3 ");
  lv_test_mouse_click_at(445, 30);
  send_set_state_with_bed_pos(4);
  TEST_ASSERT_TRUE_MESSAGE(wait_for_error_line(1000),
                           "Expected error response for invalid variant but none received");
}

static void send_set_state_with_bed_wid(const char *bed_wid_str, int bed_wid_int, int use_string) {
  char msg[1024];
  int n;

  if (use_string) {
    n = snprintf(msg, sizeof(msg),
                 "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{"
                 "\"screen\":2,"
                 "\"fts_avail\":127,"
                 "\"fts_state\":4,"
                 "\"bed_pos\":1,"
                 "\"bed_wid\":\"%s\","
                 "\"occ_size\":2,"
                 "\"mon_start\":\"2130\","
                 "\"mon_end\":\"0700\","
                 "\"bms\":2,"
                 "\"vol\":2,"
                 "\"lang\":\"English\","
                 "\"mode\":4,"
                 "\"room_number\":\"1001-a\""
                 "},\"id\":1}\n",
                 bed_wid_str);
  } else {
    n = snprintf(msg, sizeof(msg),
                 "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{"
                 "\"screen\":2,"
                 "\"fts_avail\":127,"
                 "\"fts_state\":4,"
                 "\"bed_pos\":1,"
                 "\"bed_wid\":%d,"
                 "\"occ_size\":2,"
                 "\"mon_start\":\"2130\","
                 "\"mon_end\":\"0700\","
                 "\"bms\":2,"
                 "\"vol\":2,"
                 "\"lang\":\"English\","
                 "\"mode\":4,"
                 "\"room_number\":\"1001-a\""
                 "},\"id\":1}\n",
                 bed_wid_int);
  }

  write_response(WR_FD, msg, (size_t)n);
  process_request(50);
}

static void test_bed_wid_param(void) {
  setv_variant_cmd(1, 127);
  send_set_state_with_bed_wid(NULL, 38, 0);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(300);
  lv_test_mouse_click_at(320, 260); // Click the "Bed width" button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-19-BW_38.png"),
                           "Screenshot comparison failed for bW 38 ");
  lv_test_mouse_click_at(445, 30);
  send_set_state_with_bed_wid(NULL, 42, 0);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(300);
  lv_test_mouse_click_at(320, 260); // Click the "Bed width" button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-20-BW_42.png"),
                           "Screenshot comparison failed for bW 42 ");
  lv_test_mouse_click_at(445, 30);
  send_set_state_with_bed_wid(NULL, 54, 0);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(300);
  lv_test_mouse_click_at(320, 260); // Click the "Bed width" button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-21-BW_54.png"),
                           "Screenshot comparison failed for bW 54 ");
  lv_test_mouse_click_at(445, 30);
  send_set_state_with_bed_wid(NULL, 60, 0);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(300);
  lv_test_mouse_click_at(320, 260); // Click the "Bed width" button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-22-BW_60.png"),
                           "Screenshot comparison failed for bW 60 ");
  lv_test_mouse_click_at(445, 30);
  send_set_state_with_bed_wid(NULL, 72, 0);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(300);
  lv_test_mouse_click_at(320, 260); // Click the "Bed width" button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-23-BW_72.png"),
                           "Screenshot comparison failed for bW 72 ");
  lv_test_mouse_click_at(445, 30);
  send_set_state_with_bed_wid(NULL, 76, 0);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(300);
  lv_test_mouse_click_at(320, 260); // Click the "Bed width" button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-24-BW_76.png"),
                           "Screenshot comparison failed for bW 76 ");
  lv_test_mouse_click_at(445, 30);
  send_set_state_with_bed_wid("Hi", 0, 1);
  TEST_ASSERT_TRUE_MESSAGE(wait_for_error_line(1000),
                           "Expected error response for invalid variant but none received");
}

static void send_set_state_with_dynamic_vol(int vol, int audio) {
  char msg[1024];
  int n = snprintf(msg, sizeof(msg),
                   "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{"
                   "\"screen\":2,"
                   "\"bms\":1,"
                   "\"vol\":%d,"
                   "\"lang\":\"English\","
                   "\"audio\":%d,"
                   "\"mode\":1,"
                   "\"room_number\":\"1001-a\","
                   "\"fts_avail\":121,"
                   "\"fts_state\":1"
                   "},\"id\":4}\n",
                   vol, audio);
  write_response(WR_FD, msg, (size_t)n);
  process_request(50);
}

static void test_vol_param(void) {
  setv_variant_cmd(1, 127);
  send_set_state_with_dynamic_vol(1, 1);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(300);
  lv_test_mouse_click_at(350, 110);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-25-vol_1.png"),
                           "Screenshot comparison failed for vol_1 ");
  lv_test_mouse_click_at(445, 30);
  send_set_state_with_dynamic_vol(2, 1);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(300);
  lv_test_mouse_click_at(350, 110);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-26-vol_2.png"),
                           "Screenshot comparison failed for vol_2 ");
  lv_test_mouse_click_at(445, 30);
  send_set_state_with_dynamic_vol(3, 1);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(300);
  lv_test_mouse_click_at(350, 110);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-27-vol_3.png"),
                           "Screenshot comparison failed for vol_3 ");
  lv_test_mouse_click_at(445, 30);
  send_set_state_with_dynamic_vol(5, 1);
  TEST_ASSERT_TRUE_MESSAGE(wait_for_error_line(1000),
                           "Expected error response for invalid variant but none received");
}

static void test_audio_param(void) {
  setv_variant_cmd(1, 127);
  send_set_state_with_dynamic_vol(1, 0);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(300);
  lv_test_mouse_click_at(350, 110);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-28-audio_0.png"),
                           "Screenshot comparison failed for audio_0 ");
  lv_test_mouse_click_at(445, 30);
  send_set_state_with_dynamic_vol(1, 1);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(300);
  lv_test_mouse_click_at(350, 110);
  lv_test_wait(350);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-25-vol_1.png"),
                           "Screenshot comparison failed for vol_1 ");
  lv_test_mouse_click_at(445, 30);
  send_set_state_with_dynamic_vol(5, 0);
  TEST_ASSERT_TRUE_MESSAGE(wait_for_error_line(1000),
                           "Expected error response for invalid variant but none received");
}

static void send_set_state_with_lang_int(int lang) {
  char msg[1024];
  int n = snprintf(msg, sizeof(msg),
                   "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{"
                   "\"screen\":2,"
                   "\"bms\":1,"
                   "\"vol\":1,"
                   "\"lang\":%d,"
                   "\"audio\":1,"
                   "\"mode\":1,"
                   "\"room_number\":\"1001-a\","
                   "\"fts_avail\":121,"
                   "\"fts_state\":1"
                   "},\"id\":4}\n",
                   lang);

  write_response(WR_FD, msg, (size_t)n);
  process_request(50);
}

static void send_set_state_with_lang_str(const char *lang) {
  char msg[1024];
  int n = snprintf(msg, sizeof(msg),
                   "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{"
                   "\"screen\":2,"
                   "\"bms\":1,"
                   "\"vol\":1,"
                   "\"lang\":\"%s\","
                   "\"audio\":1,"
                   "\"mode\":1,"
                   "\"room_number\":\"1001-a\","
                   "\"fts_avail\":121,"
                   "\"fts_state\":1"
                   "},\"id\":4}\n",
                   lang);

  write_response(WR_FD, msg, (size_t)n);
  process_request(50);
}

static void test_lang_param(void) {
  setv_variant_cmd(1, 127);
  send_set_state_with_lang_str("Spanish");
  lv_test_wait(300);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(300);
  lv_test_mouse_click_at(350, 110);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-29-lang_str.png"),
                           "Screenshot comparison failed for lang_str ");
  lv_test_mouse_click_at(445, 30);
  send_set_state_with_lang_int(1);
  TEST_ASSERT_TRUE_MESSAGE(wait_for_error_line(1000),
                           "Expected error response for invalid variant but none received");
}

static void send_set_state_with_occ_size_int(int occ_size) {
  char msg[1024];
  int n = snprintf(msg, sizeof(msg),
                   "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{"
                   "\"screen\":2,"
                   "\"fts_avail\":127,"
                   "\"fts_state\":4,"
                   "\"bed_pos\":1,"
                   "\"bed_wid\":60,"
                   "\"occ_size\":%d,"
                   "\"mon_start\":\"2130\","
                   "\"mon_end\":\"0700\","
                   "\"bms\":2,"
                   "\"vol\":2,"
                   "\"lang\":\"English\","
                   "\"mode\":4,"
                   "\"room_number\":\"1001-a\""
                   "},\"id\":1}\n",
                   occ_size);

  write_response(WR_FD, msg, (size_t)n);
  process_request(50);
}

static void send_set_state_with_occ_size_str(const char *occ_size) {
  char msg[1024];
  int n = snprintf(msg, sizeof(msg),
                   "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{"
                   "\"screen\":2,"
                   "\"fts_avail\":127,"
                   "\"fts_state\":4,"
                   "\"bed_pos\":1,"
                   "\"bed_wid\":60,"
                   "\"occ_size\":\"%s\","
                   "\"mon_start\":\"2130\","
                   "\"mon_end\":\"0700\","
                   "\"bms\":2,"
                   "\"vol\":2,"
                   "\"lang\":\"English\","
                   "\"mode\":4,"
                   "\"room_number\":\"1001-a\""
                   "},\"id\":1}\n",
                   occ_size);

  write_response(WR_FD, msg, (size_t)n);
  process_request(50);
}

static void test_occ_size_param(void) {
  setv_variant_cmd(1, 127);
  send_set_state_with_occ_size_int(1);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(300);
  lv_test_mouse_click_at(330, 330);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-30-OS_1.png"),
                           "Screenshot comparison failed for os 1 ");
  lv_test_mouse_click_at(445, 30);
  send_set_state_with_occ_size_int(2);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(300);
  lv_test_mouse_click_at(330, 330);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-31-OS_2.png"),
                           "Screenshot comparison failed for os 2 ");
  lv_test_mouse_click_at(445, 30);
  send_set_state_with_occ_size_int(3);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(300);
  lv_test_mouse_click_at(330, 330);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-32-OS_3.png"),
                           "Screenshot comparison failed for os 3 ");
  send_set_state_with_occ_size_str("one");
  TEST_ASSERT_TRUE_MESSAGE(wait_for_error_line(1000),
                           "Expected error response for invalid variant but none received");
  send_set_state_with_occ_size_int(6);
  TEST_ASSERT_TRUE_MESSAGE(wait_for_error_line(1000),
                           "Expected error response for invalid variant but none received");
}

static void send_set_state_with_mode_int(int mode) {
  char msg[1024];
  int n = snprintf(msg, sizeof(msg),
                   "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{"
                   "\"screen\":2,"
                   "\"bms\":1,"
                   "\"mode\":%d,"
                   "\"room_number\":\"1001-a\","
                   "\"fts_avail\":121,"
                   "\"fts_state\":1"
                   "},\"id\":4}\n",
                   mode);

  write_response(WR_FD, msg, (size_t)n);
  process_request(50);
}

static void send_set_state_with_mode_str(const char *mode) {
  char msg[1024];
  int n = snprintf(msg, sizeof(msg),
                   "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{"
                   "\"screen\":2,"
                   "\"bms\":1,"
                   "\"mode\":\"%s\","
                   "\"room_number\":\"1001-a\","
                   "\"fts_avail\":121,"
                   "\"fts_state\":1"
                   "},\"id\":4}\n",
                   mode);

  write_response(WR_FD, msg, (size_t)n);
  process_request(50);
}

static void test_mode_param(void) {
  setv_variant_cmd(1, 127);
  send_set_state_with_mode_int(1);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-33-Mode_1.png"),
                           "Screenshot comparison failed for mode 1 ");
  send_set_state_with_mode_int(2);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-34-Mode_2.png"),
                           "Screenshot comparison failed for mode 2 ");
  send_set_state_with_mode_int(3);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-35-Mode_3.png"),
                           "Screenshot comparison failed for mode 3 ");
  send_set_state_with_mode_int(4);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-36-Mode_4.png"),
                           "Screenshot comparison failed for mode 4 ");
  send_set_state_with_mode_int(5);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-37-Mode_5.png"),
                           "Screenshot comparison failed for mode 5 ");
  send_set_state_with_mode_int(-5);
  TEST_ASSERT_TRUE_MESSAGE(wait_for_error_line(1000),
                           "Expected error response for invalid variant but none received");
  send_set_state_with_mode_int(7);
  TEST_ASSERT_TRUE_MESSAGE(wait_for_error_line(1000),
                           "Expected error response for invalid variant but none received");
  send_set_state_with_mode_str("two");
  TEST_ASSERT_TRUE_MESSAGE(wait_for_error_line(1000),
                           "Expected error response for invalid variant but none received");
}

static void send_set_state_with_pause_tmr(int pause_tmr) {
  char msg[1024];
  int n = snprintf(msg, sizeof(msg),
                   "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{"
                   "\"screen\":2,"
                   "\"bms\":1,"
                   "\"mode\":1,"
                   "\"room_number\":\"1001-a\","
                   "\"pause_tmr\":%d,"
                   "\"fts_avail\":121,"
                   "\"fts_state\":1"
                   "},\"id\":4}\n",
                   pause_tmr);

  write_response(WR_FD, msg, (size_t)n);
  process_request(50);
}

static void test_pause_tmr_param(void) {
  setv_variant_cmd(1, 127);
  send_set_state_with_pause_tmr(60);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-38-pause_tmr_60.png"),
      "Screenshot comparison failed for pause_tmr_60 ");
  send_set_state_with_pause_tmr(65535);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-39-pause_tmr_65535.png"),
      "Screenshot comparison failed for pause_tmr_65535 ");
}

static void send_set_state_with_alert_int(int alert) {
  char msg[1024];
  int n = snprintf(msg, sizeof(msg),
                   "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{"
                   "\"screen\":2,"
                   "\"bms\":1,"
                   "\"mode\":1,"
                   "\"room_number\":\"1001-a\","
                   "\"alert\":%d,"
                   "\"fts_avail\":121,"
                   "\"fts_state\":1"
                   "},\"id\":4}\n",
                   alert);

  write_response(WR_FD, msg, (size_t)n);
  process_request(50);
}

static void test_alert_param(void) {
  setv_variant_cmd(1, 127);
  send_set_state_with_alert_int(1);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-40-alert_1.png"),
                           "Screenshot comparison failed for alert_1 ");
  send_set_state_with_alert_int(2);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-41-alert_2.png"),
                           "Screenshot comparison failed for alert_2 ");
  send_set_state_with_alert_int(101);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-42-alert_101.png"),
                           "Screenshot comparison failed for alert_101 ");
  send_set_state_with_alert_int(102);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-43-alert_102.png"),
                           "Screenshot comparison failed for alert_102 ");
  send_set_state_with_alert_int(-102);
  TEST_ASSERT_TRUE_MESSAGE(wait_for_error_line(1000),
                           "Expected error response for invalid variant but none received");
}

static void send_set_state_with_alert_title(const char *alert_title) {
  char msg[1024];
  int n = snprintf(msg, sizeof(msg),
                   "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{"
                   "\"screen\":2,"
                   "\"bms\":1,"
                   "\"mode\":1,"
                   "\"room_number\":\"1001-a\","
                   "\"alert_title\":\"%s\","
                   "\"fts_avail\":121,"
                   "\"fts_state\":1"
                   "},\"id\":4}\n",
                   alert_title);

  write_response(WR_FD, msg, (size_t)n);
  process_request(50);
}

static void test_alert_title_param(void) {
  setv_variant_cmd(1, 127);
  send_set_state_with_alert_title("Exit Alert");
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-44-alert_title.png"),
                           "Screenshot comparison failed for alert_title ");
  send_set_state_with_alert_title("abcdefghijklmnopqrstuvwxyzLJASFHB9UQWEQIWDOABCDEFGHIJKL");
  TEST_ASSERT_TRUE_MESSAGE(wait_for_error_line(1000),
                           "Expected error response for invalid variant but none received");
}

static void send_set_state_with_syserr(int syserr) {
  char msg[1024];
  int n = snprintf(msg, sizeof(msg),
                   "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{"
                   "\"screen\":2,"
                   "\"bms\":1,"
                   "\"mode\":1,"
                   "\"room_number\":\"1001-a\","
                   "\"syserr\":%d,"
                   "\"fts_avail\":121,"
                   "\"fts_state\":1"
                   "},\"id\":4}\n",
                   syserr);

  write_response(WR_FD, msg, (size_t)n);
  process_request(50);
}

static void test_syserr_param(void) {
  setv_variant_cmd(1, 127);
  send_set_state_with_syserr(11);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-45-syserr_11.png"),
                           "Screenshot comparison failed for syserr_11");
  send_set_state_with_syserr(12);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-46-syserr_12.png"),
                           "Screenshot comparison failed for syserr_12");
  send_set_state_with_syserr(13);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-47-syserr_13.png"),
                           "Screenshot comparison failed for syserr_13");
  send_set_state_with_syserr(41);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-48-syserr_41.png"),
                           "Screenshot comparison failed for syserr_41");
  send_set_state_with_syserr(42);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-49-syserr_42.png"),
                           "Screenshot comparison failed for syserr_42");
  send_set_state_with_syserr(61);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-50-syserr_61.png"),
                           "Screenshot comparison failed for syserr_61");
  send_set_state_with_syserr(0);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-51-syserr_0.png"),
                           "Screenshot comparison failed for syserr_0");
  send_set_state_with_syserr(18);
  TEST_ASSERT_TRUE_MESSAGE(wait_for_error_line(1000),
                           "Expected error response for invalid variant but none received");
}

static void send_set_state_with_syserr_title(const char *syserr_title) {
  char msg[1024];
  int n = snprintf(msg, sizeof(msg),
                   "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{"
                   "\"screen\":2,"
                   "\"bms\":1,"
                   "\"mode\":1,"
                   "\"room_number\":\"1001-a\","
                   "\"syserr_title\":\"%s\","
                   "\"fts_avail\":121,"
                   "\"fts_state\":1"
                   "},\"id\":4}\n",
                   syserr_title);

  write_response(WR_FD, msg, (size_t)n);
  process_request(50);
}

static void test_syserr_title_param(void) {
  setv_variant_cmd(1, 127);
  send_set_state_with_syserr_title("Exit Alert");
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-52-syserr_title.png"),
      "Screenshot comparison failed for alert_title ");
  send_set_state_with_syserr_title("abcdefghijklmnopqrstuvwxyzLJASFHB9UQWEQIWDOABCDEFGHIJKL");
  TEST_ASSERT_TRUE_MESSAGE(wait_for_error_line(1000),
                           "Expected error response for invalid variant but none received");
}

static void send_set_state_with_abc_int(int abc) {
  char msg[1024];
  int n = snprintf(msg, sizeof(msg),
                   "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{"
                   "\"screen\":2,"
                   "\"bms\":1,"
                   "\"mode\":1,"
                   "\"room_number\":\"1001-a\","
                   "\"abc\":%d,"
                   "\"fts_avail\":121,"
                   "\"fts_state\":1"
                   "},\"id\":4}\n",
                   abc);

  write_response(WR_FD, msg, (size_t)n);
  process_request(50);
}

static void test_abc_param(void) {
  setv_variant_cmd(1, 127);
  send_set_state_with_abc_int(1);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(300);
  lv_test_mouse_click_at(100, 110);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-53-abc_1.png"),
                           "Screenshot comparison failed for abc_1");
  lv_test_mouse_click_at(445, 30);
  send_set_state_with_abc_int(0);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(300);
  lv_test_mouse_click_at(100, 110);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-54-abc_0.png"),
                           "Screenshot comparison failed for abc_0");
  lv_test_mouse_click_at(445, 30);
  send_set_state_with_abc_int(22);
  TEST_ASSERT_TRUE_MESSAGE(wait_for_error_line(1000),
                           "Expected error response for invalid variant but none received");
}

static void send_set_state_with_room_number_str(const char *room_number) {
  char msg[1024];
  int n = snprintf(msg, sizeof(msg),
                   "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{"
                   "\"screen\":2,"
                   "\"bms\":1,"
                   "\"mode\":1,"
                   "\"room_number\":\"%s\","
                   "\"abc\":\"0\","
                   "\"fts_avail\":121,"
                   "\"fts_state\":1"
                   "},\"id\":4}\n",
                   room_number);

  write_response(WR_FD, msg, (size_t)n);
  process_request(50);
}

static void test_room_no_param(void) {
  setv_variant_cmd(2, 127);
  send_set_state_with_room_number_str("101-A");
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-55-Room_NO_Str.png"),
                           "Screenshot comparison failed for Room_NO_Str");
  send_set_state_with_room_number_str("");
  TEST_ASSERT_TRUE_MESSAGE(wait_for_error_line(1000),
                           "Expected error response for invalid variant but none received");
}

static void send_set_state_with_cal_int(int cal) {
  char msg[1024];
  int n = snprintf(msg, sizeof(msg),
                   "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{"
                   "\"screen\":2,"
                   "\"bms\":1,"
                   "\"mode\":1,"
                   "\"room_number\":\"1001-a\","
                   "\"cal\":%d,"
                   "\"fts_avail\":121,"
                   "\"fts_state\":1"
                   "},\"id\":4}\n",
                   cal);

  write_response(WR_FD, msg, (size_t)n);
  process_request(50);
}

static void test_cal_param(void) {
  setv_variant_cmd(1, 127);
  send_set_state_with_cal_int(1);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-56-Cal_1.png"),
                           "Screenshot comparison failed for Cal_1");
  send_set_state_with_cal_int(2);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-57-Cal_2.png"),
                           "Screenshot comparison failed for Cal_2");
  send_set_state_with_cal_int(4);
  TEST_ASSERT_TRUE_MESSAGE(wait_for_error_line(1000),
                           "Expected error response for invalid variant but none received");
  send_set_state_with_cal_int(-7);
  TEST_ASSERT_TRUE_MESSAGE(wait_for_error_line(1000),
                           "Expected error response for invalid variant but none received");
}

static void send_set_state_with_pr_inj_tmr_int(int pr_inj_tmr) {
  char msg[1024];
  int n = snprintf(msg, sizeof(msg),
                   "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{"
                   "\"screen\":2,"
                   "\"bms\":1,"
                   "\"mode\":1,"
                   "\"room_number\":\"1001-a\","
                   "\"cal\":\"0\","
                   "\"pr_inj_tmr\":%d,"
                   "\"fts_avail\":121,"
                   "\"fts_state\":7"
                   "},\"id\":4}\n",
                   pr_inj_tmr);

  write_response(WR_FD, msg, (size_t)n);
  process_request(50);
}

static void send_set_state_with_pr_inj_tmr_str(const char *pr_inj_tmr) {
  char msg[1024];
  int n = snprintf(msg, sizeof(msg),
                   "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{"
                   "\"screen\":2,"
                   "\"bms\":1,"
                   "\"mode\":1,"
                   "\"room_number\":\"1001-a\","
                   "\"cal\":\"0\","
                   "\"pr_inj_tmr\":\"%s\","
                   "\"fts_avail\":121,"
                   "\"fts_state\":7"
                   "},\"id\":4}\n",
                   pr_inj_tmr);

  write_response(WR_FD, msg, (size_t)n);
  process_request(50);
}

static void test_pr_inj_tmr_param(void) {
  setv_variant_cmd(1, 127);
  send_set_state_with_pr_inj_tmr_int(3000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-58-pr_injr_timer.png"),
      "Screenshot comparison failed for pr_injr_timer");
  send_set_state_with_pr_inj_tmr_int(7000);
  TEST_ASSERT_TRUE_MESSAGE(wait_for_error_line(1000),
                           "Expected error response for invalid variant but none received");
  send_set_state_with_pr_inj_tmr_str("7000");
  TEST_ASSERT_TRUE_MESSAGE(wait_for_error_line(1000),
                           "Expected error response for invalid variant but none received");
}

static void send_set_state_with_cal_alert_syserr(int cal, int alert, int syserr) {
  char msg[1024];
  int n = snprintf(msg, sizeof(msg),
                   "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{"
                   "\"screen\":2,"
                   "\"fts_avail\":127,"
                   "\"fts_state\":7,"
                   "\"bed_pos\":1,"
                   "\"bed_wid\":60,"
                   "\"occ_size\":2,"
                   "\"mon_start\":\"2130\","
                   "\"mon_end\":\"0700\","
                   "\"bms\":3,"
                   "\"audio\":1,"
                   "\"vol\":2,"
                   "\"lang\":\"English\","
                   "\"mode\":2,"
                   "\"abc\":0,"
                   "\"pause_tmr\":0,"
                   "\"syserr\":%d,"
                   "\"syserr_title\":\"\","
                   "\"alert\":%d,"
                   "\"alert_title\":\"dds\","
                   "\"cal\":%d,"
                   "\"room_number\":\"1001-a\""
                   "},\"id\":2}\n",
                   syserr, alert, cal);

  write_response(WR_FD, msg, (size_t)n);
  process_request(50);
}

static void test_multiple_param(void) {
  setv_variant_cmd(1, 127);
  send_set_state_with_cal_alert_syserr(1, 1, 11);
  TEST_ASSERT_TRUE_MESSAGE(wait_for_error_line(1000),
                           "Expected error response for invalid variant but none received");
}

static void send_set_toast() {
  char msg[1024];
  int n = snprintf(msg, sizeof(msg),
                   "{\"jsonrpc\":\"2.0\",\"method\":\"set_toast\",\"params\":{\"message\":"
                   "\"qwertyuiopasdfghjklzxcvb\"},\"id\":2}\n");

  write_response(WR_FD, msg, (size_t)n);
  process_request(50);
}

static void test_toast_param(void) {
  setv_variant_cmd(1, 127);
  send_set_toast();
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-CMD/CMD-59-toast_param.png"),
                           "Screenshot comparison failed for toast_param");
  char msg[1024];
  int n = snprintf(msg, sizeof(msg),
                   "{\"jsonrpc\":\"2.0\",\"method\":\"set_toast\",\"params\":{\"message\":"
                   "\"qwertyuiopasdfghjklzxcvbnmhihello\"},\"id\":2}\n");

  write_response(WR_FD, msg, (size_t)n);
  process_request(50);
  TEST_ASSERT_TRUE_MESSAGE(wait_for_error_line(1000),
                           "Expected error response for invalid variant but none received");
  n = snprintf(
      msg, sizeof(msg),
      "{\"jsonrpc\":\"2.0\",\"method\":\"set_toast\",\"params\":{\"message\":2},\"id\":2}\n");

  write_response(WR_FD, msg, (size_t)n);
  process_request(50);
  TEST_ASSERT_TRUE_MESSAGE(wait_for_error_line(1000),
                           "Expected error response for invalid variant but none received");
  n = snprintf(msg, sizeof(msg),
               "{\"jsonrpc\":\"2.0\",\"method\":\"set_toast\",\"params\":{\"message\":"
               "\"\"},\"id\":2}\n");
  write_response(WR_FD, msg, (size_t)n);
  process_request(50);
}

static int send_and_validate_get_device_info(uint32_t timeout_ms) {
  char msg[1024];
  char line[1024];
  uint32_t waited = 0;
  int n =
      snprintf(msg, sizeof(msg), "{\"jsonrpc\":\"2.0\",\"method\":\"get_device_info\",\"id\":2}\n");

  write_response(WR_FD, msg, (size_t)n);
  process_request(50);
  while (waited < timeout_ms) {
    while (read_line_nb(RD_FD, line, sizeof(line))) {
      if (strstr(line, "\"mfr\"") && strstr(line, "\"model\"") && strstr(line, "\"serial\"") &&
          strstr(line, "\"hw_ver\"") && strstr(line, "\"fw_ver\"")) {
        return 1;
      }
    }
    wait_ms(10);
    waited += 10;
  }
  return 0;
}

static int send_and_validate_get_device_status(uint32_t timeout_ms) {
  char msg[1024];
  char line[1024];
  uint32_t waited = 0;
  int n = snprintf(msg, sizeof(msg),
                   "{\"jsonrpc\":\"2.0\",\"method\":\"get_device_status\",\"id\":2}\n");

  write_response(WR_FD, msg, (size_t)n);
  process_request(50);
  while (waited < timeout_ms) {
    while (read_line_nb(RD_FD, line, sizeof(line))) {
      if (strstr(line, "\"uptime\"")) {
        return 1;
      }
    }
    wait_ms(10);
    waited += 10;
  }
  return 0;
}

static void test_get_device_info(void) {
  setv_variant_cmd(1, 127);
  send_and_validate_get_device_info(1500);
}

static void test_get_device_status(void) {
  setv_variant_cmd(1, 127);
  send_and_validate_get_device_status(1500);
}

static void test_read_TO(void) {
  setv_variant_cmd(1, 127);
  lv_test_mouse_click_at(240, 240);
  lv_test_wait(100);
  lv_test_mouse_click_at(12, 12);
  lv_test_wait(3000);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-TO/TO-01-RO_Timeout.png"),
                           "Screenshot comparison failed for RO_Timeout");
  setv_variant_cmd(2, 127);
  send_set_state_with_cal_int(1);
  lv_test_wait(100);
  lv_test_mouse_click_at(120, 420);
  lv_test_wait(3000);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare("ref_imgsNKD/NKD-TO/TO-02-GO_RO_Timeout.png"),
                           "Screenshot comparison failed for RO_Timeout");
}

static void test_system_TO(void) {
  setv_variant_cmd(1, 127);
  send_set_state_with_cal_int(1);
  wait_ms(300000);
  lv_test_screenshot_compare("ref_imgsNKD/NKD-TO/TO-03-sys_Timeout.png");
  TEST_ASSERT_TRUE(true);
  setv_variant_cmd(2, 127);
  send_set_state_with_cal_int(1);
  wait_ms(300000);
  lv_test_screenshot_compare("ref_imgsNKD/NKD-TO/TO-04-GO_sys_Timeout.png");
  TEST_ASSERT_TRUE(true);
}

TEST_GROUP(NKD_CMD);

TEST_GROUP(NKD_TO);

TEST_SETUP(NKD_CMD) {
  if (g_bail) {
    TEST_IGNORE_MESSAGE("Skipped after first failure");
  }
}

TEST_TEAR_DOWN(NKD_CMD) {
  drain_pipe_now();
  if (Unity.CurrentTestFailed) {
    g_bail = 1;
  }
}

TEST_SETUP(NKD_TO) {
  if (g_bail) {
    TEST_IGNORE_MESSAGE("Skipped after first failure");
  }
}

TEST_TEAR_DOWN(NKD_TO) {
  drain_pipe_now();
  if (Unity.CurrentTestFailed) {
    g_bail = 1;
  }
}

TEST(NKD_CMD, NKD_CMD_001) { test_setv_command(); }

TEST(NKD_CMD, NKD_CMD_002) { test_invalid_setv_cmd(); }

TEST(NKD_CMD, NKD_CMD_003) { test_edge_cases_fts_avail(); }

TEST(NKD_CMD, NKD_CMD_004) { test_missing_params(); }

TEST(NKD_CMD, NKD_CMD_005) { test_fts_options_edge_cases(); }

TEST(NKD_CMD, NKD_CMD_006) { test_screen_param(); }

TEST(NKD_CMD, NKD_CMD_007) { test_bms_param(); }

TEST(NKD_CMD, NKD_CMD_008) { test_bed_pos_param(); }

TEST(NKD_CMD, NKD_CMD_009) { test_bed_wid_param(); }

TEST(NKD_CMD, NKD_CMD_010) { test_occ_size_param(); }

TEST(NKD_CMD, NKD_CMD_011) { test_vol_param(); }

TEST(NKD_CMD, NKD_CMD_012) { test_audio_param(); }

TEST(NKD_CMD, NKD_CMD_013) { test_lang_param(); }

TEST(NKD_CMD, NKD_CMD_014) { test_mode_param(); }

TEST(NKD_CMD, NKD_CMD_015) { test_pause_tmr_param(); }

TEST(NKD_CMD, NKD_CMD_016) { test_alert_param(); }

TEST(NKD_CMD, NKD_CMD_017) { test_alert_title_param(); }

TEST(NKD_CMD, NKD_CMD_018) { test_syserr_param(); }

TEST(NKD_CMD, NKD_CMD_019) { test_syserr_title_param(); }

TEST(NKD_CMD, NKD_CMD_020) { test_abc_param(); }

TEST(NKD_CMD, NKD_CMD_021) { test_room_no_param(); }

TEST(NKD_CMD, NKD_CMD_022) { test_cal_param(); }

TEST(NKD_CMD, NKD_CMD_023) { test_pr_inj_tmr_param(); }

TEST(NKD_CMD, NKD_CMD_024) { test_multiple_param(); }

TEST(NKD_CMD, NKD_CMD_027) { test_toast_param(); }

TEST(NKD_CMD, NKD_CMD_028) { test_get_device_info(); }

TEST(NKD_CMD, NKD_CMD_029) { test_get_device_status(); }

TEST(NKD_TO, NKD_TO_001) { test_read_TO(); }

TEST(NKD_TO, NKD_TO_002) { test_system_TO(); }

TEST_GROUP_RUNNER(NKD_CMD) {
  RUN_TEST_CASE(NKD_CMD, NKD_CMD_001);
  RUN_TEST_CASE(NKD_CMD, NKD_CMD_002);
  RUN_TEST_CASE(NKD_CMD, NKD_CMD_003);
  RUN_TEST_CASE(NKD_CMD, NKD_CMD_004);
  RUN_TEST_CASE(NKD_CMD, NKD_CMD_005);
  RUN_TEST_CASE(NKD_CMD, NKD_CMD_006);
  RUN_TEST_CASE(NKD_CMD, NKD_CMD_007);
  RUN_TEST_CASE(NKD_CMD, NKD_CMD_008);
  RUN_TEST_CASE(NKD_CMD, NKD_CMD_009);
  RUN_TEST_CASE(NKD_CMD, NKD_CMD_010);
  RUN_TEST_CASE(NKD_CMD, NKD_CMD_011);
  RUN_TEST_CASE(NKD_CMD, NKD_CMD_012);
  RUN_TEST_CASE(NKD_CMD, NKD_CMD_013);
  RUN_TEST_CASE(NKD_CMD, NKD_CMD_014);
  RUN_TEST_CASE(NKD_CMD, NKD_CMD_015);
  RUN_TEST_CASE(NKD_CMD, NKD_CMD_016);
  RUN_TEST_CASE(NKD_CMD, NKD_CMD_017);
  RUN_TEST_CASE(NKD_CMD, NKD_CMD_018);
  RUN_TEST_CASE(NKD_CMD, NKD_CMD_019);
  RUN_TEST_CASE(NKD_CMD, NKD_CMD_020);
  RUN_TEST_CASE(NKD_CMD, NKD_CMD_021);
  RUN_TEST_CASE(NKD_CMD, NKD_CMD_022);
  RUN_TEST_CASE(NKD_CMD, NKD_CMD_023);
  RUN_TEST_CASE(NKD_CMD, NKD_CMD_024);
  RUN_TEST_CASE(NKD_CMD, NKD_CMD_027);
  RUN_TEST_CASE(NKD_CMD, NKD_CMD_028);
  RUN_TEST_CASE(NKD_CMD, NKD_CMD_029);
}

TEST_GROUP_RUNNER(NKD_TO) {
  RUN_TEST_CASE(NKD_TO, NKD_TO_001);
  RUN_TEST_CASE(NKD_TO, NKD_TO_002);
}