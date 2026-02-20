#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "lvgl.h"
#include "unity.h"
#include "unity_fixture.h"
#include "unity_internals.h"

#include "app.h"
#include "jsonrpc2.h"
#include "lv_test_helpers.h"
#include "lv_test_init.h"

extern int g_bail;
extern int WR_FD;
extern ssize_t write_response(int fd, const char *data, size_t len);

extern int RD_FD, UI_WR_FD, UI_RD_FD, G_OUT, G_A0;
extern void wait_ms(uint32_t ms);
extern void drain_pipe_now(void);
extern ssize_t write_response(int fd, const char *data, size_t len);
extern int read_line_nb(int fd, char *b, size_t cap);

static const char *CMDS[] = {
    "{\"jsonrpc\":\"2.0\",\"method\":\"set_variant\",\"params\":{\"variant\":3,\"fts_avail\":127},"
    "\"id\":1}\n",
    "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{\"screen\":2,\"fts_avail\":121,"
    "\"fts_state\":7,\"bms\":1,\"mode\":1,\"pause_tmr\":0,\"alert\":1,\"syserr\":0,\"room_number\":"
    "\"1001-a\"},\"id\":1}\n",
    "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{\"screen\":2,\"fts_avail\":121,"
    "\"fts_state\":7,\"bms\":1,\"mode\":1,\"pause_tmr\":0,\"alert\":2,\"syserr\":0,\"room_number\":"
    "\"1001-a\"},\"id\":1}\n",
    "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{\"screen\":2,\"fts_avail\":121,"
    "\"fts_state\":7,\"bms\":1,\"mode\":1,\"pause_tmr\":0,\"alert\":0,\"syserr\":0,\"room_number\":"
    "\"1001-a\"},\"id\":1}\n",
    "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{\"screen\":2,\"fts_avail\":121,"
    "\"fts_state\":7,\"bms\":1,\"mode\":2,\"pause_tmr\":0,\"alert\":101,\"syserr\":0,\"room_"
    "number\":\"1001-a\"},\"id\":1}\n",
    "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{\"screen\":2,\"fts_avail\":121,"
    "\"fts_state\":7,\"bms\":1,\"mode\":2,\"pause_tmr\":0,\"alert\":0,\"syserr\":0,\"room_number\":"
    "\"1001-a\"},\"id\":1}\n",
    "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{\"screen\":2,\"fts_avail\":121,"
    "\"fts_state\":7,\"bms\":1,\"mode\":2,\"pause_tmr\":0,\"alert\":102,\"syserr\":0,\"room_"
    "number\":\"1001-a\"},\"id\":1}\n",
    "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{\"screen\":2,\"fts_avail\":127,"
    "\"fts_state\":1,\"mode\":2,\"room_number\":\"1001-a\",\"pause_tmr\":6},\"id\":1}\n",
    "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{\"screen\":2,\"fts_avail\":127,"
    "\"fts_state\":1,\"mode\":2,\"room_number\":\"1001-a\",\"cal\":1},\"id\":1}\n",
    "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{\"screen\":2,\"fts_avail\":127,"
    "\"fts_state\":1,\"mode\":2,\"room_number\":\"1001-a\",\"cal\":2},\"id\":1}\n",
    "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{\"screen\":2,\"fts_avail\":121,"
    "\"fts_state\":7,\"bms\":1,\"mode\":2,\"pause_tmr\":0,\"alert\":0,\"syserr\":11,\"room_"
    "number\":\"1001-a\"},\"id\":1}\n",
    "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{\"screen\":2,\"fts_avail\":121,"
    "\"fts_state\":7,\"bms\":1,\"mode\":2,\"pause_tmr\":0,\"alert\":0,\"syserr\":61,\"room_"
    "number\":\"1001-a\"},\"id\":1}\n",
    "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{\"screen\":2,\"fts_avail\":121,"
    "\"fts_state\":7,\"bms\":1,\"mode\":2,\"pause_tmr\":0,\"alert\":0,\"syserr\":12,\"room_"
    "number\":\"1001-a\"},\"id\":1}\n",
    "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{\"screen\":2,\"fts_avail\":121,"
    "\"fts_state\":7,\"bms\":1,\"mode\":2,\"pause_tmr\":0,\"alert\":0,\"syserr\":13,\"room_"
    "number\":\"1001-a\"},\"id\":1}\n",
    "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{\"screen\":2,\"fts_avail\":121,"
    "\"fts_state\":7,\"bms\":1,\"mode\":2,\"pause_tmr\":0,\"alert\":0,\"syserr\":42,\"room_"
    "number\":\"1001-a\"},\"id\":1}\n",
    "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{\"screen\":2,\"fts_avail\":121,"
    "\"fts_state\":7,\"bms\":1,\"mode\":2,\"pause_tmr\":0,\"alert\":0,\"syserr\":41,\"room_"
    "number\":\"1001-a\"},\"id\":1}\n",
    "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{\"screen\":2,\"fts_avail\":121,"
    "\"fts_state\":7,\"bms\":1,\"mode\":4,\"pause_tmr\":0,\"alert\":0,\"syserr\":0,\"room_number\":"
    "\"1001-a\"},\"id\":1}\n",
    "{\"jsonrpc\":\"2.0\",\"method\":\"set_state\",\"params\":{\"screen\":2,\"fts_avail\":121,"
    "\"fts_state\":7,\"bms\":1,\"mode\":3,\"pause_tmr\":0,\"alert\":0,\"syserr\":0,\"room_number\":"
    "\"1001-a\"},\"id\":1}\n"};

