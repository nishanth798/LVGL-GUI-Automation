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
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-COM/IN-00-NKD_COM_001-Welcome Screen.png"),
      "Screenshot comparison failed for Welcome Screen");
}

static void setv_variant_cmd(void) {
  const char *setv = "{\"jsonrpc\":\"2.0\",\"method\":\"set_variant\",\"params\":{\"variant\":1,"
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
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare(
                               "ref_imgsNKD/NKD-O/NKD-COM/IN-01-NKD_COM_002-Deactivate Screen.png"),
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
          "ref_imgsNKD/NKD-O/NKD-COM/IN-02-NKD_COM_003-AlertScreen_with_rep_timer.png"),
      "Screenshot comparison failed for Welcome Screen");
}

static void test_activate_screen(void) {
  setv_variant_cmd();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-01-NKD_INT_001-Activate screen.png"),
      "Screenshot comparison failed for IN-01-Activate screen");
}

static void clk_activate_button(void) {
  lv_test_mouse_click_at(240, 240); // Click the "Activate" button
  lv_test_wait(250);
}

static void test_activate_button_click(void) {
  lv_test_mouse_click_at(240, 240); // Click the "Activate" button
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-02-NKD_ACT_001_Alerts_screen.png"),
      "Screenshot comparison failed for IN-02-Alerts screen btn click");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_Alerts_page(void) {
  clk_activate_button();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-02-NKD_ACT_001_Alerts_screen.png"),
      "Screenshot comparison failed for IN-02-Alerts screen btn click");
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
  clk_activate_button();
  lv_test_wait(100);
  lv_test_mouse_click_at(400, 160); // toggle "Bed Mode" / "Chair Mode" Alert
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-03-NKD_ACT_001_Toggle_btn.png"),
      "Screenshot comparison failed for IN-03-Toggle_btn.png");
  lv_test_wait(40);
  lv_test_mouse_click_at(400, 160);
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-02-NKD_ACT_001_Alerts_screen.png"),
      "Screenshot comparison failed for IN-02-Alerts screen btn click");
  toggle_btns_screen();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-04-NKD_ACT_001_toggles_enabled.png"),
      "Screenshot comparison failed for IN-04-04-NKD_ACT_001_toggles_enabled");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_back_btn_alerts(void) {
  clk_activate_button();
  lv_test_wait(100);
  lv_test_mouse_click_at(120, 420); // Click the "Back" button
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-01-NKD_INT_001-Activate screen.png"),
      "Screenshot comparison failed for IN-01-Activate screen");
  clk_activate_button();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-02-NKD_ACT_001_Alerts_screen.png"),
      "Screenshot comparison failed for IN-02-Alerts screen btn click");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_X_btn_alerts(void) {
  clk_activate_button();
  lv_test_wait(100);
  lv_test_mouse_click_at(446, 30); // Click the "X" button
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-01-NKD_INT_001-Activate screen.png"),
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
  clk_activate_button();
  lv_test_wait(100);
  flow_to_sysinfo_screen();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-05-NKD_ACT_001_Sysinfo.png"),
      "Screenshot comparison failed for IN-05-Sysinfo");
  lv_test_mouse_click_at(450, 450); // Close
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-02-NKD_ACT_001_Alerts_screen.png"),
      "Screenshot comparison failed for Sysinfo close btn click");

  flow_to_sysinfo_screen();
  lv_test_mouse_click_at(446, 30); // X
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-02-NKD_ACT_001_Alerts_screen.png"),
      "Screenshot comparison failed for Sysinfo X btn click");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void nxt_btn_to_exit_alert_screen(void) {
  lv_test_mouse_click_at(350, 420); // Click the "Next" button
  lv_test_wait(150);
}

static void test_next_btn_click(void) {
  clk_activate_button();
  lv_test_wait(100);
  toggle_btns_screen();
  lv_test_wait(200);
  nxt_btn_to_exit_alert_screen();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-06-NKD_ACT_001_nxt_btn_exit_alert.png"),
      "Screenshot comparison failed for IN-06-NKD_ACT_001_nxt_btn_exit_alert");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_exit_alert_screen(void) {
  clk_activate_button();
  lv_test_wait(100);
  toggle_btns_screen();
  lv_test_wait(200);
  nxt_btn_to_exit_alert_screen();
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-06-NKD_ACT_001_nxt_btn_exit_alert.png"),
      "Screenshot comparison failed for IN-06-NKD_ACT_001_nxt_btn_exit_alert");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_i_button_on_exit_alert_screen(void) {
  clk_activate_button();
  lv_test_wait(100);
  toggle_btns_screen();
  lv_test_wait(200);
  nxt_btn_to_exit_alert_screen();
  lv_test_wait(100);
  flow_to_sysinfo_screen();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-05-NKD_ACT_001_Sysinfo.png"),
      "Screenshot comparison failed for IN-05-Sysinfo");
  lv_test_mouse_click_at(446, 30); // Click the "X" button
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-06-NKD_ACT_001_nxt_btn_exit_alert.png"),
      "Screenshot comparison failed for IN-06-NKD_ACT_001_nxt_btn_exit_alert");
  lv_test_wait(100);
  flow_to_sysinfo_screen();
  lv_test_mouse_click_at(450, 450); // Click the "Close" button
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-06-NKD_ACT_001_nxt_btn_exit_alert.png"),
      "Screenshot comparison failed for IN-06-NKD_ACT_001_nxt_btn_exit_alert");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_x_btn_click_on_exit_alert_screen(void) {
  clk_activate_button();
  lv_test_wait(100);
  toggle_btns_screen();
  lv_test_wait(200);
  nxt_btn_to_exit_alert_screen();
  lv_test_wait(100);
  lv_test_mouse_click_at(446, 30); // Click the "X" button
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-01-NKD_INT_001-Activate screen.png"),
      "Screenshot comparison failed for IN-01-Activate screen");
}

static void test_back_button_on_exit_alert_screen(void) {
  clk_activate_button();
  toggle_btns_screen();
  nxt_btn_to_exit_alert_screen();
  lv_test_mouse_click_at(120, 420); // Click the "Back" button
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-04-NKD_ACT_001_toggles_enabled.png"),
      "Screenshot comparison failed for IN-04-04-NKD_ACT_001_toggles_enabled");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_next_button_on_exit_alert_screen(void) {
  clk_activate_button();
  toggle_btns_screen();
  nxt_btn_to_exit_alert_screen();
  lv_test_mouse_click_at(350, 420); // next button to BMS page
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-07-NKD_ACT_007_BMS_page.png"),
      "Screenshot comparison failed for IN-07-NKD_ACT_007_BMS_page");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void clk_on_cnts_mon_button(void) {
  lv_test_mouse_click_at(240, 150); // Click the "Continue Monitoring" button
  lv_test_wait(150);
}

static void test_clickon_Ctn_Monitoring(void) {
  clk_activate_button();
  toggle_btns_screen();
  nxt_btn_to_exit_alert_screen();
  clk_on_cnts_mon_button();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-08-NKD_ACT_008_Clk_Cnts_mon.png"),
      "Screenshot comparison failed for IN-08-Continue Monitoring btn click");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void goto_schedl_monitoring_screen() {
  lv_test_mouse_click_at(220, 300); // Click the "Schedule Monitoring" button
  lv_test_wait(150);
}

static void test_schdl_monitoring_screen(void) {
  clk_activate_button();
  toggle_btns_screen();
  nxt_btn_to_exit_alert_screen();
  clk_on_cnts_mon_button();
  goto_schedl_monitoring_screen();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-09-NKD_ACT_008_Clk_schdl_mon.png"),
      "Screenshot comparison failed for IN-09-Scheduled Monitoring btn click");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void clk_on_change_strtime_button(void) {
  lv_test_mouse_click_at(360, 260); // Click the "Change" button
  lv_test_wait(150);
}

static void test_strtime_on_schdl_screen(void) {
  clk_activate_button();
  toggle_btns_screen();
  nxt_btn_to_exit_alert_screen();
  clk_on_cnts_mon_button();
  goto_schedl_monitoring_screen();
  clk_on_change_strtime_button();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-10-NKD_ACT_009_Clk_strt_time.png"),
      "Screenshot comparison failed for IN-10-Start time btn click");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void clk_on_endtime_change_button(void) {
  lv_test_mouse_click_at(340, 325); // Click the "End Time Change" button
  lv_test_wait(150);
}

static void test_endtime_on_schdl_screen(void) {
  clk_activate_button();
  toggle_btns_screen();
  nxt_btn_to_exit_alert_screen();
  clk_on_cnts_mon_button();
  goto_schedl_monitoring_screen();
  clk_on_endtime_change_button();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-11-NKD_ACT_009_Clk_end_time.png"),
      "Screenshot comparison failed for IN-11-End time");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_back_btn_on_strtime_screen(void) {
  clk_activate_button();
  toggle_btns_screen();
  nxt_btn_to_exit_alert_screen();
  clk_on_cnts_mon_button();
  goto_schedl_monitoring_screen();
  clk_on_change_strtime_button();
  lv_test_wait(100);
  lv_test_mouse_click_at(34, 30); // Click the "Back" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-09-NKD_ACT_008_Clk_schdl_mon.png"),
      "Screenshot comparison failed for IN-09-Scheduled Monitoring btn click");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_Xbtn_on_strtime_screen(void) {
  clk_activate_button();
  toggle_btns_screen();
  nxt_btn_to_exit_alert_screen();
  clk_on_cnts_mon_button();
  goto_schedl_monitoring_screen();
  clk_on_change_strtime_button();
  lv_test_mouse_click_at(445, 30); // Click the "X" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-01-NKD_INT_001-Activate screen.png"),
      "Screenshot comparison failed for IN-01-Activate screen");
}

static void test_up_arrow_on_strtime_screen(void) {
  clk_activate_button();
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
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-12-NKD_ACT_010_up_arrows_strt_time.png"),
      "Screenshot comparison failed for IN-12-Up arrows start time");
  lv_test_mouse_click_at(240, 425); // Click the "Save" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-13-NKD_ACT_010_Save_strt_time.png"),
      "Screenshot comparison failed for IN-13-Save start time");
  clk_on_change_strtime_button();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-14-NKD_ACT_010_Save_btn_disabled_strt_time.png"),
      "Screenshot comparison failed for IN-14-Save btn start time");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_dwn_arrows_on_strtime_screen(void) {
  clk_activate_button();
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
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-15-NKD_ACT_010_dwn_arws_in_strt_time.png"),
      "Screenshot comparison failed for IN-15-Down Arrows on strtime screen");
  lv_test_mouse_click_at(240, 425); // Click the "Save" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-16-NKD_ACT_010_Save_strt_time.png"),
      "Screenshot comparison failed for IN-16-Save btn clk & verify time change dwn arrows");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_back_btn_on_endtime_screen(void) {
  clk_activate_button();
  toggle_btns_screen();
  nxt_btn_to_exit_alert_screen();
  clk_on_cnts_mon_button();
  goto_schedl_monitoring_screen();
  clk_on_endtime_change_button();
  lv_test_mouse_click_at(34, 30); // Click the "Back" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-09-NKD_ACT_008_Clk_schdl_mon.png"),
      "Screenshot comparison failed for IN-09-Scheduled Monitoring btn click");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_Xbtn_on_endtime_screen(void) {
  clk_activate_button();
  toggle_btns_screen();
  nxt_btn_to_exit_alert_screen();
  clk_on_cnts_mon_button();
  goto_schedl_monitoring_screen();
  clk_on_endtime_change_button();
  lv_test_mouse_click_at(445, 30); // Click the "X" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-01-NKD_INT_001-Activate screen.png"),
      "Screenshot comparison failed for IN-01-Activate screen");
}

static void test_up_arrow_on_endtime_screen(void) {
  clk_activate_button();
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
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-17-NKD_ACT_010_up_arws_in_endtime.png"),
      "Screenshot comparison failed for IN-17-Up Arrow on endtime screen");
  lv_test_mouse_click_at(240, 425); // Click the "Save" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-18-NKD_ACT_010_Save_end_time.png"),
      "Screenshot comparison failed for IN-18-Save btn clk & verify endtime change");
  clk_on_endtime_change_button();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-19-NKD_ACT_010_Save_btn_disabled_end_time.png"),
      "Screenshot comparison failed for IN-19-Save btn clk & verify endtime change");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_dwn_arrows_on_endtime_screen(void) {
  clk_activate_button();
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
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-20-NKD_ACT_010_down_arws_in_endtime.png"),
      "Screenshot comparison failed for IN-20-Down Arrows on endtime screen");
  lv_test_mouse_click_at(240, 425); // Click the "Save" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-21-NKD_ACT_010_Save_end_time.png"),
      "Screenshot comparison failed for IN-21-Save btn clk & verify endtime change dwn arrows");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void clk_nxt_to_bed_mode_sensitivity_screen(void) {
  lv_test_mouse_click_at(350, 420); // next button to BMS page
  lv_test_wait(100);
}

static void test_bed_mode_sensitivity(void) {
  clk_activate_button();
  toggle_btns_screen();
  nxt_btn_to_exit_alert_screen();
  clk_nxt_to_bed_mode_sensitivity_screen();
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-07-NKD_ACT_007_BMS_page.png"),
      "Screenshot comparison failed for IN-07-NKD_ACT_007_BMS_page");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void go_frm_activate_to_bms_screen(void) {
  clk_activate_button();
  toggle_btns_screen();
  nxt_btn_to_exit_alert_screen();
  clk_nxt_to_bed_mode_sensitivity_screen();
}

static void test_i_btn_on_bms_screen(void) {
  go_frm_activate_to_bms_screen();
  lv_test_wait(100);
  flow_to_sysinfo_screen();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-05-NKD_ACT_001_Sysinfo.png"),
      "Screenshot comparison failed for IN-05-Sysinfo");
  lv_test_mouse_click_at(446, 30); // Click the "X" button
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-07-NKD_ACT_007_BMS_page.png"),
      "Screenshot comparison failed for IN-07-NKD_ACT_007_BMS_page");
  flow_to_sysinfo_screen();
  lv_test_mouse_click_at(450, 450); // Click the "Close" button
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-07-NKD_ACT_007_BMS_page.png"),
      "Screenshot comparison failed for IN-07-NKD_ACT_007_BMS_page");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_back_btn_on_bed_mode_sensitivity_screen(void) {
  go_frm_activate_to_bms_screen();
  lv_test_wait(100);
  lv_test_mouse_click_at(120, 420); // Click the "Back" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-06-NKD_ACT_001_nxt_btn_exit_alert.png"),
      "Screenshot comparison failed for IN-06-NKD_ACT_001_nxt_btn_exit_alert");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_xbtn_on_bed_mode_sensitivity_screen(void) {
  go_frm_activate_to_bms_screen();
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30); // Click the "X" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-01-NKD_INT_001-Activate screen.png"),
      "Screenshot comparison failed for IN-01-Activate screen");
}

static void test_nxt_btn_on_bms_page(void) {
  go_frm_activate_to_bms_screen();
  lv_test_mouse_click_at(360, 420); // Click the Next button on BMS->BW button
  lv_test_wait(200);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-22-NKD_ACT_011_nxt_btn_to_BWscreen.png"),
      "Screenshot comparison failed for IN-22-BWscreen");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_sliders_on_bms_screen(void) {
  go_frm_activate_to_bms_screen();
  lv_test_wait(100);
  lv_test_mouse_click_at(50, 190); // Click and drag slider to low
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-23-NKD_ACT_012_BMS_Slider_Low.png"),
      "Screenshot comparison failed for IN-23-BMS Slider Low");
  lv_test_mouse_click_at(240, 190); // Click and drag slider to HIgh
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-24-NKD_ACT_012_BMS_Slider_High.png"),
      "Screenshot comparison failed for IN-24-BMS Slider High");
  lv_test_mouse_click_at(430, 190); // Click and drag slider to UltraHigh
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-25-NKD_ACT_012_BMS_Slider_UltraHigh.png"),
      "Screenshot comparison failed for IN-25-BMS Slider UltraHigh");
  lv_test_mouse_click_at(120, 420); // Click the "Back" button
  lv_test_mouse_click_at(350, 420); // Click the next button on exit alert screen to verify whether
                                    // the value set is still same or not
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-25-NKD_ACT_012_BMS_Slider_UltraHigh.png"),
      "Screenshot comparison failed for IN-25-BMS Slider UltraHigh");
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
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-22-NKD_ACT_011_nxt_btn_to_BWscreen.png"),
      "Screenshot comparison failed for IN-22-BWscreen");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_i_btn_on_BW_screen(void) {
  go_frm_activate_to_bms_screen();
  go_to_BW_screen();
  flow_to_sysinfo_screen();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-05-NKD_ACT_001_Sysinfo.png"),
      "Screenshot comparison failed for IN-05-Sysinfo");
  lv_test_mouse_click_at(446, 30); // Click the "X" button
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-22-NKD_ACT_011_nxt_btn_to_BWscreen.png"),
      "Screenshot comparison failed for IN-22-BWscreen");
  flow_to_sysinfo_screen();
  lv_test_mouse_click_at(240, 420); // Click the "Close" button
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-22-NKD_ACT_011_nxt_btn_to_BWscreen.png"),
      "Screenshot comparison failed for IN-22-BWscreen");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_back_btn_on_bed_width_screen(void) {
  go_frm_activate_to_bms_screen();
  go_to_BW_screen();
  lv_test_mouse_click_at(120, 420); // Click the "Back" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-07-NKD_ACT_007_BMS_page.png"),
      "Screenshot comparison failed for IN-07-NKD_ACT_007_BMS_page");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_xbtn_on_bed_width_screen(void) {
  go_frm_activate_to_bms_screen();
  go_to_BW_screen();
  lv_test_mouse_click_at(445, 30); // Click the "X" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-01-NKD_INT_001-Activate screen.png"),
      "Screenshot comparison failed for IN-01-Activate screen");
}

