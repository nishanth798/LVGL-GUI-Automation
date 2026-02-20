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

static void test_welcome_screen(void) {
  char req[256];
  int len = snprintf(req, sizeof(req),
                     "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\","
                     "\"params\":{\"screen\":6,\"fts_avail\":121,"
                     "\"fts_state\":7,\"mode\":2,\"room_number\":\"1001-a\"},\"id\":1}\n");
  write_response(WR_FD, req, (size_t)len);
  process_request(50);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-COM/GO-00-NKD_COM_001-Welcome Screen.png"),
      "Screenshot comparison failed for Welcome Screen");
}

static void setv_variant_cmd(void) {
  const char *setv = "{\"jsonrpc\":\"2.0\",\"method\":\"set_variant\",\"params\":{\"variant\":2,"
                     "\"fts_avail\":127},\"id\":1}\n";

  write_response(WR_FD, setv, strlen(setv));
  process_request(50);
}

static void test_deactivate_screen(void) {
  setv_variant_cmd();
  lv_test_wait(100);
  char req[256];
  int len = snprintf(req, sizeof(req),
                     "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\","
                     "\"params\":{\"screen\":5,\"fts_avail\":121,"
                     "\"fts_state\":7,\"mode\":2,\"room_number\":\"1001-a\"},\"id\":1}\n");

  write_response(WR_FD, req, (size_t)len);
  process_request(50);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-COM/GO-01-NKD_COM_002-Deactivate Screen.png"),
      "Screenshot comparison failed for Welcome Screen");
}

static void test_timer_on_mon_ale_screen(void) {
  setv_variant_cmd();
  lv_test_wait(100);
  char resp[768];
  int len = snprintf(
      resp, sizeof resp,
      "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
      "{\"screen\":2,\"fts_avail\":121,\"fts_state\":7,\"bed_pos\":1,\"bed_wid\":60,"
      "\"occ_size\":2,\"mon_start\":\"2130\",\"mon_end\":\"0700\",\"bms\":1,\"vol\":2,"
      "\"audio\":1,\"lang\":\"English\",\"mode\":1,\"pause_tmr\":0,\"alert\":0,\"syserr\":12,"
      "\"syserr_title\":\"\",\"abc\":0,\"room_number\":\"1001-a\"},\"id\":1}\n");

  write_response(WR_FD, resp, (size_t)len);
  process_request(50);
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-COM/GO-02-NKD_COM_003-AlertScreen_with_rep_timer.png"),
      "Screenshot comparison failed for Welcome Screen");
}

static void test_activate_screen(void) {
  setv_variant_cmd();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-01-NKD_GO_001-Activate screen.png"),
      "Screenshot comparison failed for IN-01-Activate screen");
}

static void handle_shutdown(int screen) {
  char line[512];

  for (uint32_t waited = 0; waited < 2000; waited += 10) {
    while (read_line_nb(RD_FD, line, sizeof line)) {
      if (!strstr(line, "\"method\":\"shutdown\""))
        continue;

      char resp[256];
      int len = snprintf(resp, sizeof resp,
                         "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
                         "{\"screen\":%d,\"fts_avail\":121,\"fts_state\":7,\"mode\":2,"
                         "\"room_number\":\"1001-a\"},\"id\":1}\n",
                         screen);
      write_response(WR_FD, resp, (size_t)len);
      process_request(50);
      lv_test_wait(50);
      drain_pipe_now();
      return;
    }
  }
}

static void test_shut_down_btn_functionality(void) {
  lv_test_mouse_click_at(230, 420);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-02-NKD_GO_002-ShutDown_Popup_Toast.png"),
      "Screenshot comparison failed for NKD_GO_002");
  lv_test_wait(100);
  lv_test_mouse_click_at(150, 320);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-01-NKD_GO_001-Activate screen.png"),
      "Screenshot comparison failed for IN-01-Activate screen");
  lv_test_wait(100);
  lv_test_mouse_click_at(230, 420);
  lv_test_wait(100);
  lv_test_mouse_click_at(320, 320);
  lv_test_wait(3000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-03-NKD_GO_002-ShutDown_Popup_Toast_Timeout.png"),
      "Screenshot comparison failed for NKD_GO_002_Toast_Timeout");
  lv_test_wait(1500);
  lv_test_mouse_click_at(230, 420);
  lv_test_wait(100);
  lv_test_mouse_click_at(320, 320);
  lv_test_wait(100);
  handle_shutdown(5);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-04-NKD_GO_002-ShutDown_Screen.png"),
      "Screenshot comparison failed for NKD_GO_002_Shutdown Screen");
  lv_test_wait(200);
  setv_variant_cmd();
  lv_test_wait(100);
  lv_test_mouse_click_at(230, 420);
  lv_test_wait(100);
  lv_test_mouse_click_at(320, 320);
  lv_test_wait(100);
  handle_shutdown(6);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-00-NKD_COM_001-Welcome Screen.png"),
      "Screenshot comparison failed for Welcome Screen");
}

static void Assign_room_to_system_screen(void) {
  char line[512];

  for (uint32_t waited = 0; waited < 2000; waited += 10) {
    while (read_line_nb(RD_FD, line, sizeof line)) {
      if (!strstr(line, "\"method\":\"get_room\""))
        continue;

      int id = 1;
      if (char *p = strstr(line, "\"id\":"))
        sscanf(p, "\"id\":%d", &id);

      char resp[128];
      int len =
          snprintf(resp, sizeof resp, "{\"jsonrpc\":\"2.0\",\"result\":null,\"id\":%d}\n", id);
      write_response(WR_FD, resp, (size_t)len);
      process_request(50);
      lv_test_wait(50);
      drain_pipe_now();
      return;
    }

    process_request(10);
    lv_test_wait(10);
  }

  TEST_FAIL_MESSAGE("Timed out waiting for get_room (null)");
}

void Confirm_room_assignment(void) {
  char line[512];

  for (uint32_t waited = 0; waited < 2000; waited += 10) {
    while (read_line_nb(RD_FD, line, sizeof line)) {
      if (!strstr(line, "\"method\":\"get_room\""))
        continue;

      int id = 1;
      if (char *p = strstr(line, "\"id\":"))
        sscanf(p, "\"id\":%d", &id);

      char resp[128];
      int len =
          snprintf(resp, sizeof resp, "{\"jsonrpc\":\"2.0\",\"result\":\"101\",\"id\":%d}\n", id);
      write_response(WR_FD, resp, (size_t)len);
      process_request(50);
      lv_test_wait(50);
      drain_pipe_now();
      return;
    }

    process_request(10);
    lv_test_wait(10);
  }

  TEST_FAIL_MESSAGE("Timed out waiting for get_room (\"101\")");
}

static void test_activate_btn_functionality(void) {
  setv_variant_cmd();
  lv_test_wait(100);
  lv_test_mouse_click_at(240, 200);
  lv_test_wait(3000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-03-NKD_GO_002-ShutDown_Popup_Toast_Timeout.png"),
      "Screenshot comparison failed for NKD_GO_002_Toast_Timeout");
  lv_test_wait(1500);
  lv_test_mouse_click_at(240, 200);
  lv_test_wait(100);
  Assign_room_to_system_screen();
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-05-NKD_GO_003-System_room_assignment_screen.png"),
      "Screenshot comparison failed for NKD_GO_003-System_room_assignment_screen");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
  lv_test_wait(100);
  lv_test_mouse_click_at(240, 200);
  lv_test_wait(100);
  Confirm_room_assignment();
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-06-NKD_GO_003-Confirm_room_assignment_screen.png"),
      "Screenshot comparison failed for NKD_GO_003-Confirm_room_assignment_screen");
  lv_test_mouse_click_at(445, 30);
}

static void test_presence_of_confirm_room_assignment_screen(void) {
  lv_test_mouse_click_at(240, 200);
  lv_test_wait(100);
  Confirm_room_assignment();
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-06-NKD_GO_003-Confirm_room_assignment_screen.png"),
      "Screenshot comparison failed for NKD_GO_003-Confirm_room_assignment_screen");
  lv_test_mouse_click_at(445, 30);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-01-NKD_GO_001-Activate screen.png"),
      "Screenshot comparison failed for IN-01-Activate screen");
}

static void test_validate_confirm_room_btn(void) {
  lv_test_mouse_click_at(240, 200);
  lv_test_wait(100);
  Confirm_room_assignment();
  lv_test_wait(100);
  lv_test_mouse_click_at(350, 420);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-07-NKD_GO_005-Alerts_settings_screen.png"),
      "Screenshot comparison failed for NKD_GO_005-Alerts_settings_screen");
  lv_test_mouse_click_at(445, 30);
}

void handle_get_units(void) {
  char line[512];

  for (uint32_t waited = 0; waited < 2000; waited += 10) {
    while (read_line_nb(RD_FD, line, sizeof line)) {
      if (!strstr(line, "\"method\":\"get_units\""))
        continue;

      int id = 1;
      if (char *p = strstr(line, "\"id\":"))
        sscanf(p, "\"id\":%d", &id);

      char resp[512];
      int len =
          snprintf(resp, sizeof resp,
                   "{\"jsonrpc\":\"2.0\",\"result\":["
                   "\"1\",\"2\",\"3\",\"4\",\"5\",\"6\",\"7\",\"8\",\"9\",\"10\",\"11\",\"12\","
                   "\"13\",\"14\",\"15\",\"16\",\"17\",\"18\",\"19\",\"20\",\"21\",\"22\",\"23\","
                   "\"24\",\"25\",\"26\"],\"id\":%d}\n",
                   id);

      write_response(WR_FD, resp, (size_t)len);
      process_request(50);
      lv_test_wait(50);
      drain_pipe_now();
      return;
    }

    process_request(10);
    lv_test_wait(10);
  }

  TEST_FAIL_MESSAGE("Timed out waiting for get_units");
}

static void test_validate_change_room_btn(void) {
  lv_test_mouse_click_at(240, 200);
  lv_test_wait(100);
  Confirm_room_assignment();
  lv_test_wait(100);
  lv_test_mouse_click_at(120, 420);
  lv_test_wait(100);
  handle_get_units();
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-08-NKD_GO_006-Unit_Selection_Screen.png"),
      "Screenshot comparison failed for NKD_GO_006-Unit_Selection_Screen");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_presence_of_assign_room_to_system(void) {
  lv_test_mouse_click_at(240, 200);
  lv_test_wait(100);
  Assign_room_to_system_screen();
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-05-NKD_GO_003-System_room_assignment_screen.png"),
      "Screenshot comparison failed for NKD_GO_003-System_room_assignment_screen");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-01-NKD_GO_001-Activate screen.png"),
      "Screenshot comparison failed for IN-01-Activate screen");
}

static void test_view_rooms_btn_functionality(void) {
  lv_test_mouse_click_at(240, 200);
  lv_test_wait(100);
  Assign_room_to_system_screen();
  lv_test_wait(100);
  lv_test_mouse_click_at(240, 420);
  handle_get_units();
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-08-NKD_GO_006-Unit_Selection_Screen.png"),
      "Screenshot comparison failed for NKD_GO_006-Unit_Selection_Screen");
  lv_test_mouse_click_at(445, 30);
}

void go_to_unit_selection_screen_via_view_rooms(void) {
  lv_test_mouse_click_at(240, 200);
  lv_test_wait(100);
  Assign_room_to_system_screen();
  lv_test_wait(100);
  lv_test_mouse_click_at(240, 420);
  handle_get_units();
}

void go_to_unit_selection_screen_via_change_room(void) {
  lv_test_mouse_click_at(240, 200);
  lv_test_wait(100);
  Confirm_room_assignment();
  lv_test_wait(100);
  lv_test_mouse_click_at(120, 420);
  lv_test_wait(100);
  handle_get_units();
}

static void test_presence_of_unit_selection_screen(void) {
  go_to_unit_selection_screen_via_change_room();
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-08-NKD_GO_006-Unit_Selection_Screen.png"),
      "Screenshot comparison failed for NKD_GO_006-Unit_Selection_Screen");
  lv_test_wait(100);
  lv_test_mouse_click_at(120, 420);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-06-NKD_GO_003-Confirm_room_assignment_screen.png"),
      "Screenshot comparison failed for NKD_GO_003-Confirm_room_assignment_screen");
  lv_test_wait(100);
  lv_test_mouse_click_at(120, 420);
  lv_test_wait(100);
  handle_get_units();
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-01-NKD_GO_001-Activate screen.png"),
      "Screenshot comparison failed for IN-01-Activate screen");
}

static void test_functionality_of_unit_buttons(void) {
  go_to_unit_selection_screen_via_change_room();
  lv_test_wait(300);
  lv_test_mouse_click_at(130, 110);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-09-NKD_GO_010-Unit_btns_Selection_1.png"),
      "Screenshot comparison failed for NKD_GO_010-Unit_btns_Selection_1");
  lv_test_wait(300);
  lv_test_mouse_click_at(350, 110);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-10-NKD_GO_010-Unit_btns_Selection_2.png"),
      "Screenshot comparison failed for NKD_GO_010-Unit_btns_Selection_2");
  lv_test_wait(300);
  lv_test_mouse_click_at(120, 180);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-11-NKD_GO_010-Unit_btns_Selection_3.png"),
      "Screenshot comparison failed for NKD_GO_010-Unit_btns_Selection_3");
  lv_test_wait(300);
  lv_test_mouse_click_at(350, 180);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-12-NKD_GO_010-Unit_btns_Selection_4.png"),
      "Screenshot comparison failed for NKD_GO_010-Unit_btns_Selection_4");
  lv_test_wait(300);
  lv_test_mouse_click_at(120, 260);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-13-NKD_GO_010-Unit_btns_Selection_5.png"),
      "Screenshot comparison failed for NKD_GO_010-Unit_btns_Selection_5");
  lv_test_wait(300);
  lv_test_mouse_click_at(350, 260);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-14-NKD_GO_010-Unit_btns_Selection_6.png"),
      "Screenshot comparison failed for NKD_GO_010-Unit_btns_Selection_6");
  lv_test_mouse_click_at(340, 340);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-15-NKD_GO_010-Unit_Selection_Screen_page2.png"),
      "Screenshot comparison failed for NKD_GO_006-Unit_Selection_Screen_page_2");
  lv_test_wait(300);
  lv_test_mouse_click_at(130, 110);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-16-NKD_GO_010-Unit_btns_Selection_7.png"),
      "Screenshot comparison failed for NKD_GO_010-Unit_btns_Selection_7");
  lv_test_wait(300);
  lv_test_mouse_click_at(350, 110);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-17-NKD_GO_010-Unit_btns_Selection_8.png"),
      "Screenshot comparison failed for NKD_GO_010-Unit_btns_Selection_8");
  lv_test_wait(300);
  lv_test_mouse_click_at(120, 180);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-18-NKD_GO_010-Unit_btns_Selection_9.png"),
      "Screenshot comparison failed for NKD_GO_010-Unit_btns_Selection_9");
  lv_test_wait(300);
  lv_test_mouse_click_at(350, 180);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-19-NKD_GO_010-Unit_btns_Selection_10.png"),
      "Screenshot comparison failed for NKD_GO_010-Unit_btns_Selection_10");
  lv_test_wait(300);
  lv_test_mouse_click_at(120, 260);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-20-NKD_GO_010-Unit_btns_Selection_11.png"),
      "Screenshot comparison failed for NKD_GO_010-Unit_btns_Selection_11");
  lv_test_wait(300);
  lv_test_mouse_click_at(350, 260);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-21-NKD_GO_010-Unit_btns_Selection_12.png"),
      "Screenshot comparison failed for NKD_GO_010-Unit_btns_Selection_12");
  lv_test_mouse_click_at(340, 340);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-22-NKD_GO_010-Unit_Selection_Screen_page3.png"),
      "Screenshot comparison failed for NKD_GO_006-Unit_Selection_Screen_page_3");
  lv_test_wait(300);
  lv_test_mouse_click_at(130, 110);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-23-NKD_GO_010-Unit_btns_Selection_13.png"),
      "Screenshot comparison failed for NKD_GO_010-Unit_btns_Selection_13");
  lv_test_wait(300);
  lv_test_mouse_click_at(350, 110);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-24-NKD_GO_010-Unit_btns_Selection_14.png"),
      "Screenshot comparison failed for NKD_GO_010-Unit_btns_Selection_14");
  lv_test_wait(300);
  lv_test_mouse_click_at(120, 180);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-25-NKD_GO_010-Unit_btns_Selection_15.png"),
      "Screenshot comparison failed for NKD_GO_010-Unit_btns_Selection_15");
  lv_test_wait(300);
  lv_test_mouse_click_at(350, 180);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-26-NKD_GO_010-Unit_btns_Selection_16.png"),
      "Screenshot comparison failed for NKD_GO_010-Unit_btns_Selection_16");
  lv_test_wait(300);
  lv_test_mouse_click_at(120, 260);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-27-NKD_GO_010-Unit_btns_Selection_17.png"),
      "Screenshot comparison failed for NKD_GO_010-Unit_btns_Selection_17");
  lv_test_wait(300);
  lv_test_mouse_click_at(350, 260);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-28-NKD_GO_010-Unit_btns_Selection_18.png"),
      "Screenshot comparison failed for NKD_GO_010-Unit_btns_Selection_18");
  lv_test_mouse_click_at(340, 340);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-29-NKD_GO_010-Unit_Selection_Screen_page4.png"),
      "Screenshot comparison failed for NKD_GO_006-Unit_Selection_Screen_page_4");
  lv_test_wait(300);
  lv_test_mouse_click_at(130, 110);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-30-NKD_GO_010-Unit_btns_Selection_19.png"),
      "Screenshot comparison failed for NKD_GO_010-Unit_btns_Selection_19");
  lv_test_wait(300);
  lv_test_mouse_click_at(350, 110);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-31-NKD_GO_010-Unit_btns_Selection_20.png"),
      "Screenshot comparison failed for NKD_GO_010-Unit_btns_Selection_20");
  lv_test_wait(300);
  lv_test_mouse_click_at(120, 180);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-32-NKD_GO_010-Unit_btns_Selection_21.png"),
      "Screenshot comparison failed for NKD_GO_010-Unit_btns_Selection_21");
  lv_test_wait(300);
  lv_test_mouse_click_at(350, 180);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-33-NKD_GO_010-Unit_btns_Selection_22.png"),
      "Screenshot comparison failed for NKD_GO_010-Unit_btns_Selection_22");
  lv_test_wait(300);
  lv_test_mouse_click_at(120, 260);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-34-NKD_GO_010-Unit_btns_Selection_23.png"),
      "Screenshot comparison failed for NKD_GO_010-Unit_btns_Selection_23");
  lv_test_wait(300);
  lv_test_mouse_click_at(350, 260);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-35-NKD_GO_010-Unit_btns_Selection_24.png"),
      "Screenshot comparison failed for NKD_GO_010-Unit_btns_Selection_24");
  lv_test_mouse_click_at(340, 340);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-36-NKD_GO_010-Unit_Selection_Screen_page5.png"),
      "Screenshot comparison failed for NKD_GO_006-Unit_Selection_Screen_page_5");
  lv_test_wait(300);
  lv_test_mouse_click_at(130, 110);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-37-NKD_GO_010-Unit_btns_Selection_25.png"),
      "Screenshot comparison failed for NKD_GO_010-Unit_btns_Selection_25");
  lv_test_wait(300);
  lv_test_mouse_click_at(350, 110);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-38-NKD_GO_010-Unit_btns_Selection_26.png"),
      "Screenshot comparison failed for NKD_GO_010-Unit_btns_Selection_26");
  lv_test_mouse_click_at(445, 30);
}

static void test_forward_buttons_on_unit_selection_screen(void) {
  go_to_unit_selection_screen_via_change_room();
  lv_test_wait(100);
  lv_test_mouse_click_at(350, 340);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-15-NKD_GO_010-Unit_Selection_Screen_page2.png"),
      "Screenshot comparison failed for NKD_GO_006-Unit_Selection_Screen_page_2");
  lv_test_wait(100);
  lv_test_mouse_click_at(350, 340);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-22-NKD_GO_010-Unit_Selection_Screen_page3.png"),
      "Screenshot comparison failed for NKD_GO_006-Unit_Selection_Screen_page_3");
  lv_test_wait(100);
  lv_test_mouse_click_at(340, 340);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-29-NKD_GO_010-Unit_Selection_Screen_page4.png"),
      "Screenshot comparison failed for NKD_GO_006-Unit_Selection_Screen_page_4");
  lv_test_wait(100);
  lv_test_mouse_click_at(340, 340);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-36-NKD_GO_010-Unit_Selection_Screen_page5.png"),
      "Screenshot comparison failed for NKD_GO_006-Unit_Selection_Screen_page_5");
  lv_test_wait(100);
  lv_test_mouse_click_at(45, 340);
  lv_test_wait(100);
  lv_test_mouse_click_at(430, 340); // fast forward button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-36-NKD_GO_010-Unit_Selection_Screen_page5.png"),
      "Screenshot comparison failed for NKD_GO_006-Unit_Selection_Screen_page_5");
  lv_test_mouse_click_at(445, 30);
}

static void test_backward_buttons_on_unit_selection_screen(void) {
  go_to_unit_selection_screen_via_change_room();
  lv_test_wait(100);
  lv_test_mouse_click_at(430, 340);
  lv_test_wait(100);
  lv_test_mouse_click_at(130, 340);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-29-NKD_GO_010-Unit_Selection_Screen_page4.png"),
      "Screenshot comparison failed for NKD_GO_006-Unit_Selection_Screen_page_4");
  lv_test_wait(100);
  lv_test_mouse_click_at(130, 340);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-22-NKD_GO_010-Unit_Selection_Screen_page3.png"),
      "Screenshot comparison failed for NKD_GO_006-Unit_Selection_Screen_page_3");
  lv_test_wait(100);
  lv_test_mouse_click_at(130, 340);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-15-NKD_GO_010-Unit_Selection_Screen_page2.png"),
      "Screenshot comparison failed for NKD_GO_006-Unit_Selection_Screen_page_2");
  lv_test_wait(100);
  lv_test_mouse_click_at(130, 340);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-08-NKD_GO_006-Unit_Selection_Screen.png"),
      "Screenshot comparison failed for NKD_GO_006-Unit_Selection_Screen");
  lv_test_wait(100);
  lv_test_mouse_click_at(430, 340);
  lv_test_wait(100);
  lv_test_mouse_click_at(45, 340);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-08-NKD_GO_006-Unit_Selection_Screen.png"),
      "Screenshot comparison failed for NKD_GO_006-Unit_Selection_Screen");
  lv_test_mouse_click_at(445, 30);
}

void handle_get_units_rooms(void) {
  char line[512];

  for (uint32_t waited = 0; waited < 2000; waited += 10) {
    while (read_line_nb(RD_FD, line, sizeof line)) {
      if (!strstr(line, "\"method\":\"get_rooms\""))
        continue;

      int id = 1;
      if (char *p = strstr(line, "\"id\":"))
        sscanf(p, "\"id\":%d", &id);

      char resp[768];
      int len = snprintf(
          resp, sizeof resp,
          "{\"jsonrpc\":\"2.0\",\"result\":["
          "\"101-a\",\"102-a\",\"103-a\",\"104-a\",\"105-a\",\"106-a\",\"107-a\",\"108-a\","
          "\"109-a\",\"110-a\",\"111-a\",\"112-a\",\"113-a\",\"114-a\",\"115-a\",\"116-a\","
          "\"117-a\",\"118-a\",\"119-a\",\"120-a\",\"121-a\",\"122-a\",\"123-a\",\"124-a\","
          "\"125-a\"],\"id\":%d}\n",
          id);

      write_response(WR_FD, resp, (size_t)len);
      process_request(50);
      lv_test_wait(50);
      drain_pipe_now();
      return;
    }

    process_request(10);
    lv_test_wait(10);
  }

  TEST_FAIL_MESSAGE("Timed out waiting for get_units");
}

static void test_next_btn_functionality_on_unit_selection_screen(void) {
  go_to_unit_selection_screen_via_change_room();
  lv_test_wait(100);
  lv_test_mouse_click_at(130, 110);
  lv_test_wait(100);
  lv_test_mouse_click_at(350, 420);
  lv_test_wait(100);
  handle_get_units_rooms();
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-39-NKD_GO_013-Unit_room_Selection_Screen.png"),
      "Screenshot comparison failed for NKD_GO_013-Unit_room_Selection_Screen");
  lv_test_mouse_click_at(445, 30);
}

void go_to_room_selection_screen(void) {
  go_to_unit_selection_screen_via_change_room();
  lv_test_wait(100);
  lv_test_mouse_click_at(130, 110);
  lv_test_wait(100);
  lv_test_mouse_click_at(350, 420);
  lv_test_wait(100);
  handle_get_units_rooms();
}

static void test_presence_of_room_selection_screen(void) {
  go_to_room_selection_screen();
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-01-NKD_GO_001-Activate screen.png"),
      "Screenshot comparison failed for IN-01-Activate screen");
  lv_test_wait(100);
  go_to_room_selection_screen();
  lv_test_mouse_click_at(130, 430);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-09-NKD_GO_010-Unit_btns_Selection_1.png"),
      "Screenshot comparison failed for NKD_GO_010-Unit_btns_Selection_1");
  lv_test_mouse_click_at(445, 30);
}

static void test_room_number_btns_on_screen(void) {
  go_to_room_selection_screen();
  lv_test_wait(100);
  lv_test_mouse_click_at(130, 110);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-40-NKD_GO_015-room_number_Selection_101.png"),
      "Screenshot comparison failed for NKD_GO_015-room_number_Selection_101");
  lv_test_wait(300);
  lv_test_mouse_click_at(350, 110);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-41-NKD_GO_015-room_number_Selection_102.png"),
      "Screenshot comparison failed for NKD_GO_015-room_number_Selection_102");
  lv_test_wait(300);
  lv_test_mouse_click_at(120, 180);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-42-NKD_GO_015-room_number_Selection_103.png"),
      "Screenshot comparison failed for NKD_GO_015-room_number_Selection_103");
  lv_test_wait(300);
  lv_test_mouse_click_at(350, 180);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-43-NKD_GO_015-room_number_Selection_104.png"),
      "Screenshot comparison failed for NKD_GO_015-room_number_Selection_104");
  lv_test_wait(300);
  lv_test_mouse_click_at(120, 260);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-44-NKD_GO_015-room_number_Selection_105.png"),
      "Screenshot comparison failed for NKD_GO_015-room_number_Selection_105");
  lv_test_wait(300);
  lv_test_mouse_click_at(350, 260);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-45-NKD_GO_015-room_number_Selection_106.png"),
      "Screenshot comparison failed for NKD_GO_015-room_number_Selection_106");
  lv_test_mouse_click_at(340, 340);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-46-NKD_GO_015-Room_Selection_Screen_page2.png"),
      "Screenshot comparison failed for NKD_GO_015-Room_Selection_Screen_page_2");
  lv_test_wait(300);
  lv_test_mouse_click_at(130, 110);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-47-NKD_GO_015-room_number_Selection_107.png"),
      "Screenshot comparison failed for NKD_GO_015-room_number_Selection_107");
  lv_test_wait(300);
  lv_test_mouse_click_at(350, 110);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-48-NKD_GO_015-room_number_Selection_108.png"),
      "Screenshot comparison failed for NKD_GO_015-room_number_Selection_108");
  lv_test_wait(300);
  lv_test_mouse_click_at(120, 180);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-49-NKD_GO_015-room_number_Selection_109.png"),
      "Screenshot comparison failed for NKD_GO_015-room_number_Selection_109");
  lv_test_wait(300);
  lv_test_mouse_click_at(350, 180);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-50-NKD_GO_015-room_number_Selection_110.png"),
      "Screenshot comparison failed for NKD_GO_015-room_number_Selection_110");
  lv_test_wait(300);
  lv_test_mouse_click_at(120, 260);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-51-NKD_GO_015-room_number_Selection_111.png"),
      "Screenshot comparison failed for NKD_GO_015-room_number_Selection_111");
  lv_test_wait(300);
  lv_test_mouse_click_at(350, 260);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-52-NKD_GO_015-room_number_Selection_112.png"),
      "Screenshot comparison failed for NKD_GO_015-room_number_Selection_112");
  lv_test_mouse_click_at(340, 340);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-53-NKD_GO_015-Room_Selection_Screen_page3.png"),
      "Screenshot comparison failed for NKD_GO_015-Room_Selection_Screen_page_3");
  lv_test_wait(300);
  lv_test_mouse_click_at(130, 110);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-54-NKD_GO_015-room_number_Selection_113.png"),
      "Screenshot comparison failed for NKD_GO_015-room_number_Selection_113");
  lv_test_wait(300);
  lv_test_mouse_click_at(350, 110);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-55-NKD_GO_015-room_number_Selection_114.png"),
      "Screenshot comparison failed for NKD_GO_015-room_number_Selection_114");
  lv_test_wait(300);
  lv_test_mouse_click_at(120, 180);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-56-NKD_GO_015-room_number_Selection_115.png"),
      "Screenshot comparison failed for NKD_GO_015-room_number_Selection_115");
  lv_test_wait(300);
  lv_test_mouse_click_at(350, 180);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-57-NKD_GO_015-room_number_Selection_116.png"),
      "Screenshot comparison failed for NKD_GO_015-room_number_Selection_116");
  lv_test_wait(300);
  lv_test_mouse_click_at(120, 260);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-58-NKD_GO_015-room_number_Selection_117.png"),
      "Screenshot comparison failed for NKD_GO_015-room_number_Selection_117");
  lv_test_wait(300);
  lv_test_mouse_click_at(350, 260);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-59-NKD_GO_015-room_number_Selection_118.png"),
      "Screenshot comparison failed for NKD_GO_015-room_number_Selection_118");
  lv_test_mouse_click_at(340, 340);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-60-NKD_GO_015-Room_Selection_Screen_page4.png"),
      "Screenshot comparison failed for NKD_GO_015-Room_Selection_Screen_page_4");
  lv_test_wait(300);
  lv_test_mouse_click_at(130, 110);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-61-NKD_GO_015-room_number_Selection_119.png"),
      "Screenshot comparison failed for NKD_GO_015-room_number_Selection_119");
  lv_test_wait(300);
  lv_test_mouse_click_at(350, 110);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-62-NKD_GO_015-room_number_Selection_120.png"),
      "Screenshot comparison failed for NKD_GO_015-room_number_Selection_120");
  lv_test_wait(300);
  lv_test_mouse_click_at(120, 180);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-63-NKD_GO_015-room_number_Selection_121.png"),
      "Screenshot comparison failed for NKD_GO_015-room_number_Selection_121");
  lv_test_wait(300);
  lv_test_mouse_click_at(350, 180);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-64-NKD_GO_015-room_number_Selection_122.png"),
      "Screenshot comparison failed for NKD_GO_015-room_number_Selection_122");
  lv_test_wait(300);
  lv_test_mouse_click_at(120, 260);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-65-NKD_GO_015-room_number_Selection_123.png"),
      "Screenshot comparison failed for NKD_GO_015-room_number_Selection_123");
  lv_test_wait(300);
  lv_test_mouse_click_at(350, 260);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-66-NKD_GO_015-room_number_Selection_124.png"),
      "Screenshot comparison failed for NKD_GO_015-room_number_Selection_124");
  lv_test_mouse_click_at(340, 340);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-67-NKD_GO_015-Room_Selection_Screen_page5.png"),
      "Screenshot comparison failed for NKD_GO_015-Room_Selection_Screen_page_4");
  lv_test_wait(300);
  lv_test_mouse_click_at(130, 110);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-68-NKD_GO_015-room_number_Selection_125.png"),
      "Screenshot comparison failed for NKD_GO_015-room_number_Selection_125");
  lv_test_mouse_click_at(445, 30);
}

