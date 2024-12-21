#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/WiFi/WiFiRequestItem.h"

TEST_GROUP(LogicWiFiRequestItemTestsGroup){ //
                                            TEST_SETUP(){}

                                            TEST_TEARDOWN(){}
};

TEST(LogicWiFiRequestItemTestsGroup, Compare_based_on_request_type) {
    RequestItemComparator comparator;

    RequestItem a = {};
    RequestItem b = {};

    a.type = RequestItemType::wqi_Connect;
    b.type = RequestItemType::wqi_Connect;
    CHECK_TRUE(comparator(a, b));
}