static void test_next_button_on_BW_screen(void) {
  go_frm_activate_to_bms_screen();
  go_to_BW_screen();
  lv_test_mouse_click_at(360, 420); // Click the Next button on BW->Bed Placement button
  lv_test_wait(200);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-26-NKD_ACT_013_nxt_btn_to_Bed_Placementscreen.png"),
      "Screenshot comparison failed for IN-26-Bed_placement_screen");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void go_to_bed_placement_screen(void) {
  lv_test_mouse_click_at(360, 420); // Click the Next button on BW->Bed Placement button
  lv_test_wait(200);
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
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-27-NKD_ACT_014_BW_Slider_standard.png"),
      "Screenshot comparison failed for IN-27-BW Slider Low");
  lv_test_mouse_click_at(240, 190); // Click and drag slider to Medium
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-28-NKD_ACT_014_BW_Slider_Wide.png"),
      "Screenshot comparison failed for IN-28-BW Slider Medium");
  lv_test_mouse_click_at(430, 190); // Click and drag slider to High
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare(
                               "ref_imgsNKD/NKD-O/NKD-ACT/IN-29-NKD_ACT_014_BW_Slider_Custom.png"),
                           "Screenshot comparison failed for IN-29-BW Slider Custom");
  lv_test_mouse_click_at(120, 420); // Click the "Back" button
  lv_test_mouse_click_at(360, 420); // Click next and verify the custom value stays same or not
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare(
                               "ref_imgsNKD/NKD-O/NKD-ACT/IN-29-NKD_ACT_014_BW_Slider_Custom.png"),
                           "Screenshot comparison failed for IN-29-BW Slider Custom");
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
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-30-NKD_ACT_015_-BW_Change_width_Custom.png"),
      "Screenshot comparison failed for IN-30-BW Change width Custom");
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
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare(
                               "ref_imgsNKD/NKD-O/NKD-ACT/IN-29-NKD_ACT_014_BW_Slider_Custom.png"),
                           "Screenshot comparison failed for IN-29-BW Slider Custom");
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
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-01-NKD_INT_001-Activate screen.png"),
      "Screenshot comparison failed for IN-01-Activate screen");
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
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-30-NKD_ACT_015_-BW_Change_width_Custom.png"),
      "Screenshot comparison failed for IN-30-BW Change width Custom");
  lv_test_mouse_click_at(32, 34); // click on back
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare(
                               "ref_imgsNKD/NKD-O/NKD-ACT/IN-29-NKD_ACT_014_BW_Slider_Custom.png"),
                           "Screenshot comparison failed for IN-29-BW Slider Custom");
  lv_test_mouse_click_at(335, 270);
  lv_test_mouse_click_at(340, 160);
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-31-NKD_ACT_016_BW_Change_width_Queen.png"),
      "Screenshot comparison failed for IN-31-BW Change width Queen");
  lv_test_mouse_click_at(32, 34); // click on back
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-32-NKD_ACT_016_Save_BW_CustomQueen_width.png"),
      "Screenshot comparison failed for IN-32-Save_BW_CustomQueen_width");
  lv_test_mouse_click_at(335, 270);
  lv_test_mouse_click_at(140, 240);
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-33-NKD_ACT_016-BW_Change_width_CaliKing.png"),
      "Screenshot comparison failed for IN-33-BW Change width Cali King");
  lv_test_mouse_click_at(32, 34); // click on back
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-34-NKD_ACT_016_Save_BW_CustomCaliKing_width.png"),
      "Screenshot comparison failed for IN-34-Save_BW_CustomCaliKing_width");
  lv_test_mouse_click_at(335, 270);
  lv_test_mouse_click_at(330, 240);
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-35-NKD_ACT_016_BW_Change_width_King.png"),
      "Screenshot comparison failed for IN-35-BW Change width King");
  lv_test_mouse_click_at(32, 34); // click on back
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-36-NKD_ACT_016_Save_BW_CustomKing_width.png"),
      "Screenshot comparison failed for IN-36-Save_BW_CustomKing_width");
  lv_test_mouse_click_at(120, 420); // Click the "Back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(360, 420);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-36-NKD_ACT_016_Save_BW_CustomKing_width.png"),
      "Screenshot comparison failed for IN-36-Save_BW_CustomKing_width");
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
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-37-NKD_ACT_016_BW_Custom_Value.png"),
      "Screenshot comparison failed for IN-37-BW Change width Custom Value");
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
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-38-NKD_ACT_017_BW_Custom_Value_Numpad_Input.png"),
      "Screenshot comparison failed for IN-38-BW Custom Value Numpad Input");
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
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-39-NKD_ACT_017_BW_Custom_Value_Numpad_all_digits.png"),
      "Screenshot comparison failed for IN-39-BW Custom Value Numpad Input");
  lv_test_wait(150);
  lv_test_mouse_click_at(80, 390); // Click the "backspace" button
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-40-NKD_ACT_017_BW_Custom_Value_Numpad_backspace.png"),
      "Screenshot comparison failed for IN-40-BW Custom Value Numpad backspace");
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
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-37-NKD_ACT_016_BW_Custom_Value.png"),
      "Screenshot comparison failed for IN-37-BW Change width Custom Value");
  lv_test_wait(150);
  lv_test_mouse_click_at(400, 230); // Click '4'
  lv_test_mouse_click_at(80, 310);  // Click '5'
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-41-NKD_ACT_017_BW_Custom_Valid_Value.png"),
      "Screenshot comparison failed for IN-41-BW Custom Value Entered");
  lv_test_mouse_click_at(400, 400); // Click the "OK" button
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-42-NKD_ACT_017_BW_Custom_Value_Saved.png"),
      "Screenshot comparison failed for IN-42-BW Custom Value Saved");
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
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-43-NKD_ACT_017_BW_Custom_bed_width_page.png"),
      "Screenshot comparison failed for IN-43-BW Change width King");
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
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-01-NKD_INT_001-Activate screen.png"),
      "Screenshot comparison failed for IN-01-Activate screen");
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
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-44-NKD_ACT_018_BW_Custom_Value_minimum_Toastmsg.png"),
      "Screenshot comparison failed for IN-44-BW Custom Value MIN TOAST");
  lv_test_mouse_click_at(445, 37); // close toast msg
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-45-NKD_ACT_018_BW_Close_minimum_Toastmsg.png"),
      "Screenshot comparison failed for IN-45-BW Close MIN TOAST");
  lv_test_mouse_click_at(400, 400);
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-44-NKD_ACT_018_BW_Custom_Value_minimum_Toastmsg.png"),
      "Screenshot comparison failed for IN-44-BW Custom Value MIN TOAST");
  lv_test_mouse_click_at(445, 37);  // close toast msg
  lv_test_mouse_click_at(80, 390);  // Click the "backspace" button
  lv_test_mouse_click_at(80, 390);  // Click the "backspace" button
  lv_test_mouse_click_at(390, 310); // Click '8'
  lv_test_mouse_click_at(190, 390); // Click '9'
  lv_test_mouse_click_at(400, 400); // Click the "tick" button & let toast message appear
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-46-NKD_ACT_018_BW_Custom_Value_maximum_Toastmsg.png"),
      "Screenshot comparison failed for IN-46-BW Custom Value MAX TOAST");
  lv_test_mouse_click_at(445, 37); // close toast msg
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-47-NKD_ACT_018_BW_Close_maximum_Toastmsg.png"),
      "Screenshot comparison failed for IN-47-BW Close MAX TOAST");
  lv_test_mouse_click_at(400, 400); // Click the "tick" button & let toast message appear
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-46-NKD_ACT_018_BW_Custom_Value_maximum_Toastmsg.png"),
      "Screenshot comparison failed for IN-46-BW Custom Value MAX TOAST");
  lv_test_mouse_click_at(445, 37); // close toast msg
  lv_test_wait(100);
  lv_test_mouse_click_at(34, 30); // Click the "Back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30); // Click the "X" button
}

static void test_bed_placement_screen(void) {
  go_frm_activate_to_bms_screen();
  lv_test_wait(100);
  go_to_BW_screen();
  lv_test_wait(100);
  go_to_bed_placement_screen();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-26-NKD_ACT_013_nxt_btn_to_Bed_Placementscreen.png"),
      "Screenshot comparison failed for IN-26-Bed_placement_screen");
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
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-05-NKD_ACT_001_Sysinfo.png"),
      "Screenshot comparison failed for IN-05-Sysinfo");
  lv_test_mouse_click_at(446, 30); // Click the "X" button
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-26-NKD_ACT_013_nxt_btn_to_Bed_Placementscreen.png"),
      "Screenshot comparison failed for IN-26-Bed_placement_screen");
  flow_to_sysinfo_screen();
  lv_test_mouse_click_at(450, 450); // Click the "Close" button
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-26-NKD_ACT_013_nxt_btn_to_Bed_Placementscreen.png"),
      "Screenshot comparison failed for IN-26-Bed_placement_screen");
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
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-01-NKD_INT_001-Activate screen.png"),
      "Screenshot comparison failed for IN-01-Activate screen");
}

static void test_back_button_on_bed_placement_screen(void) {
  go_frm_activate_to_bms_screen();
  go_to_BW_screen();
  go_to_bed_placement_screen();
  lv_test_mouse_click_at(120, 420); // Click the "Back" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-27-NKD_ACT_014_BW_Slider_standard.png"),
      "Screenshot comparison failed for IN-27-BW Slider Low");
  lv_test_wait(150);
  lv_test_mouse_click_at(445, 30);
}

static void test_nxt_btn_on_bed_placement_screen(void) {
  go_frm_activate_to_bms_screen();
  go_to_BW_screen();
  go_to_bed_placement_screen();
  lv_test_mouse_click_at(360, 420); // Click the Next button on Bed Placement->Occupant Size
  lv_test_wait(200);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-48-NKD_ACT_019_occupant_size.png"),
      "Screenshot comparison failed for IN-48-Occupant_Size");
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
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-49-NKD_ACT_020_Bed_Placement_Slider_Left.png"),
      "Screenshot comparison failed for IN-49-Bed Placement Slider Left");
  lv_test_mouse_click_at(240, 190); // Click and drag slider to centre
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-50-NKD_ACT_020_Bed_Placement_Slider_Centre.png"),
      "Screenshot comparison failed for IN-50-Bed Placement Slider Centre");
  lv_test_mouse_click_at(430, 190); // Click and drag slider to right wall
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-51-NKD_ACT_020_Bed_Placement_Slider_Right.png"),
      "Screenshot comparison failed for IN-51-Bed Placement SliderRIght");
  lv_test_mouse_click_at(120, 420); // Click the "Back" button
  lv_test_mouse_click_at(
      360, 420); // click next from BW screen to BP screen to check values stay same or not
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-51-NKD_ACT_020_Bed_Placement_Slider_Right.png"),
      "Screenshot comparison failed for IN-51-Bed Placement SliderRIght");
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
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-48-NKD_ACT_019_occupant_size.png"),
      "Screenshot comparison failed for IN-48-Occupant_Size");
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
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-05-NKD_ACT_001_Sysinfo.png"),
      "Screenshot comparison failed for IN-05-Sysinfo");
  lv_test_mouse_click_at(446, 30); // Click the "X" button
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-48-NKD_ACT_019_occupant_size.png"),
      "Screenshot comparison failed for IN-48-Occupant_Size");
  flow_to_sysinfo_screen();
  lv_test_mouse_click_at(450, 450); // Click the "Close" button
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-48-NKD_ACT_019_occupant_size.png"),
      "Screenshot comparison failed for IN-48-Occupant_Size");
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
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-01-NKD_INT_001-Activate screen.png"),
      "Screenshot comparison failed for IN-01-Activate screen");
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
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-26-NKD_ACT_013_nxt_btn_to_Bed_Placementscreen.png"),
      "Screenshot comparison failed for IN-26-Bed_placement_screen");
  lv_test_wait(150);
  lv_test_mouse_click_at(445, 30);
}

static void test_nxt_btn_on_occupant_size_screen(void) {
  go_frm_activate_to_bms_screen();
  go_to_BW_screen();
  go_to_bed_placement_screen();
  go_to_occupant_size_screen();
  lv_test_mouse_click_at(350, 420);
  lv_test_wait(200);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-52-NKD_ACT_020_In_room_alert_audio.png"),
      "Screenshot comparison failed for IN-52-in_room_alert_audio");
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
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-53-NKD_ACT_022_Occupant_Size_Slider_Small.png"),
      "Screenshot comparison failed for IN-53-Occupant Size Slider Small");
  lv_test_mouse_click_at(240, 190); // Click and drag slider to standard
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-54-NKD_ACT_022_Occupant_Size_Slider_Standard.png"),
      "Screenshot comparison failed for IN-54-Occupant Size Slider Standard");
  lv_test_mouse_click_at(430, 190); // Click and drag slider to Large
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-55-NKD_ACT_022_Occupant_Size_Slider_Large.png"),
      "Screenshot comparison failed for IN-55-Occupant Size Slider Large");
  lv_test_mouse_click_at(120, 420); // Click the "Back" button
  lv_test_mouse_click_at(360, 420);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-55-NKD_ACT_022_Occupant_Size_Slider_Large.png"),
      "Screenshot comparison failed for IN-55-Occupant Size Slider Large");
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
  lv_test_wait(200);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-52-NKD_ACT_020_In_room_alert_audio.png"),
      "Screenshot comparison failed for IN-52-in_room_alert_audio");
  lv_test_wait(150);
  lv_test_mouse_click_at(445, 30);
}

static void test_i_button_on_in_room_alert_screen(void) {
  go_frm_activate_to_in_room_alert_screen();
  flow_to_sysinfo_screen();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-05-NKD_ACT_001_Sysinfo.png"),
      "Screenshot comparison failed for IN-05-Sysinfo");
  lv_test_mouse_click_at(446, 30); // Click the "X" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-52-NKD_ACT_020_In_room_alert_audio.png"),
      "Screenshot comparison failed for IN-52-in_room_alert_audio");
  flow_to_sysinfo_screen();
  lv_test_mouse_click_at(450, 450); // Click the "Close" button
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-52-NKD_ACT_020_In_room_alert_audio.png"),
      "Screenshot comparison failed for IN-52-in_room_alert_audio");
  lv_test_wait(150);
  lv_test_mouse_click_at(445, 30);
}

static void test_X_button_on_in_room_alert_screen(void) {
  go_frm_activate_to_in_room_alert_screen();
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30); // Click the "X" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-01-NKD_INT_001-Activate screen.png"),
      "Screenshot comparison failed for IN-01-Activate screen");
}