static void test_assigning_room_pop_up_screen(void) {
  go_to_room_selection_screen();
  lv_test_wait(100);
  lv_test_mouse_click_at(130, 110);
  lv_test_wait(100);
  lv_test_mouse_click_at(350, 420);
  lv_test_wait(200);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-69-NKD_GO_016-Assigning_room_pup_up.png"),
      "Screenshot comparison failed for NKD_GO_016-Assigning_room_pup_up");
  lv_test_mouse_click_at(240, 350);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-70-NKD_GO_016-room_Selection_timeout_toast.png"),
      "Screenshot comparison failed for NKD_GO_016-room_Selection_timeout_toast");
  lv_test_wait(100);
  lv_test_mouse_click_at(350, 420);
  wait_ms(60000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-40-NKD_GO_015-room_number_Selection_101.png"),
      "Screenshot comparison failed for NKD_GO_015-room_number_Selection_101");
  lv_test_mouse_click_at(445, 30);
}

void reply_set_room_error(void) {
  char line[512];

  for (uint32_t waited = 0; waited < 2000; waited += 10) {
    while (read_line_nb(RD_FD, line, sizeof line)) {
      if (!strstr(line, "\"method\":\"set_room\""))
        continue;

      int id = 1;
      if (char *p = strstr(line, "\"id\":"))
        sscanf(p, "\"id\":%d", &id);

      char resp[256];
      int len = snprintf(resp, sizeof resp,
                         "{\"jsonrpc\":\"2.0\",\"error\":{\"code\":-32001,\"message\":"
                         "\"appropriate message\"},\"id\":%d}\n",
                         id);

      write_response(WR_FD, resp, (size_t)len);
      process_request(50);
      lv_test_wait(50);
      drain_pipe_now();
      return;
    }

    process_request(10);
    lv_test_wait(10);
  }

  TEST_FAIL_MESSAGE("Timed out waiting for set_room (error)");
}

void reply_set_room_result_101(void) {
  char line[512];

  for (uint32_t waited = 0; waited < 2000; waited += 10) {
    while (read_line_nb(RD_FD, line, sizeof line)) {
      if (!strstr(line, "\"method\":\"set_room\""))
        continue;

      int id = 1;
      if (char *p = strstr(line, "\"id\":"))
        sscanf(p, "\"id\":%d", &id);

      char resp[128];
      int len =
          snprintf(resp, sizeof resp, "{\"jsonrpc\":\"2.0\",\"result\":\"101\",\"id\":%d}\n", id);

      write_response(WR_FD, resp, (size_t)len);
      process_request(50);
      lv_test_wait(50);
      drain_pipe_now();
      return;
    }

    process_request(10);
    lv_test_wait(10);
  }

  TEST_FAIL_MESSAGE("Timed out waiting for set_room (\"101\")");
}

void reply_set_room_result_null(void) {
  char line[512];

  for (uint32_t waited = 0; waited < 2000; waited += 10) {
    while (read_line_nb(RD_FD, line, sizeof line)) {
      if (!strstr(line, "\"method\":\"set_room\""))
        continue;

      int id = 1;
      if (char *p = strstr(line, "\"id\":"))
        sscanf(p, "\"id\":%d", &id);

      char resp[128];
      int len =
          snprintf(resp, sizeof resp, "{\"jsonrpc\":\"2.0\",\"result\":null,\"id\":%d}\n", id);

      write_response(WR_FD, resp, (size_t)len);
      process_request(50);
      lv_test_wait(50);
      drain_pipe_now();
      return;
    }

    process_request(10);
    lv_test_wait(10);
  }

  TEST_FAIL_MESSAGE("Timed out waiting for set_room (null)");
}

static void test_room_assignment_failed_pop_up(void) {
  go_to_room_selection_screen();
  lv_test_wait(100);
  lv_test_mouse_click_at(130, 110);
  lv_test_wait(100);
  lv_test_mouse_click_at(350, 420);
  lv_test_wait(100);
  reply_set_room_error();
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-71-NKD_GO_017-Room_Assingment_Failed.png"),
      "Screenshot comparison failed for NKD_GO_017-Room_Assingment_Failed");
  lv_test_wait(100);
  lv_test_mouse_click_at(150, 370);
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_cancel_btn_on_failure_pop_up(void) {
  go_to_room_selection_screen();
  lv_test_wait(100);
  lv_test_mouse_click_at(130, 110);
  lv_test_wait(100);
  lv_test_mouse_click_at(350, 420);
  lv_test_wait(100);
  reply_set_room_error();
  lv_test_wait(100);
  lv_test_mouse_click_at(150, 370);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-40-NKD_GO_015-room_number_Selection_101.png"),
      "Screenshot comparison failed for NKD_GO_015-room_number_Selection_101");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_try_again_btn_on_failure_pop_up(void) {
  go_to_room_selection_screen();
  lv_test_wait(100);
  lv_test_mouse_click_at(130, 110);
  lv_test_wait(100);
  lv_test_mouse_click_at(350, 420);
  lv_test_wait(100);
  reply_set_room_error();
  lv_test_wait(100);
  lv_test_mouse_click_at(320, 370);
  char line[512];
  bool got_req = false;

  for (uint32_t waited = 0; waited < 2000 && !got_req; waited += 10) {
    while (read_line_nb(RD_FD, line, sizeof line)) {
      if (strstr(line, "\"method\":\"set_room\"")) {
        got_req = true;
        break;
      }
    }
    if (!got_req) {
      process_request(10);
      lv_test_wait(10);
    }
  }

  if (!got_req) {
    TEST_FAIL_MESSAGE("Timed out waiting for set_room request");
  }
  wait_ms(60000);
  lv_test_mouse_click_at(445, 30);
}

static void test_system_warning_pop_up(void) {
  go_to_room_selection_screen();
  lv_test_wait(100);
  lv_test_mouse_click_at(130, 110);
  lv_test_wait(100);
  lv_test_mouse_click_at(350, 420);
  lv_test_wait(100);
  reply_set_room_result_101();
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-72-NKD_GO_020-Room_Assingment_System_Warning.png"),
      "Screenshot comparison failed for NKD_GO_020-Room_Assingment_System_Warning");
  lv_test_wait(100);
  lv_test_mouse_click_at(150, 400);
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_confirm_btn_on_pop_up(void) {
  go_to_room_selection_screen();
  lv_test_wait(100);
  lv_test_mouse_click_at(130, 110);
  lv_test_wait(100);
  lv_test_mouse_click_at(350, 420);
  lv_test_wait(100);
  reply_set_room_result_101();
  lv_test_wait(100);
  lv_test_mouse_click_at(340, 400);
  char line[512];
  bool got_req = false;

  for (uint32_t waited = 0; waited < 2000 && !got_req; waited += 10) {
    while (read_line_nb(RD_FD, line, sizeof line)) {
      if (strstr(line, "\"method\":\"set_room\"")) {
        got_req = true;
        break;
      }
    }
    if (!got_req) {
      process_request(10);
      lv_test_wait(10);
    }
  }

  if (!got_req) {
    TEST_FAIL_MESSAGE("Timed out waiting for set_room request");
  }
  wait_ms(60000);
  lv_test_mouse_click_at(445, 30);
}

static void test_room_successful_pop_up(void) {
  go_to_room_selection_screen();
  lv_test_wait(100);
  lv_test_mouse_click_at(130, 110);
  lv_test_wait(100);
  lv_test_mouse_click_at(350, 420);
  lv_test_wait(100);
  reply_set_room_result_null();
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-73-NKD_GO_022-Room_Assingment_Successful_pop_up.png"),
      "Screenshot comparison failed for NKD_GO_022-Room_Assingment_Successful_pop_up");
  lv_test_wait(100);
  lv_test_mouse_click_at(140, 340);
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_reassign_button_on_pop_up(void) {
  go_to_room_selection_screen();
  lv_test_wait(100);
  lv_test_mouse_click_at(130, 110);
  lv_test_wait(100);
  lv_test_mouse_click_at(350, 420);
  lv_test_wait(100);
  reply_set_room_result_null();
  lv_test_wait(100);
  lv_test_mouse_click_at(140, 340);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-40-NKD_GO_015-room_number_Selection_101.png"),
      "Screenshot comparison failed for NKD_GO_015-room_number_Selection_101");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

void go_to_inactivated_settigns_flow(void) {
  go_to_room_selection_screen();
  lv_test_wait(100);
  lv_test_mouse_click_at(130, 110);
  lv_test_wait(100);
  lv_test_mouse_click_at(350, 420);
  lv_test_wait(100);
  reply_set_room_result_null();
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 340);
}

static void test_continue_button_on_pop_up(void) {
  go_to_inactivated_settigns_flow();
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-07-NKD_GO_005-Alerts_settings_screen.png"),
      "Screenshot comparison failed for NKD_GO_005-Alerts_settings_screen");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_Alerts_page(void) {
  go_to_inactivated_settigns_flow();
  lv_test_wait(200);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-07-NKD_GO_005-Alerts_settings_screen.png"),
      "Screenshot comparison failed for NKD_GO_005-Alerts_settings_screen");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void toggle_btns_screen(void) {
  lv_test_mouse_click_at(400, 160); // toggle "Bed Mode" / "Chair Mode" Alert
  lv_test_wait(40);
  lv_test_mouse_click_at(400, 235); // toggle "Fall Monitoring" Alert
  lv_test_wait(40);
  lv_test_mouse_click_at(400, 300); // toggle "Reposition Reminder" Alert
  lv_test_wait(120);
}

static void test_toggles_in_alerts_page(void) {
  go_to_inactivated_settigns_flow();
  lv_test_wait(100);
  lv_test_mouse_click_at(400, 160); // toggle "Bed Mode" / "Chair Mode" Alert
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-74-NKD_GO_ACT_001_Toggle_btn.png"),
      "Screenshot comparison failed for NKD_GO_ACT_001_Toggle_btn");
  lv_test_wait(40);
  lv_test_mouse_click_at(400, 160);
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-07-NKD_GO_005-Alerts_settings_screen.png"),
      "Screenshot comparison failed for NKD_GO_005-Alerts_settings_screen");
  toggle_btns_screen();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-75-NKD_GO_ACT_001_toggles_enabled.png"),
      "Screenshot comparison failed for NKD_GO_ACT_001_toggles_enabled");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_back_btn_alerts(void) {
  go_to_inactivated_settigns_flow();
  lv_test_wait(100);
  toggle_btns_screen();
  lv_test_wait(100);
  lv_test_mouse_click_at(120, 420); // Click the "Back" button
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-40-NKD_GO_015-room_number_Selection_101.png"),
      "Screenshot comparison failed for NKD_GO_015-room_number_Selection_101");
  lv_test_mouse_click_at(350, 420);
  lv_test_wait(100);
  reply_set_room_result_null();
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 340);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-75-NKD_GO_ACT_001_toggles_enabled.png"),
      "Screenshot comparison failed for NKD_GO_ACT_001_toggles_enabled");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_X_btn_alerts(void) {
  go_to_inactivated_settigns_flow();
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-01-NKD_GO_001-Activate screen.png"),
      "Screenshot comparison failed for IN-01-Activate screen");
}

static void flow_to_sysinfo_screen(void) {
  char line[512];
  int id = 1;

  lv_test_mouse_click_at(12, 12);
  const uint32_t timeout_ms = 2000;
  uint32_t waited = 0;

  while (waited < timeout_ms) {
    // Drain any available lines
    while (read_line_nb(RD_FD, line, sizeof(line))) {
      if (strstr(line, "\"method\":\"get_sys_info\"")) {
        if (char *p = strstr(line, "\"id\":")) {
          sscanf(p, "\"id\":%d", &id);
        }
        char resp[256];
        int len = snprintf(resp, sizeof(resp),
                           "{\"jsonrpc\":\"2.0\",\"result\":{\"sw_ver\":\"v2025-02Feb-24-"
                           "46066775-dc4ff6be-nuc\",\"comp_id\":\"00000623\"},\"id\":%d}\n",
                           id);

        write_response(WR_FD, resp, (size_t)len);
        process_request(50); // let the UI consume the response + render
        lv_test_wait(50);    // give LVGL time to draw the sysinfo overlay
        return;
      }
    }
    process_request(10);
    lv_test_wait(10);
    waited += 10;
  }

  TEST_FAIL_MESSAGE("Timed out waiting for get_sys_info request");
}

static void test_sysinfo_screen(void) {
  go_to_inactivated_settigns_flow();
  lv_test_wait(100);
  toggle_btns_screen();
  lv_test_wait(100);
  flow_to_sysinfo_screen();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-76-NKD_GO_ACT_001_Sysinfo.png"),
      "Screenshot comparison failed for IN-76-Sysinfo");
  lv_test_mouse_click_at(450, 450); // Close
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-75-NKD_GO_ACT_001_toggles_enabled.png"),
      "Screenshot comparison failed for NKD_GO_ACT_001_toggles_enabled");
  flow_to_sysinfo_screen();
  lv_test_mouse_click_at(446, 30); // X
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-75-NKD_GO_ACT_001_toggles_enabled.png"),
      "Screenshot comparison failed for NKD_GO_ACT_001_toggles_enabled");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void nxt_btn_to_exit_alert_screen(void) {
  lv_test_mouse_click_at(350, 420); // Click the "Next" button
  lv_test_wait(150);
}

static void test_next_btn_click(void) {
  go_to_inactivated_settigns_flow();
  lv_test_wait(100);
  toggle_btns_screen();
  lv_test_wait(200);
  nxt_btn_to_exit_alert_screen();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-77-NKD_GO_ACT_001_nxt_btn.png"),
      "Screenshot comparison failed for IN-06-NKD_GO_ACT_001_nxt_btn");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_exit_alert_screen(void) {
  go_to_inactivated_settigns_flow();
  lv_test_wait(100);
  toggle_btns_screen();
  lv_test_wait(100);
  nxt_btn_to_exit_alert_screen();
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-77-NKD_GO_ACT_001_nxt_btn.png"),
      "Screenshot comparison failed for IN-06-NKD_GO_ACT_001_nxt_btn");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_i_button_on_exit_alert_screen(void) {
  go_to_inactivated_settigns_flow();
  lv_test_wait(100);
  toggle_btns_screen();
  lv_test_wait(100);
  nxt_btn_to_exit_alert_screen();
  lv_test_wait(100);
  flow_to_sysinfo_screen();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-76-NKD_GO_ACT_001_Sysinfo.png"),
      "Screenshot comparison failed for IN-76-Sysinfo");
  lv_test_mouse_click_at(446, 30); // Click the "X" button
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-77-NKD_GO_ACT_001_nxt_btn.png"),
      "Screenshot comparison failed for IN-06-NKD_GO_ACT_001_nxt_btn");
  lv_test_wait(100);
  flow_to_sysinfo_screen();
  lv_test_mouse_click_at(450, 450); // Click the "Close" button
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-77-NKD_GO_ACT_001_nxt_btn.png"),
      "Screenshot comparison failed for IN-06-NKD_GO_ACT_001_nxt_btn");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_x_btn_click_on_exit_alert_screen(void) {
  go_to_inactivated_settigns_flow();
  lv_test_wait(100);
  toggle_btns_screen();
  lv_test_wait(100);
  nxt_btn_to_exit_alert_screen();
  lv_test_wait(100);
  lv_test_mouse_click_at(446, 30); // Click the "X" button
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-01-NKD_GO_001-Activate screen.png"),
      "Screenshot comparison failed for IN-01-Activate screen");
}

static void clk_on_cnts_mon_button(void) {
  lv_test_mouse_click_at(240, 150); // Click the "Continue Monitoring" button
  lv_test_wait(150);
}

static void test_clickon_Ctn_Monitoring(void) {
  go_to_inactivated_settigns_flow();
  lv_test_wait(100);
  toggle_btns_screen();
  nxt_btn_to_exit_alert_screen();
  lv_test_wait(100);
  clk_on_cnts_mon_button();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-78-NKD_GO_ACT_008_Cnts_mon.png"),
      "Screenshot comparison failed for IN-78-Continue Monitoring btn click");
  lv_test_mouse_click_at(446, 30); // Click the "X" button
}

static void goto_schedl_monitoring_screen() {
  lv_test_mouse_click_at(220, 300); // Click the "Schedule Monitoring" button
  lv_test_wait(150);
}

static void test_schdl_monitoring_screen(void) {
  go_to_inactivated_settigns_flow();
  lv_test_wait(100);
  toggle_btns_screen();
  nxt_btn_to_exit_alert_screen();
  lv_test_wait(100);
  clk_on_cnts_mon_button();
  goto_schedl_monitoring_screen();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-79-NKD_GO_ACT_008_schdl_mon.png"),
      "Screenshot comparison failed for IN-79-Scheduled Monitoring btn click");
  lv_test_mouse_click_at(446, 30);
}

static void clk_on_change_strtime_button(void) {
  lv_test_mouse_click_at(360, 260); // Click the "Change" button
  lv_test_wait(150);
}

static void test_strtime_on_schdl_screen(void) {
  go_to_inactivated_settigns_flow();
  lv_test_wait(100);
  toggle_btns_screen();
  nxt_btn_to_exit_alert_screen();
  lv_test_wait(100);
  clk_on_cnts_mon_button();
  goto_schedl_monitoring_screen();
  clk_on_change_strtime_button();
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare(
                               "ref_imgsNKD/NKD-GO/NKD-ACT/GO-80-NKD_GO_ACT_009_Clk_strt_time.png"),
                           "Screenshot comparison failed for IN-80-Start time btn click");
  lv_test_wait(100);
  lv_test_mouse_click_at(34, 30); // Click the "Back" button
  lv_test_mouse_click_at(446, 30);
}

static void clk_on_endtime_change_button(void) {
  lv_test_mouse_click_at(340, 325); // Click the "End Time Change" button
  lv_test_wait(150);
}

static void test_endtime_on_schdl_screen(void) {
  go_to_inactivated_settigns_flow();
  lv_test_wait(100);
  toggle_btns_screen();
  nxt_btn_to_exit_alert_screen();
  lv_test_wait(100);
  clk_on_cnts_mon_button();
  goto_schedl_monitoring_screen();
  clk_on_endtime_change_button();
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare(
                               "ref_imgsNKD/NKD-GO/NKD-ACT/GO-81-NKD_GO_ACT_009_Clk_end_time.png"),
                           "Screenshot comparison failed for IN-81-End time");
  lv_test_mouse_click_at(34, 30); // Click the "Back" button
  lv_test_mouse_click_at(446, 30);
}

static void test_back_btn_on_strtime_screen(void) {
  go_to_inactivated_settigns_flow();
  lv_test_wait(100);
  toggle_btns_screen();
  nxt_btn_to_exit_alert_screen();
  clk_on_cnts_mon_button();
  goto_schedl_monitoring_screen();
  clk_on_change_strtime_button();
  lv_test_wait(100);
  lv_test_mouse_click_at(34, 30); // Click the "Back" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-79-NKD_GO_ACT_008_schdl_mon.png"),
      "Screenshot comparison failed for IN-79-Scheduled Monitoring btn click");
  lv_test_mouse_click_at(446, 30);
}

static void test_Xbtn_on_strtime_screen(void) {
  go_to_inactivated_settigns_flow();
  lv_test_wait(100);
  toggle_btns_screen();
  nxt_btn_to_exit_alert_screen();
  clk_on_cnts_mon_button();
  goto_schedl_monitoring_screen();
  clk_on_change_strtime_button();
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30); // Click the "X" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-01-NKD_GO_001-Activate screen.png"),
      "Screenshot comparison failed for GO-01-Activate screen");
}

static void test_up_arrow_on_strtime_screen(void) {
  go_to_inactivated_settigns_flow();
  toggle_btns_screen();
  nxt_btn_to_exit_alert_screen();
  clk_on_cnts_mon_button();
  goto_schedl_monitoring_screen();
  clk_on_change_strtime_button();
  lv_test_wait(50);
  lv_test_mouse_click_at(95, 165); // Click the "Up Arrow" button
  lv_test_wait(50);
  lv_test_mouse_click_at(240, 165); // Click the "up arrow of mins" button
  lv_test_wait(50);
  lv_test_mouse_click_at(390, 165); // Click the "PM->AM" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-82-NKD_GO_ACT_010_up_arrows_strt_time.png"),
      "Screenshot comparison failed for GO-82-Up arrows start time");
  lv_test_mouse_click_at(240, 425); // Click the "Save" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-83-NKD_G0_ACT_010_Save_strt_time.png"),
      "Screenshot comparison failed for GO-13-Save start time");
  clk_on_change_strtime_button();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-84-NKD_GO_ACT_010_Save_btn_disabled_strt_time.png"),
      "Screenshot comparison failed for GO-84-Save btn start time");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_dwn_arrows_on_strtime_screen(void) {
  go_to_inactivated_settigns_flow();
  toggle_btns_screen();
  nxt_btn_to_exit_alert_screen();
  clk_on_cnts_mon_button();
  goto_schedl_monitoring_screen();
  clk_on_change_strtime_button();
  lv_test_wait(50);
  lv_test_mouse_click_at(95, 320); // Click the "Down Arrow" button
  lv_test_wait(50);
  lv_test_mouse_click_at(240, 310); // Click the "down arrow of mins" button
  lv_test_wait(50);
  lv_test_mouse_click_at(385, 315); // Click the "AM->PM" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-85-NKD_GO_ACT_010_dwn_arws_in_strt_time.png"),
      "Screenshot comparison failed for GO-85-Down Arrows on strtime screen");
  lv_test_mouse_click_at(240, 425); // Click the "Save" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-86-NKD_GO_ACT_010_Save_strt_time.png"),
      "Screenshot comparison failed for GO-86-Save btn clk & verify time change dwn arrows");
  clk_on_change_strtime_button();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-87-NKD_GO_ACT_010_Save_btn_disabled_strt_time.png"),
      "Screenshot comparison failed for GO-87-Save btn start time");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_back_btn_on_endtime_screen(void) {
  go_to_inactivated_settigns_flow();
  toggle_btns_screen();
  nxt_btn_to_exit_alert_screen();
  clk_on_cnts_mon_button();
  goto_schedl_monitoring_screen();
  clk_on_endtime_change_button();
  lv_test_mouse_click_at(34, 30); // Click the "Back" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-79-NKD_GO_ACT_008_schdl_mon.png"),
      "Screenshot comparison failed for GO-79-Scheduled Monitoring btn click");
  lv_test_mouse_click_at(446, 30);
}

static void test_Xbtn_on_endtime_screen(void) {
  go_to_inactivated_settigns_flow();
  toggle_btns_screen();
  nxt_btn_to_exit_alert_screen();
  clk_on_cnts_mon_button();
  goto_schedl_monitoring_screen();
  clk_on_endtime_change_button();
  lv_test_mouse_click_at(445, 30); // Click the "X" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-01-NKD_GO_001-Activate screen.png"),
      "Screenshot comparison failed for GO-01-Activate screen");
}

static void test_up_arrow_on_endtime_screen(void) {
  go_to_inactivated_settigns_flow();
  toggle_btns_screen();
  nxt_btn_to_exit_alert_screen();
  clk_on_cnts_mon_button();
  goto_schedl_monitoring_screen();
  clk_on_endtime_change_button();
  lv_test_mouse_click_at(95, 165); // Click the "Up Arrow" button
  lv_test_wait(50);
  lv_test_mouse_click_at(240, 165); // Click the "up arrow of mins" button
  lv_test_wait(50);
  lv_test_mouse_click_at(390, 330); // Click the "AM->PM" button clicks down arrow
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-88-NKD_GO_ACT_010_up_arws_in_endtime.png"),
      "Screenshot comparison failed for GO-88-Up Arrow on endtime screen");
  lv_test_mouse_click_at(240, 425); // Click the "Save" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-89-NKD_GO_ACT_010_Save_end_time.png"),
      "Screenshot comparison failed for GO-89-Save btn clk & verify endtime change");
  clk_on_endtime_change_button();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-90-NKD_GO_ACT_010_Save_btn_disabled_end_time.png"),
      "Screenshot comparison failed for GO-90-Save btn clk & verify endtime change");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_dwn_arrows_on_endtime_screen(void) {
  go_to_inactivated_settigns_flow();
  toggle_btns_screen();
  nxt_btn_to_exit_alert_screen();
  clk_on_cnts_mon_button();
  goto_schedl_monitoring_screen();
  clk_on_endtime_change_button();
  lv_test_wait(50);
  lv_test_mouse_click_at(95, 320); // Click the "Down Arrow" button
  lv_test_wait(50);
  lv_test_mouse_click_at(240, 310); // Click the "down arrow of mins" button
  lv_test_wait(50);
  lv_test_mouse_click_at(390, 160); // Click the "PM->AM" button clicks up arrow
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-91-NKD_GO_ACT_010_down_arws_in_endtime.png"),
      "Screenshot comparison failed for GO-91-Down Arrows on endtime screen");
  lv_test_mouse_click_at(240, 425); // Click the "Save" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-92-NKD_GO_ACT_010_Save_end_time.png"),
      "Screenshot comparison failed for GO-92-Save btn clk & verify endtime change dwn arrows");
  clk_on_endtime_change_button();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-93-NKD_GO_ACT_010_Save_btn_disabled_end_time.png"),
      "Screenshot comparison failed for GO-93-Save btn end time");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void clk_nxt_to_bed_mode_sensitivity_screen(void) {
  lv_test_mouse_click_at(350, 420); // next button to BMS page
  lv_test_wait(100);
}

static void test_bed_mode_sensitivity(void) {
  go_to_inactivated_settigns_flow();
  toggle_btns_screen();
  nxt_btn_to_exit_alert_screen();
  clk_nxt_to_bed_mode_sensitivity_screen();
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-94-NKD_GO_ACT_011_BMS_page.png"),
      "Screenshot comparison failed for GO-94-NKD_ACT_011_BMS_page");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void go_frm_activate_to_bms_screen(void) {
  go_to_inactivated_settigns_flow();
  toggle_btns_screen();
  nxt_btn_to_exit_alert_screen();
  clk_nxt_to_bed_mode_sensitivity_screen();
}

static void test_i_btn_on_bms_screen(void) {
  go_frm_activate_to_bms_screen();
  lv_test_wait(100);
  flow_to_sysinfo_screen();
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-76-NKD_GO_ACT_001_Sysinfo.png"),
      "Screenshot comparison failed for GO-76-Sysinfo");
  lv_test_mouse_click_at(446, 30); // Click the "X" button
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-94-NKD_GO_ACT_011_BMS_page.png"),
      "Screenshot comparison failed for GO-94-NKD_ACT_011_BMS_page");
  flow_to_sysinfo_screen();
  lv_test_mouse_click_at(450, 450); // Click the "Close" button
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-94-NKD_GO_ACT_011_BMS_page.png"),
      "Screenshot comparison failed for GO-94-NKD_ACT_011_BMS_page");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_back_btn_on_bed_mode_sensitivity_screen(void) {
  go_frm_activate_to_bms_screen();
  lv_test_wait(100);
  lv_test_mouse_click_at(120, 420); // Click the "Back" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-77-NKD_GO_ACT_001_nxt_btn.png"),
      "Screenshot comparison failed for GO-06-NKD_GO_ACT_001_nxt_btn");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_xbtn_on_bed_mode_sensitivity_screen(void) {
  go_frm_activate_to_bms_screen();
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30); // Click the "X" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-01-NKD_GO_001-Activate screen.png"),
      "Screenshot comparison failed for GO-01-Activate screen");
}

static void test_sliders_on_bms_screen(void) {
  go_frm_activate_to_bms_screen();
  lv_test_wait(100);
  lv_test_mouse_click_at(50, 190); // Click and drag slider to low
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-95-NKD_GO_ACT_012_BMS_Slider_Low.png"),
      "Screenshot comparison failed for GO-95-BMS Slider Low");
  lv_test_mouse_click_at(240, 190); // Click and drag slider to HIgh
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-96-NKD_GO_ACT_012_BMS_Slider_High.png"),
      "Screenshot comparison failed for GO-96-BMS Slider High");
  lv_test_mouse_click_at(430, 190); // Click and drag slider to UltraHigh
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-97-NKD_GO_ACT_012_BMS_Slider_UltraHigh.png"),
      "Screenshot comparison failed for GO-97-BMS Slider UltraHigh");
  lv_test_mouse_click_at(120, 420); // Click the "Back" button
  lv_test_mouse_click_at(350, 420); // Click the next button on exit alert screen to verify
                                    // whether the value set is still same or not
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-97-NKD_GO_ACT_012_BMS_Slider_UltraHigh.png"),
      "Screenshot comparison failed for GO-97-BMS Slider UltraHigh");
  lv_test_mouse_click_at(240, 190); // setting ultra high back to high for next tests
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void go_to_BW_screen(void) {
  lv_test_mouse_click_at(360, 420); // Click the Next button on BMS->BW button
  lv_test_wait(200);
}

static void test_bed_width_screen(void) {
  go_frm_activate_to_bms_screen();
  go_to_BW_screen();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-98-NKD_GO_ACT_011_BedWidth_screen.png"),
      "Screenshot comparison failed for GO-98-BWscreen");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_i_btn_on_BW_screen(void) {
  go_frm_activate_to_bms_screen();
  lv_test_wait(100);
  go_to_BW_screen();
  lv_test_wait(100);
  flow_to_sysinfo_screen();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-76-NKD_GO_ACT_001_Sysinfo.png"),
      "Screenshot comparison failed for IN-76-Sysinfo");
  lv_test_mouse_click_at(446, 30); // Click the "X" button
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-98-NKD_GO_ACT_011_BedWidth_screen.png"),
      "Screenshot comparison failed for GO-98-BWscreen");
  flow_to_sysinfo_screen();
  lv_test_mouse_click_at(240, 420); // Click the "Close" button
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-98-NKD_GO_ACT_011_BedWidth_screen.png"),
      "Screenshot comparison failed for GO-98-BWscreen");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_back_btn_on_bed_width_screen(void) {
  go_frm_activate_to_bms_screen();
  lv_test_wait(100);
  go_to_BW_screen();
  lv_test_wait(100);
  lv_test_mouse_click_at(120, 420); // Click the "Back" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-94-NKD_GO_ACT_011_BMS_page.png"),
      "Screenshot comparison failed for GO-94-NKD_ACT_011_BMS_page");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_xbtn_on_bed_width_screen(void) {
  go_frm_activate_to_bms_screen();
  lv_test_wait(100);
  go_to_BW_screen();
  lv_test_mouse_click_at(445, 30); // Click the "X" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-01-NKD_GO_001-Activate screen.png"),
      "Screenshot comparison failed for GO-01-Activate screen");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_sliders_on_BW_screen(void) {
  go_frm_activate_to_bms_screen();
  lv_test_wait(100);
  go_to_BW_screen();
  lv_test_wait(100);
  lv_test_mouse_click_at(50, 190); // Click and drag slider to low
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-98-NKD_GO_ACT_011_BedWidth_screen.png"),
      "Screenshot comparison failed for GO-98-BWscreen");
  lv_test_mouse_click_at(240, 190); // Click and drag slider to Medium
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-99-NKD_GO_ACT_014_BW_Slider_Wide.png"),
      "Screenshot comparison failed for GO-99-BW Slider Medium");
  lv_test_mouse_click_at(430, 190); // Click and drag slider to High
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-100-NKD_GO_ACT_014_BW_Slider_Custom.png"),
      "Screenshot comparison failed for IN-100-BW Slider Custom");
  lv_test_mouse_click_at(120, 420); // Click the "Back" button
  lv_test_mouse_click_at(360, 420); // Click next and verify the custom value stays same or not
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-100-NKD_GO_ACT_014_BW_Slider_Custom.png"),
      "Screenshot comparison failed for GO-100-BW Slider Custom");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_Chng_width_on_BW_screen(void) {
  go_frm_activate_to_bms_screen();
  lv_test_wait(100);
  go_to_BW_screen();
  lv_test_wait(100);
  lv_test_mouse_click_at(430, 190);
  lv_test_wait(100);
  lv_test_mouse_click_at(335, 270);
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-101-NKD_GO_ACT_015_-BW_Change_width_Custom.png"),
      "Screenshot comparison failed for GO-101-BW Change width Custom");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_back_btn_on_custom_width_screen(void) {
  go_frm_activate_to_bms_screen();
  go_to_BW_screen();
  lv_test_mouse_click_at(430, 190);
  lv_test_wait(100);
  lv_test_mouse_click_at(335, 270);
  lv_test_wait(150);
  lv_test_mouse_click_at(32, 34); // click on back
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-100-NKD_GO_ACT_014_BW_Slider_Custom.png"),
      "Screenshot comparison failed for GO-100-BW Slider Custom");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_x_btn_on_custom_width_screen(void) {
  go_frm_activate_to_bms_screen();
  go_to_BW_screen();
  lv_test_mouse_click_at(430, 190);
  lv_test_wait(100);
  lv_test_mouse_click_at(335, 270);
  lv_test_wait(150);
  lv_test_mouse_click_at(445, 30); // Click the "X" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-01-NKD_GO_001-Activate screen.png"),
      "Screenshot comparison failed for GO-01-Activate screen");
}

