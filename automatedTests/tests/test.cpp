#include "app.h"
#include "jsonrpc2.h"
#include "lv_test_helpers.h"
#include "lv_test_init.h"
#include "lvgl.h"
#include "unity_fixture.h"

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define HOR_RES 480
#define VER_RES 480

extern UNITY_FIXTURE_T UnityFixture;
int g_bail = 0;

int G_A0 = -1;     // pipe read end (for draining logs)
int G_OUT = -1;    // real terminal stdout dup
int RD_FD = -1;    // non-blocking read end of pipe (host reads UI logs)
int WR_FD = -1;    // write end (tests write responses using this)
int UI_RD_FD = -1; // read end of pipe for UI output
int UI_WR_FD = -1; // write end of pipe for UI input

extern "C" {
void UnityOutputChar(int c) {
  if (G_OUT >= 0) {
    char ch = (char)c;
    (void)write(G_OUT, &ch, 1);
  }
}

void UnityOutputFlush(void) {}
}

static void test_log_print_cb(lv_log_level_t level, const char *buf) {
  if (level < LV_LOG_LEVEL_WARN)
    return;
  if (G_OUT >= 0)
    dprintf(G_OUT, "%s", buf);
}

void setup_pipe(void) {
  int A[2], B[2];
  TEST_ASSERT_EQUAL_INT(0, pipe(A));
  TEST_ASSERT_EQUAL_INT(0, pipe(B));
  fcntl(A[0], F_SETFL, O_NONBLOCK);
  RD_FD = A[0];
  WR_FD = B[1];
  UI_RD_FD = B[0];
  UI_WR_FD = A[1];
  G_A0 = A[0];
  G_OUT = dup(STDOUT_FILENO);
  TEST_ASSERT_TRUE(dup2(UI_WR_FD, STDOUT_FILENO) >= 0);
  TEST_ASSERT_TRUE(dup2(UI_WR_FD, STDERR_FILENO) >= 0);
  TEST_ASSERT_TRUE(dup2(UI_RD_FD, STDIN_FILENO) >= 0);

  setvbuf(stdout, NULL, _IONBF, 0);
}

static int read_line_nb_impl(int fd, char *b, size_t cap) {
  size_t n = 0;
  char c;
  while (1) {
    ssize_t r = read(fd, &c, 1);
    if (r > 0) {
      if (n + 1 < cap)
        b[n++] = c;
      if (c == '\n') {
        b[n] = '\0';
        if (G_OUT >= 0)
          dprintf(G_OUT, "%s", b);
        return 1;
      }
    } else {
      return 0;
    }
  }
}

int read_line_nb(int fd, char *b, size_t cap) { return read_line_nb_impl(fd, b, cap); }

ssize_t write_response(int fd, const char *data, size_t len) {
  dprintf(STDOUT_FILENO, "[host->ui fd=%d] %.*s", fd, (int)len, data);
  return write(fd, data, len);
}

void drain_pipe_now(void) {
  char line[512];
  while (read_line_nb_impl(RD_FD, line, sizeof line)) {
  }
}

void wait_ms(uint32_t ms) {
  lv_test_wait(ms);
  process_request(0);
  lv_timer_handler();
  drain_pipe_now();
}

// Test group declarations
TEST_GROUP(GO_Common_screen);
TEST_GROUP(GO_ActivationFlow);
TEST_GROUP(GO_MonitoringScreens);
TEST_GROUP(GO_Alerts_and_error_screens);
TEST_GROUP(GO_ACT_Settings_Flow);

TEST_GROUP(INT_Common_screen);
TEST_GROUP(INT_ActivationFlow);
TEST_GROUP(INT_Monitoring_Screens);
TEST_GROUP(INT_Alerts_and_error_screens);
TEST_GROUP(INT_ACT_Settings_Flow);

TEST_GROUP(RO_Screens);
TEST_GROUP(NKD_CMD);
TEST_GROUP(NKD_TO);

static void RunAllTests(void) {
  setup_pipe();
  lv_init();
  lv_log_register_print_cb(test_log_print_cb);
  lv_test_display_create(HOR_RES, VER_RES);
  lv_test_indev_create_all();
  app_init();

  // Get the mode exported by shell script (IN, GO, RO, etc.)
  const char *mode = getenv("SIMTEST_MODE");

#define RUN_MODE(grp, prefix)                                                                      \
  if (UnityFixture.GroupFilter != NULL) {                                                          \
    if (strcmp(UnityFixture.GroupFilter, #grp) == 0)                                               \
      RUN_TEST_GROUP(grp);                                                                         \
  } else if (strcmp(mode, "ALL") == 0 || strcmp(mode, prefix) == 0) {              \
    RUN_TEST_GROUP(grp);                                                                           \
  }

  RUN_MODE(GO_Common_screen, "GO");
  RUN_MODE(GO_ActivationFlow, "GO");
  RUN_MODE(GO_MonitoringScreens, "GO");
  RUN_MODE(GO_Alerts_and_error_screens, "GO");
  RUN_MODE(GO_ACT_Settings_Flow, "GO");

  RUN_MODE(INT_Common_screen, "IN");
  RUN_MODE(INT_ActivationFlow, "IN");
  RUN_MODE(INT_Monitoring_Screens, "IN");
  RUN_MODE(INT_Alerts_and_error_screens, "IN");
  RUN_MODE(INT_ACT_Settings_Flow, "IN");

  RUN_MODE(RO_Screens, "RO");
  RUN_MODE(NKD_CMD, "CMD");
  RUN_MODE(NKD_TO, "TO");

#undef RUN_MODE

  lv_test_deinit();
  drain_pipe_now();
}

int main(int argc, const char *argv[]) {
  int result = UnityMain(argc, argv, RunAllTests);
  return result;
}
