cmake_minimum_required(VERSION 3.22)
include(FetchContent)

project(
	vst-display
	VERSION 1.0.0
	DESCRIPTION "vst display simulator"
	LANGUAGES C CXX
	)

set(VST_DISPLAY_BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build)

set(VST_INCLUDES
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../ui/include
	)

file(GLOB_RECURSE SOURCES
 ${CMAKE_CURRENT_SOURCE_DIR}/../ui/src/*.c 
 ${CMAKE_CURRENT_SOURCE_DIR}/../ui/src/*.cpp
 )

add_compile_options(-Wall)

add_executable(vst-display
	${SOURCES}
	main.cpp
	mouse_cursor_icon.c
	)
# Specify path to own LVGL config header
set(LV_BUILD_CONF_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h" CACHE PATH "" FORCE)


# Fetch LVGL from GitHub
FetchContent_Declare(
	lvgl GIT_REPOSITORY https://github.com/lvgl/lvgl.git
	GIT_TAG v9.4.0
	GIT_SHALLOW YES)

FetchContent_MakeAvailable(lvgl)

# Fetch ArduinoJson from GitHub
FetchContent_Declare(
	ArduinoJson GIT_REPOSITORY https://github.com/bblanchon/ArduinoJson.git
	GIT_TAG v7.0.3
	GIT_SHALLOW YES)

FetchContent_MakeAvailable(ArduinoJson)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

target_compile_definitions(vst-display PUBLIC SIMULATOR=1 LV_BUILD_TEST=0 USE_SDL=1 FW_VER="${FW_VER}")

target_link_libraries(vst-display PUBLIC lvgl::lvgl ${SDL2_LIBRARIES} SDL2 ArduinoJson)
target_compile_definitions(lvgl PUBLIC USE_SDL=1)
target_compile_features(vst-display PUBLIC cxx_std_11)
target_include_directories(vst-display PUBLIC ${VST_INCLUDES})

set(CMAKE_BUILD_TYPE Debug)

string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_LOWER)
set(FILE_PREFIX vst-sentry_simulator_${FW_VER}_${BUILD_TIMESTAMP}_${GIT_HASH}_${CMAKE_BUILD_TYPE_LOWER})

add_custom_target(Post_build ALL COMMAND	
	# rename all binary files to agreed upon standard format
	# see https://github.com/obhcare/call-light/issues/35
	COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_SOURCE_DIR}/../build/simulator/vst-display ${CMAKE_CURRENT_SOURCE_DIR}/../build/simulator/${FILE_PREFIX}

	# Add more commands as needed
VERBATIM)

# Make post_build dependent on your main target
add_dependencies(Post_build vst-display)