static void test_predefined_width_options_on_Custom_Width_Screen(void) {
  go_frm_activate_to_bms_screen();
  go_to_BW_screen();
  lv_test_mouse_click_at(430, 190); // Click and drag slider to Custom
  lv_test_wait(150);
  lv_test_mouse_click_at(335, 270); // click on change width
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-101-NKD_GO_ACT_015_-BW_Change_width_Custom.png"),
      "Screenshot comparison failed for GO-101-BW Change width Custom");
  lv_test_mouse_click_at(32, 34); // click on back
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-101-NKD_GO_ACT_014_BW_Slider_Custom.png"),
      "Screenshot comparison failed for GO-100-BW Slider Custom");
  lv_test_mouse_click_at(335, 270);
  lv_test_mouse_click_at(340, 160);
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-102-NKD_GO_ACT_016_BW_Change_width_Queen.png"),
      "Screenshot comparison failed for GO-102-BW Change width Queen");
  lv_test_mouse_click_at(32, 34); // click on back
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-103-NKD_GO_ACT_016_Save_BW_CustomQueen_width.png"),
      "Screenshot comparison failed for GO-103-Save_BW_CustomQueen_width");
  lv_test_mouse_click_at(335, 270);
  lv_test_mouse_click_at(140, 240);
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-104-NKD_GO_ACT_016-BW_Change_width_CaliKing.png"),
      "Screenshot comparison failed for GO-104-BW Change width Cali King");
  lv_test_mouse_click_at(32, 34); // click on back
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-105-NKD_GO_ACT_016_Save_BW_CustomCaliKing_width.png"),
      "Screenshot comparison failed for GO-105-Save_BW_CustomCaliKing_width");
  lv_test_mouse_click_at(335, 270);
  lv_test_mouse_click_at(330, 240);
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-106-NKD_GO_ACT_016_BW_Change_width_King.png"),
      "Screenshot comparison failed for GO-106-BW Change width King");
  lv_test_mouse_click_at(32, 34); // click on back
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-107-NKD_GO_ACT_016_Save_BW_CustomKing_width.png"),
      "Screenshot comparison failed for GO-107-Save_BW_CustomKing_width");
  lv_test_mouse_click_at(120, 420); // Click the "Back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(360, 420);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-107-NKD_GO_ACT_016_Save_BW_CustomKing_width.png"),
      "Screenshot comparison failed for GO-107-Save_BW_CustomKing_width");
  lv_test_wait(150);
  lv_test_mouse_click_at(445, 30);
}

static void test_clk_on_custom_value_button(void) {
  go_frm_activate_to_bms_screen();
  go_to_BW_screen();
  lv_test_mouse_click_at(430, 190);
  lv_test_wait(100);
  lv_test_mouse_click_at(335, 270);
  lv_test_wait(150);
  lv_test_mouse_click_at(230, 330); // Click the "Enter Custom Value" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-108-NKD_GO_ACT_016_BW_Custom_Value.png"),
      "Screenshot comparison failed for GO-108-BW Change width Custom Value");
  lv_test_wait(150);
  lv_test_mouse_click_at(445, 30);
}

static void test_number_keypad_screen_on_Custom_BW(void) {
  go_frm_activate_to_bms_screen();
  go_to_BW_screen();
  lv_test_mouse_click_at(430, 190);
  lv_test_wait(100);
  lv_test_mouse_click_at(335, 270);
  lv_test_wait(150);
  lv_test_mouse_click_at(230, 330); // Click the "Enter Custom Value" button
  lv_test_wait(150);
  lv_test_mouse_click_at(80, 230); // Click '1'
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-109-NKD_GO_ACT_017_BW_Custom_Value_Numpad_Input.png"),
      "Screenshot comparison failed for GO-109-BW Custom Value Numpad Input");
  lv_test_mouse_click_at(190, 230); // Click '2'
  lv_test_mouse_click_at(290, 230); // Click '3'
  lv_test_mouse_click_at(400, 230); // Click '4'
  lv_test_mouse_click_at(80, 310);  // Click '5'
  lv_test_mouse_click_at(190, 310); // Click '6'
  lv_test_mouse_click_at(290, 310); // Click '7'
  lv_test_mouse_click_at(390, 310); // Click '8'
  lv_test_mouse_click_at(190, 390); // Click '9'
  lv_test_mouse_click_at(290, 390); // Click '0'
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/"
                                 "GO-110-NKD_GO_ACT_017_BW_Custom_Value_Numpad_all_digits.png"),
      "Screenshot comparison failed for GO-110-BW Custom Value Numpad Input");
  lv_test_wait(150);
  lv_test_mouse_click_at(80, 390); // Click the "backspace" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/"
                                 "GO-111-NKD_GO_ACT_017_BW_Custom_Value_Numpad_backspace.png"),
      "Screenshot comparison failed for GO-111-BW Custom Value Numpad backspace");
  lv_test_mouse_click_at(80, 390); // Click the "backspace" button
  lv_test_mouse_click_at(80, 390); // Click the "backspace" button
  lv_test_mouse_click_at(80, 390); // Click the "backspace" button
  lv_test_mouse_click_at(80, 390); // Click the "backspace" button
  lv_test_mouse_click_at(80, 390); // Click the "backspace" button
  lv_test_mouse_click_at(80, 390); // Click the "backspace" button
  lv_test_mouse_click_at(80, 390); // Click the "backspace" button
  lv_test_mouse_click_at(80, 390); // Click the "backspace" button
  lv_test_mouse_click_at(80, 390); // Click the "backspace" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-108-NKD_GO_ACT_016_BW_Custom_Value.png"),
      "Screenshot comparison failed for GO-108-BW Change width Custom Value");
  lv_test_wait(150);
  lv_test_mouse_click_at(400, 230); // Click '4'
  lv_test_mouse_click_at(80, 310);  // Click '5'
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-112-NKD_ACT_017_BW_Custom_Valid_Value.png"),
      "Screenshot comparison failed for GO-112-BW Custom Value Entered");
  lv_test_mouse_click_at(400, 400); // Click the "OK" button
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-113-NKD_GO_ACT_017_BW_Custom_Value_Saved.png"),
      "Screenshot comparison failed for GO-113-BW Custom Value Saved");
  lv_test_wait(150);
  lv_test_mouse_click_at(445, 30);
}

static void test_back_button_on_custom_value_screen(void) {
  go_frm_activate_to_bms_screen();
  go_to_BW_screen();
  lv_test_mouse_click_at(430, 190);
  lv_test_wait(100);
  lv_test_mouse_click_at(335, 270); // click on change width
  lv_test_wait(150);
  lv_test_mouse_click_at(230, 330); // Click the "Enter Custom Value" button
  lv_test_wait(150);
  lv_test_mouse_click_at(34, 30); // Click the "Back" button
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-101-NKD_GO_ACT_015_-BW_Change_width_Custom.png"),
      "Screenshot comparison failed for GO-101-BW Change width Custom");
  lv_test_wait(150);
  lv_test_mouse_click_at(445, 30);
}

static void test_x_button_on_custom_value_screen(void) {
  go_frm_activate_to_bms_screen();
  go_to_BW_screen();
  lv_test_mouse_click_at(430, 190);
  lv_test_wait(100);
  lv_test_mouse_click_at(335, 270); // click on change width
  lv_test_wait(150);
  lv_test_mouse_click_at(230, 330); // Click the "Enter Custom Value" button
  lv_test_wait(150);
  lv_test_mouse_click_at(445, 30); // Click the "X" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-01-NKD_GO_001-Activate screen.png"),
      "Screenshot comparison failed for GO-01-Activate screen");
}

static void test_toast_messages_on_custom_value_screen(void) {
  go_frm_activate_to_bms_screen();
  go_to_BW_screen();
  lv_test_mouse_click_at(430, 190); // Click and drag slider to Custom
  lv_test_wait(150);
  lv_test_mouse_click_at(335, 270); // click on change width
  lv_test_wait(150);
  lv_test_mouse_click_at(230, 330); // Click the "Enter Custom Value" button
  lv_test_wait(150);
  lv_test_mouse_click_at(290, 230); // Click '3'
  lv_test_mouse_click_at(400, 230); // Click '4'
  lv_test_mouse_click_at(400, 400); // Click the "tick" button & let toast message appear
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/"
                                 "GO-114-NKD_GO_ACT_018_BW_Custom_Value_minimum_Toastmsg.png"),
      "Screenshot comparison failed for GO-114-BW Custom Value MIN TOAST");
  lv_test_mouse_click_at(445, 37); // close toast msg
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-115-NKD_GO_ACT_018_BW_Close_minimum_Toastmsg.png"),
      "Screenshot comparison failed for GO-115-BW Close MIN TOAST");
  lv_test_mouse_click_at(400, 400);
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-114-NKD_ACT_018_BW_Custom_Value_minimum_Toastmsg.png"),
      "Screenshot comparison failed for GO-114-BW Custom Value MIN TOAST");
  lv_test_mouse_click_at(445, 37);  // close toast msg
  lv_test_mouse_click_at(80, 390);  // Click the "backspace" button
  lv_test_mouse_click_at(80, 390);  // Click the "backspace" button
  lv_test_mouse_click_at(390, 310); // Click '8'
  lv_test_mouse_click_at(190, 390); // Click '9'
  lv_test_mouse_click_at(400, 400); // Click the "tick" button & let toast message appear
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/"
                                 "GO-116-NKD_GO_ACT_018_BW_Custom_Value_maximum_Toastmsg.png"),
      "Screenshot comparison failed for GO-116-BW Custom Value MAX TOAST");
  lv_test_mouse_click_at(445, 37); // close toast msg
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-117-NKD_GO_ACT_018_BW_Close_maximum_Toastmsg.png"),
      "Screenshot comparison failed for G0-117-BW Close MAX TOAST");
  lv_test_mouse_click_at(400, 400); // Click the "tick" button & let toast message appear
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/"
                                 "GO-116-NKD_GO_ACT_018_BW_Custom_Value_maximum_Toastmsg.png"),
      "Screenshot comparison failed for GO-116-BW Custom Value MAX TOAST");
  lv_test_mouse_click_at(445, 37); // close toast msg
  lv_test_wait(100);
  lv_test_mouse_click_at(34, 30); // Click the "Back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30); // Click the "X" button
}

static void go_to_bed_placement_screen(void) {
  lv_test_mouse_click_at(360, 420); // Click the Next button on BW->Bed Placement button
  lv_test_wait(200);
}

static void test_bed_placement_screen(void) {
  go_frm_activate_to_bms_screen();
  lv_test_wait(100);
  go_to_BW_screen();
  lv_test_wait(100);
  go_to_bed_placement_screen();
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-118-NKD_GO_ACT_019_Bed_Placementscreen.png"),
      "Screenshot comparison failed for GO-118-Bed_placement_screen");
  lv_test_wait(150);
  lv_test_mouse_click_at(445, 30);
}

static void test_i_button_on_bed_placement_screen(void) {
  go_frm_activate_to_bms_screen();
  go_to_BW_screen();
  go_to_bed_placement_screen();
  lv_test_wait(100);
  flow_to_sysinfo_screen();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-76-NKD_GO_ACT_001_Sysinfo.png"),
      "Screenshot comparison failed for IN-76-Sysinfo");
  lv_test_mouse_click_at(446, 30); // Click the "X" button
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-118-NKD_GO_ACT_019_Bed_Placementscreen.png"),
      "Screenshot comparison failed for GO-118-Bed_placement_screen");
  flow_to_sysinfo_screen();
  lv_test_mouse_click_at(450, 450); // Click the "Close" button
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-118-NKD_GO_ACT_019_Bed_Placementscreen.png"),
      "Screenshot comparison failed for GO-118-Bed_placement_screen");
  lv_test_wait(150);
  lv_test_mouse_click_at(445, 30);
}

static void test_X_button_on_bed_placement_screen(void) {
  go_frm_activate_to_bms_screen();
  go_to_BW_screen();
  go_to_bed_placement_screen();
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30); // Click the "X" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-01-NKD_GO_001-Activate screen.png"),
      "Screenshot comparison failed for GO-01-Activate screen");
}

static void test_back_button_on_bed_placement_screen(void) {
  go_frm_activate_to_bms_screen();
  go_to_BW_screen();
  go_to_bed_placement_screen();
  lv_test_mouse_click_at(120, 420); // Click the "Back" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-98-NKD_GO_ACT_011_BedWidth_screen.png"),
      "Screenshot comparison failed for GO-98-BWscreen");
  lv_test_wait(150);
  lv_test_mouse_click_at(445, 30);
}

static void test_sliders_on_bed_placement_screen(void) {
  go_frm_activate_to_bms_screen();
  go_to_BW_screen();
  go_to_bed_placement_screen();
  lv_test_wait(100);
  lv_test_mouse_click_at(50, 190); // Click and drag slider to left wall
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-119-NKD_GO_ACT_020_Bed_Placement_Slider_Left.png"),
      "Screenshot comparison failed for GO-119-Bed Placement Slider Left");
  lv_test_mouse_click_at(240, 190); // Click and drag slider to centre
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-120-NKD_GO_ACT_020_Bed_Placement_Slider_Centre.png"),
      "Screenshot comparison failed for GO-120-Bed Placement Slider Centre");
  lv_test_mouse_click_at(430, 190); // Click and drag slider to right wall
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-121-NKD_GO_ACT_020_Bed_Placement_Slider_Right.png"),
      "Screenshot comparison failed for GO-121-Bed Placement SliderRIght");
  lv_test_mouse_click_at(120, 420); // Click the "Back" button
  lv_test_mouse_click_at(
      360, 420); // click next from BW screen to BP screen to check values stay same or not
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-121-NKD_GO_ACT_020_Bed_Placement_Slider_Right.png"),
      "Screenshot comparison failed for GO-121-Bed Placement SliderRIght");
  lv_test_wait(150);
  lv_test_mouse_click_at(445, 30);
}

static void go_to_occupant_size_screen(void) {
  lv_test_mouse_click_at(360, 420); // Click the Next button on Bed Placement->Occupant Size
  lv_test_wait(200);
}

static void test_occupant_size_screen(void) {
  go_frm_activate_to_bms_screen();
  go_to_BW_screen();
  go_to_bed_placement_screen();
  go_to_occupant_size_screen();
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-122-NKD_GO_ACT_021_occupant_size.png"),
      "Screenshot comparison failed for GO-122-Occupant_Size");
  lv_test_wait(150);
  lv_test_mouse_click_at(445, 30);
}

static void test_i_button_on_occupant_size_screen(void) {
  go_frm_activate_to_bms_screen();
  go_to_BW_screen();
  go_to_bed_placement_screen();
  go_to_occupant_size_screen();
  flow_to_sysinfo_screen();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-76-NKD_GO_ACT_001_Sysinfo.png"),
      "Screenshot comparison failed for IN-76-Sysinfo");
  lv_test_mouse_click_at(446, 30); // Click the "X" button
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-122-NKD_GO_ACT_021_occupant_size.png"),
      "Screenshot comparison failed for GO-122-Occupant_Size");
  flow_to_sysinfo_screen();
  lv_test_mouse_click_at(450, 450); // Click the "Close" button
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-122-NKD_GO_ACT_021_occupant_size.png"),
      "Screenshot comparison failed for GO-122-Occupant_Size");
  lv_test_wait(150);
  lv_test_mouse_click_at(445, 30);
}

static void test_x_button_on_occupant_size_screen(void) {
  go_frm_activate_to_bms_screen();
  go_to_BW_screen();
  go_to_bed_placement_screen();
  go_to_occupant_size_screen();
  lv_test_mouse_click_at(445, 30); // Click the "X" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-01-NKD_GO_001-Activate screen.png"),
      "Screenshot comparison failed for GO-01-Activate screen");
}

static void test_back_button_on_occupant_size_screen(void) {
  go_frm_activate_to_bms_screen();
  go_to_BW_screen();
  go_to_bed_placement_screen();
  go_to_occupant_size_screen();
  lv_test_mouse_click_at(120, 420); // Click the "Back" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-120-NKD_GO_ACT_020_Bed_Placement_Slider_Centre.png"),
      "Screenshot comparison failed for GO-120-Bed Placement Slider Centre");
  lv_test_wait(150);
  lv_test_mouse_click_at(445, 30);
}

static void test_sliders_on_occupant_size_screen(void) {
  go_frm_activate_to_bms_screen();
  go_to_BW_screen();
  go_to_bed_placement_screen();
  go_to_occupant_size_screen();
  lv_test_mouse_click_at(50, 190); // Click and drag slider to Small
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-123-NKD_GO_ACT_022_Occupant_Size_Slider_Small.png"),
      "Screenshot comparison failed for GO-123-Occupant Size Slider Small");
  lv_test_mouse_click_at(240, 190); // Click and drag slider to standard
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-122-NKD_ACT_021_occupant_size.png"),
      "Screenshot comparison failed for GO-122-Occupant_Size");
  lv_test_mouse_click_at(430, 190); // Click and drag slider to Large
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-124-NKD_GO_ACT_022_Occupant_Size_Slider_Large.png"),
      "Screenshot comparison failed for GO-124-Occupant Size Slider Large");
  lv_test_mouse_click_at(120, 420); // Click the "Back" button
  lv_test_mouse_click_at(360, 420);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-124-NKD_GO_ACT_022_Occupant_Size_Slider_Large.png"),
      "Screenshot comparison failed for GO-124-Occupant Size Slider Large");
  lv_test_wait(150);
  lv_test_mouse_click_at(445, 30);
}

static void go_to_in_room_alert_screen(void) {
  lv_test_mouse_click_at(350, 420); // next button to Exit alert page
  lv_test_wait(100);
}

static void go_frm_activate_to_in_room_alert_screen(void) {
  go_frm_activate_to_bms_screen();
  go_to_BW_screen();
  go_to_bed_placement_screen();
  go_to_occupant_size_screen();
  go_to_in_room_alert_screen();
}

static void test_in_room_alert_screen(void) {
  go_frm_activate_to_in_room_alert_screen();
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-125-NKD_GO_ACT_023_In_room_alert_audio.png"),
      "Screenshot comparison failed for GO-125-in_room_alert_audio");
  lv_test_wait(150);
  lv_test_mouse_click_at(445, 30);
}

static void test_i_button_on_in_room_alert_screen(void) {
  go_frm_activate_to_in_room_alert_screen();
  flow_to_sysinfo_screen();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-76-NKD_GO_ACT_001_Sysinfo.png"),
      "Screenshot comparison failed for IN-76-Sysinfo");
  lv_test_mouse_click_at(446, 30); // Click the "X" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-125-NKD_GO_ACT_023_In_room_alert_audio.png"),
      "Screenshot comparison failed for GO-125-in_room_alert_audio");
  flow_to_sysinfo_screen();
  lv_test_mouse_click_at(450, 450); // Click the "Close" button
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-125-NKD_GO_ACT_023_In_room_alert_audio.png"),
      "Screenshot comparison failed for GO-125-in_room_alert_audio");
  lv_test_wait(150);
  lv_test_mouse_click_at(445, 30);
}

static void test_X_button_on_in_room_alert_screen(void) {
  go_frm_activate_to_in_room_alert_screen();
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30); // Click the "X" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-01-NKD_GO_001-Activate screen.png"),
      "Screenshot comparison failed for GO-01-Activate screen");
}

static void test_back_button_on_in_room_alert_screen(void) {
  go_frm_activate_to_in_room_alert_screen();
  lv_test_mouse_click_at(120, 420); // Click the "Back" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-122-NKD_ACT_021_occupant_size.png"),
      "Screenshot comparison failed for GO-122-Occupant_Size");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_in_room_alert_audio_toggle(void) {
  go_frm_activate_to_in_room_alert_screen();
  lv_test_mouse_click_at(430, 110);
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-126-NKD_GO_ACT_024_In_Room_Alert_Audio_Toggle_OFF.png"),
      "Screenshot comparison failed for GO-126-In Room Alert Audio Toggle OFF");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_lang_button_on_in_room_alert_screen(void) {
  go_frm_activate_to_in_room_alert_screen();
  lv_test_wait(100);
  lv_test_mouse_click_at(340, 210); // Click on language button
  lv_test_wait(4000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-127-NKD_GO_ACT_025_Languages_page_without_command.png"),
      "Screenshot comparison failed for GO-127-languages page without command");
  lv_test_mouse_click_at(32, 34);
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-128-NKD_GO_ACT_025_Languages_page_without_command.png"),
      "Screenshot comparison failed for GO-128-languages page without command");
  lv_test_mouse_click_at(340, 210); // Click on language button
  lv_test_wait(4000);
  lv_test_mouse_click_at(140, 160);
  lv_test_wait(1000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-129-NKD_GO_ACT_025_Languages_page_English_command.png"),
      "Screenshot comparison failed for GO-129-languages page English command");
  lv_test_mouse_click_at(340, 160);
  lv_test_wait(1000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-130-NKD_GO_ACT_025_Languages_page_Spanish_command.png"),
      "Screenshot comparison failed for GO-130-languages page Spanish command");
  lv_test_mouse_click_at(32, 34);
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-131-NKD_GO_ACT_025_Spanish_Saved.png"),
      "Screenshot comparison failed for GO-131-Spanish_Saved.png");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_lang_button_with_command(void) {
  go_frm_activate_to_in_room_alert_screen();
  lv_test_wait(100);
  char line[512];
  while (read_line_nb(RD_FD, line, sizeof(line))) {
  }
  int id = 1;
  lv_test_mouse_click_at(340, 210);
  const uint32_t timeout_ms = 2000;
  uint32_t waited = 0;
  bool handled = false;

  while (waited < timeout_ms && !handled) {
    // Drain any available lines
    while (read_line_nb(RD_FD, line, sizeof(line))) {
      if (strstr(line, "\"method\":\"get_languages\"")) {
        if (char *p = strstr(line, "\"id\":")) {
          sscanf(p, "\"id\":%d", &id);
        }
        char resp[512];
        int len = snprintf(resp, sizeof(resp),
                           "{\"jsonrpc\":\"2.0\",\"result\":["
                           "\"Arabic\",\"Cantonese\",\"English\",\"French\","
                           "\"Haitian\",\"Creole\",\"Gujarati\",\"Hindi\",\"Korean\",\"Mandarin\"],"
                           "\"id\":%d}\n",
                           id);

        write_response(WR_FD, resp, (size_t)len);
        process_request(50); // let the UI consume the response + render
        lv_test_wait(50);
        handled = true;
        break;
      }
    }
    if (!handled) {
      process_request(10);
      lv_test_wait(10);
      waited += 10;
    }
  }
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/"
          "GO-132-NKD_GO_ACT_025_In-Room_Alert_Language_Selection_Screen.png"),
      "Screenshot comparison failed for GO-132-NKD_GO_ACT_025_In Room Alert Language Selection "
      "Screen");
  lv_test_wait(250);
  lv_test_mouse_click_at(130, 170); // Click the Arabic language button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/"
          "GO-133-NKD_GO_ACT_025_In-Room_Alert_Arabic_Language_Selected_Screen.png"),
      "Screenshot comparison failed for GO-133-In Room Alert Arabic Language Selected Screen");
  lv_test_mouse_click_at(345, 165); // Click the Cantonese language button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/"
          "GO-134-NKD_G0_ACT_025_In-Room_Alert_Cantonese_Language_Selected_Screen.png"),
      "Screenshot comparison failed for GO-134-In Room Alert Cantonese Language Selected Screen");
  lv_test_mouse_click_at(135, 235); // Click the English button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/"
          "GO-135-NKD_GO_ACT_025_In-Room_Alert_English_Language_Selected_Screen.png"),
      "Screenshot comparison failed for GO-135-In Room Alert English Language Selected Screen");
  lv_test_mouse_click_at(340, 240); // Click the French button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/"
          "GO-136-NKD_GO_ACT_025_In-Room_Alert_French_Language_Selected_Screen.png"),
      "Screenshot comparison failed for GO-136-In Room Alert French Language Selected Screen");
  lv_test_mouse_click_at(140, 310); // Click the Haitian Creole button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/"
          "GO-137-NKD_GO_ACT_025_In-Room_Alert_Haitian_Language_Selected_Screen.png"),
      "Screenshot comparison failed for GO-137-In Room Alert Haitian Language Selected "
      "Screen");
  lv_test_mouse_click_at(350, 310); // Click the Gujarati button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/"
          "GO-138-NKD_GO_ACT_025_In-Room_Alert_Creole_Language_Selected_Screen.png"),
      "Screenshot comparison failed for GO-138-In Room Alert Creole Language Selected Screen");
  lv_test_mouse_click_at(390, 380); // Click the Next page button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/"
          "GO-139-NKD_GO_ACT_025_In-Room_Alert__Language_2ndpage_Selected_Screen.png"),
      "Screenshot comparison failed for GO-139-In Room Alert 2nd Page Selected Screen");
  lv_test_mouse_click_at(130, 170); // Click the Hindi button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/"
          "GO-140-NKD_GO_ACT_025_In-Room_Alert_Gujarati_Language_Selected_Screen.png"),
      "Screenshot comparison failed for GO-140-In Room Alert GUjarati Language Selected Screen");
  lv_test_mouse_click_at(350, 170); // Click the Korean button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/"
          "GO-141-NKD_GO_ACT_025_In-Room_Alert_Hindi_Language_Selected_Screen.png"),
      "Screenshot comparison failed for GO-141-In Room Alert HIndi Language Selected Screen");
  lv_test_mouse_click_at(130, 240); // Click the Mandarin button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/"
          "GO-142-NKD_GO_ACT_025_In-Room_Alert_Korean_Language_Selected_Screen.png"),
      "Screenshot comparison failed for GO-142-In Room Alert Korean Language Selected Screen");
  lv_test_mouse_click_at(350, 240); // Click the Mandarin Button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/"
          "GO-143-NKD_GO_ACT_025_In-Room_Alert_Manadarin_Language_Selected.png"),
      "Screenshot comparison failed for GO-143-In Room Alert Language");
  lv_test_mouse_click_at(34, 35); // Click back to check mandarin is saved or not
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare(
                               "ref_imgsNKD/NKD-GO/NKD-ACT/"
                               "GO-144-NKD_GO_ACT_025_In-Room_Alert_Manadarin_Language_Saved.png"),
                           "Screenshot comparison failed for GO-144-In Room Alert Language");
  lv_test_mouse_click_at(340, 210); // Click on language button
  lv_test_wait(4000);
  lv_test_mouse_click_at(140, 160); // set to english again
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_back_button_on_lang_screen(void) {
  go_frm_activate_to_in_room_alert_screen();
  lv_test_mouse_click_at(340, 210); // Click on language button
  lv_test_wait(4000);
  lv_test_mouse_click_at(34, 30); // Click the "Back" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-128-NKD_GO_ACT_025_Languages_page_without_command.png"),
      "Screenshot comparison failed for GO-128-languages page without command");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_X_btn_on_lang_screen(void) {
  go_frm_activate_to_in_room_alert_screen();
  lv_test_mouse_click_at(310, 210); // Click the "Language" button
  lv_test_wait(5000);
  lv_test_mouse_click_at(445, 30); // Click the "X" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-01-NKD_GO_001-Activate screen.png"),
      "Screenshot comparison failed for GO-01-Activate screen");
}

static void test_volume_slider_in_room_alert_screen(void) {
  go_frm_activate_to_in_room_alert_screen();
  lv_test_wait(200);
  lv_test_mouse_click_at(50, 290); // Click and drag slider to Low
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-145-NKD_GO_ACT_026_In_Room_Alert_Volume_Slider_Low.png"),
      "Screenshot comparison failed for GO-145-In Room Alert Volume Slider Low");
  lv_test_mouse_click_at(240, 290); // Click and drag slider to Medium
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/"
                                 "GO-146-NKD_GO_ACT_026_In_Room_Alert_Volume_Slider_Medium.png"),
      "Screenshot comparison failed for GO-146-In Room Alert Volume Slider Medium");
  lv_test_mouse_click_at(430, 290); // Click and drag slider to High
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/"
                                 "GO-147-NKD_GO_ACT_026_In_Room_Alert_Volume_Slider_High.png"),
      "Screenshot comparison failed for GO-147-In Room Alert Volume Slider High");
  lv_test_mouse_click_at(120, 420); // Click the "Back" button
  lv_test_mouse_click_at(350, 420);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/"
                                 "GO-147-NKD_GO_ACT_026_In_Room_Alert_Volume_Slider_High.png"),
      "Screenshot comparison failed for GO-147-In Room Alert Volume Slider High");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void flow_to_activating_screen(int screen) {
  char line[512];
  int id = 1;

  for (uint32_t waited = 0; waited < 2000; waited += 10) {
    while (read_line_nb(RD_FD, line, sizeof line)) {
      if (!strstr(line, "\"method\":\"activate_sensor\""))
        continue;

      char *p = strstr(line, "\"id\":");
      if (p)
        sscanf(p, "\"id\":%d", &id);

      // Updated response payload (screen stays as argument)
      char resp[512];
      int len =
          snprintf(resp, sizeof resp,
                   "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
                   "{\"screen\":%d,\"fts_avail\":127,\"fts_state\":1,\"bed_pos\":1,"
                   "\"bed_wid\":60,\"occ_size\":1,\"mon_start\":\"2130\",\"mon_end\":\"0700\","
                   "\"bms\":1,\"vol\":2,\"audio\":1,\"lang\":\"English\",\"mode\":1,"
                   "\"room_number\":\"1001-a\"},\"id\":%d}\n",
                   screen, id);

      write_response(WR_FD, resp, (size_t)len);
      process_request(50);
      lv_test_wait(50);
      return;
    }

    process_request(10);
    lv_test_wait(10);
  }
}