static void test_back_button_on_in_room_alert_screen(void) {
  go_frm_activate_to_in_room_alert_screen();
  lv_test_mouse_click_at(120, 420); // Click the "Back" button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-48-NKD_ACT_019_occupant_size.png"),
      "Screenshot comparison failed for IN-48-Occupant_Size");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_in_room_alert_audio_toggle(void) {
  go_frm_activate_to_in_room_alert_screen();
  lv_test_mouse_click_at(430, 110);
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-56-NKD_ACT_024_In_Room_Alert_Audio_Toggle.png"),
      "Screenshot comparison failed for IN-56-In Room Alert Audio Toggle");
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
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-57-NKD_ACT_025_Languages_page_without_command.png"),
      "Screenshot comparison failed for IN-57-languages page without command");
  lv_test_mouse_click_at(32, 34);
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-58-NKD_ACT_025_Languages_page_without_command.png"),
      "Screenshot comparison failed for IN-58-languages page without command");
  lv_test_mouse_click_at(340, 210); // Click on language button
  lv_test_wait(4000);
  lv_test_mouse_click_at(140, 160);
  lv_test_wait(1000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-59-NKD_ACT_025_Languages_page_English_command.png"),
      "Screenshot comparison failed for IN-59-languages page English command");
  lv_test_mouse_click_at(340, 160);
  lv_test_wait(1000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-60-NKD_ACT_025_Languages_page_Spanish_command.png"),
      "Screenshot comparison failed for IN-60-languages page Spanish command");
  lv_test_mouse_click_at(32, 34);
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-61-NKD_ACT_025_Spanish_Saved.png"),
      "Screenshot comparison failed for IN-61-Spanish_Saved.png");
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
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/"
                                 "IN-62-NKD_ACT_025_In-Room_Alert_Language_Selection_Screen.png"),
      "Screenshot comparison failed for IN-62-NKD_ACT_025_In Room Alert Language Selection Screen");
  lv_test_wait(250);
  lv_test_mouse_click_at(130, 170); // Click the Arabic language button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/"
          "IN-63-NKD_ACT_025_In-Room_Alert_Arabic_Language_Selected_Screen.png"),
      "Screenshot comparison failed for IN-63-In Room Alert Arabic Language Selected Screen");
  lv_test_mouse_click_at(345, 165); // Click the Cantonese language button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/"
          "IN-64-NKD_ACT_025_In-Room_Alert_Cantonese_Language_Selected_Screen.png"),
      "Screenshot comparison failed for IN-64-In Room Alert Cantonese Language Selected Screen");
  lv_test_mouse_click_at(135, 235); // Click the English button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/"
          "IN-65-NKD_ACT_025_In-Room_Alert_English_Language_Selected_Screen.png"),
      "Screenshot comparison failed for IN-65-In Room Alert English Language Selected Screen");
  lv_test_mouse_click_at(340, 240); // Click the French button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/"
          "IN-66-NKD_ACT_025_In-Room_Alert_French_Language_Selected_Screen.png"),
      "Screenshot comparison failed for IN-66-In Room Alert French Language Selected Screen");
  lv_test_mouse_click_at(140, 310); // Click the Haitian Creole button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/"
          "IN-67-NKD_ACT_025_In-Room_Alert_Haitian_Language_Selected_Screen.png"),
      "Screenshot comparison failed for IN-67-In Room Alert Haitian Language Selected "
      "Screen");
  lv_test_mouse_click_at(350, 310); // Click the Gujarati button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/"
          "IN-68-NKD_ACT_025_In-Room_Alert_Creole_Language_Selected_Screen.png"),
      "Screenshot comparison failed for IN-68-In Room Alert Creole Language Selected Screen");
  lv_test_mouse_click_at(390, 380); // Click the Next page button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/"
          "IN-69-NKD_ACT_025_In-Room_Alert__Language_2ndpage_Selected_Screen.png"),
      "Screenshot comparison failed for IN-69-In Room Alert 2nd Page Selected Screen");
  lv_test_mouse_click_at(130, 170); // Click the Hindi button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/"
          "IN-70-NKD_ACT_025_In-Room_Alert_Gujarati_Language_Selected_Screen.png"),
      "Screenshot comparison failed for IN-70-In Room Alert GUjarati Language Selected Screen");
  lv_test_mouse_click_at(350, 170); // Click the Korean button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/"
          "IN-71-NKD_ACT_025_In-Room_Alert_Hindi_Language_Selected_Screen.png"),
      "Screenshot comparison failed for IN-71-In Room Alert HIndi Language Selected Screen");
  lv_test_mouse_click_at(130, 240); // Click the Mandarin button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/"
          "IN-72-NKD_ACT_025_In-Room_Alert_Korean_Language_Selected_Screen.png"),
      "Screenshot comparison failed for IN-72-In Room Alert Korean Language Selected Screen");
  lv_test_mouse_click_at(350, 240); // Click the Mandarin Button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/"
                                 "IN-73-NKD_ACT_025_In-Room_Alert_Manadarin_Language_Selected.png"),
      "Screenshot comparison failed for IN-73-In Room Alert Language");
  lv_test_mouse_click_at(34, 35); // Click back to check mandarin is saved or not
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-74-NKD_ACT_025_In-Room_Alert_Manadarin_Language_Saved.png"),
      "Screenshot comparison failed for IN-74-In Room Alert Language");
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
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-58-NKD_ACT_025_Languages_page_without_command.png"),
      "Screenshot comparison failed for IN-58-languages page without command");
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
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-01-NKD_INT_001-Activate screen.png"),
      "Screenshot comparison failed for IN-01-Activate screen");
}

static void test_volume_slider_in_room_alert_screen(void) {
  go_frm_activate_to_in_room_alert_screen();
  lv_test_wait(200);
  lv_test_mouse_click_at(50, 290); // Click and drag slider to Low
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-75-NKD_ACT_026_In_Room_Alert_Volume_Slider_Low.png"),
      "Screenshot comparison failed for IN-75-In Room Alert Volume Slider Low");
  lv_test_mouse_click_at(240, 290); // Click and drag slider to Medium
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-76-NKD_ACT_026_In_Room_Alert_Volume_Slider_Medium.png"),
      "Screenshot comparison failed for IN-76-In Room Alert Volume Slider Medium");
  lv_test_mouse_click_at(430, 290); // Click and drag slider to High
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-77-NKD_ACT_026_In_Room_Alert_Volume_Slider_High.png"),
      "Screenshot comparison failed for IN-77-In Room Alert Volume Slider High");
  lv_test_mouse_click_at(120, 420); // Click the "Back" button
  lv_test_mouse_click_at(350, 420);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-77-NKD_ACT_026_In_Room_Alert_Volume_Slider_High.png"),
      "Screenshot comparison failed for IN-77-In Room Alert Volume Slider High");
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
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-78-NKD_ACT_27_Activating_Screen_with_value_7.png"),
      "Screenshot comparison failed for IN-78-Activating_Screen_with_value_7");
  lv_test_wait(60000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-79-NKD_ACT_27_Activating_Screen_Timeout.png"),
      "Screenshot comparison failed for IN-79-Activating_Screen_timeout");
  lv_test_mouse_click_at(360, 420); // Click the Activate button on In-Room Alert.
  flow_to_activating_screen(7);
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-78-NKD_ACT_27_Activating_Screen_with_value_7.png"),
      "Screenshot comparison failed for IN-78-Activating_Screen_with_value_7");
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
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-80-NKD_ACT_27_Activating_Screen_with_value_2.png"),
      "Screenshot comparison failed for IN-80-Activating_Screen_with_value_2");
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
          "ref_imgsNKD/NKD-O/NKD-MON/IN-01-NKD_MON_001_Bed_Monitoring_Screen.png"),
      "Screenshot comparison failed for IN-80-Activating_Screen_with_value_2");
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
          "ref_imgsNKD/NKD-O/NKD-MON/IN-02-NKD_MON_002_Bed_Mode_Screen_Toast.png"),
      "Screenshot comparison failed for IN-02-NKD_MON_002_Bed_Mode_Screen.png");
  lv_test_wait(2000);
  lv_test_mouse_click_at(100, 50);
  lv_test_wait(200);
  flow_to_activate_bed_mode();
  lv_test_wait(1250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-MON/IN-03-NKD_MON_002_Bed_Mode_Screen.png"),
      "Screenshot comparison failed for IN-03-NKD_MON_002_Bed_Mode_Screen.png");
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
          "ref_imgsNKD/NKD-O/NKD-MON/IN-04-NKD_MON_003_Chair_Mode_Screen_Toast.png"),
      "Screenshot comparison failed for IN-04-NKD_MON_003_Chair_Mode_Screen.png");
  lv_test_wait(2000);
  lv_test_mouse_click_at(270, 50);
  lv_test_wait(200);
  flow_to_activate_chair_mode();
  lv_test_wait(1250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-MON/IN-05-NKD_MON_003_Chair_Mode_Screen.png"),
      "Screenshot comparison failed for IN-05-NKD_MON_003_Chair_Mode_Screen.png");
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
          "ref_imgsNKD/NKD-O/NKD-MON/IN-06-NKD_MON_004_Pause_sensor_Screen_Toast.png"),
      "Screenshot comparison failed for IN-06-NKD_MON_004_Pause_sensor_Screen.png");
  lv_test_wait(2000);
  lv_test_mouse_click_at(120, 420);
  lv_test_wait(100);
  flow_to_pause_sensor();
  lv_test_wait(1250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-MON/IN-07-NKD_MON_004_Pause_sensor_Screen.png"),
      "Screenshot comparison failed for IN-07-NKD_MON_004_Pause_sensor_Screen.png");
  lv_test_mouse_click_at(350, 420);
  lv_test_wait(300);
  flow_to_pause_sensor();
  lv_test_wait(1250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-MON/IN-07-NKD_MON_004_Pause_sensor_Screen.png"),
      "Screenshot comparison failed for IN-07-NKD_MON_004_Pause_sensor_Screen.png");
}

static void test_settings_icon(void) {
  lv_test_wait(150);
  lv_test_mouse_click_at(420, 55);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-MON/IN-08-NKD_MON_005_Activated_settings_Screen.png"),
      "Screenshot comparison failed for IN-08-NKD_MON_005_Activated_settings_Screen.png");
  lv_test_mouse_click_at(445, 30); // Click the "X" button
}

static void test_Presence_of_Chair_Mon_screen(void) {
  lv_test_mouse_click_at(270, 50);
  lv_test_wait(200);
  flow_to_activate_chair_mode();
  lv_test_wait(1250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-MON/IN-05-NKD_MON_003_Chair_Mode_Screen.png"),
      "Screenshot comparison failed for IN-05-NKD_MON_003_Chair_Mode_Screen.png");
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
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-MON/"
                                 "IN-09-NKD_MON_007_Pressure_Injury_Risk_Screen.png"),
      "Screenshot comparison failed for IN-09-NKD_MON_007_Pressure_Injury_Risk_Screen.png");
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
          "ref_imgsNKD/NKD-O/NKD-MON/IN-10-NKD_MON_008_Schedule_Monitoring_Screen.png"),
      "Screenshot comparison failed for IN-10-NKD_MON_008_Schedule_Monitoring_Screen.png");
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
          "ref_imgsNKD/NKD-O/NKD-MON/IN-11-NKD_MON_009_Fall_Monitoring_Screen.png"),
      "Screenshot comparison failed for IN-11-NKD_MON_009_Fall_Monitoring_Screen.png");
}

static void test_bed_exit_alert_withall3_options(void) {
  const char *refs[4] = {
      NULL, "ref_imgsNKD/NKD-O/NKD_ALE/IN-01-NKD_ALE_001-Bed_Exit_Alert_low.png",
      "ref_imgsNKD/NKD-O/NKD_ALE/IN-02-NKD_ALE_001-Bed_Exit_Alert_High.png",
      "ref_imgsNKD/NKD-O/NKD_ALE/IN-03-NKD_ALE_001-Bed_Exit_Alert_UltraHigh.png"};

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
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare(
                               "ref_imgsNKD/NKD-O/NKD_ALE/IN-04-NKD_ALE_002-Chair_Exit_Alert.png"),
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
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD_ALE/IN-05-NKD_ALE_003-Fall_Alert.png"),
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
          "ref_imgsNKD/NKD-O/NKD_ALE/IN-06-NKD_ALE_004-Fall_Alert_with_bed_chair.png"),
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
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare(
                               "ref_imgsNKD/NKD-O/NKD_ALE/IN-07-NKD_ALE_005-Reposition_Alert.png"),
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
          "ref_imgsNKD/NKD-O/NKD_ALE/IN-08-NKD_ALE_006-Reposition_Alert_with_bed_chair.png"),
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
          "ref_imgsNKD/NKD-O/NKD_ALE/IN-09-NKD_ALE_007-paused_error_screen.png"),
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

  ;

  write_response(WR_FD, resp, (size_t)len);
  process_request(50);
  lv_test_wait(50);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_ALE/IN-10-NKD_ALE_008-calibration_error_screen.png"),
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
          "ref_imgsNKD/NKD-O/NKD_ALE/IN-11-NKD_ALE_009-chair_calibration_error_screen.png"),
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
          "ref_imgsNKD/NKD-O/NKD_ALE/IN-12-NKD_ALE_010-Incorrect_Mode_Error_Screen.png"),
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
          "ref_imgsNKD/NKD-O/NKD_ALE/IN-13-NKD_ALE_011-Unassigned_Error_Screen.png"),
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
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_ALE/IN-14-NKD_ALE_012-Sunlight_Interference_Error_Screen.png"),
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
          "ref_imgsNKD/NKD-O/NKD_ALE/IN-15-NKD_ALE_013-Obstructed_Sensor_Error_Screen.png"),
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
          "ref_imgsNKD/NKD-O/NKD_ALE/IN-16-NKD_ALE_014-System_Disconnected_Error_Screen.png"),
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
          "ref_imgsNKD/NKD-O/NKD_ALE/IN-17-NKD_ALE_015-Not_Monitoring_Screen.png"),
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
  setv_variant_cmd();
  send_set_state_with_fts_avail(127);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon on Non-Monitoring screen.
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  lv_test_wait(100);
  lv_test_mouse_click_at(446, 30); // X
}

static void test_i_btn_on_settings_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  flow_to_sysinfo_screen();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-05-NKD_ACT_001_Sysinfo.png"),
      "Screenshot comparison failed for IN-05-Sysinfo");
  lv_test_mouse_click_at(450, 450); // Close button
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  flow_to_sysinfo_screen();
  lv_test_mouse_click_at(446, 30); // X
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
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
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-80-NKD_ACT_27_Activating_Screen_with_value_2.png"),
      "Screenshot comparison failed for IN-80-Activating_Screen_with_value_2");
}

