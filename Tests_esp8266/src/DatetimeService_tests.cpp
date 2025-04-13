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

TEST(LogicDatetimeServiceTestsGroup, SetCurrentSecond) {
    TestableDatetimeService testable;
    int curr = testable.GetCurrentSecond();
    int new_val = 42;
    if (curr == 42) {
        new_val = 19;
    }
    testable.SetCurrentSecond(new_val);
    DOUBLES_EQUAL(new_val, testable.GetCurrentSecond(), 1);
    testable.SetCurrentSecond(curr);
}

TEST(LogicDatetimeServiceTestsGroup, SetCurrentMinute) {
    TestableDatetimeService testable;
    int curr = testable.GetCurrentMinute();
    int new_val = 42;
    if (curr == 42) {
        new_val = 19;
    }
    testable.SetCurrentMinute(new_val);
    DOUBLES_EQUAL(new_val, testable.GetCurrentMinute(), 1);
    testable.SetCurrentMinute(curr);
}

TEST(LogicDatetimeServiceTestsGroup, SetCurrentHour) {
    TestableDatetimeService testable;
    int curr = testable.GetCurrentHour();
    int new_val = 4;
    if (curr == 4) {
        new_val = 1;
    }
    testable.SetCurrentHour(new_val);
    DOUBLES_EQUAL(new_val, testable.GetCurrentHour(), 1);
    testable.SetCurrentHour(curr);
}

TEST(LogicDatetimeServiceTestsGroup, SetCurrentDay) {
    TestableDatetimeService testable;
    int curr = testable.GetCurrentDay();
    int new_val = 4;
    if (curr == 4) {
        new_val = 19;
    }
    testable.SetCurrentDay(new_val);
    DOUBLES_EQUAL(new_val, testable.GetCurrentDay(), 1);
    testable.SetCurrentDay(curr);
}

TEST(LogicDatetimeServiceTestsGroup, SetCurrentMonth) {
    TestableDatetimeService testable;
    int curr = testable.GetCurrentMonth();
    int new_val = 4;
    if (curr == 4) {
        new_val = 9;
    }
    testable.SetCurrentMonth(new_val);
    DOUBLES_EQUAL(new_val, testable.GetCurrentMonth(), 1);
    testable.SetCurrentMonth(curr);
}

TEST(LogicDatetimeServiceTestsGroup, SetCurrentYear) {
    TestableDatetimeService testable;
    int curr = testable.GetCurrentMonth();
    int new_val = 142;
    if (curr == 142) {
        new_val = 119;
    }
    testable.SetCurrentYear(new_val);
    DOUBLES_EQUAL(new_val, testable.GetCurrentYear(), 1);
    testable.SetCurrentYear(curr);
}