static const char *SNAPS[] = {
    "ref_imgsNKD/NKD-RO/ro-00-welcome.png",          "ref_imgsNKD/NKD-RO/ro-01-bed-exit.png",
    "ref_imgsNKD/NKD-RO/ro-02-chair-exit.png",       "ref_imgsNKD/NKD-RO/ro-03-bed-monitor.png",
    "ref_imgsNKD/NKD-RO/ro-04-fall-alert.png",       "ref_imgsNKD/NKD-RO/ro-05-chair-monitor.png",
    "ref_imgsNKD/NKD-RO/ro-06-reposition.png",       "ref_imgsNKD/NKD-RO/ro-07-paused.png",
    "ref_imgsNKD/NKD-RO/ro-08-cal-bed.png",          "ref_imgsNKD/NKD-RO/ro-09-cal-chair.png",
    "ref_imgsNKD/NKD-RO/ro-10-incorrect-mode.png",   "ref_imgsNKD/NKD-RO/ro-11-unassigned.png",
    "ref_imgsNKD/NKD-RO/ro-12-sunlight.png",         "ref_imgsNKD/NKD-RO/ro-13-obstructed.png",
    "ref_imgsNKD/NKD-RO/ro-14-sys-disconnected.png", "ref_imgsNKD/NKD-RO/ro-15-not-monitoring.png",
    "ref_imgsNKD/NKD-RO/ro-16-pressure-injury.png",  "ref_imgsNKD/NKD-RO/ro-17-fall-monitor.png"};

static int is_filtered_run(void) {
  return (UnityFixture.GroupFilter && UnityFixture.GroupFilter[0]) ||
         (UnityFixture.NameFilter && UnityFixture.NameFilter[0]);
}

static int g_variant_bootstrapped = 0;

static void bootstrap_variant_once(void) {
  if (g_variant_bootstrapped)
    return;

  write_response(WR_FD, CMDS[0], strlen(CMDS[0]));
  process_request(100);
  lv_test_wait(50);

  g_variant_bootstrapped = 1;
}

static void step_and_compare(int cmd_idx, int snap_idx, const char *fail_msg) {
  write_response(WR_FD, CMDS[cmd_idx], strlen(CMDS[cmd_idx]));
  process_request(100);
  TEST_ASSERT_TRUE_MESSAGE(lv_test_screenshot_compare(SNAPS[snap_idx]), fail_msg);
}

TEST_GROUP(RO_Screens);

TEST_SETUP(RO_Screens) {
  if (g_bail) {
    TEST_IGNORE_MESSAGE("Skipped after first failure");
  }
  if (is_filtered_run()) {
    bootstrap_variant_once();
  }
}

TEST_TEAR_DOWN(RO_Screens) {
  drain_pipe_now();
  if (Unity.CurrentTestFailed) {
    g_bail = 1;
  }
}

TEST(RO_Screens, NKD_RO_001_Welcome) {
  step_and_compare(0, 0, "Screenshot comparison failed for Welcome Screen");
}

TEST(RO_Screens, NKD_RO_002_Bed_Exit_Alert) {
  step_and_compare(1, 1, "Screenshot comparison failed for Bed Exit Alert Screen");
}

TEST(RO_Screens, NKD_RO_003_Chair_Exit_Alert) {
  step_and_compare(2, 2, "Screenshot comparison failed for Chair Exit Alert Screen");
}