static void test_optional_settings_available(void) {
  send_set_state_with_fts_avail(127);
  lv_test_wait(100);
  lv_test_mouse_click_at(420, 50);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  lv_test_mouse_click_at(100, 110); // Click the Display settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare(
                               "ref_imgsNKD/NKD-O/NKD_SET/IN-02-NKD_SET_002-Display_Settings.png"),
                           "Screenshot comparison failed for Display Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(335, 115); // click on In-Room-Alert Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-03-NKD_SET_002-In-Room_Alert_Settings.png"),
      "Screenshot comparison failed for In-Room Alert Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(140, 190); // click on Alerts Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD_SET/IN-04-NKD_SET_002-Alerts_Settings.png"),
      "Screenshot comparison failed for Alerts Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(340, 185); // Click the "Exit Alert" button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-05-NKD_SET_002-Exit_Alert_Settings.png"),
      "Screenshot comparison failed for Exit Alert Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(110, 260); // Click the "Bed Sensitivity" button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-06-NKD_SET_002-Bed_Sensitivity_Settings.png"),
      "Screenshot comparison failed for Bed Sensitivity Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(350, 255); // Click the "Bed width" button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-07-NKD_SET_002-Bed_Width_Settings.png"),
      "Screenshot comparison failed for Bed Width Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(120, 330); // Click the "Bed Placement" button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-08-NKD_SET_002-Bed_Placement_Settings.png"),
      "Screenshot comparison failed for Bed Placement Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(345, 330); // Click the "Occupant Size" button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-09-NKD_SET_002-Occupant_Size_Settings.png"),
      "Screenshot comparison failed for Occupant Size Settings screen");
  lv_test_mouse_click_at(445, 30);
  lv_test_wait(500);
  send_set_state_with_fts_avail(9);
  lv_test_wait(1000);
  lv_test_mouse_click_at(420, 50);
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-10-NKD_SET_002-optional_settings_with_fts_9.png"),
      "Screenshot comparison failed for optional settings with fts 9");
  lv_test_mouse_click_at(100, 110); // Click the Display settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare(
                               "ref_imgsNKD/NKD-O/NKD_SET/IN-02-NKD_SET_002-Display_Settings.png"),
                           "Screenshot comparison failed for Display Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(335, 115); // click on In-Room-Alert Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-03-NKD_SET_002-In-Room_Alert_Settings.png"),
      "Screenshot comparison failed for In-Room Alert Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(140, 190); // click on Alerts Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-11-NKD_SET_002-Alerts_Settings_with_fts_9_to_121.png"),
      "Screenshot comparison failed for Alerts Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(340, 185); // Click the "Exit Alert" button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-05-NKD_SET_002-Exit_Alert_Settings.png"),
      "Screenshot comparison failed for Exit Alert Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(110, 260); // Click the "Bed Sensitivity" button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-06-NKD_SET_002-Bed_Sensitivity_Settings.png"),
      "Screenshot comparison failed for Bed Sensitivity Settings screen");
  lv_test_mouse_click_at(445, 30);
  lv_test_wait(500);
  send_set_state_with_fts_avail(17);
  lv_test_wait(1000);
  lv_test_mouse_click_at(420, 50);
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-12-NKD_SET_002-optional_settings_with_fts_17.png"),
      "Screenshot comparison failed for optional settings with fts 17");
  lv_test_mouse_click_at(100, 110); // Click the Display settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare(
                               "ref_imgsNKD/NKD-O/NKD_SET/IN-02-NKD_SET_002-Display_Settings.png"),
                           "Screenshot comparison failed for Display Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(335, 115); // click on In-Room-Alert Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-03-NKD_SET_002-In-Room_Alert_Settings.png"),
      "Screenshot comparison failed for In-Room Alert Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(140, 190); // click on Alerts Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-11-NKD_SET_002-Alerts_Settings_with_fts_9_to_121.png"),
      "Screenshot comparison failed for Alerts Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(340, 180); // Click the "Bed Sensitivity" button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-06-NKD_SET_002-Bed_Sensitivity_Settings.png"),
      "Screenshot comparison failed for Bed Sensitivity Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(100, 260); // Click the "Bed Placement" button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-08-NKD_SET_002-Bed_Placement_Settings.png"),
      "Screenshot comparison failed for Bed Placement Settings screen");
  lv_test_mouse_click_at(445, 30);
  lv_test_wait(500);
  send_set_state_with_fts_avail(33);
  lv_test_wait(1000);
  lv_test_mouse_click_at(420, 50);
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-13-NKD_SET_002-optional_settings_with_fts_33.png"),
      "Screenshot comparison failed for optional settings with fts 33");
  lv_test_mouse_click_at(100, 110); // Click the Display settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare(
                               "ref_imgsNKD/NKD-O/NKD_SET/IN-02-NKD_SET_002-Display_Settings.png"),
                           "Screenshot comparison failed for Display Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(335, 115); // click on In-Room-Alert Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-03-NKD_SET_002-In-Room_Alert_Settings.png"),
      "Screenshot comparison failed for In-Room Alert Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(140, 190); // click on Alerts Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-11-NKD_SET_002-Alerts_Settings_with_fts_9_to_121.png"),
      "Screenshot comparison failed for Alerts Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(340, 180); // Click the "Bed Sensitivity" button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-06-NKD_SET_002-Bed_Sensitivity_Settings.png"),
      "Screenshot comparison failed for Bed Sensitivity Settings screen");
  lv_test_mouse_click_at(30, 30);
  lv_test_wait(100);
  lv_test_mouse_click_at(160, 270); // Click the "Bed width" button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-07-NKD_SET_002-Bed_Width_Settings.png"),
      "Screenshot comparison failed for Bed Width Settings screen");
  lv_test_mouse_click_at(445, 30);
  lv_test_wait(500);
  send_set_state_with_fts_avail(65);
  lv_test_wait(1000);
  lv_test_mouse_click_at(420, 50);
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-14-NKD_SET_002-optional_settings_with_fts_65.png"),
      "Screenshot comparison failed for optional settings with fts 65");
  lv_test_mouse_click_at(100, 110); // Click the Display settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare(
                               "ref_imgsNKD/NKD-O/NKD_SET/IN-02-NKD_SET_002-Display_Settings.png"),
                           "Screenshot comparison failed for Display Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(335, 115); // click on In-Room-Alert Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-03-NKD_SET_002-In-Room_Alert_Settings.png"),
      "Screenshot comparison failed for In-Room Alert Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(140, 190); // click on Alerts Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-11-NKD_SET_002-Alerts_Settings_with_fts_9_to_121.png"),
      "Screenshot comparison failed for Alerts Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(340, 180); // Click the "Bed Sensitivity" button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-06-NKD_SET_002-Bed_Sensitivity_Settings.png"),
      "Screenshot comparison failed for Bed Sensitivity Settings screen");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  lv_test_mouse_click_at(160, 270); // Click the "Occupant Size" button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-09-NKD_SET_002-Occupant_Size_Settings.png"),
      "Screenshot comparison failed for Occupant Size Settings screen");
  lv_test_mouse_click_at(445, 30);
  lv_test_wait(100);
  send_set_state_with_fts_avail(1);
  lv_test_wait(100);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-15-NKD_SET_002-optional_settings_with_fts_1.png"),
      "Screenshot comparison failed for optional settings with fts 1");
  lv_test_mouse_click_at(140, 190); // click on Alerts Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-11-NKD_SET_002-Alerts_Settings_with_fts_9_to_121.png"),
      "Screenshot comparison failed for Alerts Settings screen");
  lv_test_mouse_click_at(445, 30);
  lv_test_wait(100);
  send_set_state_with_fts_avail(3);
  lv_test_wait(100);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-15-NKD_SET_002-optional_settings_with_fts_1.png"),
      "Screenshot comparison failed for optional settings with fts 1");
  lv_test_mouse_click_at(140, 190); // click on Alerts Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-17-NKD_SET_002-Alerts_Settings_with_fts_avail_3.png"),
      "Screenshot comparison failed for Alerts Settings screen for fts avail 3");
  lv_test_mouse_click_at(445, 30);
  lv_test_wait(100);
  send_set_state_with_fts_avail(5);
  lv_test_wait(100);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-15-NKD_SET_002-optional_settings_with_fts_1.png"),
      "Screenshot comparison failed for optional settings with fts 1");
  lv_test_mouse_click_at(140, 190); // click on Alerts Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-18-NKD_SET_002-Alerts_Settings_with_fts_avail_5.png"),
      "Screenshot comparison failed for Alerts Settings screen for fts avail 5");
  lv_test_mouse_click_at(445, 30);
  lv_test_wait(100);
  send_set_state_with_fts_avail(7);
  lv_test_wait(100);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-15-NKD_SET_002-optional_settings_with_fts_1.png"),
      "Screenshot comparison failed for optional settings with fts 1");
  lv_test_mouse_click_at(140, 190); // click on Alerts Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD_SET/IN-04-NKD_SET_002-Alerts_Settings.png"),
      "Screenshot comparison failed for Alerts Settings screen");
  lv_test_mouse_click_at(445, 30);
  lv_test_wait(100);
  send_set_state_with_fts_avail(11);
  lv_test_wait(100);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-10-NKD_SET_002-optional_settings_with_fts_9.png"),
      "Screenshot comparison failed for optional settings with fts 9");
  lv_test_mouse_click_at(140, 190); // click on Alerts Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-17-NKD_SET_002-Alerts_Settings_with_fts_avail_3.png"),
      "Screenshot comparison failed for Alerts Settings screen for fts avail 3");
  lv_test_mouse_click_at(445, 30);
  lv_test_wait(100);
  send_set_state_with_fts_avail(13);
  lv_test_wait(100);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-10-NKD_SET_002-optional_settings_with_fts_9.png"),
      "Screenshot comparison failed for optional settings with fts 9");
  lv_test_mouse_click_at(140, 190); // click on Alerts Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-18-NKD_SET_002-Alerts_Settings_with_fts_avail_5.png"),
      "Screenshot comparison failed for Alerts Settings screen for fts avail 5");
  lv_test_mouse_click_at(445, 30);
  lv_test_wait(100);
  send_set_state_with_fts_avail(23);
  lv_test_wait(100);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-12-NKD_SET_002-optional_settings_with_fts_17.png"),
      "Screenshot comparison failed for optional settings with fts 17");
  lv_test_mouse_click_at(140, 190); // click on Alerts Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD_SET/IN-04-NKD_SET_002-Alerts_Settings.png"),
      "Screenshot comparison failed for Alerts Settings screen");
  lv_test_mouse_click_at(445, 30);
  lv_test_wait(100);
  send_set_state_with_fts_avail(41);
  lv_test_wait(100);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-19-NKD_SET_002-optional_settings_with_fts_41.png"),
      "Screenshot comparison failed for optional settings with fts 41");
  lv_test_mouse_click_at(140, 190); // click on Alerts Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-11-NKD_SET_002-Alerts_Settings_with_fts_9_to_121.png"),
      "Screenshot comparison failed for Alerts Settings screen");
  lv_test_mouse_click_at(445, 30);
  lv_test_wait(100);
  send_set_state_with_fts_avail(95);
  lv_test_wait(100);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-20-NKD_SET_002-optional_settings_with_fts_95.png"),
      "Screenshot comparison failed for optional settings with fts 95");
  lv_test_mouse_click_at(140, 190); // click on Alerts Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD_SET/IN-04-NKD_SET_002-Alerts_Settings.png"),
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-06-NKD_SET_002-Bed_Sensitivity_Settings.png"),
      "Screenshot comparison failed for Bed Sensitivity Settings screen");
  lv_test_mouse_click_at(445, 30);
}

static void test_functionality_of_bms_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(110, 260); // Click the "Bed Sensitivity" button
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD_SET/IN-21-NKD_SET_004-BMS_Slider_Low.png"),
      "Screenshot comparison failed for BMS Slider Low");
  lv_test_mouse_click_at(240, 190); // Click and drag slider to HIgh
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD_SET/IN-22-NKD_SET_004_BMS_Slider_High.png"),
      "Screenshot comparison failed for IN-22-BMS Slider High");
  lv_test_mouse_click_at(430, 190); // Click and drag slider to UltraHigh
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-23-NKD_SET_004_BMS_Slider_UltraHigh.png"),
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
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
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-80-NKD_ACT_27_Activating_Screen_with_value_2.png"),
      "Screenshot comparison failed for IN-80-Activating_Screen_with_value_2");
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-24-NKD_SET_006_Save_Button_BMS_Settings_Toast.png"),
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-25-NKD_SET_006_BMS_Saved_with_toast.png"),
      "Screenshot comparison failed for BMS Saved with toast");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-26-NKD_SET_006_BMS_Saved_no_toast.png"),
      "Screenshot comparison failed for BMS Saved no toast");
  lv_test_mouse_click_at(50, 190); // Click and drag slider to Low
  lv_test_wait(250);
  lv_test_mouse_click_at(230, 420); // Click the Save button
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
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-27-NKD_SET_006_BMS_Saved_low_with_toast.png"),
      "Screenshot comparison failed for BMS Saved with toast");
  lv_test_wait(3000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-28-NKD_SET_006_BMS_Saved_low_without_toast.png"),
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
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
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-80-NKD_ACT_27_Activating_Screen_with_value_2.png"),
      "Screenshot comparison failed for IN-80-Activating_Screen_with_value_2");
}

static void test_in_room_audio_alerts_on_settings_flow(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(335, 115); // click on In-Room-Alert Settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-03-NKD_SET_002-In-Room_Alert_Settings.png"),
      "Screenshot comparison failed for In-Room Alert Settings screen");
  lv_test_mouse_click_at(425, 100); // Click on Audio Alerts toggle button
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-29-NKD_SET_008_In-Room_Audio_Alerts_Off.png"),
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
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-80-NKD_ACT_27_Activating_Screen_with_value_2.png"),
      "Screenshot comparison failed for IN-80-Activating_Screen_with_value_2");
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-29-NKD_SET_008_In-Room_Audio_Alerts_Off.png"),
      "Screenshot comparison failed for In-Room Audio Alerts Off");
  lv_test_mouse_click_at(425, 100); // Click on Audio Alerts toggle button
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-03-NKD_SET_002-In-Room_Alert_Settings.png"),
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
  lv_test_wait(4000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-30-NKD_SET_009-Languages_Screen_no_Command.png"),
      "Screenshot comparison failed for Languages Screen");
  lv_test_mouse_click_at(30, 30);
  lv_test_wait(200);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-31-NKD_SET_009-No_Language_Selected.png"),
      "Screenshot comparison failed for No Language");
  lv_test_mouse_click_at(350, 210);
  lv_test_wait(3000);
  lv_test_mouse_click_at(350, 160);
  lv_test_wait(1000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-32-NKD_SET_009-Spanish_Language_Selected.png"),
      "Screenshot comparison failed for Spanish Language");
  lv_test_mouse_click_at(130, 160);
  lv_test_wait(1000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-33-NKD_SET_009-English_Language_Selected.png"),
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-03-NKD_SET_002-In-Room_Alert_Settings.png"),
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
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-80-NKD_ACT_27_Activating_Screen_with_value_2.png"),
      "Screenshot comparison failed for IN-80-Activating_Screen_with_value_2");
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-34-NKD_SET_009-Languages_Screen_with_Command.png"),
      "Screenshot comparison failed for Languages Screen with Command");
  lv_test_mouse_click_at(130, 170); // Click the Arabic language button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-35-NKD_SET_009-Arabic_Language_Selected.png"),
      "Screenshot comparison failed for Arabic Language Selected");
  lv_test_mouse_click_at(345, 165); // Click the Cantonese language button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-36-NKD_SET_009-Cantonese_Language_Selected.png"),
      "Screenshot comparison failed for Cantonese Language Selected");
  lv_test_mouse_click_at(135, 235); // Click the English button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-37-NKD_SET_009-English_Language_Selected.png"),
      "Screenshot comparison failed for English Language Selected");
  lv_test_mouse_click_at(340, 240); // Click the French button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-38-NKD_SET_009-French_Language_Selected.png"),
      "Screenshot comparison failed for French Language Selected");
  lv_test_mouse_click_at(140, 310); // Click the Haitian Creole button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-39-NKD_SET_009-Haitian_Language_Selected.png"),
      "Screenshot comparison failed for Haitian Language Selected");
  lv_test_mouse_click_at(350, 310); // Click the Gujarati button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-40-NKD_SET_009-Creole_Language_Selected.png"),
      "Screenshot comparison failed for Creole Language Selected");
  lv_test_mouse_click_at(390, 380); // Click the Next page button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/"
          "IN-41-NKD_SET_009_In-Room_Alert__Language_2ndpage_Selected_Screen.png"),
      "Screenshot comparison failed for In Room Alert 2nd Page Selected Screen");
  lv_test_mouse_click_at(130, 170); // Click the Hindi button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-42-NKD_SET_009-Gujarati_Language_Selected.png"),
      "Screenshot comparison failed for Gujarati Language Selected");
  lv_test_mouse_click_at(350, 170); // Click the Korean button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/"
          "IN-43-NKD_SET_009_In-Room_Alert_Hindi_Language_Selected_Screen.png"),
      "Screenshot comparison failed for In Room Alert HIndi Language Selected Screen");
  lv_test_mouse_click_at(130, 240); // Click the Mandarin button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/"
          "IN-44-NKD_SET_009_In-Room_Alert_Korean_Language_Selected_Screen.png"),
      "Screenshot comparison failed for IN-38-In Room Alert Korean Language Selected Screen");
  lv_test_mouse_click_at(350, 240); // Click the Mandarin Button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD_SET/"
                                 "IN-45-NKD_SET_009_In-Room_Alert_Manadarin_Language_Selected.png"),
      "Screenshot comparison failed for IN-39-In Room Alert Language");
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(100);
  flow_to_lang_screen(5);
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare(
                               "ref_imgsNKD/NKD-O/NKD_SET/"
                               "IN-46-NKD_SET_009-Languages_Screen_with_less_than_6_languages.png"),
                           "Screenshot comparison failed for Languages Screen with Command");
  lv_test_mouse_click_at(130, 170); // Click the Arabic language button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-47-NKD_SET_009-Arabic_Language_Selected.png"),
      "Screenshot comparison failed for Arabic Language Selected");
  lv_test_mouse_click_at(345, 165); // Click the Cantonese language button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-48-NKD_SET_009-Cantonese_Language_Selected.png"),
      "Screenshot comparison failed for Cantonese Language Selected");
  lv_test_mouse_click_at(135, 235); // Click the English button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-49-NKD_SET_009-English_Language_Selected.png"),
      "Screenshot comparison failed for English Language Selected");
  lv_test_mouse_click_at(340, 240); // Click the French button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-50-NKD_SET_009-French_Language_Selected.png"),
      "Screenshot comparison failed for French Language Selected");
  lv_test_mouse_click_at(140, 310); // Click the Haitian Creole button
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-51-NKD_SET_009-Haitian_Language_Selected.png"),
      "Screenshot comparison failed for Haitian Language Selected");
  lv_test_wait(150);
  lv_test_mouse_click_at(135, 235); // Click the English button
  lv_test_wait(200);
  lv_test_mouse_click_at(30, 30); // Click the "back" button
  lv_test_wait(200);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-31-NKD_SET_009-No_Language_Selected.png"),
      "Screenshot comparison failed for No Language");
  lv_test_wait(100);
  lv_test_mouse_click_at(445, 30);
}

static void test_volume_scroller_on_in_room_alert_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(335, 115); // click on In-Room-Alert Settings
  lv_test_wait(200);
  lv_test_mouse_click_at(50, 290); // Click and drag slider to Low
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare(
                               "ref_imgsNKD/NKD-O/NKD_SET/IN-52-NKD_SET_009_Volume_Slider_Low.png"),
                           "Screenshot comparison failed for Volume Slider Low");
  lv_test_mouse_click_at(240, 290); // Click and drag slider to Medium
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-53-NKD_SET_009_Volume_Slider_Medium.png"),
      "Screenshot comparison failed for Volume Slider Medium");
  lv_test_mouse_click_at(430, 290); // Click and drag slider to High
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-54-NKD_SET_009_Volume_Slider_High.png"),
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
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD_SET/"
                                 "IN-55-NKD_SET_010_Save_Button_In-Room_Alert_Settings_Toast.png"),
      "Screenshot comparison failed for Save Button In-Room Alert Settings Toast");
  lv_test_wait(3000);
  lv_test_mouse_click_at(230, 420);
  lv_test_wait(500);
  saved_in_room_alert();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-56-NKD_SET_010_In-Room_Alert_Saved_with_toast.png"),
      "Screenshot comparison failed for In-Room Alert Saved with toast");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-57-NKD_SET_010_In-Room_Alert_Saved_no_toast.png"),
      "Screenshot comparison failed for In-Room Alert Saved no toast");
  lv_test_mouse_click_at(240, 290); // Click and drag slider to Medium
  lv_test_wait(200);
  lv_test_mouse_click_at(230, 420);
  lv_test_wait(500);
  saved_in_room_alert();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-58-NKD_SET_010_In-Room_Alert_Saved_medium_with_toast.png"),
      "Screenshot comparison failed for In-Room Alert Saved medium with toast");
  lv_test_wait(3000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-59-NKD_SET_010_In-Room_Alert_Saved_toast_timeout.png"),
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-03-NKD_SET_002-In-Room_Alert_Settings.png"),
      "Screenshot comparison failed for In-Room Alert Settings screen");
  lv_test_mouse_click_at(445, 30); // Click the "X" button
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-80-NKD_ACT_27_Activating_Screen_with_value_2.png"),
      "Screenshot comparison failed for IN-80-Activating_Screen_with_value_2");
}

