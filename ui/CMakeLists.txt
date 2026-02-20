cmake_minimum_required(VERSION 3.22)

project(
	vst-display
	VERSION 1.0.0
	DESCRIPTION "vst display firmware"
	LANGUAGES C CXX
	)

set(VST_DISPLAY_BUILD_DIR ${CMAKE_SOURCE_DIR}/build/arduino)
set(VST_DISPLAY_BIN_DIR ${CMAKE_SOURCE_DIR}/build/bin)

# move debug level to cmake, instead of sketch.yaml
option(CMAKE_BUILD_TYPE "Debug")
if("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
  set(CMAKE_BUILD_TYPE Release)
else()
  set(CMAKE_BUILD_TYPE Debug)
  set(BUILD_ARGS --optimize-for-debug)
endif()
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
#set(COMPILE_COMMANDS_FILE ${VST_DISPLAY_BUILD_DIR}/compile_commands.json)

# Find arduino-cli executable
find_program(ARDUINO_CLI arduino-cli)

if(NOT ARDUINO_CLI)
	message(FATAL_ERROR "Failed to find arduino-cli executable!")
else()
	message(STATUS "Found arduino-cli executable!")
endif()

# Define sketch directory(.ino file)
set(SKETCH_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(FIRMWARE_BINARIES
    "${VST_DISPLAY_BIN_DIR}/*.bin"
    "${VST_DISPLAY_BIN_DIR}/*.elf"
    "${VST_DISPLAY_BIN_DIR}/*.map")

# Specify path to own LVGL config header
set(LV_BUILD_CONF_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include/lv_conf.h" CACHE PATH "" FORCE)

set(VST_INCLUDES
	${CMAKE_CURRENT_SOURCE_DIR}/include)

set(SENSCAP_EN_01 0)
if(SENSCAP)
  set(SENSCAP_EN_01 1)
endif()

if(SENSCAP_EN_01 EQUAL 0)
	set(FQBN "esp32:esp32:esp32s3:PSRAM=opi,FlashSize=16M,CDCOnBoot=cdc,JTAGAdapter=builtin,DebugLevel=none")
else()
	set(FQBN "esp32:esp32:esp32s3:PSRAM=opi,FlashSize=8M,CDCOnBoot=cdc,DebugLevel=none,JTAGAdapter=builtin")
endif()
# set(FQBN "esp32:esp32:esp32s3:PSRAM=opi,PartitionScheme=huge_app,FlashSize=16M,DebugLevel=none,JTAGAdapter=builtin,CDCOnBoot=cdc")

set(MAX_UPLOAD_SIZE 6291456) # bytes (6MB)

set(COMPILE_BASE_CMD
	${ARDUINO_CLI}
	compile
	--fqbn          ${FQBN}  
	--build-path    ${VST_DISPLAY_BUILD_DIR}
	--output-dir    ${VST_DISPLAY_BIN_DIR}
	-e
	--warnings      default
	--build-property
	"compiler.cpp.extra_flags=-DLV_CONF_INCLUDE_SIMPLE=1 -DLV_LVGL_H_INCLUDE_SIMPLE=1 -DLV_CONF_PATH=\"${LV_BUILD_CONF_PATH}\" -DFW_VER=\"${FW_VER}\" -I${VST_INCLUDES} -DSENSCAP_EN=${SENSCAP_EN_01}"
	--build-property
	"compiler.c.extra_flags=-DLV_CONF_INCLUDE_SIMPLE=1 -DLV_LVGL_H_INCLUDE_SIMPLE=1 -DLV_CONF_PATH=\"${LV_BUILD_CONF_PATH}\" -DFW_VER=\"${FW_VER}\" -I${VST_INCLUDES} -DSENSCAP_EN=${SENSCAP_EN_01}"
	--build-property
    "upload.maximum_size=${MAX_UPLOAD_SIZE}"
	${SKETCH_DIR})

set(COMPILE_CMD ${COMPILE_BASE_CMD} ${BUILD_ARGS})

# Add custom build target
add_custom_target(
	build ALL
	COMMENT "Building..."
	COMMAND ${COMPILE_CMD}
	DEPENDS ${ARDUINO_CLI}
	VERBATIM)

add_custom_target(
	upload
	COMMENT "Uploading firmware toport ${SERIAL_PORT}"
	COMMAND ${ARDUINO_CLI} upload --verify --input-dir
			${VST_DISPLAY_BIN_DIR} --port ${SERIAL_PORT}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS ${ARDUINO_CLI} ${FIRMWARE_BINARIES}
	VERBATIM)


string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_LOWER)

# set(FILE_PREFIX vst-sentry_${FW_VER}_${BUILD_TIMESTAMP}_${GIT_HASH}_esp32_${CMAKE_BUILD_TYPE_LOWER})

if(SENSCAP_EN_01 EQUAL 0)
set(FILE_PREFIX vst-sentry_${FW_VER}_${BUILD_TIMESTAMP}_${GIT_HASH}_esp32_${CMAKE_BUILD_TYPE_LOWER})
else()
set(FILE_PREFIX vst_senscap_display_${FW_VER}_${BUILD_TIMESTAMP}_${GIT_HASH}_esp32_${CMAKE_BUILD_TYPE_LOWER})
endif()

file(WRITE "${VST_DISPLAY_BIN_DIR}/${FILE_PREFIX}.ino" "")
file(COPY ${SKETCH_DIR}/sketch.yaml DESTINATION ${VST_DISPLAY_BIN_DIR})

# set(TAR_SOURCE_FILES "${VST_DISPLAY_BIN_DIR}/${FILE_PREFIX}.ino.bin" "${VST_DISPLAY_BIN_DIR}/${FILE_PREFIX}.ino.bootloader.bin" "${VST_DISPLAY_BIN_DIR}/${FILE_PREFIX}.ino.partitions.bin" "${VST_DISPLAY_BIN_DIR}/${FILE_PREFIX}.ino" "${VST_DISPLAY_BIN_DIR}/sketch.yaml")


add_custom_target(post_build ALL COMMAND	
	# rename all binary files to agreed upon standard format
	# see https://github.com/obhcare/call-light/issues/35
	COMMAND ${CMAKE_COMMAND} -E rename ${VST_DISPLAY_BIN_DIR}/ui.ino.bin ${VST_DISPLAY_BIN_DIR}/${FILE_PREFIX}.ino.bin
	COMMAND ${CMAKE_COMMAND} -E rename ${VST_DISPLAY_BIN_DIR}/ui.ino.bootloader.bin ${VST_DISPLAY_BIN_DIR}/${FILE_PREFIX}.ino.bootloader.bin
	COMMAND ${CMAKE_COMMAND} -E rename ${VST_DISPLAY_BIN_DIR}/ui.ino.elf ${VST_DISPLAY_BIN_DIR}/${FILE_PREFIX}.ino.elf
	COMMAND ${CMAKE_COMMAND} -E rename ${VST_DISPLAY_BIN_DIR}/ui.ino.map ${VST_DISPLAY_BIN_DIR}/${FILE_PREFIX}.ino.map
	COMMAND ${CMAKE_COMMAND} -E rename ${VST_DISPLAY_BIN_DIR}/ui.ino.partitions.bin ${VST_DISPLAY_BIN_DIR}/${FILE_PREFIX}.ino.partitions.bin

	# create tarball
	WORKING_DIRECTORY "${VST_DISPLAY_BIN_DIR}"
	COMMAND ${CMAKE_COMMAND} -E tar "czf" "${VST_DISPLAY_BIN_DIR}/${FILE_PREFIX}.tar.gz" "${VST_DISPLAY_BIN_DIR}/${FILE_PREFIX}.ino.bin" "${VST_DISPLAY_BIN_DIR}/${FILE_PREFIX}.ino.elf" "${VST_DISPLAY_BIN_DIR}/${FILE_PREFIX}.ino.bootloader.bin" "${VST_DISPLAY_BIN_DIR}/${FILE_PREFIX}.ino.partitions.bin" "${VST_DISPLAY_BIN_DIR}/${FILE_PREFIX}.ino" "${VST_DISPLAY_BIN_DIR}/sketch.yaml"

	# Add more commands as needed
VERBATIM)

# Make post_build dependent on your main target
add_dependencies(post_build build)