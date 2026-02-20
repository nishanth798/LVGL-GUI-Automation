cmake_minimum_required(VERSION 3.22)

project(
	vst-display
	VERSION 1.0.0
	DESCRIPTION "vst display"
	LANGUAGES C CXX
	)

# Get git commit hash
execute_process(
  COMMAND git rev-parse --short HEAD
  OUTPUT_VARIABLE GIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)

message(STATUS "Git Hash: ${GIT_HASH}")


# Get git tag
execute_process(
  COMMAND git describe --tags
  OUTPUT_VARIABLE GIT_TAG
  OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)

message(STATUS "Git Tag: ${GIT_TAG}")

# Define a function to remove text after the last underscore
function(remove_hash_from_tag INPUT_STRING OUTPUT_VARIABLE)
  string(FIND "${INPUT_STRING}" "-" LAST_UNDERSCORE_INDEX REVERSE)
  if(LAST_UNDERSCORE_INDEX GREATER_EQUAL 0)
      string(SUBSTRING "${INPUT_STRING}" 0 ${LAST_UNDERSCORE_INDEX} TAG_WO_HASH)
      set(${OUTPUT_VARIABLE} "${TAG_WO_HASH}" PARENT_SCOPE)
  else()
      set(${OUTPUT_VARIABLE} "${INPUT_STRING}" PARENT_SCOPE)
  endif()
endfunction()

if(GIT_TAG)
  # string(REPLACE fw "v" FW_VER ${GIT_TAG})
  remove_hash_from_tag("${GIT_TAG}" FW_VER)
endif()
message(STATUS "Firmware Version: ${FW_VER}")

string(TIMESTAMP BUILD_TIMESTAMP "%Y%m%d")
message(STATUS "Build Timestamp: ${BUILD_TIMESTAMP}")

set(CONFIG_LV_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_BUILD_DEMOS OFF CACHE BOOL "" FORCE)

# Add ui and simulator folders as sub directories
option(ARDUINO OFF)
if(ARDUINO)
    add_subdirectory(ui)
endif()

option(SIMULATOR OFF) 
if(SIMULATOR)
    add_subdirectory(simulator)
endif()

# option(ALERT OFF)
# if(ALERT)
# # print weather alert is on or not
# message(STATUS "Alert mode is turned on")
# endif() 