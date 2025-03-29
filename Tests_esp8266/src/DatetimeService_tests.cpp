#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "LogicProgram/LogicElement.h"
#include "main/Datetime/DatetimeService.h"
#include "main/LogicProgram/Controller.h"
#include "main/settings.h"

TEST_GROUP(LogicDatetimeServiceTestsGroup){ //
                                            TEST_SETUP(){}

                                            TEST_TEARDOWN(){}
};

namespace {
    class TestableDatetimeService : public DatetimeService {
      public:
        TestableDatetimeService() : DatetimeService() {
        }
        virtual ~TestableDatetimeService() {
        }
    };
} // namespace

TEST(LogicDatetimeServiceTestsGroup, GetCurrentSecond) {
    TestableDatetimeService testable;
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    DOUBLES_EQUAL(tm.tm_sec, testable.GetCurrentSecond(), 1);
}

TEST(LogicDatetimeServiceTestsGroup, GetCurrentMinute) {
    TestableDatetimeService testable;
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    DOUBLES_EQUAL(tm.tm_min, testable.GetCurrentMinute(), 1);
}

TEST(LogicDatetimeServiceTestsGroup, GetCurrentHour) {
    TestableDatetimeService testable;
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    DOUBLES_EQUAL(tm.tm_hour, testable.GetCurrentHour(), 1);
}

TEST(LogicDatetimeServiceTestsGroup, GetCurrentDay) {
    TestableDatetimeService testable;
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    DOUBLES_EQUAL(tm.tm_mday, testable.GetCurrentDay(), 1);
}

TEST(LogicDatetimeServiceTestsGroup, GetCurrentWeekday) {
    TestableDatetimeService testable;
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    DOUBLES_EQUAL(tm.tm_wday + 1, testable.GetCurrentWeekday(), 1);
}

TEST(LogicDatetimeServiceTestsGroup, GetCurrentMonth) {
    TestableDatetimeService testable;
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    DOUBLES_EQUAL(tm.tm_mon + 1, testable.GetCurrentMonth(), 1);
}

TEST(LogicDatetimeServiceTestsGroup, GetCurrentYear) {
    TestableDatetimeService testable;
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    DOUBLES_EQUAL(tm.tm_year, testable.GetCurrentYear(), 1);
}
