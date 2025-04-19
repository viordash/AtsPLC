#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "tests_utils.h"

#include "main/Datetime/Datetime.h"

TEST_GROUP(DatetimeTestsGroup){ //
                                TEST_SETUP(){}

                                TEST_TEARDOWN(){}
};

TEST(DatetimeTestsGroup, ValidateDatetime_date) {
    Datetime datetime;
    datetime.year = 2025;
    datetime.month = 01;
    datetime.day = 01;
    datetime.hour = 00;
    datetime.minute = 00;
    datetime.second = 00;

    CHECK_TRUE(ValidateDatetime(&datetime));
    datetime.year = 2019;
    CHECK_FALSE(ValidateDatetime(&datetime));
    datetime.year = 2101;
    CHECK_FALSE(ValidateDatetime(&datetime));
    datetime.year = 2020;
    CHECK_TRUE(ValidateDatetime(&datetime));
    datetime.year = 2100;
    CHECK_TRUE(ValidateDatetime(&datetime));

    datetime.month = 0;
    CHECK_FALSE(ValidateDatetime(&datetime));
    datetime.month = 13;
    CHECK_FALSE(ValidateDatetime(&datetime));
    datetime.month = 1;
    CHECK_TRUE(ValidateDatetime(&datetime));
    datetime.month = 12;
    CHECK_TRUE(ValidateDatetime(&datetime));

    datetime.day = 0;
    CHECK_FALSE(ValidateDatetime(&datetime));
    datetime.day = 32;
    CHECK_FALSE(ValidateDatetime(&datetime));
    datetime.day = 1;
    CHECK_TRUE(ValidateDatetime(&datetime));
    datetime.day = 31;
    CHECK_TRUE(ValidateDatetime(&datetime));
}

TEST(DatetimeTestsGroup, validate_time_settings) {
    Datetime datetime;
    datetime.year = 2025;
    datetime.month = 01;
    datetime.day = 01;
    datetime.hour = 00;
    datetime.minute = 00;
    datetime.second = 00;

    CHECK_TRUE(ValidateDatetime(&datetime));
    datetime.hour = 24;
    CHECK_FALSE(ValidateDatetime(&datetime));
    datetime.hour = 0;
    CHECK_TRUE(ValidateDatetime(&datetime));
    datetime.hour = 23;
    CHECK_TRUE(ValidateDatetime(&datetime));

    datetime.minute = 60;
    CHECK_FALSE(ValidateDatetime(&datetime));
    datetime.minute = 0;
    CHECK_TRUE(ValidateDatetime(&datetime));
    datetime.minute = 59;
    CHECK_TRUE(ValidateDatetime(&datetime));

    datetime.second = 60;
    CHECK_FALSE(ValidateDatetime(&datetime));
    datetime.second = 0;
    CHECK_TRUE(ValidateDatetime(&datetime));
    datetime.second = 59;
    CHECK_TRUE(ValidateDatetime(&datetime));
}