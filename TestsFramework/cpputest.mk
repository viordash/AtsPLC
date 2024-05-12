# 
#  CppUTest library
# 

_SRC := \
    CppUTest/CommandLineArguments.cpp \
    CppUTest/CommandLineTestRunner.cpp \
    CppUTest/JUnitTestOutput.cpp \
    CppUTest/MemoryLeakDetector.cpp \
    CppUTest/MemoryLeakWarningPlugin.cpp \
    CppUTest/SimpleString.cpp \
    CppUTest/SimpleStringInternalCache.cpp \
    CppUTest/SimpleMutex.cpp \
    CppUTest/TeamCityTestOutput.cpp \
    CppUTest/TestFailure.cpp \
    CppUTest/TestFilter.cpp \
    CppUTest/TestHarness_c.cpp \
    CppUTest/TestMemoryAllocator.cpp \
    CppUTest/TestOutput.cpp \
    CppUTest/TestPlugin.cpp \
    CppUTest/TestRegistry.cpp \
    CppUTest/TestResult.cpp \
    CppUTest/TestTestingFixture.cpp \
    CppUTest/Utest.cpp \
    Platforms/Gcc/UtestPlatform.cpp

_EXT_SRC := \
   CppUTestExt/CodeMemoryReportFormatter.cpp \
   CppUTestExt/GTest.cpp \
   CppUTestExt/IEEE754ExceptionsPlugin.cpp \
   CppUTestExt/MemoryReportAllocator.cpp \
   CppUTestExt/MemoryReporterPlugin.cpp \
   CppUTestExt/MemoryReportFormatter.cpp \
   CppUTestExt/MockActualCall.cpp \
   CppUTestExt/MockExpectedCall.cpp \
   CppUTestExt/MockExpectedCallsList.cpp \
   CppUTestExt/MockFailure.cpp \
   CppUTestExt/MockNamedValue.cpp \
   CppUTestExt/MockSupport.cpp \
   CppUTestExt/MockSupportPlugin.cpp \
   CppUTestExt/MockSupport_c.cpp \
   CppUTestExt/OrderedTest.cpp
  

_DIR = ../TestsFramework/CppUTest

#
# 'make libCppUTest.a'        build executable file 'main'
# 'make clean'  removes all .o and executable files
#

# define the Cpp compiler to use
LIB_CXX = g++
LIB_AR = ar

# define any compile-time flags
LIB_CXXFLAGS	:= -std=c++17 -Wall -Wextra -g -fPIC -shared -DCPPUTEST_HAVE_GETTIMEOFDAY

# define output directory
LIB_OUTPUT	:= lib

# define source directory
LIB_SRC		:= $(_DIR)/src

# define include directory
LIB_INCLUDE	:= $(_DIR)/include

ifeq ($(OS),Windows_NT)
LIB_FIXPATH = $(subst /,\,$1)
LIB_RM := del /q /f
LIB_MD	:= mkdir
LIB_CXXFLAGS	+= -D__MINGW_PRINTF_FORMAT=printf -D__USE_MINGW_ANSI_STDIO=1
else
LIB_FIXPATH = $1
LIB_RM := rm -f
LIB_MD	:= mkdir -p
endif

# define any directories containing header files other than /usr/include
LIB_INCLUDES	:= -I$(_DIR)/include 

# define the C source files
LIB_SOURCES		:= $(patsubst %, $(LIB_SRC)/%, $(_SRC) $(_EXT_SRC))

# define the C object files
LIB_OBJECTS		:= $(LIB_SOURCES:.cpp=.o)

# define the dependency output files
LIB_DEPS		:= $(LIB_OBJECTS:.o=.d)

#
# The following part of the makefile is generic; it can be used to
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

.DEFAULT_GOAL := error

error:
	@echo "Select appropriated target"

LIB_OUTPUTMAIN	:= $(call LIB_FIXPATH,$(LIB_OUTPUT)/libCppUTest.a)

$(LIB_OUTPUT):
	$(LIB_MD) $(LIB_OUTPUT)

libCppUTest.a: $(LIB_OUTPUT) $(LIB_OBJECTS)
	$(LIB_AR) rcs $(LIB_OUTPUTMAIN) $(LIB_OBJECTS)

# include all .d files
-include $(LIB_DEPS)

# this is a suffix replacement rule for building .o's and .d's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file)
# -MMD generates dependency output files same name as the .o file
# (see the gnu make manual section about automatic variables)
$(LIB_OBJECTS):
	$(LIB_CXX) $(LIB_CXXFLAGS) $(LIB_INCLUDES) -c -MMD $(@:.o=.cpp) -o $@


cleanlib:
	-$(LIB_RM) $(call LIB_FIXPATH,$(LIB_OUTPUT))
	-$(LIB_RM) $(call LIB_FIXPATH,$(LIB_DEPS))
	-$(LIB_RM) $(call LIB_FIXPATH,$(LIB_OBJECTS))
	@echo Libs cleanup complete!