static void test_activating_screen_with_2_and_7(void) {
  go_frm_activate_to_in_room_alert_screen();
  lv_test_wait(100);
  lv_test_mouse_click_at(360, 420); // Click the Activate button on In-Room Alert.
  flow_to_activating_screen(7);
  lv_test_wait(200);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-148-NKD_GO_ACT_027_Activating_Screen_with_value_7.png"),
      "Screenshot comparison failed for GO-148-Activating_Screen_with_value_7");
  lv_test_wait(60000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-149-NKD_GO_ACT_027_Activating_Screen_Timeout.png"),
      "Screenshot comparison failed for GO-149-Activating_Screen_timeout");
  lv_test_mouse_click_at(360, 420); // Click the Activate button on In-Room Alert.
  flow_to_activating_screen(7);
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-148-NKD_GO_ACT_027_Activating_Screen_with_value_7.png"),
      "Screenshot comparison failed for GO-148-Activating_Screen_with_value_7");
  char resp[512];
  int len = snprintf(resp, sizeof resp,
                     "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
                     "{\"screen\":2,\"fts_avail\":127,\"fts_state\":7,\"bed_pos\":2,"
                     "\"bed_wid\":38,\"occ_size\":2,\"mon_start\":\"\",\"mon_end\":\"\","
                     "\"bms\":1,\"vol\":2,\"audio\":1,\"lang\":\"English\",\"mode\":1,"
                     "\"room_number\":\"1001-a\"},\"id\":1}\n");

  write_response(WR_FD, resp, (size_t)len);
  process_request(50);
  lv_test_wait(50);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-150-NKD_GO_ACT_027_Activating_Screen_with_value_2.png"),
      "Screenshot comparison failed for GO-150-Activating_Screen_with_value_2");
}

static void test_bed_monitoring_screen(void) {
  char resp[512];
  int len = snprintf(resp, sizeof resp,
                     "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
                     "{\"screen\":2,\"fts_avail\":127,\"fts_state\":1,\"bed_pos\":2,"
                     "\"bed_wid\":38,\"occ_size\":2,\"mon_start\":\"\",\"mon_end\":\"\","
                     "\"bms\":1,\"vol\":2,\"audio\":1,\"lang\":\"English\",\"mode\":1,"
                     "\"room_number\":\"1001-a\"},\"id\":1}\n");

  write_response(WR_FD, resp, (size_t)len);
  process_request(50);
  lv_test_wait(50);
  lv_test_wait(200);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-MON/GO-01-NKD_GO_MON_001_Bed_Monitoring_Screen.png"),
      "Screenshot comparison failed for GO-80-Activating_Screen_with_value_2");
}

static void flow_to_activate_bed_mode(void) {
  char line[512];
  int id = 1;

  for (uint32_t waited = 0; waited < 2000; waited += 10) {
    while (read_line_nb(RD_FD, line, sizeof line)) {
      if (!strstr(line, "\"method\":\"activate_bed_mode\""))
        continue;

      char *p = strstr(line, "\"id\":");
      if (p)
        sscanf(p, "\"id\":%d", &id);

      char resp[512];
      int len =
          snprintf(resp, sizeof resp,
                   "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
                   "{\"screen\":2,\"fts_avail\":127,\"fts_state\":1,\"bed_pos\":1,"
                   "\"bed_wid\":60,\"occ_size\":1,\"mon_start\":\"2130\",\"mon_end\":\"0700\","
                   "\"bms\":1,\"vol\":2,\"audio\":1,\"lang\":\"English\",\"mode\":1,"
                   "\"room_number\":\"1001-a\"},\"id\":%d}\n",
                   id);

      write_response(WR_FD, resp, (size_t)len);
      process_request(50);
      lv_test_wait(50);
      return;
    }

    process_request(10);
    lv_test_wait(10);
  }
}

static void test_bed_mode_button(void) {
  lv_test_mouse_click_at(100, 50);
  lv_test_wait(3000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-MON/GO-02-NKD_GO_MON_002_Bed_Mode_Screen_Toast.png"),
      "Screenshot comparison failed for GO-02-NKD_GO_MON_002_Bed_Mode_Screen.png");
  lv_test_wait(2000);
  lv_test_mouse_click_at(100, 50);
  lv_test_wait(200);
  flow_to_activate_bed_mode();
  lv_test_wait(1250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-MON/GO-03-NKD_GO_MON_002_Bed_Mode_Screen.png"),
      "Screenshot comparison failed for GO-03-NKD_GO_MON_002_Bed_Mode_Screen.png");
}

static void flow_to_activate_chair_mode(void) {
  char line[512];
  int id = 1;

  for (uint32_t waited = 0; waited < 2000; waited += 10) {
    while (read_line_nb(RD_FD, line, sizeof line)) {
      if (!strstr(line, "\"method\":\"activate_chair_mode\""))
        continue;

      char *p = strstr(line, "\"id\":");
      if (p)
        sscanf(p, "\"id\":%d", &id);

      char resp[512];
      int len =
          snprintf(resp, sizeof resp,
                   "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
                   "{\"screen\":2,\"fts_avail\":127,\"fts_state\":1,\"bed_pos\":1,"
                   "\"bed_wid\":60,\"occ_size\":1,\"mon_start\":\"2130\",\"mon_end\":\"0700\","
                   "\"bms\":1,\"vol\":2,\"audio\":1,\"lang\":\"English\",\"mode\":2,"
                   "\"room_number\":\"1001-a\"},\"id\":%d}\n",
                   id);

      write_response(WR_FD, resp, (size_t)len);
      process_request(50);
      lv_test_wait(50);
      return;
    }

    process_request(10);
    lv_test_wait(10);
  }
}

static void test_chair_mode_button(void) {
  lv_test_mouse_click_at(270, 50);
  lv_test_wait(3000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-MON/GO-04-NKD_GO_MON_003_Chair_Mode_Screen_Toast.png"),
      "Screenshot comparison failed for GO-04-NKD_GO_MON_003_Chair_Mode_Screen.png");
  lv_test_wait(2000);
  lv_test_mouse_click_at(270, 50);
  lv_test_wait(200);
  flow_to_activate_chair_mode();
  lv_test_wait(1250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-MON/GO-05-NKD_GO_MON_003_Chair_Mode_Screen.png"),
      "Screenshot comparison failed for GO-05-NKD_GO_MON_003_Chair_Mode_Screen.png");
}

static void flow_to_pause_sensor(void) {
  char line[512];
  int id = 1;

  for (uint32_t waited = 0; waited < 2000; waited += 10) {
    while (read_line_nb(RD_FD, line, sizeof line)) {
      if (!strstr(line, "\"method\":\"pause_sensor\""))
        continue;

      char *p = strstr(line, "\"id\":");
      if (p)
        sscanf(p, "\"id\":%d", &id);

      char resp[512];
      int len =
          snprintf(resp, sizeof resp,
                   "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
                   "{\"screen\":2,\"fts_avail\":127,\"fts_state\":1,\"bed_pos\":1,"
                   "\"bed_wid\":60,\"occ_size\":1,\"mon_start\":\"2130\",\"mon_end\":\"0700\","
                   "\"bms\":1,\"vol\":2,\"audio\":1,\"lang\":\"English\",\"mode\":2,"
                   "\"room_number\":\"1001-a\",\"pause_tmr\":6},\"id\":%d}\n",
                   id);

      write_response(WR_FD, resp, (size_t)len);
      process_request(50);
      lv_test_wait(50);
      return;
    }

    process_request(10);
    lv_test_wait(10);
  }
}

static void test_short_pause_and_long_pause(void) {
  lv_test_mouse_click_at(100, 50);
  lv_test_wait(100);
  flow_to_activate_bed_mode();
  lv_test_wait(250);
  lv_test_mouse_click_at(120, 420);
  lv_test_wait(3000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-MON/GO-06-NKD_GO_MON_004_Pause_sensor_Screen_Toast.png"),
      "Screenshot comparison failed for GO-06-NKD_GO_MON_004_Pause_sensor_Screen.png");
  lv_test_wait(2000);
  lv_test_mouse_click_at(120, 420);
  lv_test_wait(100);
  flow_to_pause_sensor();
  lv_test_wait(1250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-MON/GO-07-NKD_GO_MON_004_Pause_sensor_Screen.png"),
      "Screenshot comparison failed for GO-07-NKD_GO_MON_004_Pause_sensor_Screen.png");
  lv_test_mouse_click_at(350, 420);
  lv_test_wait(300);
  flow_to_pause_sensor();
  lv_test_wait(1250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-MON/GO-07-NKD_GO_MON_004_Pause_sensor_Screen.png"),
      "Screenshot comparison failed for GO-07-NKD_GO_MON_004_Pause_sensor_Screen.png");
}

static void test_settings_icon(void) {
  lv_test_wait(150);
  lv_test_mouse_click_at(420, 55);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-MON/GO-08-NKD_GO_MON_005_Activated_settings_Screen.png"),
      "Screenshot comparison failed for GO-08-NKD_GO_MON_005_Activated_settings_Screen.png");
  lv_test_mouse_click_at(445, 30); // Click the "X" button
}

static void test_Presence_of_Chair_Mon_screen(void) {
  lv_test_mouse_click_at(270, 50);
  lv_test_wait(200);
  flow_to_activate_chair_mode();
  lv_test_wait(1250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-MON/GO-05-NKD_GO_MON_003_Chair_Mode_Screen.png"),
      "Screenshot comparison failed for GO-05-NKD_GO_MON_003_Chair_Mode_Screen.png");
  lv_test_mouse_click_at(100, 50);
  lv_test_wait(200);
  flow_to_activate_bed_mode();
}

static void test_pressure_injury_screen(void) {
  char resp[512];
  int len = snprintf(resp, sizeof resp,
                     "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
                     "{\"screen\":2,\"fts_avail\":127,\"fts_state\":6,\"bed_pos\":1,"
                     "\"bed_wid\":60,\"occ_size\":1,\"mon_start\":\"2130\",\"mon_end\":\"0700\","
                     "\"bms\":1,\"vol\":2,\"audio\":1,\"lang\":\"English\",\"mode\":4,"
                     "\"room_number\":\"1001-a\",\"pause_tmr\":0},\"id\":1}\n");

  write_response(WR_FD, resp, (size_t)len);
  process_request(50);
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-MON/"
                                 "GO-09-NKD_GO_MON_007_Pressure_Injury_Risk_Screen.png"),
      "Screenshot comparison failed for GO-09-NKD_GO_MON_007_Pressure_Injury_Risk_Screen.png");
}

static void test_schedule_monitoring_screen(void) {
  char resp[512];
  int len = snprintf(resp, sizeof resp,
                     "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
                     "{\"screen\":2,\"fts_avail\":127,\"fts_state\":1,\"bed_pos\":1,"
                     "\"bed_wid\":60,\"occ_size\":1,\"mon_start\":\"2200\",\"mon_end\":\"0600\","
                     "\"bms\":1,\"vol\":2,\"audio\":1,\"lang\":\"English\",\"mode\":5,"
                     "\"room_number\":\"1001-a\"},\"id\":1}\n");

  write_response(WR_FD, resp, (size_t)len);
  process_request(50);
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-MON/GO-10-NKD_GO_MON_008_Schedule_Monitoring_Screen.png"),
      "Screenshot comparison failed for GO-10-NKD_GO_MON_008_Schedule_Monitoring_Screen.png");
}

static void test_fall_monitoring_screen(void) {
  char resp[512];
  int len = snprintf(resp, sizeof resp,
                     "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
                     "{\"screen\":2,\"fts_avail\":127,\"fts_state\":6,\"bed_pos\":1,"
                     "\"bed_wid\":60,\"occ_size\":1,\"mon_start\":\"\",\"mon_end\":\"\","
                     "\"bms\":1,\"vol\":2,\"audio\":1,\"lang\":\"English\",\"mode\":3,"
                     "\"room_number\":\"1001-a\"},\"id\":1}\n");

  write_response(WR_FD, resp, (size_t)len);
  process_request(50);
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-MON/GO-11-NKD_GO_MON_009_Fall_Monitoring_Screen.png"),
      "Screenshot comparison failed for GO-11-NKD_GO_MON_009_Fall_Monitoring_Screen.png");
}

static void test_bed_exit_alert_withall3_options(void) {
  const char *refs[4] = {
      NULL, "ref_imgsNKD/NKD-GO/NKD-ALE/GO-01-NKD_GO_ALE_001-Bed_Exit_Alert_low.png",
      "ref_imgsNKD/NKD-GO/NKD-ALE/GO-02-NKD_GO_ALE_001-Bed_Exit_Alert_High.png",
      "ref_imgsNKD/NKD-GO/NKD-ALE/GO-03-NKD_GO_ALE_001-Bed_Exit_Alert_UltraHigh.png"};

  char resp[512];
  for (int bms = 1; bms <= 3; ++bms) {
    int len =
        snprintf(resp, sizeof(resp),
                 "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
                 "{\"screen\":2,\"fts_avail\":121,\"fts_state\":7,\"bed_pos\":1,"
                 "\"bed_wid\":60,\"occ_size\":2,\"mon_start\":\"2130\",\"mon_end\":\"0700\","
                 "\"bms\":%d,\"vol\":2,\"audio\":1,\"lang\":\"English\",\"mode\":1,\"pause_tmr\":0,"
                 "\"alert\":1,\"syserr\":0,\"syserr_title\":\"\",\"abc\":0,"
                 "\"room_number\":\"1001-a\"},\"id\":1}\n",
                 bms);

    write_response(WR_FD, resp, (size_t)len);
    process_request(50);
    lv_test_wait(500);
    TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare(refs[bms]),
                             "Screenshot comparison failed for Bed Exit Alert with all 3 options");
  }
}

static void test_chair_exit_alert(void) {
  char resp[512];
  int len =
      snprintf(resp, sizeof(resp),
               "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
               "{\"screen\":2,\"fts_avail\":121,\"fts_state\":7,\"bed_pos\":1,"
               "\"bed_wid\":60,\"occ_size\":2,\"mon_start\":\"2130\",\"mon_end\":\"0700\","
               "\"bms\":1,\"vol\":2,\"audio\":1,\"lang\":\"English\",\"mode\":1,\"pause_tmr\":0,"
               "\"alert\":2,\"syserr\":0,\"syserr_title\":\"\",\"abc\":0,"
               "\"room_number\":\"1001-a\"},\"id\":1}\n");

  write_response(WR_FD, resp, (size_t)len);
  process_request(50);
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ALE/GO-04-NKD_GO_ALE_002-Chair_Exit_Alert.png"),
      "Screenshot comparison failed for Chair Exit Alert");
}

static void test_fall_alert_screen(void) {
  char resp[512];
  int len =
      snprintf(resp, sizeof(resp),
               "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
               "{\"screen\":2,\"fts_avail\":121,\"fts_state\":6,\"bed_pos\":1,"
               "\"bed_wid\":60,\"occ_size\":2,\"mon_start\":\"2130\",\"mon_end\":\"0700\","
               "\"bms\":1,\"vol\":2,\"audio\":1,\"lang\":\"English\",\"mode\":1,\"pause_tmr\":0,"
               "\"alert\":101,\"syserr\":0,\"syserr_title\":\"\",\"abc\":0,"
               "\"room_number\":\"1001-a\"},\"id\":1}\n");

  write_response(WR_FD, resp, (size_t)len);
  process_request(50);
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ALE/GO-05-NKD_GO_ALE_003-Fall_Alert.png"),
      "Screenshot comparison failed for Fall Alert");
}

static void test_fall_alert_with_bed_chair_enabled(void) {
  char resp[512];
  int len =
      snprintf(resp, sizeof(resp),
               "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
               "{\"screen\":2,\"fts_avail\":121,\"fts_state\":7,\"bed_pos\":1,"
               "\"bed_wid\":60,\"occ_size\":2,\"mon_start\":\"2130\",\"mon_end\":\"0700\","
               "\"bms\":1,\"vol\":2,\"audio\":1,\"lang\":\"English\",\"mode\":1,\"pause_tmr\":0,"
               "\"alert\":101,\"syserr\":0,\"syserr_title\":\"\",\"abc\":0,"
               "\"room_number\":\"1001-a\"},\"id\":1}\n");

  write_response(WR_FD, resp, (size_t)len);
  process_request(50);
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ALE/GO-06-NKD_GO_ALE_004-Fall_Alert_with_bed_chair.png"),
      "Screenshot comparison failed for Fall Alert");
}

static void test_reposition_alert_screen(void) {
  char resp[512];
  int len =
      snprintf(resp, sizeof(resp),
               "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
               "{\"screen\":2,\"fts_avail\":121,\"fts_state\":6,\"bed_pos\":1,"
               "\"bed_wid\":60,\"occ_size\":2,\"mon_start\":\"2130\",\"mon_end\":\"0700\","
               "\"bms\":1,\"vol\":2,\"audio\":1,\"lang\":\"English\",\"mode\":1,\"pause_tmr\":0,"
               "\"alert\":102,\"syserr\":0,\"syserr_title\":\"\",\"abc\":0,"
               "\"room_number\":\"1001-a\"},\"id\":1}\n");

  write_response(WR_FD, resp, (size_t)len);
  process_request(50);
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ALE/GO-07-NKD_GO_ALE_005-Reposition_Alert.png"),
      "Screenshot comparison failed for Reposition Alert");
}

static void test_reposition_alert_with_bed_chair(void) {
  char resp[512];
  int len =
      snprintf(resp, sizeof(resp),
               "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
               "{\"screen\":2,\"fts_avail\":121,\"fts_state\":7,\"bed_pos\":1,"
               "\"bed_wid\":60,\"occ_size\":2,\"mon_start\":\"2130\",\"mon_end\":\"0700\","
               "\"bms\":1,\"vol\":2,\"audio\":1,\"lang\":\"English\",\"mode\":1,\"pause_tmr\":0,"
               "\"alert\":102,\"syserr\":0,\"syserr_title\":\"\",\"abc\":0,"
               "\"room_number\":\"1001-a\"},\"id\":1}\n");

  write_response(WR_FD, resp, (size_t)len);
  process_request(50);
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ALE/GO-08-NKD_GO_ALE_006-Reposition_Alert_with_bed_chair.png"),
      "Screenshot comparison failed for Reposition Alert with bed chair");
}

static void test_paused_error_screen(void) {
  char resp[512];
  int len = snprintf(resp, sizeof(resp),
                     "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
                     "{\"screen\":2,\"fts_avail\":127,\"fts_state\":7,\"bed_pos\":1,"
                     "\"bed_wid\":60,\"occ_size\":1,\"mon_start\":\"2130\",\"mon_end\":\"0700\","
                     "\"bms\":1,\"vol\":2,\"audio\":1,\"lang\":\"English\",\"mode\":2,"
                     "\"room_number\":\"1001-a\",\"pause_tmr\":6},\"id\":1}\n");

  write_response(WR_FD, resp, (size_t)len);
  process_request(50);
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ALE/GO-09-NKD_GO_ALE_007-paused_error_screen.png"),
      "Screenshot comparison failed for Paused Error Screen");
}

static void test_calibaration_error_screen(void) {
  char resp[512];
  int len = snprintf(resp, sizeof resp,
                     "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
                     "{\"screen\":2,\"fts_avail\":127,\"fts_state\":7,\"bed_pos\":1,"
                     "\"bed_wid\":60,\"occ_size\":1,\"mon_start\":\"2130\",\"mon_end\":\"0700\","
                     "\"bms\":1,\"vol\":2,\"audio\":1,\"lang\":\"English\",\"mode\":2,"
                     "\"room_number\":\"1001-a\",\"cal\":1},\"id\":1}\n");

  write_response(WR_FD, resp, (size_t)len);
  process_request(50);
  lv_test_wait(50);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ALE/GO-10-NKD_GO_ALE_008-calibration_error_screen.png"),
      "Screenshot comparison failed for Calibration Error Screen");
}

static void test_chair_calibarating_screen(void) {
  char resp[512];
  int len = snprintf(resp, sizeof resp,
                     "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
                     "{\"screen\":2,\"fts_avail\":127,\"fts_state\":7,\"bed_pos\":1,"
                     "\"bed_wid\":60,\"occ_size\":1,\"mon_start\":\"2130\",\"mon_end\":\"0700\","
                     "\"bms\":1,\"vol\":2,\"audio\":1,\"lang\":\"English\",\"mode\":2,"
                     "\"room_number\":\"1001-a\",\"cal\":2},\"id\":1}\n");

  write_response(WR_FD, resp, (size_t)len);
  process_request(50);
  lv_test_wait(50);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ALE/GO-11-NKD_GO_ALE_009-chair_calibration_error_screen.png"),
      "Screenshot comparison failed for chair Calibration Error Screen");
}

static void test_incorrect_mode_error_screen(void) {
  char resp[768];
  int len = snprintf(
      resp, sizeof resp,
      "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
      "{\"screen\":2,\"fts_avail\":121,\"fts_state\":7,\"bed_pos\":1,\"bed_wid\":60,"
      "\"occ_size\":2,\"mon_start\":\"2130\",\"mon_end\":\"0700\",\"bms\":1,\"vol\":2,"
      "\"audio\":1,\"lang\":\"English\",\"mode\":1,\"pause_tmr\":0,\"alert\":0,\"syserr\":11,"
      "\"syserr_title\":\"\",\"abc\":0,\"room_number\":\"1001-a\"},\"id\":1}\n");

  write_response(WR_FD, resp, (size_t)len);
  process_request(50);
  lv_test_wait(50);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ALE/GO-12-NKD_GO_ALE_010-Incorrect_Mode_Error_Screen.png"),
      "Screenshot comparison failed for Incorrect Mode Error Screen");
}

static void test_unassigned_error_screen(void) {
  char resp[768];
  int len = snprintf(
      resp, sizeof resp,
      "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
      "{\"screen\":2,\"fts_avail\":121,\"fts_state\":7,\"bed_pos\":1,\"bed_wid\":60,"
      "\"occ_size\":2,\"mon_start\":\"2130\",\"mon_end\":\"0700\",\"bms\":1,\"vol\":2,"
      "\"audio\":1,\"lang\":\"English\",\"mode\":1,\"pause_tmr\":0,\"alert\":0,\"syserr\":61,"
      "\"syserr_title\":\"\",\"abc\":0,\"room_number\":\"1001-a\"},\"id\":1}\n");

  write_response(WR_FD, resp, (size_t)len);
  process_request(50);
  lv_test_wait(50);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ALE/GO-13-NKD_GO_ALE_011-Unassigned_Error_Screen.png"),
      "Screenshot comparison failed for Unassigned Error Screen");
}

static void test_sunight_error_screen(void) {
  char resp[768];
  int len = snprintf(
      resp, sizeof resp,
      "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
      "{\"screen\":2,\"fts_avail\":121,\"fts_state\":7,\"bed_pos\":1,\"bed_wid\":60,"
      "\"occ_size\":2,\"mon_start\":\"2130\",\"mon_end\":\"0700\",\"bms\":1,\"vol\":2,"
      "\"audio\":1,\"lang\":\"English\",\"mode\":1,\"pause_tmr\":0,\"alert\":0,\"syserr\":12,"
      "\"syserr_title\":\"\",\"abc\":0,\"room_number\":\"1001-a\"},\"id\":1}\n");

  write_response(WR_FD, resp, (size_t)len);
  process_request(50);
  lv_test_wait(50);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ALE/"
                                 "GO-14-NKD_GO_ALE_012-Sunlight_Interference_Error_Screen.png"),
      "Screenshot comparison failed for Sunlight Interference Error Screen");
}

static void test_obstructed_error_screen(void) {
  char resp[768];
  int len = snprintf(
      resp, sizeof resp,
      "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
      "{\"screen\":2,\"fts_avail\":121,\"fts_state\":7,\"bed_pos\":1,\"bed_wid\":60,"
      "\"occ_size\":2,\"mon_start\":\"2130\",\"mon_end\":\"0700\",\"bms\":1,\"vol\":2,"
      "\"audio\":1,\"lang\":\"English\",\"mode\":1,\"pause_tmr\":0,\"alert\":0,\"syserr\":13,"
      "\"syserr_title\":\"\",\"abc\":0,\"room_number\":\"1001-a\"},\"id\":1}\n");

  write_response(WR_FD, resp, (size_t)len);
  process_request(50);
  lv_test_wait(50);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ALE/GO-15-NKD_GO_ALE_013-Obstructed_Sensor_Error_Screen.png"),
      "Screenshot comparison failed for Obstructed Sensor Error Screen");
}

static void test_system_disconnected_error_screen(void) {
  char resp[768];
  int len = snprintf(
      resp, sizeof resp,
      "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
      "{\"screen\":2,\"fts_avail\":121,\"fts_state\":7,\"bed_pos\":1,\"bed_wid\":60,"
      "\"occ_size\":2,\"mon_start\":\"2130\",\"mon_end\":\"0700\",\"bms\":1,\"vol\":2,"
      "\"audio\":1,\"lang\":\"English\",\"mode\":1,\"pause_tmr\":0,\"alert\":0,\"syserr\":42,"
      "\"syserr_title\":\"\",\"abc\":0,\"room_number\":\"1001-a\"},\"id\":1}\n");

  write_response(WR_FD, resp, (size_t)len);
  process_request(50);
  lv_test_wait(50);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ALE/GO-16-NKD_GO_ALE_014-System_Disconnected_Error_Screen.png"),
      "Screenshot comparison failed for System Disconnected Error Screen");
}

static void test_not_monitoring_screen(void) {
  char resp[768];
  int len = snprintf(
      resp, sizeof resp,
      "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
      "{\"screen\":2,\"fts_avail\":121,\"fts_state\":7,\"bed_pos\":1,\"bed_wid\":60,"
      "\"occ_size\":2,\"mon_start\":\"2130\",\"mon_end\":\"0700\",\"bms\":1,\"vol\":2,"
      "\"audio\":1,\"lang\":\"English\",\"mode\":1,\"pause_tmr\":0,\"alert\":0,\"syserr\":41,"
      "\"syserr_title\":\"\",\"abc\":0,\"room_number\":\"1001-a\"},\"id\":1}\n");

  write_response(WR_FD, resp, (size_t)len);
  process_request(50);
  lv_test_wait(50);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ALE/GO-17-NKD_GO_ALE_015-Not_Monitoring_Screen.png"),
      "Screenshot comparison failed for Not Monitoring Screen");
}

static void send_set_state_with_fts_avail(int fts_avail) {
  char resp[768];

  int len = snprintf(
      resp, sizeof resp,
      "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
      "{\"screen\":2,\"fts_avail\":%d,\"fts_state\":7,\"bed_pos\":1,\"bed_wid\":60,"
      "\"occ_size\":2,\"mon_start\":\"2130\",\"mon_end\":\"0700\",\"bms\":1,\"vol\":2,"
      "\"audio\":1,\"lang\":\"English\",\"mode\":1,\"pause_tmr\":0,\"alert\":0,\"syserr\":0,"
      "\"syserr_title\":\"\",\"abc\":0,\"room_number\":\"1001-a\"},\"id\":1}\n",
      fts_avail);

  write_response(WR_FD, resp, (size_t)len);
  process_request(50);
  lv_test_wait(50);
}

static void test_settings_Screen_with_buttons(void) {
  send_set_state_with_fts_avail(127);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  lv_test_wait(100);
  lv_test_mouse_click_at(446, 30); // X
}

static void test_i_btn_on_settings_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  flow_to_sysinfo_screen();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-76-NKD_GO_ACT_001_Sysinfo.png"),
      "Screenshot comparison failed for IN-76-Sysinfo");
  lv_test_mouse_click_at(450, 450); // Close button
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  flow_to_sysinfo_screen();
  lv_test_mouse_click_at(446, 30); // X
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  lv_test_wait(100);
  lv_test_mouse_click_at(446, 30); // X
}

static void test_X_btn_on_settings_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30); // Click the "X" button on Settings screen
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-150-NKD_GO_ACT_027_Activating_Screen_with_value_2.png"),
      "Screenshot comparison failed for GO-150-Activating_Screen_with_value_2");
}