static void test_display_page_on_activated_settings_flow(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(100, 100); // Click the Display settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare(
                               "ref_imgsNKD/NKD-O/NKD_SET/IN-02-NKD_SET_002-Display_Settings.png"),
                           "Screenshot comparison failed for Display Settings Screen");
  lv_test_mouse_click_at(400, 270);
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare(
                               "ref_imgsNKD/NKD-O/NKD_SET/IN-60-NKD_SET_011_Display_Toggle_On.png"),
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
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
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-80-NKD_ACT_27_Activating_Screen_with_value_2.png"),
      "Screenshot comparison failed for IN-80-Activating_Screen_with_value_2");
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-61-NKD_SET_012_Brightness_Slider_Low.png"),
      "Screenshot comparison failed for Brightness Slider Low");
  lv_test_mouse_click_at(390, 155); // Click and drag slider to Medium
  lv_test_wait(200);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-62-NKD_SET_012_Brightness_Slider_Medium.png"),
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-63-NKD_SET_013_Adaptive_Toggle_Off.png"),
      "Screenshot comparison failed for Adaptive Toggle Off");
  lv_test_mouse_click_at(400, 270);
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-64-NKD_SET_013_Adaptive_Toggle_On.png"),
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
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD_SET/"
                                 "IN-65-NKD_SET_014_Save_Button_Display_Settings_Toast.png"),
      "Screenshot comparison failed for Save Button Display Settings Toast");
  lv_test_wait(3000);
  lv_test_mouse_click_at(230, 420);
  saved_display_settings();
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-66-NKD_SET_014_Display_Settings_Saved_with_toast.png"),
      "Screenshot comparison failed for Display Settings Saved with toast");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-67-NKD_SET_014_Display_Settings_Saved_no_toast.png"),
      "Screenshot comparison failed for Display Settings Saved no toast");
  lv_test_mouse_click_at(400, 270); // Click Adaptive toggle button
  lv_test_wait(200);
  lv_test_mouse_click_at(230, 420); // Click the Save button
  saved_display_settings();
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/"
          "IN-68-NKD_SET_014_Display_Settings_Saved_adaptiveOFF_with_toast.png"),
      "Screenshot comparison failed for Display Settings Saved adaptive with toast");
  lv_test_wait(3000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/"
          "IN-69-NKD_SET_014_Display_Settings_Saved_Adaptive0FF_toast_timeout.png"),
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
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
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-80-NKD_ACT_27_Activating_Screen_with_value_2.png"),
      "Screenshot comparison failed for IN-80-Activating_Screen_with_value_2");
}

static void test_Alerts_page_on_settings_flow(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(60, 185); // Click the Alerts settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD_SET/IN-04-NKD_SET_002-Alerts_Settings.png"),
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-70-NKD_SET_015_Toggles_on_Alert_screen.png"),
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
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-80-NKD_ACT_27_Activating_Screen_with_value_2.png"),
      "Screenshot comparison failed for IN-80-Activating_Screen_with_value_2");
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
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
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD_SET/"
                                 "IN-71-NKD_SET_016_Save_Button_Alerts_Settings_Toast.png"),
      "Screenshot comparison failed for Save Button Alerts Settings Toast");
  lv_test_wait(3000);
  lv_test_mouse_click_at(230, 420);
  lv_test_wait(500);
  saved_alerts();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-72-NKD_SET_016_Alerts_Settings_Saved_with_toast.png"),
      "Screenshot comparison failed for Alerts Settings Saved with toast");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-73-NKD_SET_016_Alerts_Settings_Saved_no_toast.png"),
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
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD_SET/IN-04-NKD_SET_002-Alerts_Settings.png"),
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
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
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-80-NKD_ACT_27_Activating_Screen_with_value_2.png"),
      "Screenshot comparison failed for IN-80-Activating_Screen_with_value_2");
}

static void test_exit_alert_schedule_on_settings_flow(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 180); // Click the Alert Schedule settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-05-NKD_SET_002-Exit_Alert_Settings.png"),
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
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
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-80-NKD_ACT_27_Activating_Screen_with_value_2.png"),
      "Screenshot comparison failed for IN-80-Activating_Screen_with_value_2");
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
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD_SET/"
                                 "IN-74-NKD_SET_018_Save_Button_Exit_Alert_Schedule_Toast.png"),
      "Screenshot comparison failed for Save Button Exit Alert Schedule Toast");
  lv_test_wait(3000);
  lv_test_mouse_click_at(230, 420);
  saved_mon_sch();
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-75-NKD_SET_018_Exit_Alert_Schedule_Saved_with_toast.png"),
      "Screenshot comparison failed for Exit Alert Schedule Saved with toast");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-76-NKD_SET_018_Exit_Alert_Schedule_Saved_no_toast.png"),
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
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD_SET/"
                                 "IN-77-NKD_SET_018_Exit_Alert_Schedule_Saved_toast_timeout.png"),
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
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
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-80-NKD_ACT_27_Activating_Screen_with_value_2.png"),
      "Screenshot comparison failed for IN-80-Activating_Screen_with_value_2");
}

static void test_bed_width_screen_on_settings_flow(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 250); // Click the Bed Width settings button
  lv_test_wait(300);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-07-NKD_SET_002-Bed_Width_Settings.png"),
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
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
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-80-NKD_ACT_27_Activating_Screen_with_value_2.png"),
      "Screenshot comparison failed for IN-80-Activating_Screen_with_value_2");
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-78-NKD_SET_019_BW_Slider_Standard.png"),
      "Screenshot comparison failed for IN-78-BW Slider standard");
  lv_test_mouse_click_at(240, 190); // Click and drag slider to Medium
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD_SET/IN-79-NKD_SET_019_BW_Slider_wide.png"),
      "Screenshot comparison failed for IN-79-BW Slider Wide");
  lv_test_mouse_click_at(430, 190); // Click and drag slider to High
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare(
                               "ref_imgsNKD/NKD-O/NKD_SET/IN-80-NKD_SET_019_BW_Slider_Custom.png"),
                           "Screenshot comparison failed for IN-80-BW Slider Custom");
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-81-NKD_SET_019_Custom_Bed_Width_Screen.png"),
      "Screenshot comparison failed for IN-81-Custom_Bed_Width_Screen");
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-07-NKD_SET_002-Bed_Width_Settings.png"),
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
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-80-NKD_ACT_27_Activating_Screen_with_value_2.png"),
      "Screenshot comparison failed for IN-80-Activating_Screen_with_value_2");
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-81-NKD_SET_019_Custom_Bed_Width_Screen.png"),
      "Screenshot comparison failed for IN-81-Custom_Bed_Width_Screen");
  lv_test_mouse_click_at(32, 34); // click on back
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-07-NKD_SET_002-Bed_Width_Settings.png"),
      "Screenshot comparison failed for Bed Width Settings screen");
  lv_test_mouse_click_at(335, 270);
  lv_test_mouse_click_at(120, 160);
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD_SET/"
                                 "IN-82-NKD_SET_019_BW_Change_width_FULL_FULL(XL).png"),
      "Screenshot comparison failed for IN-82-BW Change width FULL(XL)");
  lv_test_mouse_click_at(32, 34); // click on back
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-83-NKD_SET_019_Save_BW_CustomFULL_width.png"),
      "Screenshot comparison failed for IN-83-Save_BW_CustomFULL_width");
  lv_test_mouse_click_at(335, 270);
  lv_test_mouse_click_at(120, 240);
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-84-NKD_SET_019_BW_Change_width_CaliKing.png"),
      "Screenshot comparison failed for IN-84-BW Change width CaliKing");
  lv_test_mouse_click_at(32, 34); // click on back
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-85-NKD_SET_019_Save_BW_CustomCaliKing_width.png"),
      "Screenshot comparison failed for IN-85-Save_BW_CustomCaliKing_width");
  lv_test_mouse_click_at(335, 270);
  lv_test_mouse_click_at(330, 240);
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-86-NKD_SET_019_BW_Change_width_King.png"),
      "Screenshot comparison failed for IN-86-BW Change width King");
  lv_test_mouse_click_at(32, 34); // click on back
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-87-NKD_SET_019_Save_BW_CustomKing_width.png"),
      "Screenshot comparison failed for IN-87-Save_BW_CustomKing_width");
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-88-NKD_SET_019_BW_Custom_Value_Screen.png"),
      "Screenshot comparison failed for IN-88-BW_Custom_Value_Screen");
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-89-NKD_SET_019_Keypad_on_Custom_Value_Screen.png"),
      "Screenshot comparison failed for IN-89-Keypad_on_Custom_Value_Screen");
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
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare(
                               "ref_imgsNKD/NKD-O/NKD_SET/IN-90-NKD_SET_019_Input_Keypad_all.png"),
                           "Screenshot comparison failed for IN-90-Input_Keypad_all");
  lv_test_wait(150);
  lv_test_mouse_click_at(80, 390); // Click the "backspace" button
  lv_test_mouse_click_at(80, 390); // Click the "backspace" button
  lv_test_wait(1000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-91-NKD_SET_019_Numpad_backspace.png"),
      "Screenshot comparison failed for IN-91-BW Custom Value Numpad backspace");
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
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD_SET/IN-92-NKD_SET_019_Custom_Value.png"),
      "Screenshot comparison failed for IN-92-BW Custom Value 45");
  lv_test_mouse_click_at(400, 400); // Click the "OK" button
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-93-NKD_SET_019_Custom_Value_Saved.png"),
      "Screenshot comparison failed for IN-93-BW Custom Value saved 45");
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
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-94-NKD_SET_019_Custom_Bed_Width_Screen_No_Selection.png"),
      "Screenshot comparison failed for IN-94-Custom_Bed_Width_Screen");
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
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-80-NKD_ACT_27_Activating_Screen_with_value_2.png"),
      "Screenshot comparison failed for IN-80-Activating_Screen_with_value_2");
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-95-NKD_SET_019_BW_Custom_Value_minimum_Toastmsg.png"),
      "Screenshot comparison failed for IN-95-BW Custom Value MIN TOAST");
  lv_test_mouse_click_at(445, 37); // close toast msg
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-96-NKD_SET_019_BW_Close_minimum_Toastmsg.png"),
      "Screenshot comparison failed for IN-96-BW Close MIN TOAST");
  lv_test_mouse_click_at(400, 400);
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-95-NKD_SET_019_BW_Custom_Value_minimum_Toastmsg.png"),
      "Screenshot comparison failed for IN-95-BW Custom Value MIN TOAST");
  lv_test_mouse_click_at(445, 37);  // close toast msg
  lv_test_mouse_click_at(80, 390);  // Click the "backspace" button
  lv_test_mouse_click_at(80, 390);  // Click the "backspace" button
  lv_test_mouse_click_at(390, 310); // Click '8'
  lv_test_mouse_click_at(190, 390); // Click '9'
  lv_test_mouse_click_at(400, 400); // Click the "tick" button & let toast message appear
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-97-NKD_SET_019_BW_maximum_Toastmsg.png"),
      "Screenshot comparison failed for IN-97-BW Close MAX TOAST");
  lv_test_mouse_click_at(445, 37); // close toast msg
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-98-NKD_SET_018_BW_Close_maximum_Toastmsg.png"),
      "Screenshot comparison failed for IN-98-BW Close MAX TOAST");
  lv_test_mouse_click_at(400, 400); // Click the "tick" button & let toast message appear
  lv_test_wait(250);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-97-NKD_SET_019_BW_maximum_Toastmsg.png"),
      "Screenshot comparison failed for IN-97-BW Close MAX TOAST");
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-99-NKD_SET_020_Save_Button_Bed_Width_Toast.png"),
      "Screenshot comparison failed for Save Button Bed Width Toast");
  lv_test_wait(3000);
  lv_test_mouse_click_at(230, 420);
  saved_bed_wid();
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-100-NKD_SET_020_Bed_Width_Saved_with_toast.png"),
      "Screenshot comparison failed for Bed Width Saved with toast");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-101-NKD_SET_020_Bed_Width_Saved_no_toast.png"),
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-102-NKD_SET_020_Bed_Width_Saved_no_toast_Timout.png"),
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-08-NKD_SET_002-Bed_Placement_Settings.png"),
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
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
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-80-NKD_ACT_27_Activating_Screen_with_value_2.png"),
      "Screenshot comparison failed for IN-80-Activating_Screen_with_value_2");
}

static void test_bed_placement_scrollers_on_settings_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(110, 330); // Click the Bed Placement settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(240, 190); // Click and drag slider to centre
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare(
                               "ref_imgsNKD/NKD-O/NKD_SET/IN-103-NKD_SET_021_BP_Slider_Center.png"),
                           "Screenshot comparison failed for IN-103-BP Slider Center");
  lv_test_mouse_click_at(430, 190); // Click and drag slider to right wall
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-104-NKD_SET_021_BP_Slider_Right_Wall.png"),
      "Screenshot comparison failed for IN-104-BP Slider Right Wall");
  lv_test_mouse_click_at(50, 190); // Click and drag slider to left wall
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-08-NKD_SET_002-Bed_Placement_Settings.png"),
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-105-NKD_SET_022_Save_Button_Bed_Placement_Toast.png"),
      "Screenshot comparison failed for Save Button Bed Placement Toast");
  lv_test_wait(3000);
  lv_test_mouse_click_at(230, 420);
  saved_bed_pos();
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-106-NKD_SET_022_Bed_Placement_Saved_with_toast.png"),
      "Screenshot comparison failed for Bed Placement Saved with toast");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-107-NKD_SET_022_Bed_Placement_Saved_no_toast.png"),
      "Screenshot comparison failed for Bed Placement Saved no toast");
  lv_test_mouse_click_at(50, 190); // Click and drag slider to left wall
  lv_test_wait(150);
  lv_test_mouse_click_at(230, 420); // Click the Save button
  lv_test_wait(200);
  saved_bed_pos();
  lv_test_wait(3000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-108-NKD_SET_022_Bed_Placement_Saved_no_toast_Timeout.png"),
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-09-NKD_SET_002-Occupant_Size_Settings.png"),
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
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
          "ref_imgsNKD/NKD-O/NKD-ACT/IN-80-NKD_ACT_27_Activating_Screen_with_value_2.png"),
      "Screenshot comparison failed for IN-80-Activating_Screen_with_value_2");
}

