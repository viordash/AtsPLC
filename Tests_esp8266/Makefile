include $(PWD)/../TestsFramework/cpputest.mk
#
# 'make'        build executable file 'main'
# 'make clean'  removes all .o and executable files
#


SOURCE_PROJECT_DIR := -I$(PWD)/../PLC_esp8266 -I$(PWD)/../PLC_esp8266/main -I$(PWD)/../PLC_esp8266/main/Display
CPPUTEST_INCLUDE_DIR := $(PWD)/../TestsFramework/CppUTest/include

# define the Cpp compiler to use
CXX = g++
CC = gcc

# define any compile-time flags
CXXFLAGS	:= -Wall -Wextra -g -std=gnu++0x
CFLAGS	:= -Wall -Wextra -g

# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like:
LFLAGS = -lCppUTest

# define output directory
OUTPUT	:= output

# define source directory
SRC		:= src

INCLUDE := params


ifeq ($(OS),Windows_NT)
MAIN	:= main.exe
SOURCEDIRS	:= $(SRC)
INCLUDEDIRS	:= $(INCLUDE)
LIBDIRS		:= $(LIB)
FIXPATH = $(subst /,\,$1)
RM			:= del /q /f
MD	:= mkdir
CXXFLAGS	+= -D__MINGW_PRINTF_FORMAT=printf -D__USE_MINGW_ANSI_STDIO=1
else
MAIN	:= main
SOURCEDIRS	:= $(shell find $(SRC) -type d)
INCLUDEDIRS	:= $(shell find $(INCLUDE) -type d)
LIBDIRS		:= $(shell find $(LIB) -type d)
FIXPATH = $1
RM = rm -f
MD	:= mkdir -p
endif

INCLUDES	:= $(patsubst %,-I%, $(SOURCEDIRS)) $(patsubst %,-I%, $(INCLUDEDIRS)) -I$(CPPUTEST_INCLUDE_DIR) $(SOURCE_PROJECT_DIR) \
	-include $(CPPUTEST_INCLUDE_DIR)/CppUTest/MemoryLeakDetectorMallocMacros.h \
	-include $(CPPUTEST_INCLUDE_DIR)/CppUTest/MemoryLeakDetectorNewMacros.h

# define the C libs
LIBS		:= -Llib

# define the C source files
CXX_SOURCES		:= $(wildcard $(patsubst %,%/*.cpp, $(SOURCEDIRS)))
C_SOURCES		:= $(wildcard $(patsubst %,%/*.c, $(SOURCEDIRS)))

# define the C object files
CXX_OBJECTS		:= $(CXX_SOURCES:.cpp=.o)
C_OBJECTS		:= $(C_SOURCES:.c=.o)

# define the dependency output files
DEPS		:= $(CXX_OBJECTS:.o=.d) $(C_OBJECTS:.o=.d)

#
# The following part of the makefile is generic; it can be used to
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

.DEFAULT_GOAL := all

OUTPUTMAIN	:= $(call FIXPATH,$(OUTPUT)/$(MAIN))

all: $(OUTPUT) $(MAIN)
	@echo Executing 'all' complete!

$(OUTPUT):
	$(MD) $(OUTPUT)

$(MAIN): libCppUTest.a $(CXX_OBJECTS) $(C_OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(OUTPUTMAIN) $(CXX_OBJECTS) $(C_OBJECTS) $(LIBS) $(LFLAGS)

# include all .d files
-include $(DEPS)

# this is a suffix replacement rule for building .o's and .d's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file)
# -MMD generates dependency output files same name as the .o file
# (see the gnu make manual section about automatic variables)
.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -MMD $<  -o $@

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c -MMD $<  -o $@	

.PHONY: clean
clean: cleanlib
	-$(RM) $(call FIXPATH,$(OUTPUTMAIN))
	-$(RM) $(call FIXPATH,$(DEPS))
	-$(RM) $(call FIXPATH,$(CXX_OBJECTS))
	-$(RM) $(call FIXPATH,$(C_OBJECTS))
	@echo Cleanup complete!

run: all
	./$(OUTPUTMAIN)
	@echo Executing 'run: all' complete!