static void test_optional_settings_available(void) {
  send_set_state_with_fts_avail(127);
  lv_test_wait(100);
  lv_test_mouse_click_at(420, 50);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  lv_test_mouse_click_at(100, 110); // Click the Display settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-02-NKD_GO_SET_002-Display_Settings.png"),
      "Screenshot comparison failed for Display Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(335, 115); // click on In-Room-Alert Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-03-NKD_GO_SET_002-In-Room_Alert_Settings.png"),
      "Screenshot comparison failed for In-Room Alert Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(140, 190); // click on Alerts Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-04-NKD_GO_SET_002-Alerts_Settings.png"),
      "Screenshot comparison failed for Alerts Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(340, 185); // Click the "Exit Alert" button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-05-NKD_GO_SET_002-Exit_Alert_Settings.png"),
      "Screenshot comparison failed for Exit Alert Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(110, 260); // Click the "Bed Sensitivity" button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-06-NKD_GO_SET_002-Bed_Sensitivity_Settings.png"),
      "Screenshot comparison failed for Bed Sensitivity Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(350, 255); // Click the "Bed width" button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-07-NKD_GO_SET_002-Bed_Width_Settings.png"),
      "Screenshot comparison failed for Bed Width Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(120, 330); // Click the "Bed Placement" button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-08-NKD_GO_SET_002-Bed_Placement_Settings.png"),
      "Screenshot comparison failed for Bed Placement Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(345, 330); // Click the "Occupant Size" button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-09-NKD_GO_SET_002-Occupant_Size_Settings.png"),
      "Screenshot comparison failed for Occupant Size Settings screen");
  lv_test_mouse_click_at(445, 30);
  lv_test_wait(500);
  send_set_state_with_fts_avail(9);
  lv_test_wait(1000);
  lv_test_mouse_click_at(420, 50);
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-10-NKD_GO_SET_002-optional_settings_with_fts_9.png"),
      "Screenshot comparison failed for optional settings with fts 9");
  lv_test_mouse_click_at(100, 110); // Click the Display settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-02-NKD_GO_SET_002-Display_Settings.png"),
      "Screenshot comparison failed for Display Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(335, 115); // click on In-Room-Alert Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-03-NKD_GO_SET_002-In-Room_Alert_Settings.png"),
      "Screenshot comparison failed for In-Room Alert Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(140, 190); // click on Alerts Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-11-NKD_GO_SET_002-Alerts_Settings_with_fts_9_to_121.png"),
      "Screenshot comparison failed for Alerts Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(340, 185); // Click the "Exit Alert" button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-05-NKD_GO_SET_002-Exit_Alert_Settings.png"),
      "Screenshot comparison failed for Exit Alert Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(110, 260); // Click the "Bed Sensitivity" button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-06-NKD_GO_SET_002-Bed_Sensitivity_Settings.png"),
      "Screenshot comparison failed for Bed Sensitivity Settings screen");
  lv_test_mouse_click_at(445, 30);
  lv_test_wait(500);
  send_set_state_with_fts_avail(17);
  lv_test_wait(1000);
  lv_test_mouse_click_at(420, 50);
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-12-NKD_GO_SET_002-optional_settings_with_fts_17.png"),
      "Screenshot comparison failed for optional settings with fts 17");
  lv_test_mouse_click_at(100, 110); // Click the Display settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-02-NKD_GO_SET_002-Display_Settings.png"),
      "Screenshot comparison failed for Display Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(335, 115); // click on In-Room-Alert Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-03-NKD_GO_SET_002-In-Room_Alert_Settings.png"),
      "Screenshot comparison failed for In-Room Alert Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(140, 190); // click on Alerts Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-11-NKD_GO_SET_002-Alerts_Settings_with_fts_9_to_121.png"),
      "Screenshot comparison failed for Alerts Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(340, 180); // Click the "Bed Sensitivity" button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-06-NKD_GO_SET_002-Bed_Sensitivity_Settings.png"),
      "Screenshot comparison failed for Bed Sensitivity Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(100, 260); // Click the "Bed Placement" button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-08-NKD_GO_SET_002-Bed_Placement_Settings.png"),
      "Screenshot comparison failed for Bed Placement Settings screen");
  lv_test_mouse_click_at(445, 30);
  lv_test_wait(500);
  send_set_state_with_fts_avail(33);
  lv_test_wait(1000);
  lv_test_mouse_click_at(420, 50);
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-13-NKD_GO_SET_002-optional_settings_with_fts_33.png"),
      "Screenshot comparison failed for optional settings with fts 33");
  lv_test_mouse_click_at(100, 110); // Click the Display settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-02-NKD_GO_SET_002-Display_Settings.png"),
      "Screenshot comparison failed for Display Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(335, 115); // click on In-Room-Alert Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-03-NKD_GO_SET_002-In-Room_Alert_Settings.png"),
      "Screenshot comparison failed for In-Room Alert Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(140, 190); // click on Alerts Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-11-NKD_GO_SET_002-Alerts_Settings_with_fts_9_to_121.png"),
      "Screenshot comparison failed for Alerts Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(340, 180); // Click the "Bed Sensitivity" button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-06-NKD_GO_SET_002-Bed_Sensitivity_Settings.png"),
      "Screenshot comparison failed for Bed Sensitivity Settings screen");
  lv_test_mouse_click_at(30, 30);
  lv_test_wait(100);
  lv_test_mouse_click_at(160, 270); // Click the "Bed width" button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-07-NKD_GO_SET_002-Bed_Width_Settings.png"),
      "Screenshot comparison failed for Bed Width Settings screen");
  lv_test_mouse_click_at(445, 30);
  lv_test_wait(500);
  send_set_state_with_fts_avail(65);
  lv_test_wait(1000);
  lv_test_mouse_click_at(420, 50);
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-14-NKD_GO_SET_002-optional_settings_with_fts_65.png"),
      "Screenshot comparison failed for optional settings with fts 65");
  lv_test_mouse_click_at(100, 110); // Click the Display settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-02-NKD_GO_SET_002-Display_Settings.png"),
      "Screenshot comparison failed for Display Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(335, 115); // click on In-Room-Alert Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-03-NKD_GO_SET_002-In-Room_Alert_Settings.png"),
      "Screenshot comparison failed for In-Room Alert Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(140, 190); // click on Alerts Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-11-NKD_GO_SET_002-Alerts_Settings_with_fts_9_to_121.png"),
      "Screenshot comparison failed for Alerts Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(340, 180); // Click the "Bed Sensitivity" button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-06-NKD_GO_SET_002-Bed_Sensitivity_Settings.png"),
      "Screenshot comparison failed for Bed Sensitivity Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(160, 270); // Click the "Occupant Size" button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-09-NKD_GO_SET_002-Occupant_Size_Settings.png"),
      "Screenshot comparison failed for Occupant Size Settings screen");
  lv_test_mouse_click_at(445, 30);
  lv_test_wait(100);
  send_set_state_with_fts_avail(1);
  lv_test_wait(100);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-15-NKD_GO_SET_002-optional_settings_with_fts_1.png"),
      "Screenshot comparison failed for optional settings with fts 1");
  lv_test_mouse_click_at(140, 190); // click on Alerts Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-11-NKD_GO_SET_002-Alerts_Settings_with_fts_9_to_121.png"),
      "Screenshot comparison failed for Alerts Settings screen");
  lv_test_mouse_click_at(445, 30);
  lv_test_wait(100);
  send_set_state_with_fts_avail(3);
  lv_test_wait(100);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-15-NKD_GO_SET_002-optional_settings_with_fts_1.png"),
      "Screenshot comparison failed for optional settings with fts 1");
  lv_test_mouse_click_at(140, 190); // click on Alerts Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-17-NKD_GO_SET_002-Alerts_Settings_with_fts_avail_3.png"),
      "Screenshot comparison failed for Alerts Settings screen for fts avail 3");
  lv_test_mouse_click_at(445, 30);
  lv_test_wait(100);
  send_set_state_with_fts_avail(5);
  lv_test_wait(100);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-15-NKD_GO_SET_002-optional_settings_with_fts_1.png"),
      "Screenshot comparison failed for optional settings with fts 1");
  lv_test_mouse_click_at(140, 190); // click on Alerts Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-18-NKD_GO_SET_002-Alerts_Settings_with_fts_avail_5.png"),
      "Screenshot comparison failed for Alerts Settings screen for fts avail 5");
  lv_test_mouse_click_at(445, 30);
  lv_test_wait(100);
  send_set_state_with_fts_avail(7);
  lv_test_wait(100);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-15-NKD_GO_SET_002-optional_settings_with_fts_1.png"),
      "Screenshot comparison failed for optional settings with fts 1");
  lv_test_mouse_click_at(140, 190); // click on Alerts Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-04-NKD_GO_SET_002-Alerts_Settings.png"),
      "Screenshot comparison failed for Alerts Settings screen");
  lv_test_mouse_click_at(445, 30);
  lv_test_wait(100);
  send_set_state_with_fts_avail(11);
  lv_test_wait(100);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-10-NKD_GO_SET_002-optional_settings_with_fts_9.png"),
      "Screenshot comparison failed for optional settings with fts 9");
  lv_test_mouse_click_at(140, 190); // click on Alerts Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-17-NKD_GO_SET_002-Alerts_Settings_with_fts_avail_3.png"),
      "Screenshot comparison failed for Alerts Settings screen for fts avail 3");
  lv_test_mouse_click_at(445, 30);
  lv_test_wait(100);
  send_set_state_with_fts_avail(13);
  lv_test_wait(100);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-10-NKD_GO_SET_002-optional_settings_with_fts_9.png"),
      "Screenshot comparison failed for optional settings with fts 9");
  lv_test_mouse_click_at(140, 190); // click on Alerts Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-18-NKD_GO_SET_002-Alerts_Settings_with_fts_avail_5.png"),
      "Screenshot comparison failed for Alerts Settings screen for fts avail 5");
  lv_test_mouse_click_at(445, 30);
  lv_test_wait(100);
  send_set_state_with_fts_avail(23);
  lv_test_wait(100);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-12-NKD_GO_SET_002-optional_settings_with_fts_17.png"),
      "Screenshot comparison failed for optional settings with fts 17");
  lv_test_mouse_click_at(140, 190); // click on Alerts Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-04-NKD_GO_SET_002-Alerts_Settings.png"),
      "Screenshot comparison failed for Alerts Settings screen");
  lv_test_mouse_click_at(445, 30);
  lv_test_wait(100);
  send_set_state_with_fts_avail(41);
  lv_test_wait(100);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-19-NKD_GO_SET_002-optional_settings_with_fts_41.png"),
      "Screenshot comparison failed for optional settings with fts 41");
  lv_test_mouse_click_at(140, 190); // click on Alerts Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-11-NKD_GO_SET_002-Alerts_Settings_with_fts_9_to_121.png"),
      "Screenshot comparison failed for Alerts Settings screen");
  lv_test_mouse_click_at(445, 30);
  lv_test_wait(100);
  send_set_state_with_fts_avail(95);
  lv_test_wait(100);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-20-NKD_GO_SET_002-optional_settings_with_fts_95.png"),
      "Screenshot comparison failed for optional settings with fts 95");
  lv_test_mouse_click_at(140, 190); // click on Alerts Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-04-NKD_GO_SET_002-Alerts_Settings.png"),
      "Screenshot comparison failed for Alerts Settings screen");
  lv_test_mouse_click_at(445, 30);
}

static void test_bms_on_settings_flow(void) {
  send_set_state_with_fts_avail(127);
  lv_test_wait(100);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(110, 260); // Click the "Bed Sensitivity" button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-06-NKD_GO_SET_002-Bed_Sensitivity_Settings.png"),
      "Screenshot comparison failed for Bed Sensitivity Settings screen");
  lv_test_mouse_click_at(445, 30);
}

static void test_functionality_of_bms_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(110, 260); // Click the "Bed Sensitivity" button
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-21-NKD_GO_SET_004-BMS_Slider_Low.png"),
      "Screenshot comparison failed for BMS Slider Low");
  lv_test_mouse_click_at(240, 190); // Click and drag slider to HIgh
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-22-NKD_GO_SET_004_BMS_Slider_High.png"),
      "Screenshot comparison failed for IN-22-BMS Slider High");
  lv_test_mouse_click_at(430, 190); // Click and drag slider to UltraHigh
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-23-NKD_GO_SET_004_BMS_Slider_UltraHigh.png"),
      "Screenshot comparison failed for IN-23-BMS Slider UltraHigh");
  lv_test_mouse_click_at(50, 190); // Click and drag slider to Low
  lv_test_wait(250);
  lv_test_mouse_click_at(445, 30);
}

static void test_back_btn_on_bms_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(110, 260); // Click the "Bed Sensitivity" button
  lv_test_wait(100);
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  lv_test_wait(250);
  lv_test_mouse_click_at(445, 30);
}

static void test_X_btn_on_bms_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(110, 260); // Click the "Bed Sensitivity" button
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30); // Click the "X" button
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-150-NKD_GO_ACT_027_Activating_Screen_with_value_2.png"),
      "Screenshot comparison failed for GO-150-Activating_Screen_with_value_2");
}

static void test_save_button_on_bms_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(110, 260); // Click the "Bed Sensitivity" button
  lv_test_mouse_click_at(240, 190); // Click and drag slider to HIgh
  lv_test_wait(250);
  lv_test_mouse_click_at(230, 420); // Click the Save button
  lv_test_wait(3000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-24-NKD_GO_SET_006_Save_Button_BMS_Settings_Toast.png"),
      "Screenshot comparison failed for Save Button BMS Settings Toast");
  lv_test_wait(1500);
  lv_test_mouse_click_at(230, 420); // Click the Save button
  char line[512];
  const uint32_t timeout_ms = 2000;
  uint32_t waited = 0;
  bool handled = false;

  while (waited < timeout_ms && !handled) {
    while (read_line_nb(RD_FD, line, sizeof(line))) {
      if (strstr(line, "\"method\":\"save_bms\"")) {

        // Extract bms from request params
        int bms_req = 0;
        if (char *b = strstr(line, "\"bms\":")) {
          sscanf(b, "\"bms\":%d", &bms_req);
        }

        // Respond with set_state using bms from request (id fixed to 1)
        char resp[512];
        int len =
            snprintf(resp, sizeof(resp),
                     "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
                     "{\"screen\":2,\"fts_avail\":127,\"fts_state\":7,\"bed_pos\":1,"
                     "\"bed_wid\":60,\"occ_size\":1,\"mon_start\":\"2130\",\"mon_end\":\"0700\","
                     "\"bms\":%d,\"vol\":2,\"audio\":1,\"lang\":\"English\",\"mode\":1,"
                     "\"room_number\":\"1001-a\"},\"id\":1}\n",
                     bms_req);

        write_response(WR_FD, resp, (size_t)len);
        process_request(50);
        lv_test_wait(50);
        handled = true;
        break;
      }
    }

    if (!handled) {
      process_request(10);
      lv_test_wait(10);
      waited += 10;
    }
  }
  lv_test_wait(1500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-25-NKD_GO_SET_006_BMS_Saved_with_toast.png"),
      "Screenshot comparison failed for BMS Saved with toast");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-26-NKD_GO_SET_006_BMS_Saved_no_toast.png"),
      "Screenshot comparison failed for BMS Saved no toast");
  lv_test_mouse_click_at(50, 190); // Click and drag slider to Low
  lv_test_wait(250);
  lv_test_mouse_click_at(230,
                         420); // Click the Save button
                               // Respond with set_state using bms from request (id fixed to 1)
  char resp[512];
  int len = snprintf(resp, sizeof(resp),
                     "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
                     "{\"screen\":2,\"fts_avail\":127,\"fts_state\":7,\"bed_pos\":1,"
                     "\"bed_wid\":60,\"occ_size\":1,\"mon_start\":\"2130\",\"mon_end\":\"0700\","
                     "\"bms\":1,\"vol\":2,\"audio\":1,\"lang\":\"English\",\"mode\":1,"
                     "\"room_number\":\"1001-a\"},\"id\":1}\n");

  write_response(WR_FD, resp, (size_t)len);
  process_request(50);
  lv_test_wait(350);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-27-NKD_GO_SET_006_BMS_Saved_low_with_toast.png"),
      "Screenshot comparison failed for BMS Saved with toast");
  lv_test_wait(3000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-28-NKD_GO_SET_006_BMS_Saved_low_without_toast.png"),
      "Screenshot comparison failed for BMS Saved without toast");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30); // Click the "X" button
}

static void test_back_button_after_BMS_saved(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(110, 260); // Click the "Bed Sensitivity" button
  lv_test_wait(100);
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30); // Click the "X" button
}

static void test_X_button_after_BMS_saved(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(110, 260); // Click the "Bed Sensitivity" button
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30); // Click the "X" button
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-150-NKD_GO_ACT_027_Activating_Screen_with_value_2.png"),
      "Screenshot comparison failed for GO-150-Activating_Screen_with_value_2");
}

static void test_in_room_audio_alerts_on_settings_flow(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(335, 115); // click on In-Room-Alert Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-03-NKD_GO_SET_002-In-Room_Alert_Settings.png"),
      "Screenshot comparison failed for In-Room Alert Settings screen");
  lv_test_mouse_click_at(425, 100); // Click on Audio Alerts toggle button
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-29-NKD_GO_SET_008_In-Room_Audio_Alerts_Off.png"),
      "Screenshot comparison failed for In-Room Audio Alerts Off");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30); // Click the "X" button
}

static void test_X_btn_on_in_room_audio_alerts_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(335, 115); // click on In-Room-Alert Settings button
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30); // Click the "X" button
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-150-NKD_GO_ACT_027_Activating_Screen_with_value_2.png"),
      "Screenshot comparison failed for GO-150-Activating_Screen_with_value_2");
}

static void test_back_btn_on_in_room_audio_alerts_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(335, 115); // click on In-Room-Alert Settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30); // Click the "X" button
}

static void test_toggle_on_in_room_audio_alerts_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(335, 115); // click on In-Room-Alert Settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(425, 100); // Click on Audio Alerts toggle button
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-29-NKD_GO_SET_008_In-Room_Audio_Alerts_Off.png"),
      "Screenshot comparison failed for In-Room Audio Alerts Off");
  lv_test_mouse_click_at(425, 100); // Click on Audio Alerts toggle button
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-03-NKD_GO_SET_002-In-Room_Alert_Settings.png"),
      "Screenshot comparison failed for In-Room Alert Settings screen");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30); // Click the "X" button
}

static void test_languages_screen_with_no_command(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(335, 115); // click on In-Room-Alert Settings button
  lv_test_wait(100);
  lv_test_mouse_click_at(350, 210);
  lv_test_wait(5000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-30-NKD_GO_SET_009-Languages_Screen_no_Command.png"),
      "Screenshot comparison failed for Languages Screen");
  lv_test_mouse_click_at(30, 30);
  lv_test_wait(200);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-31-NKD_GO_SET_009-No_Language_Selected.png"),
      "Screenshot comparison failed for No Language");
  lv_test_mouse_click_at(350, 210);
  lv_test_wait(3000);
  lv_test_mouse_click_at(350, 160);
  lv_test_wait(1000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-32-NKD_GO_SET_009-Spanish_Language_Selected.png"),
      "Screenshot comparison failed for Spanish Language");
  lv_test_mouse_click_at(130, 160);
  lv_test_wait(1000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-33-NKD_GO_SET_009-English_Language_Selected.png"),
      "Screenshot comparison failed for English Language");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30); // Click the "X" button
}

static void test_back_btn_on_languages_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(335, 115); // click on In-Room-Alert Settings button
  lv_test_wait(100);
  lv_test_mouse_click_at(350, 210);
  lv_test_wait(3000);
  lv_test_mouse_click_at(30, 30);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-03-NKD_GO_SET_002-In-Room_Alert_Settings.png"),
      "Screenshot comparison failed for In-Room Alert Settings screen");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_X_btn_on_languages_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(335, 115); // click on In-Room-Alert Settings button
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-150-NKD_GO_ACT_027_Activating_Screen_with_value_2.png"),
      "Screenshot comparison failed for GO-150-Activating_Screen_with_value_2");
}

static void flow_to_lang_screen(int count) {
  if (count < 0)
    count = 0;
  if (count > 10)
    count = 10;

  static const char *langs[] = {
      "Arabic", "Cantonese", "English", "French", "Haitian",
      "Creole", "Gujarati",  "Hindi",   "Korean", "Mandarin",
  };

  char line[512];
  while (read_line_nb(RD_FD, line, sizeof(line))) {
  }

  int id = 1;
  lv_test_mouse_click_at(340, 210);

  const uint32_t timeout_ms = 2000;
  uint32_t waited = 0;
  bool handled = false;

  while (waited < timeout_ms && !handled) {
    while (read_line_nb(RD_FD, line, sizeof(line))) {
      if (strstr(line, "\"method\":\"get_languages\"")) {

        if (char *p = strstr(line, "\"id\":")) {
          sscanf(p, "\"id\":%d", &id);
        }

        char resp[512];
        int pos = 0;

        pos +=
            snprintf(resp + pos, sizeof(resp) - (size_t)pos, "{\"jsonrpc\":\"2.0\",\"result\":[");
        for (int i = 0; i < count; i++) {
          pos += snprintf(resp + pos, sizeof(resp) - (size_t)pos, "%s\"%s\"", (i ? "," : ""),
                          langs[i]);
        }
        pos += snprintf(resp + pos, sizeof(resp) - (size_t)pos, "],\"id\":%d}\n", id);

        write_response(WR_FD, resp, (size_t)pos);
        process_request(50);
        lv_test_wait(50);
        handled = true;
        break;
      }
    }

    if (!handled) {
      process_request(10);
      lv_test_wait(10);
      waited += 10;
    }
  }
}

static void test_languages_screen_with_command(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(335, 115); // click on In-Room-Alert Settings button
  lv_test_wait(300);
  flow_to_lang_screen(10);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-34-NKD_GO_SET_009-Languages_Screen_with_Command.png"),
      "Screenshot comparison failed for Languages Screen with Command");
  lv_test_mouse_click_at(130, 170); // Click the Arabic language button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-35-NKD_GO_SET_009-Arabic_Language_Selected.png"),
      "Screenshot comparison failed for Arabic Language Selected");
  lv_test_mouse_click_at(345, 165); // Click the Cantonese language button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-36-NKD_GO_SET_009-Cantonese_Language_Selected.png"),
      "Screenshot comparison failed for Cantonese Language Selected");
  lv_test_mouse_click_at(135, 235); // Click the English button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-37-NKD_GO_SET_009-English_Language_Selected.png"),
      "Screenshot comparison failed for English Language Selected");
  lv_test_mouse_click_at(340, 240); // Click the French button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-38-NKD_GO_SET_009-French_Language_Selected.png"),
      "Screenshot comparison failed for French Language Selected");
  lv_test_mouse_click_at(140, 310); // Click the Haitian Creole button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-39-NKD_GO_SET_009-Haitian_Language_Selected.png"),
      "Screenshot comparison failed for Haitian Language Selected");
  lv_test_mouse_click_at(350, 310); // Click the Gujarati button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-40-NKD_GO_SET_009-Creole_Language_Selected.png"),
      "Screenshot comparison failed for Creole Language Selected");
  lv_test_mouse_click_at(390, 380); // Click the Next page button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/"
          "GO-41-NKD_GO_SET_009_In-Room_Alert__Language_2ndpage_Selected_Screen.png"),
      "Screenshot comparison failed for In Room Alert 2nd Page Selected Screen");
  lv_test_mouse_click_at(130, 170); // Click the Hindi button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-42-NKD_GO_SET_009-Gujarati_Language_Selected.png"),
      "Screenshot comparison failed for Gujarati Language Selected");
  lv_test_mouse_click_at(350, 170); // Click the Korean button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/"
          "GO-43-NKD_GO_SET_009_In-Room_Alert_Hindi_Language_Selected_Screen.png"),
      "Screenshot comparison failed for In Room Alert HIndi Language Selected Screen");
  lv_test_mouse_click_at(130, 240); // Click the Mandarin button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/"
          "GO-44-NKD_GO_SET_009_In-Room_Alert_Korean_Language_Selected_Screen.png"),
      "Screenshot comparison failed for IN-38-In Room Alert Korean Language Selected Screen");
  lv_test_mouse_click_at(350, 240); // Click the Mandarin Button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/"
          "GO-45-NKD_GO_SET_009_In-Room_Alert_Manadarin_Language_Selected.png"),
      "Screenshot comparison failed for IN-39-In Room Alert Language");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  flow_to_lang_screen(5);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/"
          "GO-46-NKD_GO_SET_009-Languages_Screen_with_less_than_6_languages.png"),
      "Screenshot comparison failed for Languages Screen with Command");
  lv_test_mouse_click_at(130, 170); // Click the Arabic language button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-47-NKD_GO_SET_009-Arabic_Language_Selected.png"),
      "Screenshot comparison failed for Arabic Language Selected");
  lv_test_mouse_click_at(345, 165); // Click the Cantonese language button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-48-NKD_GO_SET_009-Cantonese_Language_Selected.png"),
      "Screenshot comparison failed for Cantonese Language Selected");
  lv_test_mouse_click_at(135, 235); // Click the English button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-49-NKD_GO_SET_009-English_Language_Selected.png"),
      "Screenshot comparison failed for English Language Selected");
  lv_test_mouse_click_at(340, 240); // Click the French button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-50-NKD_GO_SET_009-French_Language_Selected.png"),
      "Screenshot comparison failed for French Language Selected");
  lv_test_mouse_click_at(140, 310); // Click the Haitian Creole button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-51-NKD_GO_SET_009-Haitian_Language_Selected.png"),
      "Screenshot comparison failed for Haitian Language Selected");
  lv_test_wait(150);
  lv_test_mouse_click_at(135, 235); // Click the English button
  lv_test_wait(200);
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(200);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-31-NKD_GO_SET_009-No_Language_Selected.png"),
      "Screenshot comparison failed for No Language");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_volume_scroller_on_in_room_alert_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(335, 115); // click on In-Room-Alert Settings button
  lv_test_wait(200);
  lv_test_mouse_click_at(50, 290); // Click and drag slider to Low
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-52-NKD_GO_SET_009_Volume_Slider_Low.png"),
      "Screenshot comparison failed for Volume Slider Low");
  lv_test_mouse_click_at(240, 290); // Click and drag slider to Medium
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-53-NKD_GO_SET_009_Volume_Slider_Medium.png"),
      "Screenshot comparison failed for Volume Slider Medium");
  lv_test_mouse_click_at(430, 290); // Click and drag slider to High
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-54-NKD_GO_SET_009_Volume_Slider_High.png"),
      "Screenshot comparison failed for Volume Slider High");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void saved_in_room_alert(void) {
  char line[512];

  for (uint32_t waited = 0; waited < 2000; waited += 10) {
    while (read_line_nb(RD_FD, line, sizeof line)) {
      if (!strstr(line, "\"method\":\"save_audio\""))
        continue;

      char *v = strstr(line, "\"vol\":");
      if (!v)
        TEST_FAIL_MESSAGE("save_audio missing vol");

      char *end = NULL;
      long vol_l = strtol(v + 6, &end, 10); // 6 == strlen("\"vol\":")
      if (end == v + 6)
        TEST_FAIL_MESSAGE("save_audio vol not a number");

      int vol = (int)vol_l;

      char resp[768];
      int len = snprintf(
          resp, sizeof resp,
          "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
          "{\"screen\":2,\"fts_avail\":121,\"fts_state\":7,\"bed_pos\":1,\"bed_wid\":60,"
          "\"occ_size\":2,\"mon_start\":\"2130\",\"mon_end\":\"0700\",\"bms\":1,\"vol\":%d,"
          "\"audio\":1,\"lang\":\"English\",\"mode\":1,\"pause_tmr\":0,\"alert\":0,\"syserr\":0,"
          "\"syserr_title\":\"\",\"abc\":0,\"room_number\":\"1001-a\"},\"id\":1}\n",
          vol);

      write_response(WR_FD, resp, (size_t)len);
      process_request(50);
      lv_test_wait(50);
      drain_pipe_now();
      return;
    }

    process_request(10);
    lv_test_wait(10);
  }

  TEST_FAIL_MESSAGE("Timed out waiting for save_audio");
}

static void test_save_btn_on_in_room_alert_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(335, 115); // click on In-Room-Alert Settings button
  lv_test_wait(200);
  lv_test_mouse_click_at(430, 290); // Click and drag slider to High
  lv_test_wait(500);
  lv_test_mouse_click_at(230, 420); // Click the Save button
  lv_test_wait(3000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/"
          "GO-55-NKD_GO_SET_010_Save_Button_In-Room_Alert_Settings_Toast.png"),
      "Screenshot comparison failed for Save Button In-Room Alert Settings Toast");
  lv_test_wait(3000);
  lv_test_mouse_click_at(230, 420);
  lv_test_wait(500);
  saved_in_room_alert();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-56-NKD_GO_SET_010_In-Room_Alert_Saved_with_toast.png"),
      "Screenshot comparison failed for In-Room Alert Saved with toast");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-57-NKD_GO_SET_010_In-Room_Alert_Saved_no_toast.png"),
      "Screenshot comparison failed for In-Room Alert Saved no toast");
  lv_test_mouse_click_at(240, 290); // Click and drag slider to Medium
  lv_test_wait(200);
  lv_test_mouse_click_at(230, 420);
  lv_test_wait(500);
  saved_in_room_alert();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-58-NKD_GO_SET_010_In-Room_Alert_Saved_medium_with_toast.png"),
      "Screenshot comparison failed for In-Room Alert Saved medium with toast");
  lv_test_wait(3000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-59-NKD_GO_SET_010_In-Room_Alert_Saved_toast_timeout.png"),
      "Screenshot comparison failed for In-Room Alert Saved toast timeout");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_back_btn_on_in_room_alert_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(335, 115); // click on In-Room-Alert Settings button
  lv_test_wait(200);
  lv_test_mouse_click_at(30, 30); // Click the "Back" button
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_X_btn_on_in_room_alert_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(335, 115); // click on In-Room-Alert Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-03-NKD_GO_SET_002-In-Room_Alert_Settings.png"),
      "Screenshot comparison failed for In-Room Alert Settings screen");
  lv_test_mouse_click_at(445, 30); // Click the "X" button
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-150-NKD_GO_ACT_027_Activating_Screen_with_value_2.png"),
      "Screenshot comparison failed for GO-150-Activating_Screen_with_value_2");
}

static void test_display_page_on_activated_settings_flow(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(100, 100); // Click the Display settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-02-NKD_GO_SET_002-Display_Settings.png"),
      "Screenshot comparison failed for Display Settings Screen");
  lv_test_mouse_click_at(400, 270);
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-60-NKD_GO_SET_011_Display_Toggle_On.png"),
      "Screenshot comparison failed for Display Toggle On");
  lv_test_mouse_click_at(400, 270);
  lv_test_wait(250);
  lv_test_mouse_click_at(445, 30); // Click the "X" button
}

static void test_back_btn_on_display_settings_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(100, 100); // Click the Display settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30); // Click the "X" button
}

static void test_X_btn_on_display_settings_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(100, 110); // Click the Display settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(445, 30); // Click the "X" button
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-150-NKD_GO_ACT_027_Activating_Screen_with_value_2.png"),
      "Screenshot comparison failed for GO-150-Activating_Screen_with_value_2");
}

static void test_brightness_scroller_on_display_settings_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(100, 110); // Click the Display settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(100, 155); // Click and drag slider to Low
  lv_test_wait(200);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-61-NKD_GO_SET_012_Brightness_Slider_Low.png"),
      "Screenshot comparison failed for Brightness Slider Low");
  lv_test_mouse_click_at(390, 155); // Click and drag slider to Medium
  lv_test_wait(200);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-62-NKD_GO_SET_012_Brightness_Slider_Medium.png"),
      "Screenshot comparison failed for Brightness Slider Medium");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30); // Click the "X" button
}

static void test_adaptive_toggle_btn(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(100, 110); // Click the Display settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-63-NKD_GO_SET_013_Adaptive_Toggle_Off.png"),
      "Screenshot comparison failed for Adaptive Toggle Off");
  lv_test_mouse_click_at(400, 270);
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-64-NKD_SET_013_Adaptive_Toggle_On.png"),
      "Screenshot comparison failed for Adaptive Toggle On");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void saved_display_settings(void) {
  char line[512];

  for (uint32_t waited = 0; waited < 2000; waited += 10) {
    while (read_line_nb(RD_FD, line, sizeof line)) {
      if (!strstr(line, "\"method\":\"save_display\""))
        continue;

      char *a = strstr(line, "\"abc\":");
      if (!a)
        TEST_FAIL_MESSAGE("save_display missing abc");

      char *end = NULL;
      long abc_l = strtol(a + 6, &end, 10); // 6 == strlen("\"abc\":")
      if (end == a + 6)
        TEST_FAIL_MESSAGE("save_display abc not a number");

      int abc = (int)abc_l;

      char resp[768];
      int len = snprintf(
          resp, sizeof resp,
          "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
          "{\"screen\":2,\"fts_avail\":127,\"fts_state\":7,\"bed_pos\":1,\"bed_wid\":60,"
          "\"occ_size\":2,\"mon_start\":\"2130\",\"mon_end\":\"0700\",\"bms\":1,\"vol\":3,"
          "\"audio\":1,\"lang\":\"English\",\"mode\":1,\"pause_tmr\":0,\"alert\":0,\"syserr\":0,"
          "\"syserr_title\":\"\",\"abc\":%d,\"room_number\":\"1001-a\"},\"id\":1}\n",
          abc);

      write_response(WR_FD, resp, (size_t)len);
      process_request(50);
      lv_test_wait(50);
      drain_pipe_now();
      return;
    }

    process_request(10);
    lv_test_wait(10);
  }

  TEST_FAIL_MESSAGE("Timed out waiting for save_display");
}

static void test_save_btn_on_display_settings_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(100, 110); // Click the Display settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(400, 270);
  lv_test_wait(250);
  lv_test_mouse_click_at(230, 420); // Click the Save button
  lv_test_wait(3000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-65-NKD_GO_SET_014_Save_Button_Display_Settings_Toast.png"),
      "Screenshot comparison failed for Save Button Display Settings Toast");
  lv_test_wait(3000);
  lv_test_mouse_click_at(230, 420);
  saved_display_settings();
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-66-NKD_GO_SET_014_Display_Settings_Saved_with_toast.png"),
      "Screenshot comparison failed for Display Settings Saved with toast");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-67-NKD_GO_SET_014_Display_Settings_Saved_no_toast.png"),
      "Screenshot comparison failed for Display Settings Saved no toast");
  lv_test_mouse_click_at(400, 270); // Click Adaptive toggle button
  lv_test_wait(200);
  lv_test_mouse_click_at(230, 420); // Click the Save button
  saved_display_settings();
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/"
          "GO-68-NKD_GO_SET_014_Display_Settings_Saved_adaptiveOFF_with_toast.png"),
      "Screenshot comparison failed for Display Settings Saved adaptive with toast");
  lv_test_wait(3000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/"
          "GO-69-NKD_GO_SET_014_Display_Settings_Saved_Adaptive0FF_toast_timeout.png"),
      "Screenshot comparison failed for Display Settings Saved toast timeout");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_back_btn_on_display_settings(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(100, 110); // Click the Display settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(30, 30); // Click the "Back" button
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_X_btn_on_display_settings(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(100, 110); // Click the Display settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(445, 30); // Click the "X" button
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-150-NKD_GO_ACT_027_Activating_Screen_with_value_2.png"),
      "Screenshot comparison failed for GO-150-Activating_Screen_with_value_2");
}