TEST(RO_Screens, NKD_RO_004_Bed_Monitoring) {
  step_and_compare(3, 3, "Screenshot comparison failed for Bed Monitoring Screen");
}

TEST(RO_Screens, NKD_RO_005_Fall_Alert) {
  step_and_compare(4, 4, "Screenshot comparison failed for Fall Alert Screen");
}

TEST(RO_Screens, NKD_RO_006_Chair_Monitoring) {
  step_and_compare(5, 5, "Screenshot comparison failed for Chair Monitoring Screen");
}

TEST(RO_Screens, NKD_RO_007_Reposition_Alert) {
  step_and_compare(6, 6, "Screenshot comparison failed for Reposition Alert Screen");
}

TEST(RO_Screens, NKD_RO_008_Paused) {
  step_and_compare(7, 7, "Screenshot comparison failed for Paused Screen");
}

TEST(RO_Screens, NKD_RO_009_Calibrating_Bed) {
  step_and_compare(8, 8, "Screenshot comparison failed for Calibrating Bed Screen");
}

TEST(RO_Screens, NKD_RO_010_Calibrating_Chair) {
  step_and_compare(9, 9, "Screenshot comparison failed for Calibrating Chair Screen");
}

TEST(RO_Screens, NKD_RO_011_Incorrect_Mode) {
  step_and_compare(10, 10, "Screenshot comparison failed for Incorrect Mode Screen");
}

TEST(RO_Screens, NKD_RO_012_Unassigned) {
  step_and_compare(11, 11, "Screenshot comparison failed for Unassigned Screen");
}

TEST(RO_Screens, NKD_RO_013_Sunlight_Interference) {
  step_and_compare(12, 12, "Screenshot comparison failed for Sunlight Interference Screen");
}

TEST(RO_Screens, NKD_RO_014_Obstructed) {
  step_and_compare(13, 13, "Screenshot comparison failed for Obstructed Screen");
}

TEST(RO_Screens, NKD_RO_015_System_Disconnected) {
  step_and_compare(14, 14, "Screenshot comparison failed for System Disconnected Screen");
}

TEST(RO_Screens, NKD_RO_016_Not_Monitoring) {
  step_and_compare(15, 15, "Screenshot comparison failed for Not Monitoring Screen");
}

TEST(RO_Screens, NKD_RO_017_Pressure_Injury_Alert) {
  step_and_compare(16, 16, "Screenshot comparison failed for Pressure Injury Alert Screen");
}

TEST(RO_Screens, NKD_RO_018_Fall_Monitoring) {
  step_and_compare(17, 17, "Screenshot comparison failed for Fall Monitoring Screen");
}

/* -------------------- group runner -------------------- */
TEST_GROUP_RUNNER(RO_Screens) {
  RUN_TEST_CASE(RO_Screens, NKD_RO_001_Welcome);
  RUN_TEST_CASE(RO_Screens, NKD_RO_002_Bed_Exit_Alert);
  RUN_TEST_CASE(RO_Screens, NKD_RO_003_Chair_Exit_Alert);
  RUN_TEST_CASE(RO_Screens, NKD_RO_004_Bed_Monitoring);
  RUN_TEST_CASE(RO_Screens, NKD_RO_005_Fall_Alert);
  RUN_TEST_CASE(RO_Screens, NKD_RO_006_Chair_Monitoring);
  RUN_TEST_CASE(RO_Screens, NKD_RO_007_Reposition_Alert);
  RUN_TEST_CASE(RO_Screens, NKD_RO_008_Paused);
  RUN_TEST_CASE(RO_Screens, NKD_RO_009_Calibrating_Bed);
  RUN_TEST_CASE(RO_Screens, NKD_RO_010_Calibrating_Chair);
  RUN_TEST_CASE(RO_Screens, NKD_RO_011_Incorrect_Mode);
  RUN_TEST_CASE(RO_Screens, NKD_RO_012_Unassigned);
  RUN_TEST_CASE(RO_Screens, NKD_RO_013_Sunlight_Interference);
  RUN_TEST_CASE(RO_Screens, NKD_RO_014_Obstructed);
  RUN_TEST_CASE(RO_Screens, NKD_RO_015_System_Disconnected);
  RUN_TEST_CASE(RO_Screens, NKD_RO_016_Not_Monitoring);
  RUN_TEST_CASE(RO_Screens, NKD_RO_017_Pressure_Injury_Alert);
  RUN_TEST_CASE(RO_Screens, NKD_RO_018_Fall_Monitoring);
}