static void test_scrollers_on_occupant_size_settings_screen(void) {
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 330); // Click the Occupant Size settings button
  lv_test_wait(300);
  lv_test_mouse_click_at(50, 190); // Click and drag slider to Small
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare(
                               "ref_imgsNKD/NKD-O/NKD_SET/IN-109-NKD_SET_023_OS_Slider_small.png"),
                           "Screenshot comparison failed for IN-109-OS Slider Small");
  lv_test_mouse_click_at(430, 190); // Click and drag slider to Large
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare(
                               "ref_imgsNKD/NKD-O/NKD_SET/IN-110-NKD_SET_023_OS_Slider_large.png"),
                           "Screenshot comparison failed for IN-110-OS Slider Large");
  lv_test_mouse_click_at(240, 190); // Click and drag slider to standard
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-09-NKD_SET_002-Occupant_Size_Settings.png"),
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-111-NKD_SET_024_Save_Button_Occupant_Size_Toast.png"),
      "Screenshot comparison failed for Save Button Occupant Size Toast");
  lv_test_wait(3000);
  lv_test_mouse_click_at(230, 420);
  saved_occ_size();
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-112-NKD_SET_024_Occupant_Size_Saved_with_toast.png"),
      "Screenshot comparison failed for Occupant Size Saved with toast");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-113-NKD_SET_024_Occupant_Size_Saved_no_toast.png"),
      "Screenshot comparison failed for Occupant Size Saved no toast");
  lv_test_wait(200);
  lv_test_mouse_click_at(50, 190); // Click and drag slider to Small
  lv_test_wait(150);
  lv_test_mouse_click_at(230, 420); // Click the Save button
  lv_test_wait(200);
  saved_occ_size();
  lv_test_wait(3000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-114-NKD_SET_024_Occupant_Size_Saved_no_toast_Timeout.png"),
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-115-NKD_SET_025-Settings_Save_Pop_Up.png"),
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-105-NKD_SET_022_Save_Button_Bed_Placement_Toast.png"),
      "Screenshot comparison failed for Save Button Bed Placement Toast");
  lv_test_wait(3000);
  lv_test_mouse_click_at(230, 420);
  saved_bed_pos();
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-106-NKD_SET_022_Bed_Placement_Saved_with_toast.png"),
      "Screenshot comparison failed for Bed Placement Saved with toast");
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-116-NKD_SET_028_Pop_Up_Toast_on_Settings_Flow.png"),
      "Screenshot comparison failed for Pop Up Toast on Settings Flow");
  lv_test_wait(3000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-117-NKD_SET_022_Pop_Up_Toast_on_Settings_Flow_Timeout.png"),
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-107-NKD_SET_022_Bed_Placement_Saved_no_toast.png"),
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
          "ref_imgsNKD/NKD-O/NKD_SET/IN-118-NKD_SET_030_Bed_Exit_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Bed exit toast on settings flow");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 1, 0);
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 0, 0);
  lv_test_wait(1000);
  send_set_state_with_alert_syserr(7, 1, 0);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-118-NKD_SET_030_Bed_Exit_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Bed exit toast on settings flow");
  lv_test_wait(200);
  lv_test_mouse_click_at(200, 40); // click on toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_ALE/IN-02-NKD_ALE_001-Bed_Exit_Alert_High.png"),
      "Screenshot comparison failed for Bed exit screen");
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 2, 0);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-119-NKD_SET_030_Chair_Exit_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Chair exit toast on settings flow");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 2, 0);
  lv_test_wait(150);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 0, 0);
  lv_test_wait(1000);
  send_set_state_with_alert_syserr(7, 2, 0);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-119-NKD_SET_030_Chair_Exit_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Chair exit toast on settings flow");
  lv_test_wait(200);
  lv_test_mouse_click_at(200, 40); // click on toast
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare(
                               "ref_imgsNKD/NKD-O/NKD_ALE/IN-04-NKD_ALE_002-Chair_Exit_Alert.png"),
                           "Screenshot comparison failed for Chair Exit Alert");
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  send_set_state_with_alert_syserr(6, 101, 0);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-120-NKD_SET_030_Fall_Alert_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Fall Alert toast on settings flow");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(6, 101, 0);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(6, 0, 0);
  lv_test_wait(1000);
  send_set_state_with_alert_syserr(6, 101, 0);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-120-NKD_SET_030_Fall_Alert_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Fall Alert toast on settings flow");
  lv_test_mouse_click_at(200, 40); // click on toast
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD_ALE/IN-05-NKD_ALE_003-Fall_Alert.png"),
      "Screenshot comparison failed for Fall Alert");
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  send_set_state_with_alert_syserr(6, 0, 0);
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 101, 0);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-120-NKD_SET_030_Fall_Alert_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Fall Alert toast on settings flow");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 101, 0);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 0, 0);
  lv_test_wait(1000);
  send_set_state_with_alert_syserr(7, 101, 0);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-120-NKD_SET_030_Fall_Alert_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Fall Alert toast on settings flow");
  lv_test_mouse_click_at(200, 40); // click on toast
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_ALE/IN-06-NKD_ALE_004-Fall_Alert_with_bed_chair.png"),
      "Screenshot comparison failed for Fall Alert with bed chair");
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 102, 0);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-121-NKD_SET_030_Reposition_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Reposition toast on settings flow");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 102, 0);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 0, 0);
  lv_test_wait(1000);
  send_set_state_with_alert_syserr(7, 102, 0);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-121-NKD_SET_030_Reposition_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Reposition toast on settings flow");
  lv_test_mouse_click_at(200, 40); // click on toast
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_ALE/IN-08-NKD_ALE_006-Reposition_Alert_with_bed_chair.png"),
      "Screenshot comparison failed for Reposition Alert with bed chair");
  send_set_state_with_alert_syserr(7, 0, 0);
  lv_test_wait(100);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  send_set_state_with_alert_syserr(6, 102, 0);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-121-NKD_SET_030_Reposition_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Reposition toast on settings flow");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(6, 102, 0);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(6, 0, 0);
  lv_test_wait(1000);
  send_set_state_with_alert_syserr(6, 102, 0);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-121-NKD_SET_030_Reposition_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Reposition toast on settings flow");
  lv_test_mouse_click_at(200, 40); // click on toast
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare(
                               "ref_imgsNKD/NKD-O/NKD_ALE/IN-07-NKD_ALE_005-Reposition_Alert.png"),
                           "Screenshot comparison failed for Reposition Alert toast");
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 0, 11);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-122-NKD_SET_030_Incorrect_Mode_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Incorrect_Mode on settings flow");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 0, 11);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 0, 0);
  lv_test_wait(1000);
  send_set_state_with_alert_syserr(7, 0, 11);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-122-NKD_SET_030_Incorrect_Mode_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Incorrect_Mode on settings flow");
  lv_test_mouse_click_at(200, 40); // click on toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_ALE/IN-12-NKD_ALE_010-Incorrect_Mode_Error_Screen.png"),
      "Screenshot comparison failed for Incorrect Mode Error toast");
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 0, 61);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD_SET/"
                                 "IN-123-NKD_SET_030_Unassigned_Error_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Unassigned Error on settings flow");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 0, 61);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 0, 0);
  lv_test_wait(1000);
  send_set_state_with_alert_syserr(7, 0, 61);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD_SET/"
                                 "IN-123-NKD_SET_030_Unassigned_Error_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Unassigned Error on settings flow");
  lv_test_mouse_click_at(200, 40); // click on toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_ALE/IN-13-NKD_ALE_011-Unassigned_Error_Screen.png"),
      "Screenshot comparison failed for Unassigned Error Screen toast");
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 0, 12);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD_SET/"
                                 "IN-124-NKD_SET_030_Sunlight_Error_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Sunlight Error on settings flow");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 0, 12);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 0, 0);
  lv_test_wait(1000);
  send_set_state_with_alert_syserr(7, 0, 12);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD_SET/"
                                 "IN-124-NKD_SET_030_Sunlight_Error_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Sunlight Error on settings flow");
  lv_test_mouse_click_at(200, 40); // click on toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_ALE/IN-14-NKD_ALE_012-Sunlight_Interference_Error_Screen.png"),
      "Screenshot comparison failed for Sunlight Error Screen toast");
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 0, 13);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD_SET/"
                                 "IN-125-NKD_SET_030_Obstructed_Error_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Obstructed Error on settings flow");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 0, 13);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 0, 0);
  lv_test_wait(1000);
  send_set_state_with_alert_syserr(7, 0, 13);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD_SET/"
                                 "IN-125-NKD_SET_030_Obstructed_Error_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Obstructed Error on settings flow");
  lv_test_mouse_click_at(200, 40); // click on toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_ALE/IN-15-NKD_ALE_013-Obstructed_Sensor_Error_Screen.png"),
      "Screenshot comparison failed for Obstructed_Sensor Error Screen toast");
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 0, 42);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/"
          "IN-126-NKD_SET_030_System_Disconnected_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for System Disconnected Error on settings flow");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 0, 42);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 0, 0);
  lv_test_wait(1000);
  send_set_state_with_alert_syserr(7, 0, 42);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/"
          "IN-126-NKD_SET_030_System_Disconnected_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for System Disconnected Error on settings flow");
  lv_test_mouse_click_at(200, 40); // click on toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_ALE/IN-16-NKD_ALE_014-System_Disconnected_Error_Screen.png"),
      "Screenshot comparison failed for System Disconnected Error Screen");
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 0, 41);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD_SET/"
                                 "IN-127-NKD_SET_030_Not_Monitoring_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Not Monitoring Error on settings flow");
  lv_test_mouse_click_at(445, 37); // Click on X to close toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 0, 41);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  send_set_state_with_alert_syserr(7, 0, 0);
  lv_test_wait(1000);
  send_set_state_with_alert_syserr(7, 0, 41);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD_SET/"
                                 "IN-127-NKD_SET_030_Not_Monitoring_Toast_on_settings_flow.png"),
      "Screenshot comparison failed for Not Monitoring Error on settings flow");
  lv_test_mouse_click_at(200, 40); // click on toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_ALE/IN-17-NKD_ALE_015-Not_Monitoring_Screen.png"),
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
          "ref_imgsNKD/NKD-O/NKD_SET/"
          "IN-128-NKD_SET_030_Reposition_Toast_on_display_settings_page.png"),
      "Screenshot comparison failed for reposition alert on display setting page");
  wait_ms(120000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_ALE/IN-08-NKD_ALE_006-Reposition_Alert_with_bed_chair.png"),
      "Screenshot comparison failed for Reposition Alert with bed chair");
  lv_test_wait(100);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(110, 110);
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 101, 0);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare(
                               "ref_imgsNKD/NKD-O/NKD_SET/"
                               "IN-129-NKD_SET_030_Fall_Alert_Toast_on_display_settings_page.png"),
                           "Screenshot comparison failed for fall alert on display setting page");
  wait_ms(120000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_ALE/IN-06-NKD_ALE_004-Fall_Alert_with_bed_chair.png"),
      "Screenshot comparison failed for Fall Alert with bed chair");
  lv_test_wait(100);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(110, 110);
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 0, 13);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/"
          "IN-130-NKD_SET_030_Obstructed_Error_Toast_on_display_settings_page.png"),
      "Screenshot comparison failed for Obstructed Error on display setting page");
  lv_test_wait(100);
  lv_test_mouse_click_at(200, 40); // click on toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_ALE/IN-15-NKD_ALE_013-Obstructed_Sensor_Error_Screen.png"),
      "Screenshot comparison failed for Obstructed_Sensor Error Screen toast");
  lv_test_wait(100);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(350, 120);
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 0, 11);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/"
          "IN-131-NKD_SET_030_Incorrect_Error_Toast_on_in_room_alert_settings_page.png"),
      "Screenshot comparison failed for Incorrect Error on in room alert setting page");
  wait_ms(120000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_ALE/IN-12-NKD_ALE_010-Incorrect_Mode_Error_Screen.png"),
      "Screenshot comparison failed for Incorrect Mode Error toast");
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(350, 120);
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 0, 41);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/"
          "IN-132-NKD_SET_030_Not_Monitoring_Error_Toast_on_in_room_alert_settings_page.png"),
      "Screenshot comparison failed for Not Monitoring Error on in room alert setting page");
  lv_test_mouse_click_at(200, 40); // click on toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_ALE/IN-17-NKD_ALE_015-Not_Monitoring_Screen.png"),
      "Screenshot comparison failed for Not Monitoring Screen");
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(120, 180);
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 0, 42);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/"
          "IN-133-NKD_SET_030_System_Disconnected_Error_Toast_on_alert_settings_page.png"),
      "Screenshot comparison failed for Not Monitoring Error on alert setting page");
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 0, 61);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/"
          "IN-134-NKD_SET_030_Unassigned_Error_Toast_on_alert_settings_page.png"),
      "Screenshot comparison failed for Unassigned Error on alert setting page");
  wait_ms(120000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_ALE/IN-13-NKD_ALE_011-Unassigned_Error_Screen.png"),
      "Screenshot comparison failed for Unassigned Error Screen toast");
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(340, 190);
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 0, 12);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/"
          "IN-135-NKD_SET_030_Sunlight_Error_Toast_on_exit_alert_settings_page.png"),
      "Screenshot comparison failed for Sunlight Error on exit alert setting page");
  lv_test_mouse_click_at(200, 40); // click on toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_ALE/IN-14-NKD_ALE_012-Sunlight_Interference_Error_Screen.png"),
      "Screenshot comparison failed for Sunlight Error Screen toast");
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(340, 190);
  lv_test_wait(100);
  send_set_state_with_alert_syserr(6, 102, 0);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/"
          "IN-136-NKD_SET_030_Reposition_alert_Toast_on_exit_alert_settings_page.png"),
      "Screenshot comparison failed for Reposition alert on exit alert setting page");
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 1, 0);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/"
          "IN-137-NKD_SET_030_Bed_Exit_alert_Toast_on_exit_alert_settings_page.png"),
      "Screenshot comparison failed for Bed Exit alert on exit alert setting page");
  wait_ms(120000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_ALE/IN-02-NKD_ALE_001-Bed_Exit_Alert_High.png"),
      "Screenshot comparison failed for Bed exit screen");
  lv_test_wait(100);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(100, 270);
  send_set_state_with_alert_syserr(7, 2, 0);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/"
          "IN-138-NKD_SET_030_Chair_Exit_alert_Toast_on_BMS_settings_page.png"),
      "Screenshot comparison failed for Chair Exit alert on BMS setting page");
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 0, 13);
  lv_test_wait(200);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare(
                               "ref_imgsNKD/NKD-O/NKD_SET/"
                               "IN-139-NKD_SET_030_Obstruted_Error_Toast_on_BMS_settings_page.png"),
                           "Screenshot comparison failed for obstructed error on BMS setting page");
  lv_test_mouse_click_at(200, 40); // click on toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_ALE/IN-15-NKD_ALE_013-Obstructed_Sensor_Error_Screen.png"),
      "Screenshot comparison failed for Obstructed_Sensor Error Screen toast");
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(300, 260);
  send_set_state_with_alert_syserr(7, 0, 12);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/"
          "IN-140-NKD_SET_030_Sunlight_Error_Toast_on_Bed_Width_settings_page.png"),
      "Screenshot comparison failed for Sunlight Error on BW setting page");
  send_set_state_with_alert_syserr(7, 0, 41);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/"
          "IN-141-NKD_SET_030_Not_Monitroing_Error_Toast_on_Bed_Width_settings_page.png"),
      "Screenshot comparison failed for Not Monitoring Error on BW setting page");
  lv_test_mouse_click_at(200, 40); // click on toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_ALE/IN-17-NKD_ALE_015-Not_Monitoring_Screen.png"),
      "Screenshot comparison failed for Not Monitoring Screen");
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(120, 340);
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 101, 0);
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/"
          "IN-142-NKD_SET_030_Fall_Alert_Toast_on_Bed_Placement_settings_page.png"),
      "Screenshot comparison failed for Fall Alert on BP setting page");
  send_set_state_with_alert_syserr(7, 102, 0);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/"
          "IN-143-NKD_SET_030_Reposition_Alert_Toast_on_Bed_Placement_settings_page.png"),
      "Screenshot comparison failed for Reposition Alert on BP setting page");
  lv_test_mouse_click_at(200, 40); // click on toast
  lv_test_wait(100);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_ALE/IN-08-NKD_ALE_006-Reposition_Alert_with_bed_chair.png"),
      "Screenshot comparison failed for Reposition Alert with bed chair");
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 330);
  lv_test_wait(100);
  send_set_state_with_alert_syserr(7, 2, 0);
  lv_test_wait(200);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/"
          "IN-144-NKD_SET_030_Chair_Exit_Alert_Toast_on_Occupant_Size_settings_page.png"),
      "Screenshot comparison failed for Chair Exit Alert on Occupant Size setting page");
  send_set_state_with_alert_syserr(7, 0, 12);
  lv_test_wait(500);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/"
          "IN-145-NKD_SET_030_Sunlight_Error_Toast_on_Occupant_Size_settings_page.png"),
      "Screenshot comparison failed for Sunlight Error on Occupant Size setting page");
  lv_test_mouse_click_at(200, 40); // click on toast
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_ALE/IN-14-NKD_ALE_012-Sunlight_Interference_Error_Screen.png"),
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
  lv_test_mouse_click_at(240, 420); // Click the Deactivate button
  lv_test_wait(200);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD_SET/"
                                 "IN-146-NKD_SET_031_Deactivate_pop_up_settings_flow.png"),
      "Screenshot comparison failed for Deactivate pop up on settings flow");
  lv_test_mouse_click_at(150, 320);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD_SET/IN-01-NKD_SET_001-Settings_Screen_with_buttons.png"),
      "Screenshot comparison failed for Settings Screen with buttons");
  lv_test_mouse_click_at(240, 420); // Click the Deactivate button
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 320);
  lv_test_wait(3000);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD_SET/"
                                 "IN-147-NKD_SET_031_Deactivate_pop_up_toast_timeout.png"),
      "Screenshot comparison failed for Deactivate pop up toast_timeout");
  lv_test_wait(3000);
  lv_test_mouse_click_at(240, 420);
  lv_test_wait(100);
  lv_test_mouse_click_at(330, 320);
  lv_test_wait(100);
  deactivate_sensor_with_screen(5);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD_SET/"
                                 "IN-147-NKD_SET_031_Deactivated_Screen_5.png"),
      "Screenshot comparison failed for Deactivated screen 5");
  send_set_state_with_alert_syserr(7, 0, 0);
  lv_test_wait(100);
  lv_test_mouse_click_at(420, 50); // Click the Settings icon
  lv_test_wait(100);
  lv_test_mouse_click_at(240, 420); // Click the Deactivate button
  lv_test_wait(200);
  lv_test_mouse_click_at(330, 320);
  lv_test_wait(100);
  deactivate_sensor_with_screen(6);
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD-ACT/IN-00-NKD_COM_001-Welcome Screen.png"),
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
      lv_test_screenshot_compare("ref_imgsNKD/NKD-O/NKD_ALE/IN-05-NKD_ALE_003-Fall_Alert.png"),
      "Screenshot comparison failed for Fall Alert");
  lv_test_mouse_click_at(220, 420);
  lv_test_wait(100);
  resume_fall_mon_clear_alert_response();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-MON/IN-05-NKD_MON_003_Chair_Mode_Screen.png"),
      "Screenshot comparison failed for IN-05-NKD_MON_003_Chair_Mode_Screen.png");
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
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare(
                               "ref_imgsNKD/NKD-O/NKD_ALE/IN-07-NKD_ALE_005-Reposition_Alert.png"),
                           "Screenshot comparison failed for Reposition Alert");
  lv_test_mouse_click_at(220, 420);
  lv_test_wait(100);
  resume_fall_mon_clear_alert_response();
  TEST_ASSERT_TRUE_MESSAGE(
      lv_test_screenshot_compare(
          "ref_imgsNKD/NKD-O/NKD-MON/IN-05-NKD_MON_003_Chair_Mode_Screen.png"),
      "Screenshot comparison failed for IN-05-NKD_MON_003_Chair_Mode_Screen.png");
}

TEST_GROUP(INT_Common_screen);

TEST_GROUP(INT_ActivationFlow);

TEST_GROUP(INT_Monitoring_Screens);

TEST_GROUP(INT_Alerts_and_error_screens);

TEST_GROUP(INT_ACT_Settings_Flow);

