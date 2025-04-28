#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := AtsPLC-esp8266

export IDF_PATH := $(PWD)/../SDK/ESP8266_RTOS_SDK

CURR_DIR := $(PWD)
ROOT_DIR := $(shell git rev-parse --show-toplevel)
PATH  := $(PATH):$(PWD)/../SDK/xtensa-lx106-elf/bin
SHELL := env PATH=$(PATH) /bin/bash

project_prepare: .project_prepare .apply-patches 

.project_prepare:
	@echo "----------------------- Preparing"
	@tar -xzf $(PWD)/../SDK/xtensa-lx106-elf-gcc8_4_0-esp-2020r3-linux-amd64.tar.gz -C $(PWD)/../SDK/
	@mkdir -p $(PWD)/main/MigrateAnyData
	@ln -sf $(PWD)/../SDK/MigrateAnyCppData/MigrateAnyData/MigrateAnyData.cpp $(PWD)/main/MigrateAnyData/MigrateAnyData.cpp
	@ln -sf $(PWD)/../SDK/MigrateAnyCppData/MigrateAnyData/MigrateAnyData.h $(PWD)/main/MigrateAnyData/MigrateAnyData.h

ifeq ($(filter project_prepare,$(MAKECMDGOALS)),)
include $(IDF_PATH)/make/project.mk
endif

PATCH_DIR := $(CURR_DIR)/patches
PATCH_FILES := $(sort $(wildcard $(PATCH_DIR)/*.patch))

.apply-patches: clean-apply-patches .project_prepare $(PATCH_FILES)
	@echo "----------------------- Patching"
	@for patch in $(PATCH_FILES); do \
		echo "Checking patch: $$patch"; \
		pushd $(ROOT_DIR); \
		git apply --numstat $$patch | \
			awk 'NF == 3 {print $$3}' | \
			while read file; do \
				echo "Marking $$file as assume-unchanged..."; \
				git update-index --assume-unchanged "$$file"; \
			done; \
		echo "Applying patch: $$patch"; \
		git apply $$patch || exit 1; \
		popd; \
	done
	@touch ".apply-patches"

clean-apply-patches:
	@echo "----------------------- Reverse Patches"
	@for patch in $(PATCH_FILES); do \
		pushd $(ROOT_DIR); \
		git apply -R $$patch || true; \
		popd; \
	done
	rm -f ".apply-patches"


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
	@CodeChecker check --build "make -j$$(nproc) project_prepare && make -j$$(nproc) clean && make -j$$(nproc) size" --analyzers cppcheck \
		--no-missing-checker-error \
		--disable android-cloexec-accept \
		--skip CodeChecker.skipfile \
		--output build/CodeChecker/check_static/reports --clean	|| true
	CodeChecker parse --export html --output build/CodeChecker/reports/check_static build/CodeChecker/check_static/reports > /dev/null 2>&1	