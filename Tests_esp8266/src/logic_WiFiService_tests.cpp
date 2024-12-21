#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/WiFi/WiFiService.h"

TEST_GROUP(LogicWiFiServiceTestsGroup){ //
                                        TEST_SETUP(){}

                                        TEST_TEARDOWN(){}
};

namespace {
    class TestableWiFiService : public WiFiService {
      public:
        TestableWiFiService() : WiFiService() {
        }
        virtual ~TestableWiFiService() {
        }

        std::unordered_set<RequestItem, RequestItemHash, RequestItemComparator> *
        PublicMorozov_Get_requests() {
            return &requests;
        }

        EventGroupHandle_t PublicMorozov_Get_event() {
            return event;
        }
    };
} // namespace

TEST(LogicWiFiServiceTestsGroup, Scan_requests_are_unique) {
    mock().expectOneCall("xEventGroupWaitBits").ignoreOtherParameters();

    TestableWiFiService testable;
    char buffer[32];
    sprintf(buffer, "0x%08X", WiFiService::NEW_REQUEST_BIT);
    mock(buffer)
        .expectNCalls(3, "xEventGroupSetBits")
        .withPointerParameter("xEventGroup", testable.PublicMorozov_Get_event());

    CHECK_EQUAL(0, testable.PublicMorozov_Get_requests()->size());

    testable.Scan("ssid_0");
    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests()->size());

    testable.Scan("ssid_1");
    CHECK_EQUAL(2, testable.PublicMorozov_Get_requests()->size());

    testable.Scan("ssid_1");
    CHECK_EQUAL(2, testable.PublicMorozov_Get_requests()->size());

    testable.Scan("ssid_0");
    CHECK_EQUAL(2, testable.PublicMorozov_Get_requests()->size());

    testable.Scan("ssid_2");
    CHECK_EQUAL(3, testable.PublicMorozov_Get_requests()->size());
}

TEST(LogicWiFiServiceTestsGroup, Generate_requests_are_unique) {
    mock().expectOneCall("xEventGroupWaitBits").ignoreOtherParameters();

    TestableWiFiService testable;
    char buffer[32];
    sprintf(buffer, "0x%08X", WiFiService::NEW_REQUEST_BIT);
    mock(buffer)
        .expectNCalls(3, "xEventGroupSetBits")
        .withPointerParameter("xEventGroup", testable.PublicMorozov_Get_event());

    CHECK_EQUAL(0, testable.PublicMorozov_Get_requests()->size());

    testable.Generate("ssid_0");
    CHECK_EQUAL(1, testable.PublicMorozov_Get_requests()->size());

    testable.Generate("ssid_1");
    CHECK_EQUAL(2, testable.PublicMorozov_Get_requests()->size());

    testable.Generate("ssid_1");
    CHECK_EQUAL(2, testable.PublicMorozov_Get_requests()->size());

    testable.Generate("ssid_0");
    CHECK_EQUAL(2, testable.PublicMorozov_Get_requests()->size());

    testable.Generate("ssid_2");
    CHECK_EQUAL(3, testable.PublicMorozov_Get_requests()->size());
}
