#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := AtsPLC-esp8266

export IDF_PATH := $(PWD)/../SDK/ESP8266_RTOS_SDK

PATH  := $(PATH):$(PWD)/../SDK/xtensa-lx106-elf/bin
SHELL := env PATH=$(PATH) /bin/bash

project_prepare:
	@tar -xzf $(PWD)/../SDK/xtensa-lx106-elf-gcc8_4_0-esp-2020r3-linux-amd64.tar.gz -C $(PWD)/../SDK/
	@ln -sf $(PWD)/../SDK/MigrateAnyCppData/MigrateAnyData/MigrateAnyData.cpp $(PWD)/main/MigrateAnyData/MigrateAnyData.cpp
	@ln -sf $(PWD)/../SDK/MigrateAnyCppData/MigrateAnyData/MigrateAnyData.h $(PWD)/main/MigrateAnyData/MigrateAnyData.h

ifeq ($(filter project_prepare,$(MAKECMDGOALS)),)
include $(IDF_PATH)/make/project.mk
endif