static void test_Alerts_page_on_settings_flow(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(60, 185); // Click the Alerts settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-04-NKD_GO_SET_002-Alerts_Settings.png"),
      "Screenshot comparison failed for Alerts Settings screen");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_toggles_on_alert_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(60, 185); // Click the Alerts settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(400, 230); // Click on Fall Alerts toggle button
  lv_test_wait(250);
  lv_test_mouse_click_at(400, 300); // Click on Reposition Alerts toggle button
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-70-NKD_GO_SET_015_Toggles_on_Alert_screen.png"),
      "Screenshot comparison failed for Fall Alerts Off");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_X_btn_on_alerts_settings_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(60, 185); // Click the Alerts settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(445, 30); // Click the "X" button
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-150-NKD_GO_ACT_027_Activating_Screen_with_value_2.png"),
      "Screenshot comparison failed for GO-150-Activating_Screen_with_value_2");
}

static void test_back_btn_on_alerts_settings_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(100, 195); // Click the Alerts settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void saved_alerts(void) {
  char line[512];

  for (uint32_t waited = 0; waited < 2000; waited += 10) {
    while (read_line_nb(RD_FD, line, sizeof line)) {
      if (!strstr(line, "\"method\":\"save_alerts\""))
        continue;

      char *f = strstr(line, "\"fts_state\":");
      if (!f)
        TEST_FAIL_MESSAGE("save_alerts missing fts_state");

      char *end = NULL;
      long fts_l = strtol(f + 12, &end, 10); // 12 == strlen("\"fts_state\":")
      if (end == f + 12)
        TEST_FAIL_MESSAGE("save_alerts fts_state not a number");

      int fts_state = (int)fts_l;

      char resp[768];
      int len = snprintf(
          resp, sizeof resp,
          "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
          "{\"screen\":2,\"fts_avail\":127,\"fts_state\":%d,\"bed_pos\":1,\"bed_wid\":60,"
          "\"occ_size\":2,\"mon_start\":\"2130\",\"mon_end\":\"0700\",\"bms\":1,\"vol\":3,"
          "\"audio\":1,\"lang\":\"English\",\"mode\":1,\"pause_tmr\":0,\"alert\":0,\"syserr\":0,"
          "\"syserr_title\":\"\",\"abc\":1,\"room_number\":\"1001-a\"},\"id\":1}\n",
          fts_state);

      write_response(WR_FD, resp, (size_t)len);
      process_request(50);
      lv_test_wait(50);
      drain_pipe_now();
      return;
    }

    process_request(10);
    lv_test_wait(10);
  }

  TEST_FAIL_MESSAGE("Timed out waiting for save_alerts");
}

static void test_save_btn_on_alerts_settings_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(100, 195); // Click the Alerts settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(400, 230); // Click on Fall Alerts toggle button
  lv_test_wait(250);
  lv_test_mouse_click_at(400, 300); // Click on Reposition Alerts toggle button
  lv_test_wait(250);
  lv_test_mouse_click_at(230, 420); // Click the Save button
  lv_test_wait(3000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-71-NKD_GO_SET_016_Save_Button_Alerts_Settings_Toast.png"),
      "Screenshot comparison failed for Save Button Alerts Settings Toast");
  lv_test_wait(3000);
  lv_test_mouse_click_at(230, 420);
  lv_test_wait(500);
  saved_alerts();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-72-NKD_GO_SET_016_Alerts_Settings_Saved_with_toast.png"),
      "Screenshot comparison failed for Alerts Settings Saved with toast");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-73-NKD_GO_SET_016_Alerts_Settings_Saved_no_toast.png"),
      "Screenshot comparison failed for Alerts Settings Saved no toast");
  lv_test_wait(100);
  lv_test_mouse_click_at(400, 230); // Click on Fall Alerts toggle button
  lv_test_wait(250);
  lv_test_mouse_click_at(400, 300); // Click on Reposition Alerts toggle button
  lv_test_wait(250);
  lv_test_mouse_click_at(230, 420); // Click the Save button
  lv_test_wait(500);
  saved_alerts();
  lv_test_wait(3000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-04-NKD_GO_SET_002-Alerts_Settings.png"),
      "Screenshot comparison failed for Alerts Settings screen");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_back_button_on_alerts_settings_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(100, 195); // Click the Alerts settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(30, 30); // Click the "Back" button
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_X_button_on_alerts_settings_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(100, 195); // Click the Alerts settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(445, 30); // Click the "X" button
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-150-NKD_GO_ACT_027_Activating_Screen_with_value_2.png"),
      "Screenshot comparison failed for GO-150-Activating_Screen_with_value_2");
}

static void test_exit_alert_schedule_on_settings_flow(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 180); // Click the Alert Schedule settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-05-NKD_GO_SET_002-Exit_Alert_Settings.png"),
      "Screenshot comparison failed for Exit Alert Settings screen");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_back_btn_on_exit_alert_schedule_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 180); // Click the Alert Schedule settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_X_btn_on_exit_alert_schedule_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 180); // Click the Alert Schedule settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(445, 30); // Click the "X" button
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-150-NKD_GO_ACT_027_Activating_Screen_with_value_2.png"),
      "Screenshot comparison failed for GO-150-Activating_Screen_with_value_2");
}

static void saved_mon_sch(void) {
  char line[512];

  for (uint32_t waited = 0; waited < 2000; waited += 10) {
    while (read_line_nb(RD_FD, line, sizeof line)) {
      if (!strstr(line, "\"method\":\"save_mon_sch\""))
        continue;

      char *ms = strstr(line, "\"mon_start\":\"");
      char *me = strstr(line, "\"mon_end\":\"");
      if (!ms || !me)
        TEST_FAIL_MESSAGE("save_mon_sch missing mon_start/mon_end");

      ms += 13; // "mon_start":"
      me += 11; // "mon_end":"

      char mon_start[8] = {0}, mon_end[8] = {0};
      for (int i = 0; ms[i] && ms[i] != '"' && i < 7; i++)
        mon_start[i] = ms[i];
      for (int i = 0; me[i] && me[i] != '"' && i < 7; i++)
        mon_end[i] = me[i];

      char resp[768];
      int len = snprintf(
          resp, sizeof resp,
          "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
          "{\"screen\":2,\"fts_avail\":127,\"fts_state\":7,\"bed_pos\":1,\"bed_wid\":60,"
          "\"occ_size\":2,\"mon_start\":\"%s\",\"mon_end\":\"%s\",\"bms\":1,\"vol\":3,"
          "\"audio\":1,\"lang\":\"English\",\"mode\":1,\"pause_tmr\":0,\"alert\":0,\"syserr\":0,"
          "\"syserr_title\":\"\",\"abc\":1,\"room_number\":\"1001-a\"},\"id\":1}\n",
          mon_start, mon_end);

      write_response(WR_FD, resp, (size_t)len);
      process_request(50);
      lv_test_wait(50);
      drain_pipe_now();
      return;
    }

    process_request(10);
    lv_test_wait(10);
  }

  TEST_FAIL_MESSAGE("Timed out waiting for save_mon_sch");
}

static void test_save_btn_on_exit_alert_schedule_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 180); // Click the Alert Schedule settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(240, 150); // Click the "Schedule Monitoring" button
  lv_test_wait(150);
  lv_test_mouse_click_at(240, 240); // Click the "Continue Monitoring" button
  lv_test_wait(150);
  lv_test_mouse_click_at(230, 420); // Click the Save button
  lv_test_wait(3000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-74-NKD_GO_SET_018_Save_Button_Exit_Alert_Schedule_Toast.png"),
      "Screenshot comparison failed for Save Button Exit Alert Schedule Toast");
  lv_test_wait(3000);
  lv_test_mouse_click_at(230, 420);
  saved_mon_sch();
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-75-NKD_GO_SET_018_Exit_Alert_Schedule_Saved_with_toast.png"),
      "Screenshot comparison failed for Exit Alert Schedule Saved with toast");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-76-NKD_GO_SET_018_Exit_Alert_Schedule_Saved_no_toast.png"),
      "Screenshot comparison failed for Exit Alert Schedule Saved no toast");
  lv_test_mouse_click_at(240, 150); // Click the "Continous Monitoring" button
  lv_test_wait(150);
  lv_test_mouse_click_at(230, 300); // Click the "Schedule Monitoring" button
  lv_test_wait(150);
  lv_test_mouse_click_at(230, 420); // Click the Save button
  lv_test_wait(500);
  saved_mon_sch();
  lv_test_wait(3000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/"
          "GO-77-NKD_GO_SET_018_Exit_Alert_Schedule_Saved_toast_timeout.png"),
      "Screenshot comparison failed for Exit Alert Schedule Saved toast timeout");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_back_button_on_exit_alert_schedule_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 180); // Click the Alert Schedule settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(30, 30); // Click the "Back" button
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_X_button_on_exit_alert_schedule_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 180); // Click the Alert Schedule settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(445, 30); // Click the "X" button
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-150-NKD_GO_ACT_027_Activating_Screen_with_value_2.png"),
      "Screenshot comparison failed for GO-150-Activating_Screen_with_value_2");
}

static void test_bed_width_screen_on_settings_flow(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 250); // Click the Bed Width settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-07-NKD_GO_SET_002-Bed_Width_Settings.png"),
      "Screenshot comparison failed for Bed Width Settings screen");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_back_btn_on_bed_width_settings_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 250); // Click the Bed Width settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_X_btn_on_bed_width_settings_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 250); // Click the Bed Width settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(445, 30); // Click the "X" button
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-150-NKD_GO_ACT_027_Activating_Screen_with_value_2.png"),
      "Screenshot comparison failed for GO-150-Activating_Screen_with_value_2");
}

static void test_bed_width_scroller_on_settings_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 250); // Click the Bed Width settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(50, 190); // Click and drag slider to low
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-78-NKD_GO_SET_019_BW_Slider_Standard.png"),
      "Screenshot comparison failed for GO-78-BW Slider standard");
  lv_test_mouse_click_at(240, 190); // Click and drag slider to Medium
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-79-NKD_GO_SET_019_BW_Slider_wide.png"),
      "Screenshot comparison failed for GO-79-BW Slider Wide");
  lv_test_mouse_click_at(430, 190); // Click and drag slider to High
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-80-NKD_GO_SET_019_BW_Slider_Custom.png"),
      "Screenshot comparison failed for GO-80-BW Slider Custom");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_custom_bed_width_screen_on_settings_flow(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 250); // Click the Bed Width settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(430, 190); // Click and drag slider to Custom
  lv_test_wait(250);
  lv_test_mouse_click_at(335, 270);
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-81-NKD_GO_SET_019_Custom_Bed_Width_Screen.png"),
      "Screenshot comparison failed for GO-81-Custom_Bed_Width_Screen");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_back_button_on_Custom_BW_screen_on_settings_flow(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 250); // Click the Bed Width settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(430, 190); // Click and drag slider to Custom
  lv_test_wait(250);
  lv_test_mouse_click_at(335, 270);
  lv_test_wait(150);
  lv_test_mouse_click_at(32, 34); // click on back
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-07-NKD_GO_SET_002-Bed_Width_Settings.png"),
      "Screenshot comparison failed for Bed Width Settings screen");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_x_button_on_Custom_BW_screen_on_settings_flow(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 250); // Click the Bed Width settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(430, 190); // Click and drag slider to Custom
  lv_test_wait(250);
  lv_test_mouse_click_at(335, 270);
  lv_test_wait(150);
  lv_test_mouse_click_at(445, 30); // Click the "X" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-150-NKD_GO_ACT_027_Activating_Screen_with_value_2.png"),
      "Screenshot comparison failed for GO-150-Activating_Screen_with_value_2");
}

static void test_predefined_width_options_on_Custom_BW_screen_on_settings_flow(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 250); // Click the Bed Width settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(430, 190); // Click and drag slider to Custom
  lv_test_wait(150);
  lv_test_mouse_click_at(335, 270); // click on change width
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-81-NKD_GO_SET_019_Custom_Bed_Width_Screen.png"),
      "Screenshot comparison failed for GO-81-Custom_Bed_Width_Screen");
  lv_test_mouse_click_at(32, 34); // click on back
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-07-NKD_GO_SET_002-Bed_Width_Settings.png"),
      "Screenshot comparison failed for Bed Width Settings screen");
  lv_test_mouse_click_at(335, 270);
  lv_test_mouse_click_at(120, 160);
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-82-NKD_GO_SET_019_BW_Change_width_FULL_FULL(XL).png"),
      "Screenshot comparison failed for GO-82-BW Change width FULL(XL)");
  lv_test_mouse_click_at(32, 34); // click on back
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-83-NKD_GO_SET_019_Save_BW_CustomFULL_width.png"),
      "Screenshot comparison failed for GO-83-Save_BW_CustomFULL_width");
  lv_test_mouse_click_at(335, 270);
  lv_test_mouse_click_at(120, 240);
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-84-NKD_GO_SET_019_BW_Change_width_CaliKing.png"),
      "Screenshot comparison failed for GO-84-BW Change width CaliKing");
  lv_test_mouse_click_at(32, 34); // click on back
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-85-NKD_GO_SET_019_Save_BW_CustomCaliKing_width.png"),
      "Screenshot comparison failed for GO-85-Save_BW_CustomCaliKing_width");
  lv_test_mouse_click_at(335, 270);
  lv_test_mouse_click_at(330, 240);
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-86-NKD_GO_SET_019_BW_Change_width_King.png"),
      "Screenshot comparison failed for GO-86-BW Change width King");
  lv_test_mouse_click_at(32, 34); // click on back
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-87-NKD_GO_SET_019_Save_BW_CustomKing_width.png"),
      "Screenshot comparison failed for GO-87-Save_BW_CustomKing_width");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_custom_value_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 250); // Click the Bed Width settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(430, 190); // Click and drag slider to Custom
  lv_test_wait(150);
  lv_test_mouse_click_at(335, 270);
  lv_test_wait(150);
  lv_test_mouse_click_at(230, 330); // Click the "Enter Custom Value" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-88-NKD_GO_SET_019_BW_Custom_Value_Screen.png"),
      "Screenshot comparison failed for GO-88-BW_Custom_Value_Screen");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_keypad_screen_on_Custom_Value_Screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 250); // Click the Bed Width settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(430, 190); // Click and drag slider to Custom
  lv_test_wait(150);
  lv_test_mouse_click_at(335, 270);
  lv_test_wait(150);
  lv_test_mouse_click_at(230, 330); // Click the "Enter Custom Value" button
  lv_test_wait(150);
  lv_test_mouse_click_at(80, 230); // Click '1'
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-89-NKD_GO_SET_019_Keypad_on_Custom_Value_Screen.png"),
      "Screenshot comparison failed for GO-89-Keypad_on_Custom_Value_Screen");
  lv_test_mouse_click_at(190, 230); // Click '2'
  lv_test_mouse_click_at(290, 230); // Click '3'
  lv_test_mouse_click_at(400, 230); // Click '4'
  lv_test_mouse_click_at(80, 310);  // Click '5'
  lv_test_mouse_click_at(190, 310); // Click '6'
  lv_test_mouse_click_at(290, 310); // Click '7'
  lv_test_mouse_click_at(390, 310); // Click '8'
  lv_test_mouse_click_at(190, 390); // Click '9'
  lv_test_mouse_click_at(290, 390); // Click '0'
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-90-NKD_GO_SET_019_Input_Keypad_all.png"),
      "Screenshot comparison failed for GO-90-Input_Keypad_all");
  lv_test_wait(150);
  lv_test_mouse_click_at(80, 390); // Click the "backspace" button
  lv_test_mouse_click_at(80, 390); // Click the "backspace" button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-91-NKD_GO_SET_019_Numpad_backspace.png"),
      "Screenshot comparison failed for GO-91-BW Custom Value Numpad backspace");
  lv_test_mouse_click_at(80, 390); // Click the "backspace" button
  lv_test_mouse_click_at(80, 390); // Click the "backspace" button
  lv_test_mouse_click_at(80, 390); // Click the "backspace" button
  lv_test_mouse_click_at(80, 390); // Click the "backspace" button
  lv_test_mouse_click_at(80, 390); // Click the "backspace" button
  lv_test_mouse_click_at(80, 390); // Click the "backspace" button
  lv_test_mouse_click_at(80, 390); // Click the "backspace" button
  lv_test_mouse_click_at(80, 390); // Click the "backspace" button
  lv_test_wait(150);
  lv_test_mouse_click_at(400, 230); // Click '4'
  lv_test_mouse_click_at(80, 310);  // Click '5'
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare(
                               "ref_imgsNKD/NKD-GO/NKD_SET/GO-92-NKD_GO_SET_019_Custom_Value.png"),
                           "Screenshot comparison failed for GO-92-BW Custom Value 45");
  lv_test_mouse_click_at(400, 400); // Click the "OK" button
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-93-NKD_GO_SET_019_Custom_Value_Saved.png"),
      "Screenshot comparison failed for GO-93-BW Custom Value saved 45");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_back_button_on_custom_input_screen_on_settings_flow(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 250); // Click the Bed Width settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(430, 190); // Click and drag slider to Custom
  lv_test_wait(150);
  lv_test_mouse_click_at(335, 270); // click on change width
  lv_test_wait(150);
  lv_test_mouse_click_at(230, 330); // Click the "Enter Custom Value" button
  lv_test_wait(150);
  lv_test_mouse_click_at(34, 30); // Click the "Back" button
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-94-NKD_GO_SET_019_Custom_Bed_Width_Screen_No_Selection.png"),
      "Screenshot comparison failed for GO-94-Custom_Bed_Width_Screen");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_x_button_on_custom_input_screen_settings_flow(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 250); // Click the Bed Width settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(430, 190); // Click and drag slider to Custom
  lv_test_wait(150);
  lv_test_mouse_click_at(335, 270); // click on change width
  lv_test_wait(150);
  lv_test_mouse_click_at(230, 330); // Click the "Enter Custom Value" button
  lv_test_wait(150);
  lv_test_mouse_click_at(445, 30); // Click the "X" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-150-NKD_GO_ACT_027_Activating_Screen_with_value_2.png"),
      "Screenshot comparison failed for GO-150-Activating_Screen_with_value_2");
}

static void test_toast_messages_on_custom_input_screen_on_settings_flow(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 250); // Click the Bed Width settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(430, 190); // Click and drag slider to Custom
  lv_test_wait(150);
  lv_test_mouse_click_at(335, 270); // click on change width
  lv_test_wait(150);
  lv_test_mouse_click_at(230, 330); // Click the "Enter Custom Value" button
  lv_test_wait(150);
  lv_test_mouse_click_at(290, 230); // Click '3'
  lv_test_mouse_click_at(400, 230); // Click '4'
  lv_test_mouse_click_at(400, 400); // Click the "tick" button & let toast message appear
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-95-NKD_GO_SET_019_BW_Custom_Value_minimum_Toastmsg.png"),
      "Screenshot comparison failed for GO-95-BW Custom Value MIN TOAST");
  lv_test_mouse_click_at(445, 37); // close toast msg
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-96-NKD_GO_SET_019_BW_Close_minimum_Toastmsg.png"),
      "Screenshot comparison failed for GO-96-BW Close MIN TOAST");
  lv_test_mouse_click_at(400, 400);
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-95-NKD_GO_SET_019_BW_Custom_Value_minimum_Toastmsg.png"),
      "Screenshot comparison failed for GO-95-BW Custom Value MIN TOAST");
  lv_test_mouse_click_at(445, 37);  // close toast msg
  lv_test_mouse_click_at(80, 390);  // Click the "backspace" button
  lv_test_mouse_click_at(80, 390);  // Click the "backspace" button
  lv_test_mouse_click_at(390, 310); // Click '8'
  lv_test_mouse_click_at(190, 390); // Click '9'
  lv_test_mouse_click_at(400, 400); // Click the "tick" button & let toast message appear
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-97-NKD_GO_SET_019_BW_maximum_Toastmsg.png"),
      "Screenshot comparison failed for GO-97-BW Close MAX TOAST");
  lv_test_mouse_click_at(445, 37); // close toast msg
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-98-NKD_GO_SET_018_BW_Close_maximum_Toastmsg.png"),
      "Screenshot comparison failed for GO-98-BW Close MAX TOAST");
  lv_test_mouse_click_at(400, 400); // Click the "tick" button & let toast message appear
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-97-NKD_GO_SET_019_BW_maximum_Toastmsg.png"),
      "Screenshot comparison failed for GO-97-BW Close MAX TOAST");
  lv_test_mouse_click_at(445, 37); // close toast msg
  lv_test_wait(100);
  lv_test_mouse_click_at(34, 30); // Click the "Back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void saved_bed_wid(void) {
  char line[512];

  for (uint32_t waited = 0; waited < 2000; waited += 10) {
    while (read_line_nb(RD_FD, line, sizeof line)) {
      if (!strstr(line, "\"method\":\"save_bed_wid\""))
        continue;

      char *p = strstr(line, "\"width\":");
      if (!p)
        TEST_FAIL_MESSAGE("save_bed_wid missing width");

      char *end = NULL;
      long w_l = strtol(p + 8, &end, 10); // 8 == strlen("\"width\":")
      if (end == p + 8)
        TEST_FAIL_MESSAGE("save_bed_wid width not a number");

      int width = (int)w_l;

      char resp[768];
      int len = snprintf(
          resp, sizeof resp,
          "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
          "{\"screen\":2,\"fts_avail\":127,\"fts_state\":7,\"bed_pos\":1,\"bed_wid\":%d,"
          "\"occ_size\":2,\"mon_start\":\"2130\",\"mon_end\":\"0700\",\"bms\":1,\"vol\":3,"
          "\"audio\":1,\"lang\":\"English\",\"mode\":1,\"pause_tmr\":0,\"alert\":0,\"syserr\":0,"
          "\"syserr_title\":\"\",\"abc\":1,\"room_number\":\"1001-a\"},\"id\":1}\n",
          width);

      write_response(WR_FD, resp, (size_t)len);
      process_request(50);
      lv_test_wait(50);
      drain_pipe_now();
      return;
    }

    process_request(10);
    lv_test_wait(10);
  }

  TEST_FAIL_MESSAGE("Timed out waiting for save_bed_wid");
}

static void test_save_btn_on_BW_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 250); // Click the Bed Width settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(430, 190); // Click and drag slider to Custom
  lv_test_wait(150);
  lv_test_mouse_click_at(335, 270); // click on change width
  lv_test_wait(150);
  lv_test_mouse_click_at(120, 160);
  lv_test_wait(100);
  lv_test_mouse_click_at(34, 30); // Click the "Back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(230, 420); // Click the Save button
  lv_test_wait(3000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-99-NKD_GO_SET_020_Save_Button_Bed_Width_Toast.png"),
      "Screenshot comparison failed for Save Button Bed Width Toast");
  lv_test_wait(3000);
  lv_test_mouse_click_at(230, 420);
  saved_bed_wid();
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-100-NKD_GO_SET_020_Bed_Width_Saved_with_toast.png"),
      "Screenshot comparison failed for Bed Width Saved with toast");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-101-NKD_GO_SET_020_Bed_Width_Saved_no_toast.png"),
      "Screenshot comparison failed for Bed Width Saved no toast");
  lv_test_mouse_click_at(335, 270); // click on change width
  lv_test_wait(150);
  lv_test_mouse_click_at(340, 170); // click on full width
  lv_test_wait(500);
  lv_test_mouse_click_at(30, 30); // click on back
  lv_test_wait(100);
  lv_test_mouse_click_at(230, 420); // Click the Save button
  lv_test_wait(200);
  saved_bed_wid();
  lv_test_wait(3000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-102-NKD_GO_SET_020_Bed_Width_Saved_no_toast_Timout.png"),
      "Screenshot comparison failed for Bed Width Saved no toast");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_bed_placement_screen_on_settings_flow(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(110, 330); // Click the Bed Placement settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-08-NKD_GO_SET_002-Bed_Placement_Settings.png"),
      "Screenshot comparison failed for Bed Placement Settings screen");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_back_button_on_bed_placement_settings_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(110, 330); // Click the Bed Placement settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_X_button_on_bed_placement_settings_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(110, 330); // Click the Bed Placement settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(445, 30); // Click the "X" button
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-150-NKD_GO_ACT_027_Activating_Screen_with_value_2.png"),
      "Screenshot comparison failed for GO-150-Activating_Screen_with_value_2");
}

static void test_bed_placement_scrollers_on_settings_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(110, 330); // Click the Bed Placement settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(240, 190); // Click and drag slider to centre
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-103-NKD_GO_SET_021_BP_Slider_Center.png"),
      "Screenshot comparison failed for GO-103-BP Slider Center");
  lv_test_mouse_click_at(430, 190); // Click and drag slider to right wall
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-104-NKD_GO_SET_021_BP_Slider_Right_Wall.png"),
      "Screenshot comparison failed for GO-104-BP Slider Right Wall");
  lv_test_mouse_click_at(50, 190); // Click and drag slider to left wall
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-08-NKD_GO_SET_002-Bed_Placement_Settings.png"),
      "Screenshot comparison failed for Bed Placement Settings screen");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void saved_bed_pos(void) {
  char line[512];

  for (uint32_t waited = 0; waited < 2000; waited += 10) {
    while (read_line_nb(RD_FD, line, sizeof line)) {
      if (!strstr(line, "\"method\":\"save_bed_pos\""))
        continue;

      char *p = strstr(line, "\"pos\":");
      if (!p)
        TEST_FAIL_MESSAGE("save_bed_pos missing pos");

      char *end = NULL;
      long pos_l = strtol(p + 6, &end, 10); // 6 == strlen("\"pos\":")
      if (end == p + 6)
        TEST_FAIL_MESSAGE("save_bed_pos pos not a number");

      int pos = (int)pos_l;

      char resp[768];
      int len = snprintf(
          resp, sizeof resp,
          "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
          "{\"screen\":2,\"fts_avail\":127,\"fts_state\":7,\"bed_pos\":%d,\"bed_wid\":54,"
          "\"occ_size\":2,\"mon_start\":\"2130\",\"mon_end\":\"0700\",\"bms\":1,\"vol\":3,"
          "\"audio\":1,\"lang\":\"English\",\"mode\":1,\"pause_tmr\":0,\"alert\":0,\"syserr\":0,"
          "\"syserr_title\":\"\",\"abc\":1,\"room_number\":\"1001-a\"},\"id\":1}\n",
          pos);

      write_response(WR_FD, resp, (size_t)len);
      process_request(50);
      lv_test_wait(50);
      drain_pipe_now();
      return;
    }

    process_request(10);
    lv_test_wait(10);
  }

  TEST_FAIL_MESSAGE("Timed out waiting for save_bed_pos");
}

static void test_save_btn_on_BP_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(110, 330); // Click the Bed Placement settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(240, 190); // Click and drag slider to centre
  lv_test_wait(150);
  lv_test_mouse_click_at(230, 420); // Click the Save button
  lv_test_wait(3000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-105-NKD_GO_SET_022_Save_Button_Bed_Placement_Toast.png"),
      "Screenshot comparison failed for Save Button Bed Placement Toast");
  lv_test_wait(3000);
  lv_test_mouse_click_at(230, 420);
  saved_bed_pos();
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-106-NKD_GO_SET_022_Bed_Placement_Saved_with_toast.png"),
      "Screenshot comparison failed for Bed Placement Saved with toast");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-107-NKD_GO_SET_022_Bed_Placement_Saved_no_toast.png"),
      "Screenshot comparison failed for Bed Placement Saved no toast");
  lv_test_mouse_click_at(50, 190); // Click and drag slider to left wall
  lv_test_wait(150);
  lv_test_mouse_click_at(230, 420); // Click the Save button
  lv_test_wait(200);
  saved_bed_pos();
  lv_test_wait(3000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-108-NKD_GO_SET_022_Bed_Placement_Saved_no_toast_Timeout.png"),
      "Screenshot comparison failed for Bed Placement Saved no toast Timeout");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_Occupant_Size_screen_on_settings_flow(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 330); // Click the Occupant Size settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-09-NKD_GO_SET_002-Occupant_Size_Settings.png"),
      "Screenshot comparison failed for Occupant Size Settings screen");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_back_button_on_occupant_size_settings_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 330); // Click the Occupant Size settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_X_button_on_occupant_size_settings_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 330); // Click the Occupant Size settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(445, 30); // Click the "X" button
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ACT/GO-150-NKD_GO_ACT_027_Activating_Screen_with_value_2.png"),
      "Screenshot comparison failed for GO-150-Activating_Screen_with_value_2");
}

static void test_scrollers_on_occupant_size_settings_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 330); // Click the Occupant Size settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(50, 190); // Click and drag slider to Small
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-109-NKD_GO_SET_023_OS_Slider_small.png"),
      "Screenshot comparison failed for GO-109-OS Slider Small");
  lv_test_mouse_click_at(430, 190); // Click and drag slider to Large
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-110-NKD_GO_SET_023_OS_Slider_large.png"),
      "Screenshot comparison failed for GO-110-OS Slider Large");
  lv_test_mouse_click_at(240, 190); // Click and drag slider to standard
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-09-NKD_GO_SET_002-Occupant_Size_Settings.png"),
      "Screenshot comparison failed for Occupant Size Settings screen");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void saved_occ_size(void) {
  char line[512];

  for (uint32_t waited = 0; waited < 2000; waited += 10) {
    while (read_line_nb(RD_FD, line, sizeof line)) {
      if (!strstr(line, "\"method\":\"save_occ_size\""))
        continue;

      char *p = strstr(line, "\"size\":");
      if (!p)
        TEST_FAIL_MESSAGE("save_occ_size missing size");

      char *end = NULL;
      long size_l = strtol(p + 7, &end, 10); // 7 == strlen("\"size\":")
      if (end == p + 7)
        TEST_FAIL_MESSAGE("save_occ_size size not a number");

      int size = (int)size_l;

      char resp[768];
      int len = snprintf(
          resp, sizeof resp,
          "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
          "{\"screen\":2,\"fts_avail\":121,\"fts_state\":7,\"bed_pos\":1,\"bed_wid\":54,"
          "\"occ_size\":%d,\"mon_start\":\"2130\",\"mon_end\":\"0700\",\"bms\":1,\"vol\":3,"
          "\"audio\":1,\"lang\":\"English\",\"mode\":1,\"pause_tmr\":0,\"alert\":0,\"syserr\":0,"
          "\"syserr_title\":\"\",\"abc\":1,\"room_number\":\"1001-a\"},\"id\":1}\n",
          size);

      write_response(WR_FD, resp, (size_t)len);
      process_request(50);
      lv_test_wait(50);
      drain_pipe_now();
      return;
    }

    process_request(10);
    lv_test_wait(10);
  }

  TEST_FAIL_MESSAGE("Timed out waiting for save_occ_size");
}

