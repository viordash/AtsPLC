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
	@mkdir -p $(PWD)/main/MigrateAnyData
	@ln -sf $(PWD)/../SDK/MigrateAnyCppData/MigrateAnyData/MigrateAnyData.cpp $(PWD)/main/MigrateAnyData/MigrateAnyData.cpp
	@ln -sf $(PWD)/../SDK/MigrateAnyCppData/MigrateAnyData/MigrateAnyData.h $(PWD)/main/MigrateAnyData/MigrateAnyData.h

ifeq ($(filter project_prepare,$(MAKECMDGOALS)),)
include $(IDF_PATH)/make/project.mk
endif

# define source directory
CF_SRC				:= main
CF_EXCLUDE_DIRS 	:= MigrateAnyData Display/fonts Display/ssd1306
CF_SOURCE_DIRS := $(shell find $(CF_SRC) $(foreach dir,$(CF_EXCLUDE_DIRS),-path '$(CF_SRC)/$(dir)' -prune -o) -type d -print)

# define the C header files
CF_HEADERS		:= $(wildcard $(patsubst %,%/*.h, $(CF_SOURCE_DIRS)))
# define the C source files
CF_SOURCES		:= $(wildcard $(patsubst %,%/*.c, $(CF_SOURCE_DIRS)))
# define the CPP source files
CF_SOURCES_CXX		:= $(wildcard $(patsubst %,%/*.cpp, $(CF_SOURCE_DIRS)))

apply_format:
	@clang-format --style=file:.clang-format $(CF_SOURCES) $(CF_SOURCES_CXX) $(CF_HEADERS) -i

check_format:
	@echo "----------------------- check_format"
	@clang-format --style=file:.clang-format $(CF_SOURCES) $(CF_SOURCES_CXX) $(CF_HEADERS) --Werror --dry-run

code_check:
	@echo "----------------------- Static analysis"
	CodeChecker check --build "make -j$$(nproc) project_prepare && make -j$$(nproc) clean && make -j$$(nproc) size" --analyzers cppcheck clangsa clang-tidy --ctu --disable-all \
		--enable severity:CRITICAL --enable severity:HIGH --enable severity:MEDIUM --enable severity:STYLE --no-missing-checker-error \
		--disable cppcheck-missingIncludeSystem \
		--disable security.insecureAPI.DeprecatedOrUnsafeBufferHandling \
		--disable clang-diagnostic-declaration-after-statement \
		--disable android-cloexec-accept \
		--disable clang-diagnostic-padded \
		--disable cppcheck-preprocessorErrorDirective \
		--skip CodeChecker.skipfile \
		--output build/CodeChecker/check_static/reports --clean	|| true
	CodeChecker parse --export html --output build/CodeChecker/reports/check_static build/CodeChecker/check_static/reports > /dev/null 2>&1	