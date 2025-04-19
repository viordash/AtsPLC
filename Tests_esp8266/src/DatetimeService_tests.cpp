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

TEST(LogicDatetimeServiceTestsGroup, Set_new_datetime) {
    TestableDatetimeService testable;
    time_t t = time(NULL);
    struct tm tm_curr = *localtime(&t);
    Datetime datetime;

    datetime.second = tm_curr.tm_sec - 10;
    if (--datetime.second > 59) {
        datetime.second = 59;
    }
    datetime.minute = tm_curr.tm_min - 5;
    if (datetime.minute > 59) {
        datetime.minute = 59;
    }
    datetime.hour = tm_curr.tm_hour - 5;
    if (datetime.hour > 23) {
        datetime.hour = 23;
    }
    datetime.day = tm_curr.tm_mday - 5;
    if (datetime.day > 31) {
        datetime.day = 25;
    }
    datetime.month = (tm_curr.tm_mon + 1) - 5;
    if (datetime.month > 12) {
        datetime.month = 12;
    }
    datetime.year = tm_curr.tm_year - 5;
    if (datetime.year > 150) {
        datetime.year = 120;
    }

    testable.ManualSet(&datetime);
    DOUBLES_EQUAL(datetime.second, testable.GetCurrentSecond(), 1);
    DOUBLES_EQUAL(datetime.minute, testable.GetCurrentMinute(), 1);
    DOUBLES_EQUAL(datetime.hour, testable.GetCurrentHour(), 1);
    DOUBLES_EQUAL(datetime.day, testable.GetCurrentDay(), 1);
    DOUBLES_EQUAL(datetime.month, testable.GetCurrentMonth(), 1);
    DOUBLES_EQUAL(datetime.year, testable.GetCurrentYear(), 1);

    datetime.second = tm_curr.tm_sec;
    datetime.minute = tm_curr.tm_min;
    datetime.hour = tm_curr.tm_hour;
    datetime.day = tm_curr.tm_mday;
    datetime.month = tm_curr.tm_mon + 1;
    datetime.year = tm_curr.tm_year;
    testable.ManualSet(&datetime);
}