static int is_filtered_run(void) { return (UnityFixture.NameFilter && UnityFixture.NameFilter[0]); }

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
                      "\"params\":{\"variant\":1,\"fts_avail\":127},\"id\":1}\n");

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
                        "\"params\":{\"variant\":1,\"fts_avail\":127},\"id\":1}\n");

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
                      "\"params\":{\"variant\":1,\"fts_avail\":127},\"id\":1}\n");

  write_response(WR_FD, v, (size_t)vlen);
  process_request(50);
  lv_test_wait(50);
  drain_pipe_now();

  if (G_OUT >= 0)
    dprintf(G_OUT, "[alerts-bootstrap] setup done\n");
}

static void settings_common(void) {
  if (G_OUT >= 0) {
  }

  {
    char v[256];
    int vlen = snprintf(v, sizeof v,
                        "{\"jsonrpc\":\"2.0\",\"method\":\"set_variant\","
                        "\"params\":{\"variant\":1,\"fts_avail\":127},\"id\":1}\n");
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

TEST_SETUP(INT_Common_screen) {
  if (g_bail) {
    TEST_IGNORE_MESSAGE("Skipped after first failure");
  }
}

TEST_TEAR_DOWN(INT_Common_screen) {
  drain_pipe_now();
  if (Unity.CurrentTestFailed) {
    g_bail = 1;
  }
}

TEST_SETUP(INT_ActivationFlow) {
  if (g_bail) {
    TEST_IGNORE_MESSAGE("Skipped after first failure");
  }
  /* Only when user runs with -g and/or -n */
  if (is_filtered_run()) {
    act_flow_common();
  }
}

TEST_TEAR_DOWN(INT_ActivationFlow) {
  drain_pipe_now();
  if (Unity.CurrentTestFailed) {
    g_bail = 1;
  }
}

TEST_SETUP(INT_Monitoring_Screens) {
  if (g_bail) {
    TEST_IGNORE_MESSAGE("Skipped after first failure");
  }

  /* Only when user runs with -g and/or -n */
  if (is_grp_filtered_run()) {
    monitoring_common();
  }
}

TEST_TEAR_DOWN(INT_Monitoring_Screens) {
  drain_pipe_now();
  if (Unity.CurrentTestFailed) {
    g_bail = 1;
  }
}

TEST_SETUP(INT_Alerts_and_error_screens) {
  if (g_bail) {
    TEST_IGNORE_MESSAGE("Skipped after first failure");
  }

  /* Only when user runs with -g and/or -n */
  if (is_grp_filtered_run()) {
    alerts_common();
  }
}

TEST_TEAR_DOWN(INT_Alerts_and_error_screens) {
  drain_pipe_now();
  if (Unity.CurrentTestFailed) {
    g_bail = 1;
  }
}

TEST_SETUP(INT_ACT_Settings_Flow) {
  if (g_bail) {
    TEST_IGNORE_MESSAGE("Skipped after first failure");
  }
  /* Only when user runs with -g and/or -n */
  if (is_filtered_run()) {
    settings_common();
  }
}

TEST_TEAR_DOWN(INT_ACT_Settings_Flow) {
  drain_pipe_now();
  if (Unity.CurrentTestFailed) {
    g_bail = 1;
  }
}

TEST(INT_Common_screen, NKD_COM_001) { test_welcome_screen(); }

TEST(INT_Common_screen, NKD_COM_002) { test_deactivate_screen(); }

TEST(INT_Common_screen, NKD_COM_003) { test_timer_on_mon_ale_screen(); }

TEST(INT_ActivationFlow, NKD_INT_001) { test_activate_screen(); }

TEST(INT_ActivationFlow, NKD_INT_002) { test_activate_button_click(); }

TEST(INT_ActivationFlow, NKD_ACT_001_Presence_of_Alerts_Page) { test_Alerts_page(); }

TEST(INT_ActivationFlow, NKD_ACT_001_Toggles) { test_toggles_in_alerts_page(); }

TEST(INT_ActivationFlow, NKD_ACT_001_back_and_forth) { test_back_btn_alerts(); }

TEST(INT_ActivationFlow, NKD_ACT_001_X_btn) { test_X_btn_alerts(); }

TEST(INT_ActivationFlow, NKD_ACT_001_i_btn_Alerts) { test_sysinfo_screen(); }

TEST(INT_ActivationFlow, NKD_ACT_001_nxt_btn_Alert) { test_next_btn_click(); }

TEST(INT_ActivationFlow, NKD_ACT_007_Presence_of_Exit_Alert) { test_exit_alert_screen(); }

TEST(INT_ActivationFlow, NKD_ACT_007_i_btn_Exit_alert) { test_i_button_on_exit_alert_screen(); }

TEST(INT_ActivationFlow, NKD_ACT_007_X_btn) { test_x_btn_click_on_exit_alert_screen(); }

TEST(INT_ActivationFlow, NKD_ACT_007_back_btn) { test_back_button_on_exit_alert_screen(); }

TEST(INT_ActivationFlow, NKD_ACT_007_nxt_btn_exit_alert) {
  test_next_button_on_exit_alert_screen();
}

TEST(INT_ActivationFlow, NKD_ACT_008_Contnious_Monitoring_Dropdown) {
  test_clickon_Ctn_Monitoring();
}

TEST(INT_ActivationFlow, NKD_ACT_008_Schedule_Monitoring_Page) { test_schdl_monitoring_screen(); }

TEST(INT_ActivationFlow, NKD_ACT_009_Scheduled_Start_time) { test_strtime_on_schdl_screen(); }

TEST(INT_ActivationFlow, NKD_ACT_009_Scheduled_End_time) { test_endtime_on_schdl_screen(); }

TEST(INT_ActivationFlow, NKD_ACT_010_back_btn) { test_back_btn_on_strtime_screen(); }

TEST(INT_ActivationFlow, NKD_ACT_010_X_btn) { test_Xbtn_on_strtime_screen(); }

TEST(INT_ActivationFlow, NKD_ACT_010_Up_arrows_on_strt_time_and_save) {
  test_up_arrow_on_strtime_screen();
}

TEST(INT_ActivationFlow, NKD_ACT_010_down_arrows_on_strt_time_and_save) {
  test_dwn_arrows_on_strtime_screen();
}

TEST(INT_ActivationFlow, NKD_ACT_010_back_btn_on_end_time) { test_back_btn_on_endtime_screen(); }

TEST(INT_ActivationFlow, NKD_ACT_010_X_btn_on_end_time) { test_Xbtn_on_endtime_screen(); }

TEST(INT_ActivationFlow, NKD_ACT_010_Up_Arrows_on_end_time_and_save) {
  test_up_arrow_on_endtime_screen();
}

TEST(INT_ActivationFlow, NKD_ACT_010_Down_arrows_on_end_time_and_save) {
  test_dwn_arrows_on_endtime_screen();
}

TEST(INT_ActivationFlow, NKD_ACT_011_Presence_of_BMS_Page) { test_bed_mode_sensitivity(); }

TEST(INT_ActivationFlow, NKD_ACT_011_i_btn_on_BMS) { test_i_btn_on_bms_screen(); }

TEST(INT_ActivationFlow, NKD_ACT_011_Back_btn_on_BMS) {
  test_back_btn_on_bed_mode_sensitivity_screen();
}

TEST(INT_ActivationFlow, NKD_ACT_011_X_btn_on_BMS) { test_xbtn_on_bed_mode_sensitivity_screen(); }

TEST(INT_ActivationFlow, NKD_ACT_011_nxt_btn_on_BMS) { test_nxt_btn_on_bms_page(); }

TEST(INT_ActivationFlow, NKD_ACT_012_Sliders_with_back_forth) { test_sliders_on_bms_screen(); }

TEST(INT_ActivationFlow, NKD_ACT_013_Presence_of_BW_Page) { test_bed_width_screen(); }

TEST(INT_ActivationFlow, NKD_ACT_013_i_btn_on_BW_Screen) { test_i_btn_on_BW_screen(); }

TEST(INT_ActivationFlow, NKD_ACT_013_Back_btn_on_BW_page) { test_back_btn_on_bed_width_screen(); }

TEST(INT_ActivationFlow, NKD_ACT_013_X_btn_on_BW_page) { test_xbtn_on_bed_width_screen(); }

TEST(INT_ActivationFlow, NKD_ACT_013_nxt_btn_on_BW_page) { test_next_button_on_BW_screen(); }

TEST(INT_ActivationFlow, NKD_ACT_014_Sliders_with_back_forth) { test_sliders_on_BW_screen(); }

TEST(INT_ActivationFlow, NKD_ACT_015_Custom_Bed_Width_Selection) { test_Chng_width_on_BW_screen(); }

TEST(INT_ActivationFlow, NKD_ACT_016_back_btn_on_Custom_width_selection_page) {
  test_back_btn_on_custom_width_screen();
}

TEST(INT_ActivationFlow, NKD_ACT_016_X_btn_on_Custom_width_selection_page) {
  test_x_btn_on_custom_width_screen();
}

TEST(INT_ActivationFlow, NKD_ACT_016_Predefined_options_on_Custom_width_screen) {
  test_predefined_width_options_on_Custom_Width_Screen();
}

TEST(INT_ActivationFlow, NKD_ACT_016_Custom_value_screen) { test_clk_on_custom_value_button(); }

TEST(INT_ActivationFlow, NKD_ACT_017_Custom_Numerical_Inputs) {
  test_number_keypad_screen_on_Custom_BW();
}

TEST(INT_ActivationFlow, NKD_ACT_017_Back_btn_on_custom_value_screen) {
  test_back_button_on_custom_value_screen();
}

TEST(INT_ActivationFlow, NKD_ACT_017_X_btn_on_custom_value_screen) {
  test_x_button_on_custom_value_screen();
}

TEST(INT_ActivationFlow, NKD_ACT_018_ToastMessage_of_min_value) {
  test_toast_messages_on_custom_value_screen();
}

TEST(INT_ActivationFlow, NKD_ACT_019_Presence_of_Bed_Placement_Screen) {
  test_bed_placement_screen();
}

TEST(INT_ActivationFlow, NKD_ACT_019_i_btn_bed_placement_screen) {
  test_i_button_on_bed_placement_screen();
}

TEST(INT_ActivationFlow, NKD_ACT_019_X_btn_on_bed_placement_screen) {
  test_X_button_on_bed_placement_screen();
}

TEST(INT_ActivationFlow, NKD_ACT_019_Back_btn_on_bed_placement_screen) {
  test_back_button_on_bed_placement_screen();
}

TEST(INT_ActivationFlow, NKD_ACT_019_nxt_btn_on_bed_placement_screen) {
  test_nxt_btn_on_bed_placement_screen();
}

TEST(INT_ActivationFlow, NKD_ACT_020_Sliders_with_back_forth) {
  test_sliders_on_bed_placement_screen();
}

TEST(INT_ActivationFlow, NKD_ACT_021_Presence_of_Occupant_Size_Screen) {
  test_occupant_size_screen();
}

TEST(INT_ActivationFlow, NKD_ACT_021_i_btn_on_Occupant_Size_Screen) {
  test_i_button_on_occupant_size_screen();
}

TEST(INT_ActivationFlow, NKD_ACT_021_X_btn_on_occupant_size_screen) {
  test_x_button_on_occupant_size_screen();
}

TEST(INT_ActivationFlow, NKD_ACT_021_back_btn_on_occupant_size_screen) {
  test_back_button_on_occupant_size_screen();
}

TEST(INT_ActivationFlow, NKD_ACT_021_nxt_btn_on_occupant_size_screen) {
  test_nxt_btn_on_occupant_size_screen();
}

TEST(INT_ActivationFlow, NKD_ACT_022_Sliders_with_back_forth) {
  test_sliders_on_occupant_size_screen();
}

TEST(INT_ActivationFlow, NKD_ACT_023_Presence_of_InRoomAlert_Screen) {
  test_in_room_alert_screen();
}

TEST(INT_ActivationFlow, NKD_ACT_023_i_btn_on_inroom_audio_alert) {
  test_i_button_on_in_room_alert_screen();
}

TEST(INT_ActivationFlow, NKD_ACT_023_X_btn_on_inroom_audio_alert) {
  test_X_button_on_in_room_alert_screen();
}

TEST(INT_ActivationFlow, NKD_ACT_023_back_btn_on_inroom_audio_alert) {
  test_back_button_on_in_room_alert_screen();
}

TEST(INT_ActivationFlow, NKD_ACT_024_InRoomAlert_AudioToggle) { test_in_room_alert_audio_toggle(); }

TEST(INT_ActivationFlow, NKD_ACT_025_InRoomAlert_LanguagesPage) {
  test_lang_button_on_in_room_alert_screen();
}

TEST(INT_ActivationFlow, NKD_ACT_025_Language_Page_with_command) {
  test_lang_button_with_command();
}

TEST(INT_ActivationFlow, NKD_ACT_025_back_btn_on_lang_screen) { test_back_button_on_lang_screen(); }

TEST(INT_ActivationFlow, NKD_ACT_025_X_btn_on_lang_screen) { test_X_btn_on_lang_screen(); }

TEST(INT_ActivationFlow, NKD_ACT_026_Sliders_with_back_and_forth) {
  test_volume_slider_in_room_alert_screen();
}

TEST(INT_ActivationFlow, NKD_ACT_027_Activation_button) { test_activating_screen_with_2_and_7(); }

TEST(INT_Monitoring_Screens, NKD_MON_001) { test_bed_monitoring_screen(); }

TEST(INT_Monitoring_Screens, NKD_MON_002) { test_bed_mode_button(); }

TEST(INT_Monitoring_Screens, NKD_MON_003) { test_chair_mode_button(); }

TEST(INT_Monitoring_Screens, NKD_MON_004) { test_short_pause_and_long_pause(); }

TEST(INT_Monitoring_Screens, NKD_MON_005) { test_settings_icon(); }

TEST(INT_Monitoring_Screens, NKD_MON_006) { test_Presence_of_Chair_Mon_screen(); }

TEST(INT_Monitoring_Screens, NKD_MON_007) { test_pressure_injury_screen(); }

TEST(INT_Monitoring_Screens, NKD_MON_008) { test_schedule_monitoring_screen(); }

TEST(INT_Monitoring_Screens, NKD_MON_009) { test_fall_monitoring_screen(); }

TEST(INT_Alerts_and_error_screens, NKD_ALE_001) { test_bed_exit_alert_withall3_options(); }

TEST(INT_Alerts_and_error_screens, NKD_ALE_002) { test_chair_exit_alert(); }

TEST(INT_Alerts_and_error_screens, NKD_ALE_003) { test_fall_alert_screen(); }

TEST(INT_Alerts_and_error_screens, NKD_ALE_004) { test_fall_alert_with_bed_chair_enabled(); }

TEST(INT_Alerts_and_error_screens, NKD_ALE_005) { test_reposition_alert_screen(); }

TEST(INT_Alerts_and_error_screens, NKD_ALE_006) { test_reposition_alert_with_bed_chair(); }

TEST(INT_Alerts_and_error_screens, NKD_ALE_007) { test_paused_error_screen(); }

TEST(INT_Alerts_and_error_screens, NKD_ALE_008) { test_calibaration_error_screen(); }

TEST(INT_Alerts_and_error_screens, NKD_ALE_009) { test_chair_calibarating_screen(); }

TEST(INT_Alerts_and_error_screens, NKD_ALE_010) { test_incorrect_mode_error_screen(); }

TEST(INT_Alerts_and_error_screens, NKD_ALE_011) { test_unassigned_error_screen(); }

TEST(INT_Alerts_and_error_screens, NKD_ALE_012) { test_sunight_error_screen(); }

TEST(INT_Alerts_and_error_screens, NKD_ALE_013) { test_obstructed_error_screen(); }

TEST(INT_Alerts_and_error_screens, NKD_ALE_014) { test_system_disconnected_error_screen(); }

TEST(INT_Alerts_and_error_screens, NKD_ALE_015) { test_not_monitoring_screen(); }

TEST(INT_ACT_Settings_Flow, NKD_SET_001_Presence_of_settings_screen) {
  test_settings_Screen_with_buttons();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_001_i_btn_on_settings_screen) {
  test_i_btn_on_settings_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_001_X_btn_on_settings_screen) {
  test_X_btn_on_settings_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_002_optional_settings_with_diff_fts_values) {
  test_optional_settings_available();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_003_presence_of_bms_screen_settings_flow) {
  test_bms_on_settings_flow();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_004_functionality_of_bms_screen_Scrollers) {
  test_functionality_of_bms_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_004_back_btn_on_bms_screen) { test_back_btn_on_bms_screen(); }

TEST(INT_ACT_Settings_Flow, NKD_SET_004_X_btn_on_bms_screen) { test_X_btn_on_bms_screen(); }

TEST(INT_ACT_Settings_Flow, NKD_SET_006_save_button_on_bms_screen_with_toasts) {
  test_save_button_on_bms_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_006_back_button_after_BMS_saved) {
  test_back_button_after_BMS_saved();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_006_X_button_after_BMS_saved) {
  test_X_button_after_BMS_saved();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_007_presence_of_in_room_audio_alerts) {
  test_in_room_audio_alerts_on_settings_flow();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_007_X_btn_on_in_room_audio_alerts_screen) {
  test_X_btn_on_in_room_audio_alerts_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_007_back_btn_on_in_room_audio_alerts_screen) {
  test_back_btn_on_in_room_audio_alerts_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_008_toggle_on_in_room_audio_alerts_screen) {
  test_toggle_on_in_room_audio_alerts_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_009_languages_screen_with_no_command) {
  test_languages_screen_with_no_command();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_009_back_btn_on_languages_screen) {
  test_back_btn_on_languages_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_009_X_btn_on_languages_screen) {
  test_X_btn_on_languages_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_009_languages_screen_with_command) {
  test_languages_screen_with_command();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_009_volume_scroller_on_in_room_alert_screen) {
  test_volume_scroller_on_in_room_alert_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_010_save_btn_on_in_room_alert_screen) {
  test_save_btn_on_in_room_alert_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_010_back_btn_on_in_room_alert_screen) {
  test_back_btn_on_in_room_alert_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_010_X_btn_on_in_room_alert_screen) {
  test_X_btn_on_in_room_alert_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_011_Presence_display_settings) {
  test_display_page_on_activated_settings_flow();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_011_back_btn_on_display_settings_screen) {
  test_back_btn_on_display_settings_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_011_X_btn_on_display_settings_screen) {
  test_X_btn_on_display_settings_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_012_brightness_scroller_on_display_settings_screen) {
  test_brightness_scroller_on_display_settings_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_013_adaptive_toggle_btn) { test_adaptive_toggle_btn(); }

TEST(INT_ACT_Settings_Flow, NKD_SET_014_save_btn_on_display_settings_screen) {
  test_save_btn_on_display_settings_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_014_back_btn_on_display_settings_after_save) {
  test_back_btn_on_display_settings();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_014_X_btn_on_display_settings_after_save) {
  test_X_btn_on_display_settings();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_015_Presence_of_Alerts_page_on_settings_flow) {
  test_Alerts_page_on_settings_flow();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_015_toggles_on_alert_screen) { test_toggles_on_alert_screen(); }

TEST(INT_ACT_Settings_Flow, NKD_SET_015_X_btn_on_alerts_settings_screen) {
  test_X_btn_on_alerts_settings_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_015_back_btn_on_alerts_settings_screen) {
  test_back_btn_on_alerts_settings_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_016_save_btn_on_alerts_settings_screen) {
  test_save_btn_on_alerts_settings_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_016_back_button_on_alerts_settings_screen) {
  test_back_button_on_alerts_settings_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_016_X_button_on_alerts_settings_screen) {
  test_X_button_on_alerts_settings_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_017_presence_of_exit_alert_schdl_on_settings_flow) {
  test_exit_alert_schedule_on_settings_flow();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_017_back_btn_on_exit_alert_schedule_screen) {
  test_back_btn_on_exit_alert_schedule_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_017_X_btn_on_exit_alert_schedule_screen) {
  test_X_btn_on_exit_alert_schedule_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_018_save_btn_on_exit_alert_schedule_screen) {
  test_save_btn_on_exit_alert_schedule_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_018_back_button_on_exit_alert_schedule_screen) {
  test_back_button_on_exit_alert_schedule_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_018_X_button_on_exit_alert_schedule_screen) {
  test_X_button_on_exit_alert_schedule_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_019_presence_of_bed_width_screen_on_settings_flow) {
  test_bed_width_screen_on_settings_flow();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_019_back_btn_on_bed_width_settings_screen) {
  test_back_btn_on_bed_width_settings_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_019_X_btn_on_bed_width_settings_screen) {
  test_X_btn_on_bed_width_settings_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_019_bed_width_scroller_on_settings_screen) {
  test_bed_width_scroller_on_settings_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_019_Custom_bed_width_settings_screen) {
  test_custom_bed_width_screen_on_settings_flow();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_019_back_btn_on_custom_width_screen_on_settings_flow) {
  test_back_button_on_Custom_BW_screen_on_settings_flow();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_019_X_btn_on_custom_width_screen_on_settings_flow) {
  test_x_button_on_Custom_BW_screen_on_settings_flow();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_019_predefined_options_on_custom_BW_screen_on_settings_flow) {
  test_predefined_width_options_on_Custom_BW_screen_on_settings_flow();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_019_custom_value_screen_on_settings_flow) {
  test_custom_value_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_019_keypad_screen_on_BW_settings_flow) {
  test_keypad_screen_on_Custom_Value_Screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_019_back_btn_on_custom_input_screen_on_settings_flow) {
  test_back_button_on_custom_input_screen_on_settings_flow();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_019_X_btn_on_custom_input_screen_on_settings_flow) {
  test_x_button_on_custom_input_screen_settings_flow();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_019_toast_messages_on_custom_input_screen_on_settings_flow) {
  test_toast_messages_on_custom_input_screen_on_settings_flow();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_020_Save_btn_on_BW_screen) { test_save_btn_on_BW_screen(); }

TEST(INT_ACT_Settings_Flow, NKD_SET_021_Presence_of_bed_placement_screen_on_settings_flow) {
  test_bed_placement_screen_on_settings_flow();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_021_back_btn_on_bed_placement_settings_screen) {
  test_back_button_on_bed_placement_settings_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_021_X_btn_on_bed_placement_settings_screen) {
  test_X_button_on_bed_placement_settings_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_021_bed_placement_scrollers_on_settings_screen) {
  test_bed_placement_scrollers_on_settings_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_022_save_btn_on_bed_placement_screen) {
  test_save_btn_on_BP_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_023_Presence_of_occupant_size_screen_on_settings_flow) {
  test_Occupant_Size_screen_on_settings_flow();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_023_back_btn_on_occupant_size_settings_screen) {
  test_back_button_on_occupant_size_settings_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_023_X_btn_on_occupant_size_settings_screen) {
  test_X_button_on_occupant_size_settings_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_023_occupant_size_scroller_on_settings_screen) {
  test_scrollers_on_occupant_size_settings_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_024_Save_btn_on_occupant_size_screen) {
  test_save_btn_on_OS_screen();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_025_Settings_Save_Pop_Up) {
  test_save_pop_up_message_on_settings_flow();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_026_Dont_Save_button_on_settings_flow) {
  test_Dont_Save_button_on_settings_flow();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_027_save_btn_functionality_on_settings_flow) {
  test_save_btn_functionality_on_settings_flow();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_028_presence_of_pop_up_toast_on_settings_flow) {
  test_presence_of_pop_up_toast_on_settings_flow();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_029_X_btn_on_toast_message_on_settings_flow) {
  test_X_btn_on_toast_message_on_settings_flow();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_030_Presence_of_alerts_and_error_toasts_on_settings_flow) {
  test_Alerts_and_error_toast_msgs_on_settings_flow();
}

TEST(INT_ACT_Settings_Flow,
     NKD_SET_030_Presence_of_alerts_and_error_toasts_timeouts_on_settings_pages) {
  test_alerts_and_error_toasts_on_settings_pages();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_031_Deactivate_btn_Functionality_on_Settings_Flow) {
  test_deactivate_button_on_settings_flow();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_032_Resume_Fall_mon_btn_Functionality) {
  test_resume_fall_monitoring_button();
}

TEST(INT_ACT_Settings_Flow, NKD_SET_033_clear_alert_btn_Functionality) {
  test_clear_alert_btn_functionality();
}

TEST_GROUP_RUNNER(INT_Common_screen) {
  RUN_TEST_CASE(INT_Common_screen, NKD_COM_001);
  RUN_TEST_CASE(INT_Common_screen, NKD_COM_002);
  RUN_TEST_CASE(INT_Common_screen, NKD_COM_003);
}

TEST_GROUP_RUNNER(INT_ActivationFlow) {
  RUN_TEST_CASE(INT_ActivationFlow, NKD_INT_001);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_INT_002);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_001_Presence_of_Alerts_Page);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_001_Toggles);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_001_back_and_forth);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_001_X_btn);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_001_i_btn_Alerts);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_001_nxt_btn_Alert);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_007_Presence_of_Exit_Alert);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_007_i_btn_Exit_alert);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_007_X_btn);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_007_back_btn);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_007_nxt_btn_exit_alert);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_008_Contnious_Monitoring_Dropdown);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_008_Schedule_Monitoring_Page);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_009_Scheduled_Start_time);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_009_Scheduled_End_time);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_010_back_btn);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_010_X_btn);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_010_Up_arrows_on_strt_time_and_save);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_010_down_arrows_on_strt_time_and_save);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_010_back_btn_on_end_time);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_010_X_btn_on_end_time);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_010_Up_Arrows_on_end_time_and_save);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_010_Down_arrows_on_end_time_and_save);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_011_Presence_of_BMS_Page);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_011_i_btn_on_BMS);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_011_Back_btn_on_BMS);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_011_X_btn_on_BMS);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_011_nxt_btn_on_BMS);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_012_Sliders_with_back_forth);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_013_Presence_of_BW_Page);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_013_i_btn_on_BW_Screen);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_013_Back_btn_on_BW_page);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_013_X_btn_on_BW_page);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_013_nxt_btn_on_BW_page);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_014_Sliders_with_back_forth);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_015_Custom_Bed_Width_Selection);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_016_back_btn_on_Custom_width_selection_page);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_016_X_btn_on_Custom_width_selection_page);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_016_Predefined_options_on_Custom_width_screen);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_016_Custom_value_screen);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_017_Custom_Numerical_Inputs);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_017_Back_btn_on_custom_value_screen);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_017_X_btn_on_custom_value_screen);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_018_ToastMessage_of_min_value);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_019_Presence_of_Bed_Placement_Screen);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_019_i_btn_bed_placement_screen);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_019_X_btn_on_bed_placement_screen);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_019_Back_btn_on_bed_placement_screen);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_019_nxt_btn_on_bed_placement_screen);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_020_Sliders_with_back_forth);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_021_Presence_of_Occupant_Size_Screen);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_021_i_btn_on_Occupant_Size_Screen);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_021_X_btn_on_occupant_size_screen);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_021_back_btn_on_occupant_size_screen);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_021_nxt_btn_on_occupant_size_screen);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_022_Sliders_with_back_forth);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_023_Presence_of_InRoomAlert_Screen);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_023_i_btn_on_inroom_audio_alert);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_023_X_btn_on_inroom_audio_alert);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_023_back_btn_on_inroom_audio_alert);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_024_InRoomAlert_AudioToggle);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_025_InRoomAlert_LanguagesPage);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_025_Language_Page_with_command);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_025_back_btn_on_lang_screen);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_025_X_btn_on_lang_screen);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_026_Sliders_with_back_and_forth);
  RUN_TEST_CASE(INT_ActivationFlow, NKD_ACT_027_Activation_button);
}

