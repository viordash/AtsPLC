#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/WiFi/WiFiRequests.h"

TEST_GROUP(LogicWiFiRequestsTestsGroup){ //
                                         TEST_SETUP(){}

                                         TEST_TEARDOWN(){}
};

namespace {
    class TestableWiFiRequests : public WiFiRequests {
      public:
        bool PublicMorozov_Equals(const RequestItem *a, const RequestItem *b) const {
            return Equals(a, b);
        }
    };
} // namespace

TEST(LogicWiFiRequestsTestsGroup, Equals_by_Type) {
    TestableWiFiRequests testable;

    RequestItem a = {};
    RequestItem b = {};

    a.Type = RequestItemType::wqi_Station;
    b.Type = RequestItemType::wqi_Station;
    CHECK_TRUE(testable.PublicMorozov_Equals(&a, &b));

    a.Type = RequestItemType::wqi_Station;
    b.Type = RequestItemType::wqi_Scanner;
    CHECK_FALSE(testable.PublicMorozov_Equals(&a, &b));

    a.Type = RequestItemType::wqi_Scanner;
    b.Type = RequestItemType::wqi_Scanner;
    CHECK_TRUE(testable.PublicMorozov_Equals(&a, &b));

    a.Type = RequestItemType::wqi_Scanner;
    b.Type = RequestItemType::wqi_AccessPoint;
    CHECK_FALSE(testable.PublicMorozov_Equals(&a, &b));

    a.Type = RequestItemType::wqi_AccessPoint;
    b.Type = RequestItemType::wqi_AccessPoint;
    CHECK_TRUE(testable.PublicMorozov_Equals(&a, &b));
}

TEST(LogicWiFiRequestsTestsGroup, Equals_by_Scanner_payload) {
    TestableWiFiRequests testable;

    char ssid[16];
    char ssid_with_same_value_but_diff_address[16];
    strcpy(ssid, "test");
    strcpy(ssid_with_same_value_but_diff_address, "test");

    RequestItem a = {};
    RequestItem b = {};

    a.Type = RequestItemType::wqi_Scanner;
    b.Type = RequestItemType::wqi_Scanner;
    a.Payload.Scanner.ssid = ssid;
    b.Payload.Scanner.ssid = ssid;
    CHECK_TRUE(testable.PublicMorozov_Equals(&a, &b));

    b.Payload.Scanner.ssid = ssid_with_same_value_but_diff_address;
    CHECK_FALSE(testable.PublicMorozov_Equals(&a, &b));

    a.Payload.Scanner.ssid = ssid_with_same_value_but_diff_address;
    CHECK_TRUE(testable.PublicMorozov_Equals(&a, &b));

    CHECK_TRUE_TEXT(testable.PublicMorozov_Equals(&a, &b), "status should not affect comparison");

    CHECK_TRUE_TEXT(testable.PublicMorozov_Equals(&a, &b), "status should not affect comparison");
}

TEST(LogicWiFiRequestsTestsGroup, Equals_by_AccessPoint_payload) {
    TestableWiFiRequests testable;

    char ssid[16];
    char ssid_with_same_value_but_diff_address[16];
    strcpy(ssid, "test");
    strcpy(ssid_with_same_value_but_diff_address, "test");

    RequestItem a = {};
    RequestItem b = {};

    a.Type = RequestItemType::wqi_AccessPoint;
    b.Type = RequestItemType::wqi_AccessPoint;
    a.Payload.AccessPoint.ssid = ssid;
    b.Payload.AccessPoint.ssid = ssid;
    CHECK_TRUE(testable.PublicMorozov_Equals(&a, &b));

    b.Payload.AccessPoint.ssid = ssid_with_same_value_but_diff_address;
    CHECK_FALSE(testable.PublicMorozov_Equals(&a, &b));

    a.Payload.AccessPoint.ssid = ssid_with_same_value_but_diff_address;
    CHECK_TRUE(testable.PublicMorozov_Equals(&a, &b));
}

