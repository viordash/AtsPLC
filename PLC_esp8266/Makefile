#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := AtsPLC-esp8266

export IDF_PATH := $(PWD)/../SDK/ESP8266_RTOS_SDK

PATH  := $(PATH):$(PWD)/../SDK/xtensa-lx106-elf/bin
SHELL := env PATH=$(PATH) /bin/bash

xtensa-lx106-elf-gcc:
	@tar -xzf $(PWD)/../SDK/xtensa-lx106-elf-gcc8_4_0-esp-2020r3-linux-amd64.tar.gz -C $(PWD)/../SDK/

ifeq ($(filter xtensa-lx106-elf-gcc,$(MAKECMDGOALS)),)
include $(IDF_PATH)/make/project.mk
endif