static void test_save_btn_on_OS_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 330); // Click the Occupant Size settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(430, 190); // Click and drag slider to Large
  lv_test_wait(150);
  lv_test_mouse_click_at(230, 420); // Click the Save button
  lv_test_wait(3000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-111-NKD_GO_SET_024_Save_Button_Occupant_Size_Toast.png"),
      "Screenshot comparison failed for Save Button Occupant Size Toast");
  lv_test_wait(3000);
  lv_test_mouse_click_at(230, 420);
  saved_occ_size();
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-112-NKD_GO_SET_024_Occupant_Size_Saved_with_toast.png"),
      "Screenshot comparison failed for Occupant Size Saved with toast");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-113-NKD_GO_SET_024_Occupant_Size_Saved_no_toast.png"),
      "Screenshot comparison failed for Occupant Size Saved no toast");
  lv_test_wait(200);
  lv_test_mouse_click_at(50, 190); // Click and drag slider to Small
  lv_test_wait(150);
  lv_test_mouse_click_at(230, 420); // Click the Save button
  lv_test_wait(200);
  saved_occ_size();
  lv_test_wait(3000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-114-NKD_GO_SET_024_Occupant_Size_Saved_no_toast_Timeout.png"),
      "Screenshot comparison failed for Occupant Size Saved no toast Timeout");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_save_pop_up_message_on_settings_flow(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 330); // Click the Occupant Size settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(430, 190); // Click and drag slider to Large
  lv_test_wait(150);
  lv_test_mouse_click_at(30, 30); // Click the "Back" button
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-115-NKD_GO_SET_025_Settings_Save_Pop_Up.png"),
      "Screenshot comparison failed for Settings Save Pop Up");
  lv_test_wait(100);
  lv_test_mouse_click_at(155, 280);
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_Dont_Save_button_on_settings_flow(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 330); // Click the Occupant Size settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(430, 190); // Click and drag slider to Large
  lv_test_wait(150);
  lv_test_mouse_click_at(30, 30); // Click the "Back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(155, 280); // Click the "Don't Save" button
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_save_btn_functionality_on_settings_flow(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(110, 330); // Click the Bed Placement settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(240, 190); // Click and drag slider to centre
  lv_test_wait(150);
  lv_test_mouse_click_at(230, 420); // Click the Save button
  lv_test_wait(3000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-105-NKD_GO_SET_022_Save_Button_Bed_Placement_Toast.png"),
      "Screenshot comparison failed for Save Button Bed Placement Toast");
  lv_test_wait(3000);
  lv_test_mouse_click_at(230, 420);
  saved_bed_pos();
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-106-NKD_GO_SET_022_Bed_Placement_Saved_with_toast.png"),
      "Screenshot comparison failed for Bed Placement Saved with toast");
  lv_test_wait(3000);
  lv_test_mouse_click_at(445, 30);
}

static void test_presence_of_pop_up_toast_on_settings_flow(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(110, 330); // Click the Bed Placement settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(430, 190); // Click and drag slider to Large
  lv_test_wait(150);
  lv_test_mouse_click_at(230, 420); // Click the Save button
  lv_test_wait(200);
  saved_bed_pos();
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-116-NKD_GO_SET_028_Pop_Up_Toast_on_Settings_Flow.png"),
      "Screenshot comparison failed for Pop Up Toast on Settings Flow");
  lv_test_wait(3000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-117-NKD_GO_SET_028_Pop_Up_Toast_on_Settings_Flow_Timeout.png"),
      "Screenshot comparison failed for Bed Placement Saved no toast Timeout");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_X_btn_on_toast_message_on_settings_flow(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(110, 330); // Click the Bed Placement settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(240, 190); // Click and drag slider to centre
  lv_test_wait(150);
  lv_test_mouse_click_at(230, 420); // Click the Save button
  lv_test_wait(200);
  saved_bed_pos();
  lv_test_wait(500);
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-107-NKD_GO_SET_022_Bed_Placement_Saved_no_toast.png"),
      "Screenshot comparison failed for Bed Placement Saved no toast");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void send_set_state_with_alert_syserr(int fts_state, int alert, int syserr) {
  char resp[512];
  int len =
      snprintf(resp, sizeof(resp),
               "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
               "{\"screen\":2,\"fts_avail\":121,\"fts_state\":%d,\"bed_pos\":1,"
               "\"bed_wid\":60,\"occ_size\":2,\"mon_start\":\"2130\",\"mon_end\":\"0700\","
               "\"bms\":2,\"vol\":2,\"audio\":1,\"lang\":\"English\",\"mode\":1,\"pause_tmr\":0,"
               "\"alert\":%d,\"syserr\":%d,\"syserr_title\":\"\",\"abc\":0,"
               "\"room_number\":\"1001-a\"},\"id\":1}\n",
               fts_state, alert, syserr);

  write_response(WR_FD, resp, (size_t)len);
  process_request(50);
}

static void test_Alerts_and_error_toast_msgs_on_settings_flow(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 1, 0);
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-118-NKD_GO_SET_030_Bed_Exit_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Bed exit toast on settings flow");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 1, 0);
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 0, 0);
  lv_test_wait(1000);
  send_set_state_with_alert_syserr(7, 1, 0);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-118-NKD_GO_SET_030_Bed_Exit_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Bed exit toast on settings flow");
  lv_test_wait(200);
  lv_test_mouse_click_at(200, 40); // click on toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ALE/GO-02-NKD_GO_ALE_001-Bed_Exit_Alert_High.png"),
      "Screenshot comparison failed for Bed exit screen");
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 2, 0);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-119-NKD_GO_SET_030_Chair_Exit_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Chair exit toast on settings flow");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 2, 0);
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 0, 0);
  lv_test_wait(1000);
  send_set_state_with_alert_syserr(7, 2, 0);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-119-NKD_GO_SET_030_Chair_Exit_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Chair exit toast on settings flow");
  lv_test_wait(200);
  lv_test_mouse_click_at(200, 40); // click on toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ALE/GO-04-NKD_GO_ALE_002-Chair_Exit_Alert.png"),
      "Screenshot comparison failed for Chair Exit Alert");
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  send_set_state_with_alert_syserr(6, 101, 0);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-120-NKD_GO_SET_030_Fall_Alert_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Fall Alert toast on settings flow");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(6, 101, 0);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(6, 0, 0);
  lv_test_wait(1000);
  send_set_state_with_alert_syserr(6, 101, 0);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-120-NKD_GO_SET_030_Fall_Alert_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Fall Alert toast on settings flow");
  lv_test_mouse_click_at(200, 40); // click on toast
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ALE/GO-05-NKD_GO_ALE_003-Fall_Alert.png"),
      "Screenshot comparison failed for Fall Alert");
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  send_set_state_with_alert_syserr(6, 0, 0);
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 101, 0);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-120-NKD_GO_SET_030_Fall_Alert_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Fall Alert toast on settings flow");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 101, 0);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 0, 0);
  lv_test_wait(1000);
  send_set_state_with_alert_syserr(7, 101, 0);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-120-NKD_GO_SET_030_Fall_Alert_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Fall Alert toast on settings flow");
  lv_test_mouse_click_at(200, 40); // click on toast
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ALE/GO-06-NKD_GO_ALE_004-Fall_Alert_with_bed_chair.png"),
      "Screenshot comparison failed for Fall Alert with bed chair");
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 102, 0);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-121-NKD_GO_SET_030_Reposition_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Reposition toast on settings flow");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 102, 0);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 0, 0);
  lv_test_wait(1000);
  send_set_state_with_alert_syserr(7, 102, 0);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-121-NKD_GO_SET_030_Reposition_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Reposition toast on settings flow");
  lv_test_mouse_click_at(200, 40); // click on toast
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ALE/GO-08-NKD_GO_ALE_006-Reposition_Alert_with_bed_chair.png"),
      "Screenshot comparison failed for Reposition Alert with bed chair");
  send_set_state_with_alert_syserr(7, 0, 0);
  lv_test_wait(100);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  send_set_state_with_alert_syserr(6, 102, 0);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-121-NKD_GO_SET_030_Reposition_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Reposition toast on settings flow");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(6, 102, 0);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(6, 0, 0);
  lv_test_wait(1000);
  send_set_state_with_alert_syserr(6, 102, 0);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-119-NKD_GO_SET_030_Reposition_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Reposition toast on settings flow");
  lv_test_mouse_click_at(200, 40); // click on toast
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ALE/GO-07-NKD_GO_ALE_005-Reposition_Alert.png"),
      "Screenshot comparison failed for Reposition Alert toast");
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 0, 11);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-122-NKD_GO_SET_030_Incorrect_Mode_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Incorrect_Mode on settings flow");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 0, 11);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 0, 0);
  lv_test_wait(1000);
  send_set_state_with_alert_syserr(7, 0, 11);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-122-NKD_GO_SET_030_Incorrect_Mode_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Incorrect_Mode on settings flow");
  lv_test_mouse_click_at(200, 40); // click on toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ALE/GO-12-NKD_GO_ALE_010-Incorrect_Mode_Error_Screen.png"),
      "Screenshot comparison failed for Incorrect Mode Error toast");
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 0, 61);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare(
                               "ref_imgsNKD/NKD-GO/NKD_SET/"
                               "GO-123-NKD_GO_SET_030_Unassigned_Error_Toast_on_settings_flow.png"),
                           "Screenshot comparison failed for Unassigned Error on settings flow");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 0, 61);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 0, 0);
  lv_test_wait(1000);
  send_set_state_with_alert_syserr(7, 0, 61);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare(
                               "ref_imgsNKD/NKD-GO/NKD_SET/"
                               "GO-123-NKD_GO_SET_030_Unassigned_Error_Toast_on_settings_flow.png"),
                           "Screenshot comparison failed for Unassigned Error on settings flow");
  lv_test_mouse_click_at(200, 40); // click on toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ALE/GO-13-NKD_GO_ALE_011-Unassigned_Error_Screen.png"),
      "Screenshot comparison failed for Unassigned Error Screen toast");
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 0, 12);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-124-NKD_GO_SET_030_Sunlight_Error_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Sunlight Error on settings flow");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 0, 12);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 0, 0);
  lv_test_wait(1000);
  send_set_state_with_alert_syserr(7, 0, 12);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-124-NKD_GO_SET_030_Sunlight_Error_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Sunlight Error on settings flow");
  lv_test_mouse_click_at(200, 40); // click on toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ALE/"
                                 "GO-14-NKD_GO_ALE_012-Sunlight_Interference_Error_Screen.png"),
      "Screenshot comparison failed for Sunlight Error Screen toast");
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 0, 13);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-125-NKD_SET_030_Obstructed_Error_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Obstructed Error on settings flow");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 0, 13);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 0, 0);
  lv_test_wait(1000);
  send_set_state_with_alert_syserr(7, 0, 13);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare(
                               "ref_imgsNKD/NKD-GO/NKD_SET/"
                               "GO-125-NKD_GO_SET_030_Obstructed_Error_Toast_on_settings_flow.png"),
                           "Screenshot comparison failed for Obstructed Error on settings flow");
  lv_test_mouse_click_at(200, 40); // click on toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ALE/GO-15-NKD_GO_ALE_013-Obstructed_Sensor_Error_Screen.png"),
      "Screenshot comparison failed for Obstructed_Sensor Error Screen toast");
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 0, 42);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/"
          "GO-126-NKD_GO_SET_030_System_Disconnected_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for System Disconnected Error on settings flow");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 0, 42);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 0, 0);
  lv_test_wait(1000);
  send_set_state_with_alert_syserr(7, 0, 42);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/"
          "GO-126-NKD_GO_SET_030_System_Disconnected_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for System Disconnected Error on settings flow");
  lv_test_mouse_click_at(200, 40); // click on toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ALE/GO-16-NKD_GO_ALE_014-System_Disconnected_Error_Screen.png"),
      "Screenshot comparison failed for System Disconnected Error Screen");
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 0, 41);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-127-NKD_GO_SET_030_Not_Monitoring_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Not Monitoring Error on settings flow");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 0, 41);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 0, 0);
  lv_test_wait(1000);
  send_set_state_with_alert_syserr(7, 0, 41);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-127-NKD_GO_SET_030_Not_Monitoring_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Not Monitoring Error on settings flow");
  lv_test_mouse_click_at(200, 40); // click on toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ALE/GO-17-NKD_GO_ALE_015-Not_Monitoring_Screen.png"),
      "Screenshot comparison failed for Not Monitoring Screen");
}

static void test_alerts_and_error_toasts_on_settings_pages(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(110, 110);
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 102, 0);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/"
          "GO-128-NKD_GO_SET_030_Reposition_Toast_on_display_settings_page.png"),
      "Screenshot comparison failed for reposition alert on display setting page");
  wait_ms(120000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ALE/GO-08-NKD_GO_ALE_006-Reposition_Alert_with_bed_chair.png"),
      "Screenshot comparison failed for Reposition Alert with bed chair");
  lv_test_wait(100);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(110, 110);
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 101, 0);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/"
          "GO-129-NKD_GO_SET_030_Fall_Alert_Toast_on_display_settings_page.png"),
      "Screenshot comparison failed for fall alert on display setting page");
  wait_ms(120000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ALE/GO-06-NKD_GO_ALE_004-Fall_Alert_with_bed_chair.png"),
      "Screenshot comparison failed for Fall Alert with bed chair");
  lv_test_wait(100);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(110, 110);
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 0, 13);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/"
          "GO-130-NKD_GO_SET_030_Obstructed_Error_Toast_on_display_settings_page.png"),
      "Screenshot comparison failed for Obstructed Error on display setting page");
  lv_test_wait(100);
  lv_test_mouse_click_at(200, 40); // click on toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ALE/GO-15-NKD_GO_ALE_013-Obstructed_Sensor_Error_Screen.png"),
      "Screenshot comparison failed for Obstructed_Sensor Error Screen toast");
  lv_test_wait(100);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(350, 120);
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 0, 11);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/"
          "GO-131-NKD_GO_SET_030_Incorrect_Error_Toast_on_in_room_alert_settings_page.png"),
      "Screenshot comparison failed for Incorrect Error on in room alert setting page");
  wait_ms(120000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ALE/GO-12-NKD_GO_ALE_010-Incorrect_Mode_Error_Screen.png"),
      "Screenshot comparison failed for Incorrect Mode Error toast");
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(350, 120);
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 0, 41);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/"
          "GO-132-NKD_GO_SET_030_Not_Monitoring_Error_Toast_on_in_room_alert_settings_page.png"),
      "Screenshot comparison failed for Not Monitoring Error on in room alert setting page");
  lv_test_mouse_click_at(200, 40); // click on toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ALE/GO-17-NKD_GO_ALE_015-Not_Monitoring_Screen.png"),
      "Screenshot comparison failed for Not Monitoring Screen");
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(120, 180);
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 0, 42);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/"
          "GO-133-NKD_GO_SET_030_System_Disconnected_Error_Toast_on_alert_settings_page.png"),
      "Screenshot comparison failed for Not Monitoring Error on alert setting page");
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 0, 61);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/"
          "GO-134-NKD_GO_SET_030_Unassigned_Error_Toast_on_alert_settings_page.png"),
      "Screenshot comparison failed for Unassigned Error on alert setting page");
  wait_ms(120000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ALE/GO-13-NKD_GO_ALE_011-Unassigned_Error_Screen.png"),
      "Screenshot comparison failed for Unassigned Error Screen toast");
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(340, 190);
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 0, 12);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/"
          "GO-135-NKD_GO_SET_030_Sunlight_Error_Toast_on_exit_alert_settings_page.png"),
      "Screenshot comparison failed for Sunlight Error on exit alert setting page");
  lv_test_mouse_click_at(200, 40); // click on toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ALE/"
                                 "GO-14-NKD_GO_ALE_012-Sunlight_Interference_Error_Screen.png"),
      "Screenshot comparison failed for Sunlight Error Screen toast");
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(340, 190);
  lv_test_wait(100);
  send_set_state_with_alert_syserr(6, 102, 0);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/"
          "GO-136-NKD_GO_SET_030_Reposition_alert_Toast_on_exit_alert_settings_page.png"),
      "Screenshot comparison failed for Reposition alert on exit alert setting page");
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 1, 0);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/"
          "GO-137-NKD_GO_SET_030_Bed_Exit_alert_Toast_on_exit_alert_settings_page.png"),
      "Screenshot comparison failed for Bed Exit alert on exit alert setting page");
  wait_ms(120000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ALE/GO-02-NKD_ALE_001-Bed_Exit_Alert_High.png"),
      "Screenshot comparison failed for Bed exit screen");
  lv_test_wait(100);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(100, 270);
  send_set_state_with_alert_syserr(7, 2, 0);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/"
          "GO-138-NKD_GO_SET_030_Chair_Exit_alert_Toast_on_BMS_settings_page.png"),
      "Screenshot comparison failed for Chair Exit alert on BMS setting page");
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 0, 13);
  lv_test_wait(200);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/"
          "GO-139-NKD_GO_SET_030_Obstruted_Error_Toast_on_BMS_settings_page.png"),
      "Screenshot comparison failed for obstructed error on BMS setting page");
  lv_test_mouse_click_at(200, 40); // click on toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ALE/GO-15-NKD_GO_ALE_013-Obstructed_Sensor_Error_Screen.png"),
      "Screenshot comparison failed for Obstructed_Sensor Error Screen toast");
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(300, 260);
  send_set_state_with_alert_syserr(7, 0, 12);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/"
          "GO-140-NKD_GO_SET_030_Sunlight_Error_Toast_on_Bed_Width_settings_page.png"),
      "Screenshot comparison failed for Sunlight Error on BW setting page");
  send_set_state_with_alert_syserr(7, 0, 41);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/"
          "GO-141-NKD_GO_SET_030_Not_Monitroing_Error_Toast_on_Bed_Width_settings_page.png"),
      "Screenshot comparison failed for Not Monitoring Error on BW setting page");
  lv_test_mouse_click_at(200, 40); // click on toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ALE/GO-17-NKD_GO_ALE_015-Not_Monitoring_Screen.png"),
      "Screenshot comparison failed for Not Monitoring Screen");
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(120, 340);
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 101, 0);
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/"
          "GO-142-NKD_G0_SET_030_Fall_Alert_Toast_on_Bed_Placement_settings_page.png"),
      "Screenshot comparison failed for Fall Alert on BP setting page");
  send_set_state_with_alert_syserr(7, 102, 0);
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/"
          "GO-143-NKD_GO_SET_030_Reposition_Alert_Toast_on_Bed_Placement_settings_page.png"),
      "Screenshot comparison failed for Reposition Alert on BP setting page");
  lv_test_mouse_click_at(200, 40); // click on toast
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ALE/GO-08-NKD_GO_ALE_006-Reposition_Alert_with_bed_chair.png"),
      "Screenshot comparison failed for Reposition Alert with bed chair");
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 330);
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 2, 0);
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/"
          "GO-144-NKD_GO_SET_030_Chair_Exit_Alert_Toast_on_Occupant_Size_settings_page.png"),
      "Screenshot comparison failed for Chair Exit Alert on Occupant Size setting page");
  send_set_state_with_alert_syserr(7, 0, 12);
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/"
          "GO-145-NKD_GO_SET_030_Sunlight_Error_Toast_on_Occupant_Size_settings_page.png"),
      "Screenshot comparison failed for Sunlight Error on Occupant Size setting page");
  lv_test_mouse_click_at(200, 40); // click on toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ALE/"
                                 "GO-14-NKD_GO_ALE_012-Sunlight_Interference_Error_Screen.png"),
      "Screenshot comparison failed for Sunlight Error Screen toast");
}

static void deactivate_sensor_with_screen(int screen) {
  char line[512];

  for (uint32_t waited = 0; waited < 2000; waited += 10) {
    while (read_line_nb(RD_FD, line, sizeof line)) {
      if (!strstr(line, "\"method\":\"deactivate_sensor\""))
        continue;

      char resp[256];
      int len = snprintf(resp, sizeof resp,
                         "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
                         "{\"screen\":%d,\"fts_avail\":127,\"fts_state\":7,\"mode\":2,"
                         "\"room_number\":\"1001-a\"},\"id\":1}\n",
                         screen);

      write_response(WR_FD, resp, (size_t)len);
      process_request(50);
      lv_test_wait(50);
      drain_pipe_now();
      return;
    }

    process_request(10);
    lv_test_wait(10);
  }

  TEST_FAIL_MESSAGE("Timed out waiting for deactivate_sensor");
}

static void test_deactivate_button_on_settings_flow(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(350, 420); // Click the Deactivate button
  lv_test_wait(200);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-146-NKD_GO_SET_031_Deactivate_pop_up_settings_flow.png"),
      "Screenshot comparison failed for Deactivate pop up on settings flow");
  lv_test_mouse_click_at(150, 320);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD_SET/GO-01-NKD_GO_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  lv_test_mouse_click_at(350, 420); // Click the Deactivate button
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 320);
  lv_test_wait(3000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-147-NKD_GO_SET_031_Deactivate_pop_up_toast_timeout.png"),
      "Screenshot comparison failed for Deactivate pop up toast_timeout");
  lv_test_wait(3000);
  lv_test_mouse_click_at(350, 420);
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 320);
  lv_test_wait(100);
  deactivate_sensor_with_screen(5);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD_SET/"
                                 "GO-148-NKD_GO_SET_031_Deactivated_Screen_5.png"),
      "Screenshot comparison failed for Deactivated screen 5");
  send_set_state_with_alert_syserr(7, 0, 0);
  lv_test_wait(100);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(350, 420); // Click the Deactivate button
  lv_test_wait(200);
  lv_test_mouse_click_at(330, 320);
  lv_test_wait(100);
  deactivate_sensor_with_screen(6);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ACT/GO-00-NKD_COM_001-Welcome Screen.png"),
      "Screenshot comparison failed for Welcome Screen");
}

static void resume_fall_mon_clear_alert_response(void) {

  char resp[256];

  int len = snprintf(resp, sizeof resp,
                     "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
                     "{\"screen\":2,\"fts_avail\":127,\"fts_state\":1,\"alert\":0,\"mode\":2,"
                     "\"room_number\":\"1001-a\"},\"id\":1}\n");

  write_response(WR_FD, resp, (size_t)len);
  process_request(50);
}

static void test_resume_fall_monitoring_button(void) {
  setv_variant_cmd();
  lv_test_wait(150);
  char resp[512];
  int len =
      snprintf(resp, sizeof(resp),
               "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
               "{\"screen\":2,\"fts_avail\":121,\"fts_state\":6,\"bed_pos\":1,"
               "\"bed_wid\":60,\"occ_size\":2,\"mon_start\":\"2130\",\"mon_end\":\"0700\","
               "\"bms\":1,\"vol\":2,\"audio\":1,\"lang\":\"English\",\"mode\":1,\"pause_tmr\":0,"
               "\"alert\":101,\"syserr\":0,\"syserr_title\":\"\",\"abc\":0,"
               "\"room_number\":\"1001-a\"},\"id\":1}\n");

  write_response(WR_FD, resp, (size_t)len);
  process_request(50);
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-GO/NKD-ALE/GO-05-NKD_GO_ALE_003-Fall_Alert.png"),
      "Screenshot comparison failed for Fall Alert");
  lv_test_mouse_click_at(220, 420);
  lv_test_wait(100);
  resume_fall_mon_clear_alert_response();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-MON/GO-05-NKD_GO_MON_003_Chair_Mode_Screen.png"),
      "Screenshot comparison failed for GO-05-NKD_MON_003_Chair_Mode_Screen.png");
}

static void test_clear_alert_btn_functionality(void) {
  char resp[512];
  int len =
      snprintf(resp, sizeof(resp),
               "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
               "{\"screen\":2,\"fts_avail\":121,\"fts_state\":6,\"bed_pos\":1,"
               "\"bed_wid\":60,\"occ_size\":2,\"mon_start\":\"2130\",\"mon_end\":\"0700\","
               "\"bms\":1,\"vol\":2,\"audio\":1,\"lang\":\"English\",\"mode\":1,\"pause_tmr\":0,"
               "\"alert\":102,\"syserr\":0,\"syserr_title\":\"\",\"abc\":0,"
               "\"room_number\":\"1001-a\"},\"id\":1}\n");

  write_response(WR_FD, resp, (size_t)len);
  process_request(50);
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-ALE/GO-07-NKD_GO_ALE_005-Reposition_Alert.png"),
      "Screenshot comparison failed for Reposition Alert");
  lv_test_mouse_click_at(220, 420);
  lv_test_wait(100);
  resume_fall_mon_clear_alert_response();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-GO/NKD-MON/GO-05-NKD_GO_MON_003_Chair_Mode_Screen.png"),
      "Screenshot comparison failed for GO-05-NKD_MON_003_Chair_Mode_Screen.png");
}

TEST_GROUP(GO_Common_screen);

TEST_GROUP(GO_ActivationFlow);

TEST_GROUP(GO_MonitoringScreens);

TEST_GROUP(GO_Alerts_and_error_screens);

TEST_GROUP(GO_ACT_Settings_Flow);

static int is_grp_filtered_run(void) {
  return (UnityFixture.GroupFilter && UnityFixture.GroupFilter[0]) ||
         (UnityFixture.NameFilter && UnityFixture.NameFilter[0]);
}

static void act_flow_common(void) {
  if (G_OUT >= 0) {
  }

  char v[256];
  int vlen = snprintf(v, sizeof v,
                      "{\"jsonrpc\":\"2.0\",\"method\":\"set_variant\","
                      "\"params\":{\"variant\":2,\"fts_avail\":127},\"id\":1}\n");

  write_response(WR_FD, v, (size_t)vlen);
  process_request(50);
  lv_test_wait(50);
  drain_pipe_now();

  if (G_OUT >= 0)
    dprintf(G_OUT, "[alerts-bootstrap] setup done\n");
}

static void monitoring_common(void) {
  if (G_OUT >= 0) {
  }

  {
    char v[256];
    int vlen = snprintf(v, sizeof v,
                        "{\"jsonrpc\":\"2.0\",\"method\":\"set_variant\","
                        "\"params\":{\"variant\":2,\"fts_avail\":127},\"id\":1}\n");

    write_response(WR_FD, v, (size_t)vlen);
    process_request(50);
    lv_test_wait(50);
    drain_pipe_now();
  }
  {
    char resp[512];
    int len = snprintf(resp, sizeof resp,
                       "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
                       "{\"screen\":2,\"fts_avail\":127,\"fts_state\":1,"
                       "\"bed_pos\":1,\"bed_wid\":60,\"occ_size\":1,"
                       "\"mon_start\":\"2130\",\"mon_end\":\"0700\","
                       "\"bms\":1,\"vol\":2,\"audio\":1,\"lang\":\"English\","
                       "\"mode\":1,\"room_number\":\"1001-a\"},\"id\":2}\n");

    write_response(WR_FD, resp, (size_t)len);
    process_request(50);
    lv_test_wait(200);
    drain_pipe_now();
  }

  if (G_OUT >= 0)
    dprintf(G_OUT, "[mon-bootstrap] setup done\n");
}

static void alerts_common(void) {
  if (G_OUT >= 0) {
  }

  char v[256];
  int vlen = snprintf(v, sizeof v,
                      "{\"jsonrpc\":\"2.0\",\"method\":\"set_variant\","
                      "\"params\":{\"variant\":2,\"fts_avail\":127},\"id\":1}\n");

  write_response(WR_FD, v, (size_t)vlen);
  process_request(50);
  lv_test_wait(50);
  drain_pipe_now();

  if (G_OUT >= 0)
    dprintf(G_OUT, "[alerts-bootstrap] setup done\n");
}

TEST_SETUP(GO_Common_screen) {
  if (g_bail) {
    TEST_IGNORE_MESSAGE("Skipped after first failure");
  }
}

TEST_TEAR_DOWN(GO_Common_screen) {
  drain_pipe_now();
  if (Unity.CurrentTestFailed) {
    g_bail = 1;
  }
}

static void settings_common(void) {
  if (G_OUT >= 0) {
  }

  {
    char v[256];
    int vlen = snprintf(v, sizeof v,
                        "{\"jsonrpc\":\"2.0\",\"method\":\"set_variant\","
                        "\"params\":{\"variant\":2,\"fts_avail\":127},\"id\":1}\n");

    write_response(WR_FD, v, (size_t)vlen);
    process_request(50);
    lv_test_wait(50);
    drain_pipe_now();
  }
  {
    char resp[512];
    int len = snprintf(resp, sizeof resp,
                       "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":"
                       "{\"screen\":2,\"fts_avail\":127,\"fts_state\":7,"
                       "\"bed_pos\":1,\"bed_wid\":60,\"occ_size\":2,"
                       "\"mon_start\":\"2130\",\"mon_end\":\"0700\","
                       "\"bms\":1,\"vol\":2,\"audio\":1,\"lang\":\"English\","
                       "\"mode\":1,\"room_number\":\"1001-a\"},\"id\":2}\n");

    write_response(WR_FD, resp, (size_t)len);
    process_request(50);
    lv_test_wait(200);
    drain_pipe_now();
  }

  if (G_OUT >= 0)
    dprintf(G_OUT, "[mon-bootstrap] setup done\n");
}

TEST_SETUP(GO_ActivationFlow) {
  if (g_bail) {
    TEST_IGNORE_MESSAGE("Skipped after first failure");
  }
  /* Only when user runs with -g and/or -n */
  if (is_grp_filtered_run()) {
    act_flow_common();
  }
}

TEST_TEAR_DOWN(GO_ActivationFlow) {
  drain_pipe_now();
  if (Unity.CurrentTestFailed) {
    g_bail = 1;
  }
}

TEST_SETUP(GO_MonitoringScreens) {
  if (g_bail) {
    TEST_IGNORE_MESSAGE("Skipped after first failure");
  }
  /* Only when user runs with -g and/or -n */
  if (is_grp_filtered_run()) {
    monitoring_common();
  }
}

TEST_TEAR_DOWN(GO_MonitoringScreens) {
  drain_pipe_now();
  if (Unity.CurrentTestFailed) {
    g_bail = 1;
  }
}

TEST_SETUP(GO_Alerts_and_error_screens) {
  if (g_bail) {
    TEST_IGNORE_MESSAGE("Skipped after first failure");
  }
  /* Only when user runs with -g and/or -n */
  if (is_grp_filtered_run()) {
    alerts_common();
  }
}

TEST_TEAR_DOWN(GO_Alerts_and_error_screens) {
  drain_pipe_now();
  if (Unity.CurrentTestFailed) {
    g_bail = 1;
  }
}

TEST_SETUP(GO_ACT_Settings_Flow) {
  if (g_bail) {
    TEST_IGNORE_MESSAGE("Skipped after first failure");
  }
  /* Only when user runs with -g and/or -n */
  if (is_grp_filtered_run()) {
    settings_common();
  }
}

TEST_TEAR_DOWN(GO_ACT_Settings_Flow) {
  drain_pipe_now();
  if (Unity.CurrentTestFailed) {
    g_bail = 1;
  }
}

TEST(GO_Common_screen, NKD_COM_001) { test_welcome_screen(); }

TEST(GO_Common_screen, NKD_COM_002) { test_deactivate_screen(); }

TEST(GO_Common_screen, NKD_COM_003) { test_timer_on_mon_ale_screen(); }

TEST(GO_ActivationFlow, NKD_GO_001) { test_activate_screen(); }

TEST(GO_ActivationFlow, NKD_GO_002) { test_shut_down_btn_functionality(); }

TEST(GO_ActivationFlow, NKD_GO_003) { test_activate_btn_functionality(); }

TEST(GO_ActivationFlow, NKD_GO_004) { test_presence_of_confirm_room_assignment_screen(); }

TEST(GO_ActivationFlow, NKD_GO_005) { test_validate_confirm_room_btn(); }

TEST(GO_ActivationFlow, NKD_GO_006) { test_validate_change_room_btn(); }

TEST(GO_ActivationFlow, NKD_GO_007) { test_presence_of_assign_room_to_system(); }

TEST(GO_ActivationFlow, NKD_GO_008) { test_view_rooms_btn_functionality(); }

TEST(GO_ActivationFlow, NKD_GO_009) { test_presence_of_unit_selection_screen(); }

TEST(GO_ActivationFlow, NKD_GO_010) { test_functionality_of_unit_buttons(); }

TEST(GO_ActivationFlow, NKD_GO_011) { test_forward_buttons_on_unit_selection_screen(); }

TEST(GO_ActivationFlow, NKD_GO_012) { test_backward_buttons_on_unit_selection_screen(); }

TEST(GO_ActivationFlow, NKD_GO_013) { test_next_btn_functionality_on_unit_selection_screen(); }