TEST(LogicWiFiRequestsTestsGroup, Scan_is_unique) {
    TestableWiFiRequests testable;

    CHECK_EQUAL(0, testable.size());

    const char *ssid = "test";

    CHECK_TRUE(testable.Scan(ssid));
    CHECK_EQUAL(1, testable.size());

    CHECK_FALSE(testable.Scan(ssid));
    CHECK_EQUAL(1, testable.size());
    CHECK_EQUAL(RequestItemType::wqi_Scanner, testable.back().Type);
    CHECK_EQUAL(ssid, testable.back().Payload.Scanner.ssid);
}

TEST(LogicWiFiRequestsTestsGroup, Pop_is_FIFO_compliant) {
    TestableWiFiRequests testable;

    const char *ssid_0 = "test_0";
    const char *ssid_1 = "test_1";

    CHECK_TRUE(testable.Scan(ssid_0));
    CHECK_TRUE(testable.AccessPoint(ssid_0));
    CHECK_TRUE(testable.Scan(ssid_1));
    CHECK_TRUE(testable.AccessPoint(ssid_1));
    CHECK_EQUAL(4, testable.size());

    RequestItem request;
    CHECK_TRUE(testable.Pop(&request));
    CHECK_EQUAL(RequestItemType::wqi_Scanner, request.Type);
    STRCMP_EQUAL("test_0", request.Payload.Scanner.ssid);
    testable.pop_back();

    CHECK_TRUE(testable.Pop(&request));
    CHECK_EQUAL(RequestItemType::wqi_AccessPoint, request.Type);
    STRCMP_EQUAL("test_0", request.Payload.AccessPoint.ssid);
    testable.pop_back();

    CHECK_TRUE(testable.Pop(&request));
    CHECK_EQUAL(RequestItemType::wqi_Scanner, request.Type);
    STRCMP_EQUAL("test_1", request.Payload.Scanner.ssid);
    testable.pop_back();

    CHECK_TRUE(testable.Pop(&request));
    CHECK_EQUAL(RequestItemType::wqi_AccessPoint, request.Type);
    STRCMP_EQUAL("test_1", request.Payload.AccessPoint.ssid);
    testable.pop_back();
}

TEST(LogicWiFiRequestsTestsGroup, StationDone_removes_Station_request) {
    TestableWiFiRequests testable;

    const char *ssid_0 = "test_0";

    testable.Scan(ssid_0);
    testable.AccessPoint(ssid_0);
    testable.Station();
    CHECK_EQUAL(3, testable.size());

    testable.RemoveStation();
    CHECK_EQUAL(2, testable.size());

    for (auto it = testable.begin(); it != testable.end(); it++) {
        auto request = *it;
        CHECK(request.Type != RequestItemType::wqi_Station);
    }
}

TEST(LogicWiFiRequestsTestsGroup, RemoveAccessPoint) {
    TestableWiFiRequests testable;

    const char *ssid_0 = "test_0";

    testable.Scan(ssid_0);
    testable.AccessPoint(ssid_0);
    testable.Station();
    CHECK_EQUAL(3, testable.size());

    testable.RemoveAccessPoint(ssid_0);
    CHECK_EQUAL(2, testable.size());

    for (auto it = testable.begin(); it != testable.end(); it++) {
        auto request = *it;
        CHECK(request.Type != RequestItemType::wqi_AccessPoint);
    }
}

TEST(LogicWiFiRequestsTestsGroup, RemoveScanner) {
    TestableWiFiRequests testable;

    const char *ssid_0 = "test_0";
    const char *ssid_0_but_diff_address = "test_0";
    const char *ssid_1 = "test_1";

    testable.AccessPoint(ssid_0);
    testable.Scan(ssid_0);
    testable.Scan(ssid_1);
    testable.AccessPoint(ssid_1);
    testable.Station();
    CHECK_EQUAL(5, testable.size());

    CHECK_TRUE(testable.RemoveScanner(ssid_0));
    CHECK_EQUAL(4, testable.size());

    CHECK_FALSE(testable.RemoveScanner(ssid_0_but_diff_address));
    CHECK_EQUAL(4, testable.size());
}