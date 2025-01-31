#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

COMPONENT_PRIV_INCLUDEDIRS := . params Display LogicProgram WiFi Maintenance
COMPONENT_SRCDIRS := . MigrateAnyData HttpServer Display Display/ssd1306 Display/fonts Display/bitmaps \
	LogicProgram LogicProgram/Inputs LogicProgram/Outputs LogicProgram/Serializer LogicProgram/Bindings \
	WiFi Maintenance


DEVICE_VERSION:=0x20250107 #use for main/settings.cpp
DEVICE_VERSION_D:=$(shell printf "%d" $(DEVICE_VERSION))
CURRENT_DATE:=$(shell printf "%d" 0x$(shell date +'%Y%m%d'))
current_seconds:=$(shell date +'%s')                  
start_of_day=$(shell date -d 'today 00:00:00' +'%s')  
minutes_since_midnight=$(shell echo "($(current_seconds) - $(start_of_day)) / 60" | bc)
BUILD_NUMBER:=$(shell echo "($(CURRENT_DATE) - $(DEVICE_VERSION_D)) * 1440 + $(minutes_since_midnight)" | bc)
DEVICE_VERSION_D_HEX:=$(shell echo "obase=16; $(DEVICE_VERSION_D)" | bc)
BUILD_NUMBER_HEX:=$(shell echo "obase=16; $(BUILD_NUMBER)" | bc)
$(info ----build version---------------------v$(DEVICE_VERSION_D_HEX).$(BUILD_NUMBER_HEX)------ )
CPPFLAGS := -DDEVICE_SETTINGS_VERSION=${DEVICE_VERSION} -DBUILD_NUMBER=${BUILD_NUMBER}