TEST(GO_ActivationFlow, NKD_GO_014) { test_presence_of_room_selection_screen(); }

TEST(GO_ActivationFlow, NKD_GO_015) { test_room_number_btns_on_screen(); }

TEST(GO_ActivationFlow, NKD_GO_016) { test_assigning_room_pop_up_screen(); }

TEST(GO_ActivationFlow, NKD_GO_017) { test_room_assignment_failed_pop_up(); }

TEST(GO_ActivationFlow, NKD_GO_018) { test_cancel_btn_on_failure_pop_up(); }

TEST(GO_ActivationFlow, NKD_GO_019) { test_try_again_btn_on_failure_pop_up(); }

TEST(GO_ActivationFlow, NKD_GO_020) { test_system_warning_pop_up(); }

TEST(GO_ActivationFlow, NKD_GO_021) { test_confirm_btn_on_pop_up(); }

TEST(GO_ActivationFlow, NKD_GO_022) { test_room_successful_pop_up(); }

TEST(GO_ActivationFlow, NKD_GO_023) { test_reassign_button_on_pop_up(); }

TEST(GO_ActivationFlow, NKD_GO_024) { test_continue_button_on_pop_up(); }

TEST(GO_ActivationFlow, NKD_GO_ACT_001_Presence_of_alerts_page) { test_Alerts_page(); }

TEST(GO_ActivationFlow, NKD_GO_ACT_001_Toggles) { test_toggles_in_alerts_page(); }

TEST(GO_ActivationFlow, NKD_GO_ACT_001_back_and_forth) { test_back_btn_alerts(); }

TEST(GO_ActivationFlow, NKD_GO_ACT_001_X_btn) { test_X_btn_alerts(); }

TEST(GO_ActivationFlow, NKD_GO_ACT_001_i_btn_Alerts) { test_sysinfo_screen(); }

TEST(GO_ActivationFlow, NKD_GO_ACT_001_nxt_btn_Alert) { test_next_btn_click(); }

TEST(GO_ActivationFlow, NKD_GO_ACT_007_Presence_of_Exit_Alert) { test_exit_alert_screen(); }

TEST(GO_ActivationFlow, NKD_GO_ACT_007_i_btn_Exit_alert) { test_i_button_on_exit_alert_screen(); }

TEST(GO_ActivationFlow, NKD_GO_ACT_007_X_btn) { test_x_btn_click_on_exit_alert_screen(); }

TEST(GO_ActivationFlow, NKD_GO_ACT_008_Contnious_Monitoring_Dropdown) {
  test_clickon_Ctn_Monitoring();
}

TEST(GO_ActivationFlow, NKD_GO_ACT_008_Schedule_Monitoring_Page) { test_schdl_monitoring_screen(); }

TEST(GO_ActivationFlow, NKD_GO_ACT_009_Scheduled_Start_time) { test_strtime_on_schdl_screen(); }

TEST(GO_ActivationFlow, NKD_GO_ACT_009_Scheduled_End_time) { test_endtime_on_schdl_screen(); }

TEST(GO_ActivationFlow, NKD_GO_ACT_010_back_btn) { test_back_btn_on_strtime_screen(); }

TEST(GO_ActivationFlow, NKD_GO_ACT_010_X_btn) { test_Xbtn_on_strtime_screen(); }

TEST(GO_ActivationFlow, NKD_GO_ACT_010_Up_arrows_on_strt_time_and_save) {
  test_up_arrow_on_strtime_screen();
}

TEST(GO_ActivationFlow, NKD_GO_ACT_010_down_arrows_on_strt_time_and_save) {
  test_dwn_arrows_on_strtime_screen();
}

TEST(GO_ActivationFlow, NKD_GO_ACT_010_back_btn_on_end_time) { test_back_btn_on_endtime_screen(); }

TEST(GO_ActivationFlow, NKD_GO_ACT_010_X_btn_on_end_time) { test_Xbtn_on_endtime_screen(); }

TEST(GO_ActivationFlow, NKD_GO_ACT_010_Up_Arrows_on_end_time_and_save) {
  test_up_arrow_on_endtime_screen();
}

TEST(GO_ActivationFlow, NKD_GO_ACT_010_Down_arrows_on_end_time_and_save) {
  test_dwn_arrows_on_endtime_screen();
}

TEST(GO_ActivationFlow, NKD_GO_ACT_011_Presence_of_BMS_Page) { test_bed_mode_sensitivity(); }

TEST(GO_ActivationFlow, NKD_GO_ACT_011_i_btn_on_BMS) { test_i_btn_on_bms_screen(); }

TEST(GO_ActivationFlow, NKD_GO_ACT_011_Back_btn_on_BMS) {
  test_back_btn_on_bed_mode_sensitivity_screen();
}

TEST(GO_ActivationFlow, NKD_GO_ACT_011_X_btn_on_BMS) { test_xbtn_on_bed_mode_sensitivity_screen(); }

TEST(GO_ActivationFlow, NKD_GO_ACT_012_Sliders_with_back_forth) { test_sliders_on_bms_screen(); }

TEST(GO_ActivationFlow, NKD_GO_ACT_013_Presence_of_BW_Page) { test_bed_width_screen(); }

TEST(GO_ActivationFlow, NKD_GO_ACT_013_i_btn_on_BW_Screen) { test_i_btn_on_BW_screen(); }

TEST(GO_ActivationFlow, NKD_GO_ACT_013_Back_btn_on_BW_page) { test_back_btn_on_bed_width_screen(); }

TEST(GO_ActivationFlow, NKD_GO_ACT_013_X_btn_on_BW_page) { test_xbtn_on_bed_width_screen(); }

TEST(GO_ActivationFlow, NKD_GO_ACT_014_Sliders_with_back_forth) { test_sliders_on_BW_screen(); }

TEST(GO_ActivationFlow, NKD_GO_ACT_015_Custom_Bed_Width_Selection) {
  test_Chng_width_on_BW_screen();
}

TEST(GO_ActivationFlow, NKD_GO_ACT_016_back_btn_on_Custom_width_selection_page) {
  test_back_btn_on_custom_width_screen();
}

TEST(GO_ActivationFlow, NKD_GO_ACT_016_X_btn_on_Custom_width_selection_page) {
  test_x_btn_on_custom_width_screen();
}

TEST(GO_ActivationFlow, NKD_GO_ACT_016_Predefined_options_on_Custom_width_screen) {
  test_predefined_width_options_on_Custom_Width_Screen();
}

TEST(GO_ActivationFlow, NKD_GO_ACT_016_Custom_value_screen) { test_clk_on_custom_value_button(); }

TEST(GO_ActivationFlow, NKD_GO_ACT_017_Custom_Numerical_Inputs) {
  test_number_keypad_screen_on_Custom_BW();
}

TEST(GO_ActivationFlow, NKD_GO_ACT_017_Back_btn_on_custom_value_screen) {
  test_back_button_on_custom_value_screen();
}

TEST(GO_ActivationFlow, NKD_GO_ACT_017_X_btn_on_custom_value_screen) {
  test_x_button_on_custom_value_screen();
}

TEST(GO_ActivationFlow, NKD_GO_ACT_018_ToastMessage_of_min_value) {
  test_toast_messages_on_custom_value_screen();
}

TEST(GO_ActivationFlow, NKD_GO_ACT_019_Presence_of_Bed_Placement_Screen) {
  test_bed_placement_screen();
}

TEST(GO_ActivationFlow, NKD_GO_ACT_019_i_btn_bed_placement_screen) {
  test_i_button_on_bed_placement_screen();
}

TEST(GO_ActivationFlow, NKD_GO_ACT_019_X_btn_on_bed_placement_screen) {
  test_X_button_on_bed_placement_screen();
}

TEST(GO_ActivationFlow, NKD_GO_ACT_019_Back_btn_on_bed_placement_screen) {
  test_back_button_on_bed_placement_screen();
}

TEST(GO_ActivationFlow, NKD_GO_ACT_020_Sliders_with_back_forth) {
  test_sliders_on_bed_placement_screen();
}

TEST(GO_ActivationFlow, NKD_GO_ACT_021_Presence_of_Occupant_Size_Screen) {
  test_occupant_size_screen();
}

TEST(GO_ActivationFlow, NKD_GO_ACT_021_i_btn_on_Occupant_Size_Screen) {
  test_i_button_on_occupant_size_screen();
}

TEST(GO_ActivationFlow, NKD_GO_ACT_021_X_btn_on_occupant_size_screen) {
  test_x_button_on_occupant_size_screen();
}

TEST(GO_ActivationFlow, NKD_GO_ACT_021_back_btn_on_occupant_size_screen) {
  test_back_button_on_occupant_size_screen();
}

TEST(GO_ActivationFlow, NKD_GO_ACT_022_Sliders_with_back_forth) {
  test_sliders_on_occupant_size_screen();
}

TEST(GO_ActivationFlow, NKD_GO_ACT_023_Presence_of_InRoomAlert_Screen) {
  test_in_room_alert_screen();
}

TEST(GO_ActivationFlow, NKD_GO_ACT_023_i_btn_on_inroom_audio_alert) {
  test_i_button_on_in_room_alert_screen();
}

TEST(GO_ActivationFlow, NKD_GO_ACT_023_X_btn_on_inroom_audio_alert) {
  test_X_button_on_in_room_alert_screen();
}

TEST(GO_ActivationFlow, NKD_GO_ACT_023_back_btn_on_inroom_audio_alert) {
  test_back_button_on_in_room_alert_screen();
}

TEST(GO_ActivationFlow, NKD_GO_ACT_024_InRoomAlert_AudioToggle) {
  test_in_room_alert_audio_toggle();
}

TEST(GO_ActivationFlow, NKD_GO_ACT_025_InRoomAlert_LanguagesPage) {
  test_lang_button_on_in_room_alert_screen();
}

TEST(GO_ActivationFlow, NKD_GO_ACT_025_Language_Page_with_command) {
  test_lang_button_with_command();
}

TEST(GO_ActivationFlow, NKD_GO_ACT_025_back_btn_on_lang_screen) {
  test_back_button_on_lang_screen();
}

TEST(GO_ActivationFlow, NKD_GO_ACT_025_X_btn_on_lang_screen) { test_X_btn_on_lang_screen(); }

TEST(GO_ActivationFlow, NKD_GO_ACT_026_Sliders_with_back_and_forth) {
  test_volume_slider_in_room_alert_screen();
}

TEST(GO_ActivationFlow, NKD_GO_ACT_027_Activation_button) { test_activating_screen_with_2_and_7(); }

TEST(GO_MonitoringScreens, NKD_GO_MON_001) { test_bed_monitoring_screen(); }

TEST(GO_MonitoringScreens, NKD_GO_MON_002) { test_bed_mode_button(); }

TEST(GO_MonitoringScreens, NKD_GO_MON_003) { test_chair_mode_button(); }

TEST(GO_MonitoringScreens, NKD_GO_MON_004) { test_short_pause_and_long_pause(); }

TEST(GO_MonitoringScreens, NKD_GO_MON_005) { test_settings_icon(); }

TEST(GO_MonitoringScreens, NKD_GO_MON_006) { test_Presence_of_Chair_Mon_screen(); }

TEST(GO_MonitoringScreens, NKD_GO_MON_007) { test_pressure_injury_screen(); }

TEST(GO_MonitoringScreens, NKD_GO_MON_008) { test_schedule_monitoring_screen(); }

TEST(GO_MonitoringScreens, NKD_GO_MON_009) { test_fall_monitoring_screen(); }

TEST(GO_Alerts_and_error_screens, NKD_GO_ALE_001) { test_bed_exit_alert_withall3_options(); }

TEST(GO_Alerts_and_error_screens, NKD_GO_ALE_002) { test_chair_exit_alert(); }

TEST(GO_Alerts_and_error_screens, NKD_GO_ALE_003) { test_fall_alert_screen(); }

TEST(GO_Alerts_and_error_screens, NKD_GO_ALE_004) { test_fall_alert_with_bed_chair_enabled(); }

TEST(GO_Alerts_and_error_screens, NKD_GO_ALE_005) { test_reposition_alert_screen(); }

TEST(GO_Alerts_and_error_screens, NKD_GO_ALE_006) { test_reposition_alert_with_bed_chair(); }

TEST(GO_Alerts_and_error_screens, NKD_GO_ALE_007) { test_paused_error_screen(); }

TEST(GO_Alerts_and_error_screens, NKD_GO_ALE_008) { test_calibaration_error_screen(); }

TEST(GO_Alerts_and_error_screens, NKD_GO_ALE_009) { test_chair_calibarating_screen(); }

TEST(GO_Alerts_and_error_screens, NKD_GO_ALE_010) { test_incorrect_mode_error_screen(); }

TEST(GO_Alerts_and_error_screens, NKD_GO_ALE_011) { test_unassigned_error_screen(); }

TEST(GO_Alerts_and_error_screens, NKD_GO_ALE_012) { test_sunight_error_screen(); }

TEST(GO_Alerts_and_error_screens, NKD_GO_ALE_013) { test_obstructed_error_screen(); }

TEST(GO_Alerts_and_error_screens, NKD_GO_ALE_014) { test_system_disconnected_error_screen(); }

TEST(GO_Alerts_and_error_screens, NKD_GO_ALE_015) { test_not_monitoring_screen(); }

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_001_Presence_of_settings_screen) {
  test_settings_Screen_with_buttons();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_001_i_btn_on_settings_screen) {
  test_i_btn_on_settings_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_001_X_btn_on_settings_screen) {
  test_X_btn_on_settings_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_002_optional_settings_with_diff_fts_values) {
  test_optional_settings_available();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_003_presence_of_bms_screen_settings_flow) {
  test_bms_on_settings_flow();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_004_functionality_of_bms_screen_Scrollers) {
  test_functionality_of_bms_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_004_back_btn_on_bms_screen) { test_back_btn_on_bms_screen(); }

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_004_X_btn_on_bms_screen) { test_X_btn_on_bms_screen(); }

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_006_save_button_on_bms_screen_with_toasts) {
  test_save_button_on_bms_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_006_back_button_after_BMS_saved) {
  test_back_button_after_BMS_saved();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_006_X_button_after_BMS_saved) {
  test_X_button_after_BMS_saved();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_007_presence_of_in_room_audio_alerts) {
  test_in_room_audio_alerts_on_settings_flow();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_007_X_btn_on_in_room_audio_alerts_screen) {
  test_X_btn_on_in_room_audio_alerts_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_007_back_btn_on_in_room_audio_alerts_screen) {
  test_back_btn_on_in_room_audio_alerts_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_008_toggle_on_in_room_audio_alerts_screen) {
  test_toggle_on_in_room_audio_alerts_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_009_languages_screen_with_no_command) {
  test_languages_screen_with_no_command();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_009_back_btn_on_languages_screen) {
  test_back_btn_on_languages_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_009_X_btn_on_languages_screen) {
  test_X_btn_on_languages_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_009_languages_screen_with_command) {
  test_languages_screen_with_command();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_009_volume_scroller_on_in_room_alert_screen) {
  test_volume_scroller_on_in_room_alert_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_010_save_btn_on_in_room_alert_screen) {
  test_save_btn_on_in_room_alert_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_010_back_btn_on_in_room_alert_screen) {
  test_back_btn_on_in_room_alert_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_010_X_btn_on_in_room_alert_screen) {
  test_X_btn_on_in_room_alert_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_011_Presence_display_settings) {
  test_display_page_on_activated_settings_flow();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_011_back_btn_on_display_settings_screen) {
  test_back_btn_on_display_settings_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_011_X_btn_on_display_settings_screen) {
  test_X_btn_on_display_settings_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_012_brightness_scroller_on_display_settings_screen) {
  test_brightness_scroller_on_display_settings_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_013_adaptive_toggle_btn) { test_adaptive_toggle_btn(); }

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_014_save_btn_on_display_settings_screen) {
  test_save_btn_on_display_settings_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_014_back_btn_on_display_settings_after_save) {
  test_back_btn_on_display_settings();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_014_X_btn_on_display_settings_after_save) {
  test_X_btn_on_display_settings();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_015_Presence_of_Alerts_page_on_settings_flow) {
  test_Alerts_page_on_settings_flow();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_015_toggles_on_alert_screen) {
  test_toggles_on_alert_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_015_X_btn_on_alerts_settings_screen) {
  test_X_btn_on_alerts_settings_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_015_back_btn_on_alerts_settings_screen) {
  test_back_btn_on_alerts_settings_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_016_save_btn_on_alerts_settings_screen) {
  test_save_btn_on_alerts_settings_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_016_back_button_on_alerts_settings_screen) {
  test_back_button_on_alerts_settings_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_016_X_button_on_alerts_settings_screen) {
  test_X_button_on_alerts_settings_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_017_presence_of_exit_alert_schdl_on_settings_flow) {
  test_exit_alert_schedule_on_settings_flow();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_017_back_btn_on_exit_alert_schedule_screen) {
  test_back_btn_on_exit_alert_schedule_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_017_X_btn_on_exit_alert_schedule_screen) {
  test_X_btn_on_exit_alert_schedule_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_018_save_btn_on_exit_alert_schedule_screen) {
  test_save_btn_on_exit_alert_schedule_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_018_back_button_on_exit_alert_schedule_screen) {
  test_back_button_on_exit_alert_schedule_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_018_X_button_on_exit_alert_schedule_screen) {
  test_X_button_on_exit_alert_schedule_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_019_presence_of_bed_width_screen_on_settings_flow) {
  test_bed_width_screen_on_settings_flow();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_019_back_btn_on_bed_width_settings_screen) {
  test_back_btn_on_bed_width_settings_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_019_X_btn_on_bed_width_settings_screen) {
  test_X_btn_on_bed_width_settings_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_019_bed_width_scroller_on_settings_screen) {
  test_bed_width_scroller_on_settings_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_019_Custom_bed_width_settings_screen) {
  test_custom_bed_width_screen_on_settings_flow();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_019_back_btn_on_custom_width_screen_on_settings_flow) {
  test_back_button_on_Custom_BW_screen_on_settings_flow();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_019_X_btn_on_custom_width_screen_on_settings_flow) {
  test_x_button_on_Custom_BW_screen_on_settings_flow();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_019_predefined_options_on_custom_BW_screen_on_settings_flow) {
  test_predefined_width_options_on_Custom_BW_screen_on_settings_flow();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_019_custom_value_screen_on_settings_flow) {
  test_custom_value_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_019_keypad_screen_on_BW_settings_flow) {
  test_keypad_screen_on_Custom_Value_Screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_019_back_btn_on_custom_input_screen_on_settings_flow) {
  test_back_button_on_custom_input_screen_on_settings_flow();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_019_X_btn_on_custom_input_screen_on_settings_flow) {
  test_x_button_on_custom_input_screen_settings_flow();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_019_toast_messages_on_custom_input_screen_on_settings_flow) {
  test_toast_messages_on_custom_input_screen_on_settings_flow();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_020_Save_btn_on_BW_screen) { test_save_btn_on_BW_screen(); }

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_021_Presence_of_bed_placement_screen_on_settings_flow) {
  test_bed_placement_screen_on_settings_flow();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_021_back_btn_on_bed_placement_settings_screen) {
  test_back_button_on_bed_placement_settings_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_021_X_btn_on_bed_placement_settings_screen) {
  test_X_button_on_bed_placement_settings_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_021_bed_placement_scrollers_on_settings_screen) {
  test_bed_placement_scrollers_on_settings_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_022_save_btn_on_bed_placement_screen) {
  test_save_btn_on_BP_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_023_Presence_of_occupant_size_screen_on_settings_flow) {
  test_Occupant_Size_screen_on_settings_flow();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_023_back_btn_on_occupant_size_settings_screen) {
  test_back_button_on_occupant_size_settings_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_023_X_btn_on_occupant_size_settings_screen) {
  test_X_button_on_occupant_size_settings_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_023_occupant_size_scroller_on_settings_screen) {
  test_scrollers_on_occupant_size_settings_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_024_Save_btn_on_occupant_size_screen) {
  test_save_btn_on_OS_screen();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_025_Settings_Save_Pop_Up) {
  test_save_pop_up_message_on_settings_flow();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_026_Dont_Save_button_on_settings_flow) {
  test_Dont_Save_button_on_settings_flow();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_027_save_btn_functionality_on_settings_flow) {
  test_save_btn_functionality_on_settings_flow();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_028_presence_of_pop_up_toast_on_settings_flow) {
  test_presence_of_pop_up_toast_on_settings_flow();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_029_X_btn_on_toast_message_on_settings_flow) {
  test_X_btn_on_toast_message_on_settings_flow();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_030_Presence_of_alerts_and_error_toasts_on_settings_flow) {
  test_Alerts_and_error_toast_msgs_on_settings_flow();
}

TEST(GO_ACT_Settings_Flow,
     NKD_GO_SET_030_Presence_of_alerts_and_error_toasts_timeouts_on_settings_pages) {
  test_alerts_and_error_toasts_on_settings_pages();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_031_Deactivate_btn_Functionality_on_Settings_Flow) {
  test_deactivate_button_on_settings_flow();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_032_Resume_Fall_mon_btn_Functionality) {
  test_resume_fall_monitoring_button();
}

TEST(GO_ACT_Settings_Flow, NKD_GO_SET_033_clear_alert_btn_Functionality) {
  test_clear_alert_btn_functionality();
}

TEST_GROUP_RUNNER(GO_Common_screen) {
  RUN_TEST_CASE(GO_Common_screen, NKD_COM_001);
  RUN_TEST_CASE(GO_Common_screen, NKD_COM_002);
  RUN_TEST_CASE(GO_Common_screen, NKD_COM_003);
}

TEST_GROUP_RUNNER(GO_ActivationFlow) {
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_001);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_002);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_003);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_004);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_005);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_006);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_007);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_008);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_009);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_010);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_011);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_012);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_013);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_014);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_015);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_016);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_017);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_018);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_019);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_020);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_021);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_022);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_023);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_024);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_001_Presence_of_alerts_page);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_001_Toggles);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_001_back_and_forth);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_001_X_btn);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_001_i_btn_Alerts);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_001_nxt_btn_Alert);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_007_Presence_of_Exit_Alert);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_007_i_btn_Exit_alert);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_007_X_btn);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_008_Contnious_Monitoring_Dropdown);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_008_Schedule_Monitoring_Page);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_009_Scheduled_Start_time);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_009_Scheduled_End_time);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_010_back_btn);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_010_X_btn);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_010_Up_arrows_on_strt_time_and_save);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_010_down_arrows_on_strt_time_and_save);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_010_back_btn_on_end_time);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_010_X_btn_on_end_time);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_010_Up_Arrows_on_end_time_and_save);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_010_Down_arrows_on_end_time_and_save);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_011_Presence_of_BMS_Page);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_011_i_btn_on_BMS);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_011_Back_btn_on_BMS);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_011_X_btn_on_BMS);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_012_Sliders_with_back_forth);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_013_Presence_of_BW_Page);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_013_i_btn_on_BW_Screen);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_013_Back_btn_on_BW_page);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_013_X_btn_on_BW_page);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_014_Sliders_with_back_forth);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_015_Custom_Bed_Width_Selection);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_016_back_btn_on_Custom_width_selection_page);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_016_X_btn_on_Custom_width_selection_page);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_016_Predefined_options_on_Custom_width_screen);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_016_Custom_value_screen);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_017_Custom_Numerical_Inputs);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_017_Back_btn_on_custom_value_screen);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_017_X_btn_on_custom_value_screen);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_018_ToastMessage_of_min_value);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_019_Presence_of_Bed_Placement_Screen);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_019_i_btn_bed_placement_screen);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_019_X_btn_on_bed_placement_screen);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_019_Back_btn_on_bed_placement_screen);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_020_Sliders_with_back_forth);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_021_Presence_of_Occupant_Size_Screen);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_021_i_btn_on_Occupant_Size_Screen);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_021_X_btn_on_occupant_size_screen);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_021_back_btn_on_occupant_size_screen);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_022_Sliders_with_back_forth);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_023_Presence_of_InRoomAlert_Screen);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_023_i_btn_on_inroom_audio_alert);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_023_X_btn_on_inroom_audio_alert);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_023_back_btn_on_inroom_audio_alert);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_024_InRoomAlert_AudioToggle);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_025_InRoomAlert_LanguagesPage);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_025_Language_Page_with_command);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_025_back_btn_on_lang_screen);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_025_X_btn_on_lang_screen);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_026_Sliders_with_back_and_forth);
  RUN_TEST_CASE(GO_ActivationFlow, NKD_GO_ACT_027_Activation_button);
}

TEST_GROUP_RUNNER(GO_MonitoringScreens) {
  RUN_TEST_CASE(GO_MonitoringScreens, NKD_GO_MON_001);
  RUN_TEST_CASE(GO_MonitoringScreens, NKD_GO_MON_002);
  RUN_TEST_CASE(GO_MonitoringScreens, NKD_GO_MON_003);
  RUN_TEST_CASE(GO_MonitoringScreens, NKD_GO_MON_004);
  RUN_TEST_CASE(GO_MonitoringScreens, NKD_GO_MON_005);
  RUN_TEST_CASE(GO_MonitoringScreens, NKD_GO_MON_006);
  RUN_TEST_CASE(GO_MonitoringScreens, NKD_GO_MON_007);
  RUN_TEST_CASE(GO_MonitoringScreens, NKD_GO_MON_008);
  RUN_TEST_CASE(GO_MonitoringScreens, NKD_GO_MON_009);
}

TEST_GROUP_RUNNER(GO_Alerts_and_error_screens) {
  RUN_TEST_CASE(GO_Alerts_and_error_screens, NKD_GO_ALE_001);
  RUN_TEST_CASE(GO_Alerts_and_error_screens, NKD_GO_ALE_002);
  RUN_TEST_CASE(GO_Alerts_and_error_screens, NKD_GO_ALE_003);
  RUN_TEST_CASE(GO_Alerts_and_error_screens, NKD_GO_ALE_004);
  RUN_TEST_CASE(GO_Alerts_and_error_screens, NKD_GO_ALE_005);
  RUN_TEST_CASE(GO_Alerts_and_error_screens, NKD_GO_ALE_006);
  RUN_TEST_CASE(GO_Alerts_and_error_screens, NKD_GO_ALE_007);
  RUN_TEST_CASE(GO_Alerts_and_error_screens, NKD_GO_ALE_008);
  RUN_TEST_CASE(GO_Alerts_and_error_screens, NKD_GO_ALE_009);
  RUN_TEST_CASE(GO_Alerts_and_error_screens, NKD_GO_ALE_010);
  RUN_TEST_CASE(GO_Alerts_and_error_screens, NKD_GO_ALE_011);
  RUN_TEST_CASE(GO_Alerts_and_error_screens, NKD_GO_ALE_012);
  RUN_TEST_CASE(GO_Alerts_and_error_screens, NKD_GO_ALE_013);
  RUN_TEST_CASE(GO_Alerts_and_error_screens, NKD_GO_ALE_014);
  RUN_TEST_CASE(GO_Alerts_and_error_screens, NKD_GO_ALE_015);
}

TEST_GROUP_RUNNER(GO_ACT_Settings_Flow) {
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_001_Presence_of_settings_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_001_i_btn_on_settings_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_001_X_btn_on_settings_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_002_optional_settings_with_diff_fts_values);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_003_presence_of_bms_screen_settings_flow);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_004_functionality_of_bms_screen_Scrollers);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_004_back_btn_on_bms_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_004_X_btn_on_bms_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_006_save_button_on_bms_screen_with_toasts);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_006_back_button_after_BMS_saved);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_006_X_button_after_BMS_saved);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_007_presence_of_in_room_audio_alerts);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_007_X_btn_on_in_room_audio_alerts_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_007_back_btn_on_in_room_audio_alerts_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_008_toggle_on_in_room_audio_alerts_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_009_languages_screen_with_no_command);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_009_back_btn_on_languages_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_009_X_btn_on_languages_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_009_languages_screen_with_command);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_009_volume_scroller_on_in_room_alert_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_010_save_btn_on_in_room_alert_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_010_back_btn_on_in_room_alert_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_010_X_btn_on_in_room_alert_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_011_Presence_display_settings);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_011_back_btn_on_display_settings_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_011_X_btn_on_display_settings_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow,
                NKD_GO_SET_012_brightness_scroller_on_display_settings_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_013_adaptive_toggle_btn);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_014_save_btn_on_display_settings_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_014_back_btn_on_display_settings_after_save);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_014_X_btn_on_display_settings_after_save);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_015_Presence_of_Alerts_page_on_settings_flow);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_015_toggles_on_alert_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_015_X_btn_on_alerts_settings_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_015_back_btn_on_alerts_settings_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_016_save_btn_on_alerts_settings_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_016_back_button_on_alerts_settings_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_016_X_button_on_alerts_settings_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_017_presence_of_exit_alert_schdl_on_settings_flow);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_017_back_btn_on_exit_alert_schedule_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_017_X_btn_on_exit_alert_schedule_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_018_save_btn_on_exit_alert_schedule_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_018_back_button_on_exit_alert_schedule_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_018_X_button_on_exit_alert_schedule_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_019_presence_of_bed_width_screen_on_settings_flow);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_019_back_btn_on_bed_width_settings_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_019_X_btn_on_bed_width_settings_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_019_bed_width_scroller_on_settings_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_019_Custom_bed_width_settings_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow,
                NKD_GO_SET_019_back_btn_on_custom_width_screen_on_settings_flow);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_019_X_btn_on_custom_width_screen_on_settings_flow);

  RUN_TEST_CASE(GO_ACT_Settings_Flow,
                NKD_GO_SET_019_predefined_options_on_custom_BW_screen_on_settings_flow);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_019_custom_value_screen_on_settings_flow);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_019_keypad_screen_on_BW_settings_flow);
  RUN_TEST_CASE(GO_ACT_Settings_Flow,
                NKD_GO_SET_019_back_btn_on_custom_input_screen_on_settings_flow);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_019_X_btn_on_custom_input_screen_on_settings_flow);
  RUN_TEST_CASE(GO_ACT_Settings_Flow,
                NKD_GO_SET_019_toast_messages_on_custom_input_screen_on_settings_flow);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_020_Save_btn_on_BW_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow,
                NKD_GO_SET_021_Presence_of_bed_placement_screen_on_settings_flow);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_021_back_btn_on_bed_placement_settings_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_021_X_btn_on_bed_placement_settings_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_021_bed_placement_scrollers_on_settings_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_022_save_btn_on_bed_placement_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow,
                NKD_GO_SET_023_Presence_of_occupant_size_screen_on_settings_flow);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_023_back_btn_on_occupant_size_settings_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_023_X_btn_on_occupant_size_settings_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_023_occupant_size_scroller_on_settings_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_024_Save_btn_on_occupant_size_screen);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_025_Settings_Save_Pop_Up);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_026_Dont_Save_button_on_settings_flow);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_027_save_btn_functionality_on_settings_flow);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_028_presence_of_pop_up_toast_on_settings_flow);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_029_X_btn_on_toast_message_on_settings_flow);
  RUN_TEST_CASE(GO_ACT_Settings_Flow,
                NKD_GO_SET_030_Presence_of_alerts_and_error_toasts_on_settings_flow);
  RUN_TEST_CASE(GO_ACT_Settings_Flow,
                NKD_GO_SET_030_Presence_of_alerts_and_error_toasts_timeouts_on_settings_pages);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_031_Deactivate_btn_Functionality_on_Settings_Flow);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_032_Resume_Fall_mon_btn_Functionality);
  RUN_TEST_CASE(GO_ACT_Settings_Flow, NKD_GO_SET_033_clear_alert_btn_Functionality);
}
