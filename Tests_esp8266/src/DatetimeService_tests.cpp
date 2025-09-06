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
        TestableDatetimeService() : DatetimeService(), time_value{} {
            setenv("TZ", "Etc/GMT-3", 1);
        }
        virtual ~TestableDatetimeService() {
        }
        bool PublicMorozov_EnableSntp() {
            return EnableSntp();
        }

        timeval time_value;

        void GetCurrent(timeval *tv) override {
            // struct tm tm = {};
            // tm.tm_sec = 0;
            // tm.tm_min = 0;
            // tm.tm_hour = 0;
            // tm.tm_mday = 1;
            // tm.tm_mon = 0;
            // tm.tm_year = 2025 - DatetimeService::YearOffset;
            *tv = time_value; // { mktime(&tm), 0 };
        }

        void SetCurrent(const timeval *tv) override {
            time_value = *tv;
        }
    };
} // namespace

TEST(LogicDatetimeServiceTestsGroup, GetCurrentSecond) {
    TestableDatetimeService testable;
    testable.time_value.tv_sec = 1626381779;

    CHECK_EQUAL(59, testable.GetCurrentSecond());
}

TEST(LogicDatetimeServiceTestsGroup, GetCurrentMinute) {
    TestableDatetimeService testable;
    testable.time_value.tv_sec = 1626381779;

    CHECK_EQUAL(42, testable.GetCurrentMinute());
}

TEST(LogicDatetimeServiceTestsGroup, GetCurrentHour) {
    TestableDatetimeService testable;
    testable.time_value.tv_sec = 1626381779;

    CHECK_EQUAL(23, testable.GetCurrentHour());
}

TEST(LogicDatetimeServiceTestsGroup, GetCurrentDay) {
    TestableDatetimeService testable;
    testable.time_value.tv_sec = 1626381779;

    CHECK_EQUAL(15, testable.GetCurrentDay());
}

TEST(LogicDatetimeServiceTestsGroup, GetCurrentWeekday) {
    TestableDatetimeService testable;
    const time_t Monday = 1626716579;
    const time_t Tuesday = 1626802979;
    const time_t Wednesday = 1626889379;
    const time_t Thursday = 1626381779;
    const time_t Friday = 1626457379;
    const time_t Saturday = 1626543779;
    const time_t Sunday = 1626630179;

    testable.time_value.tv_sec = Monday;
    CHECK_EQUAL(01, testable.GetCurrentWeekday());
    testable.time_value.tv_sec = Tuesday;
    CHECK_EQUAL(02, testable.GetCurrentWeekday());
    testable.time_value.tv_sec = Wednesday;
    CHECK_EQUAL(03, testable.GetCurrentWeekday());
    testable.time_value.tv_sec = Thursday;
    CHECK_EQUAL(04, testable.GetCurrentWeekday());
    testable.time_value.tv_sec = Friday;
    CHECK_EQUAL(05, testable.GetCurrentWeekday());
    testable.time_value.tv_sec = Saturday;
    CHECK_EQUAL(06, testable.GetCurrentWeekday());
    testable.time_value.tv_sec = Sunday;
    CHECK_EQUAL(07, testable.GetCurrentWeekday());
}

TEST(LogicDatetimeServiceTestsGroup, GetCurrentMonth) {
    TestableDatetimeService testable;
    testable.time_value.tv_sec = 1626381779;

    CHECK_EQUAL(07, testable.GetCurrentMonth());
}

TEST(LogicDatetimeServiceTestsGroup, GetCurrentYear) {
    TestableDatetimeService testable;
    testable.time_value.tv_sec = 1626381779;

    CHECK_EQUAL(121, testable.GetCurrentYear());
}

TEST(LogicDatetimeServiceTestsGroup, Set_new_datetime) {
    mock().expectNCalls(1, "xTaskGenericNotify").ignoreOtherParameters();

    TestableDatetimeService testable;
    Datetime datetime;
    datetime.second = 59;
    datetime.minute = 42;
    datetime.hour = 23;
    datetime.day = 15;
    datetime.month = 07;
    datetime.year = 2021;

    testable.ManualSet(&datetime);
    CHECK_EQUAL(1626381779, testable.time_value.tv_sec);
    CHECK_EQUAL(0, testable.time_value.tv_usec);
}

TEST(LogicDatetimeServiceTestsGroup, EnableSntp_depends_on_sntp_servers) {
    TestableDatetimeService testable;

    strcpy(settings.datetime.sntp_server_primary, "pool0.ntp.org");
    strcpy(settings.datetime.sntp_server_secondary, "pool1.ntp.org");
    CHECK_TRUE(testable.PublicMorozov_EnableSntp());

    settings.datetime.sntp_server_primary[0] = 0;
    settings.datetime.sntp_server_secondary[0] = 0;
    CHECK_FALSE(testable.PublicMorozov_EnableSntp());
}