cmake_minimum_required(VERSION 3.22)
include(FetchContent)

project(
  test
  DESCRIPTION "test"
  LANGUAGES C CXX
)

set(TEST_BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build)

set(TEST_INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../ui/include
)

file(GLOB_RECURSE SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/../ui/src/*.c
  ${CMAKE_CURRENT_SOURCE_DIR}/../ui/src/*.cpp
)

add_compile_options(-Wall -Wextra -Wno-unused-parameter)

add_executable(simtest
  ${SOURCES}
  tests/test.cpp
  tests/testIN.cpp
  tests/testGO.cpp
  tests/testRO.cpp
  tests/testCMD.cpp
)


# Specify path to own LVGL config header
set(LV_BUILD_CONF_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h" CACHE PATH "" FORCE)
set(CONFIG_LV_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_BUILD_DEMOS OFF CACHE BOOL "" FORCE)

# Fetch LVGL from GitHub
FetchContent_Declare(
  lvgl
  GIT_REPOSITORY https://github.com/lvgl/lvgl.git
  GIT_TAG v9.4.0
  GIT_SHALLOW YES
)
FetchContent_MakeAvailable(lvgl)

# Fetch ArduinoJson from GitHub
FetchContent_Declare(
  ArduinoJson
  GIT_REPOSITORY https://github.com/bblanchon/ArduinoJson.git
  GIT_TAG v7.0.3
  GIT_SHALLOW YES
)
FetchContent_MakeAvailable(ArduinoJson)

# Fetch Unity (ThrowTheSwitch) from GitHub
FetchContent_Declare(
  unity
  GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
  GIT_TAG v2.6.0
  GIT_SHALLOW YES
)
FetchContent_MakeAvailable(unity)

set(CMAKE_BUILD_TYPE Debug)
set(LVGL_TEST_DIR ${lvgl_SOURCE_DIR}/tests)
set(LVGL_SRC_DIR  ${lvgl_SOURCE_DIR}/src/others/test)

# Unity source dirs (ThrowTheSwitch)
set(UNITY_SRC_DIR        ${unity_SOURCE_DIR}/src)
set(UNITY_FIXTURE_DIR    ${unity_SOURCE_DIR}/extras/fixture/src)
set(UNITY_MEMORY_DIR     ${unity_SOURCE_DIR}/extras/memory/src)

# -------------------------------------------------------------------
# Patch Unity colors: change background to bold foreground (no edits
# in the fetched source). We generate a patched unity_fg.c in the
# build dir and use it in our Unity library.
# -------------------------------------------------------------------
set(UNITY_ORIG    "${UNITY_SRC_DIR}/unity.c")
set(UNITY_PATCHED "${CMAKE_BINARY_DIR}/unity_fg.c")

file(READ "${UNITY_ORIG}" UNITY_SRC)

string(REPLACE "\\033[42mPASS\\033[0m"   "\\033[1;32mPASS\\033[0m"   UNITY_SRC "${UNITY_SRC}")
string(REPLACE "\\033[42mOK\\033[0m"     "\\033[1;32mOK\\033[0m"     UNITY_SRC "${UNITY_SRC}")
string(REPLACE "\\033[41mFAIL\\033[0m"   "\\033[1;31mFAIL\\033[0m"   UNITY_SRC "${UNITY_SRC}")
string(REPLACE "\\033[43mIGNORE\\033[0m" "\\033[1;33mIGNORE\\033[0m" UNITY_SRC "${UNITY_SRC}")

file(WRITE "${UNITY_PATCHED}" "${UNITY_SRC}")

get_filename_component(UNITY_ORIG_REAL    "${UNITY_ORIG}"    REALPATH)
get_filename_component(UNITY_PATCHED_REAL "${UNITY_PATCHED}" REALPATH)

message(STATUS "Unity original : ${UNITY_ORIG_REAL}")
message(STATUS "Unity patched  : ${UNITY_PATCHED_REAL}")
# -------------------------------------------------------------------

# Unity + Unity Fixture library (ThrowTheSwitch)
add_library(unity_fixture_lib STATIC
  ${UNITY_PATCHED_REAL}
  ${UNITY_FIXTURE_DIR}/unity_fixture.c
  ${UNITY_MEMORY_DIR}/unity_memory.c
)

target_include_directories(unity_fixture_lib PUBLIC
  ${UNITY_SRC_DIR}
  ${UNITY_FIXTURE_DIR}
  ${UNITY_MEMORY_DIR}
  ${unity_SOURCE_DIR}
)

# ✅ IMPORTANT: apply output hook macros HERE (Unity is compiled in this target)
target_compile_definitions(unity_fixture_lib PUBLIC
  UNITY_OUTPUT_COLOR
  UNITY_OUTPUT_CHAR=UnityOutputChar
  UNITY_OUTPUT_FLUSH=UnityOutputFlush
  UNITY_INCLUDE_PRINT_FORMATTED=1
)


set(LVGL_TEST_SRCS
  ${LVGL_TEST_DIR}/src/lv_test_init.c
  ${LVGL_SRC_DIR}/lv_test_indev.c
  ${LVGL_SRC_DIR}/lv_test_helpers.c
  ${LVGL_SRC_DIR}/lv_test_screenshot_compare.c
)

add_library(lvgl_test_helpers STATIC ${LVGL_TEST_SRCS})

target_include_directories(lvgl_test_helpers PUBLIC
  ${CMAKE_SOURCE_DIR}
  ${LVGL_SRC_DIR}
  ${LVGL_TEST_DIR}/src
  ${lvgl_SOURCE_DIR}/src
)

# ✅ IMPORTANT: lvgl test helpers also include Unity headers/macros
target_compile_definitions(lvgl_test_helpers PUBLIC
  LV_CONF_INCLUDE_SIMPLE=1
  LV_BUILD_CONF_PATH="${CMAKE_SOURCE_DIR}/lv_conf.h"
  LV_BUILD_TEST=1
  LV_USE_TEST=1
  UNITY_OUTPUT_COLOR
  UNITY_OUTPUT_CHAR=UnityOutputChar
  UNITY_OUTPUT_FLUSH=UnityOutputFlush
)

target_compile_features(lvgl_test_helpers PRIVATE c_std_11)
target_compile_options(lvgl_test_helpers PRIVATE -Wall -Wextra -Wno-unused-parameter)

target_link_libraries(lvgl_test_helpers PUBLIC
  unity_fixture_lib
)

target_compile_definitions(simtest PUBLIC
  SIMULATOR=1
  LV_USE_TEST=1
  LV_USE_LODEPNG=1
  LV_USE_TEST_SCREENSHOT_COMPARE=1
  LV_USE_SNAPSHOT=1
  LV_USE_LOG=1
  FW_VER="${FW_VER}"
  UNITY_OUTPUT_CHAR=UnityOutputChar
  UNITY_OUTPUT_FLUSH=UnityOutputFlush
)

target_link_libraries(simtest PUBLIC lvgl::lvgl ArduinoJson lvgl_test_helpers unity_fixture_lib)
target_compile_features(simtest PUBLIC cxx_std_11)

target_include_directories(simtest PUBLIC
  ${TEST_INCLUDES}
  ${lvgl_SOURCE_DIR}/tests
  ${UNITY_SRC_DIR}
  ${UNITY_FIXTURE_DIR}
  ${UNITY_MEMORY_DIR}
)