TEST_GROUP_RUNNER(INT_Monitoring_Screens) {
  RUN_TEST_CASE(INT_Monitoring_Screens, NKD_MON_001);
  RUN_TEST_CASE(INT_Monitoring_Screens, NKD_MON_002);
  RUN_TEST_CASE(INT_Monitoring_Screens, NKD_MON_003);
  RUN_TEST_CASE(INT_Monitoring_Screens, NKD_MON_004);
  RUN_TEST_CASE(INT_Monitoring_Screens, NKD_MON_005);
  RUN_TEST_CASE(INT_Monitoring_Screens, NKD_MON_006);
  RUN_TEST_CASE(INT_Monitoring_Screens, NKD_MON_007);
  RUN_TEST_CASE(INT_Monitoring_Screens, NKD_MON_008);
  RUN_TEST_CASE(INT_Monitoring_Screens, NKD_MON_009);
}

TEST_GROUP_RUNNER(INT_Alerts_and_error_screens) {
  RUN_TEST_CASE(INT_Alerts_and_error_screens, NKD_ALE_001);
  RUN_TEST_CASE(INT_Alerts_and_error_screens, NKD_ALE_002);
  RUN_TEST_CASE(INT_Alerts_and_error_screens, NKD_ALE_003);
  RUN_TEST_CASE(INT_Alerts_and_error_screens, NKD_ALE_004);
  RUN_TEST_CASE(INT_Alerts_and_error_screens, NKD_ALE_005);
  RUN_TEST_CASE(INT_Alerts_and_error_screens, NKD_ALE_006);
  RUN_TEST_CASE(INT_Alerts_and_error_screens, NKD_ALE_007);
  RUN_TEST_CASE(INT_Alerts_and_error_screens, NKD_ALE_008);
  RUN_TEST_CASE(INT_Alerts_and_error_screens, NKD_ALE_009);
  RUN_TEST_CASE(INT_Alerts_and_error_screens, NKD_ALE_010);
  RUN_TEST_CASE(INT_Alerts_and_error_screens, NKD_ALE_011);
  RUN_TEST_CASE(INT_Alerts_and_error_screens, NKD_ALE_012);
  RUN_TEST_CASE(INT_Alerts_and_error_screens, NKD_ALE_013);
  RUN_TEST_CASE(INT_Alerts_and_error_screens, NKD_ALE_014);
  RUN_TEST_CASE(INT_Alerts_and_error_screens, NKD_ALE_015);
}

TEST_GROUP_RUNNER(INT_ACT_Settings_Flow) {
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_001_Presence_of_settings_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_001_i_btn_on_settings_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_001_X_btn_on_settings_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_002_optional_settings_with_diff_fts_values);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_003_presence_of_bms_screen_settings_flow);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_004_functionality_of_bms_screen_Scrollers);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_004_back_btn_on_bms_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_004_X_btn_on_bms_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_006_save_button_on_bms_screen_with_toasts);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_006_back_button_after_BMS_saved);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_006_X_button_after_BMS_saved);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_007_presence_of_in_room_audio_alerts);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_007_X_btn_on_in_room_audio_alerts_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_007_back_btn_on_in_room_audio_alerts_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_008_toggle_on_in_room_audio_alerts_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_009_languages_screen_with_no_command);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_009_back_btn_on_languages_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_009_X_btn_on_languages_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_009_languages_screen_with_command);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_009_volume_scroller_on_in_room_alert_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_010_save_btn_on_in_room_alert_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_010_back_btn_on_in_room_alert_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_010_X_btn_on_in_room_alert_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_011_Presence_display_settings);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_011_back_btn_on_display_settings_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_011_X_btn_on_display_settings_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_012_brightness_scroller_on_display_settings_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_013_adaptive_toggle_btn);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_014_save_btn_on_display_settings_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_014_back_btn_on_display_settings_after_save);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_014_X_btn_on_display_settings_after_save);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_015_Presence_of_Alerts_page_on_settings_flow);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_015_toggles_on_alert_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_015_X_btn_on_alerts_settings_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_015_back_btn_on_alerts_settings_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_016_save_btn_on_alerts_settings_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_016_back_button_on_alerts_settings_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_016_X_button_on_alerts_settings_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_017_presence_of_exit_alert_schdl_on_settings_flow);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_017_back_btn_on_exit_alert_schedule_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_017_X_btn_on_exit_alert_schedule_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_018_save_btn_on_exit_alert_schedule_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_018_back_button_on_exit_alert_schedule_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_018_X_button_on_exit_alert_schedule_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_019_presence_of_bed_width_screen_on_settings_flow);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_019_back_btn_on_bed_width_settings_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_019_X_btn_on_bed_width_settings_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_019_bed_width_scroller_on_settings_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_019_Custom_bed_width_settings_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow,
                NKD_SET_019_back_btn_on_custom_width_screen_on_settings_flow);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_019_X_btn_on_custom_width_screen_on_settings_flow);
  RUN_TEST_CASE(INT_ACT_Settings_Flow,
                NKD_SET_019_predefined_options_on_custom_BW_screen_on_settings_flow);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_019_custom_value_screen_on_settings_flow);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_019_keypad_screen_on_BW_settings_flow);
  RUN_TEST_CASE(INT_ACT_Settings_Flow,
                NKD_SET_019_back_btn_on_custom_input_screen_on_settings_flow);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_019_X_btn_on_custom_input_screen_on_settings_flow);
  RUN_TEST_CASE(INT_ACT_Settings_Flow,
                NKD_SET_019_toast_messages_on_custom_input_screen_on_settings_flow);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_020_Save_btn_on_BW_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow,
                NKD_SET_021_Presence_of_bed_placement_screen_on_settings_flow);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_021_back_btn_on_bed_placement_settings_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_021_X_btn_on_bed_placement_settings_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_021_bed_placement_scrollers_on_settings_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_022_save_btn_on_bed_placement_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow,
                NKD_SET_023_Presence_of_occupant_size_screen_on_settings_flow);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_023_back_btn_on_occupant_size_settings_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_023_X_btn_on_occupant_size_settings_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_023_occupant_size_scroller_on_settings_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_024_Save_btn_on_occupant_size_screen);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_025_Settings_Save_Pop_Up);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_026_Dont_Save_button_on_settings_flow);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_027_save_btn_functionality_on_settings_flow);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_028_presence_of_pop_up_toast_on_settings_flow);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_029_X_btn_on_toast_message_on_settings_flow);
  RUN_TEST_CASE(INT_ACT_Settings_Flow,
                NKD_SET_030_Presence_of_alerts_and_error_toasts_on_settings_flow);
  RUN_TEST_CASE(INT_ACT_Settings_Flow,
                NKD_SET_030_Presence_of_alerts_and_error_toasts_timeouts_on_settings_pages);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_031_Deactivate_btn_Functionality_on_Settings_Flow);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_032_Resume_Fall_mon_btn_Functionality);
  RUN_TEST_CASE(INT_ACT_Settings_Flow, NKD_SET_033_clear_alert_btn_Functionality);
}
