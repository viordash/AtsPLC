#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

COMPONENT_PRIV_INCLUDEDIRS := . params Display LogicProgram WiFi Maintenance Datetime
COMPONENT_SRCDIRS := . MigrateAnyData HttpServer Display Display/ssd1306 Display/fonts Display/bitmaps \
	LogicProgram LogicProgram/Inputs LogicProgram/Outputs LogicProgram/Serializer LogicProgram/Bindings \
	LogicProgram/Settings LogicProgram/Flow WiFi Maintenance Datetime


#use for main/settings.cpp
DEVICE_VERSION:=20250619
DEVICE_VERSION_seconds:=$(shell date -d '$(DEVICE_VERSION)' +'%s')
current_seconds:=$(shell date +'%s')
minutes_since=$(shell echo "($(current_seconds) - $(DEVICE_VERSION_seconds)) / 60" | bc)
BUILD_NUMBER:=$(minutes_since)
$(info ----build version-----------------v$(DEVICE_VERSION).$(BUILD_NUMBER)------ )
ROOT_DIR := $(shell git rev-parse --show-toplevel)

# must match the ota_0/ota_1 partition size in partitions.csv
CPPFLAGS :=-std=gnu++17 -DDEVICE_SETTINGS_VERSION=0x${DEVICE_VERSION} -DBUILD_NUMBER=${BUILD_NUMBER} \
	-DFIRMWARE_MAXSIZE=1048576

COMMON_FLAGS :=
include $(ROOT_DIR)/build-tools/mk/web_files.mk

CPPFLAGS+=$(COMMON_FLAGS) -DSCRATCH_BUFSIZE=4096
COMPONENT_EMBED_FILES := $(WEB_DIST_SOURCES)
