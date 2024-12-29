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

TEST(LogicWiFiRequestsTestsGroup, Equals_by_type) {
    TestableWiFiRequests testable;

    RequestItem a = {};
    RequestItem b = {};

    a.type = RequestItemType::wqi_Station;
    b.type = RequestItemType::wqi_Station;
    CHECK_TRUE(testable.PublicMorozov_Equals(&a, &b));

    a.type = RequestItemType::wqi_Station;
    b.type = RequestItemType::wqi_Scanner;
    CHECK_FALSE(testable.PublicMorozov_Equals(&a, &b));

    a.type = RequestItemType::wqi_Scanner;
    b.type = RequestItemType::wqi_Scanner;
    CHECK_TRUE(testable.PublicMorozov_Equals(&a, &b));

    a.type = RequestItemType::wqi_Scanner;
    b.type = RequestItemType::wqi_AccessPoint;
    CHECK_FALSE(testable.PublicMorozov_Equals(&a, &b));

    a.type = RequestItemType::wqi_AccessPoint;
    b.type = RequestItemType::wqi_AccessPoint;
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

    a.type = RequestItemType::wqi_Scanner;
    b.type = RequestItemType::wqi_Scanner;
    a.Payload.Scanner.ssid = ssid;
    b.Payload.Scanner.ssid = ssid;
    CHECK_TRUE(testable.PublicMorozov_Equals(&a, &b));

    b.Payload.Scanner.ssid = ssid_with_same_value_but_diff_address;
    CHECK_FALSE(testable.PublicMorozov_Equals(&a, &b));

    a.Payload.Scanner.ssid = ssid_with_same_value_but_diff_address;
    CHECK_TRUE(testable.PublicMorozov_Equals(&a, &b));

    a.Payload.Scanner.status = true;
    b.Payload.Scanner.status = false;
    CHECK_TRUE_TEXT(testable.PublicMorozov_Equals(&a, &b), "status should not affect comparison");

    a.Payload.Scanner.status = true;
    b.Payload.Scanner.status = true;
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

    a.type = RequestItemType::wqi_AccessPoint;
    b.type = RequestItemType::wqi_AccessPoint;
    a.Payload.AccessPoint.ssid = ssid;
    b.Payload.AccessPoint.ssid = ssid;
    CHECK_TRUE(testable.PublicMorozov_Equals(&a, &b));

    b.Payload.AccessPoint.ssid = ssid_with_same_value_but_diff_address;
    CHECK_FALSE(testable.PublicMorozov_Equals(&a, &b));

    a.Payload.AccessPoint.ssid = ssid_with_same_value_but_diff_address;
    CHECK_TRUE(testable.PublicMorozov_Equals(&a, &b));
}

TEST(LogicWiFiRequestsTestsGroup, Add_is_unique) {
    TestableWiFiRequests testable;

    CHECK_EQUAL(0, testable.size());

    const char *ssid = "test";
    RequestItem request = {};
    request.type = RequestItemType::wqi_Scanner;
    request.Payload.Scanner.ssid = ssid;
    request.Payload.Scanner.status = false;

    CHECK_TRUE(testable.Add(&request));
    CHECK_EQUAL(1, testable.size());

    CHECK_FALSE(testable.Add(&request));
    CHECK_EQUAL(1, testable.size());
    CHECK_EQUAL(RequestItemType::wqi_Scanner, testable.back().type);
    CHECK_EQUAL(ssid, testable.back().Payload.Scanner.ssid);
}

TEST(LogicWiFiRequestsTestsGroup, AddOrReAddIfStatus_is_unique) {
    TestableWiFiRequests testable;

    CHECK_EQUAL(0, testable.size());

    const char *ssid = "test";
    RequestItem request = {};
    request.type = RequestItemType::wqi_Scanner;
    request.Payload.Scanner.ssid = ssid;
    request.Payload.Scanner.status = false;

    bool status;
    CHECK_TRUE(testable.AddOrReAddIfStatus(&request, &status));
    CHECK_EQUAL(1, testable.size());

    CHECK_FALSE(testable.AddOrReAddIfStatus(&request, &status));
    CHECK_EQUAL(1, testable.size());
    CHECK_EQUAL(RequestItemType::wqi_Scanner, testable.back().type);
    CHECK_EQUAL(ssid, testable.back().Payload.Scanner.ssid);
}

TEST(LogicWiFiRequestsTestsGroup,
     AddOrReAddIfStatus_return_status_of_previously_added_Scan_request) {
    TestableWiFiRequests testable;

    CHECK_EQUAL(0, testable.size());

    const char *ssid = "test";
    RequestItem request = {};
    request.type = RequestItemType::wqi_Scanner;
    request.Payload.Scanner.ssid = ssid;
    request.Payload.Scanner.status = false;

    bool status;
    CHECK_TRUE(testable.AddOrReAddIfStatus(&request, &status));
    CHECK_EQUAL(1, testable.size());
    CHECK_FALSE(status);

    CHECK_FALSE(testable.AddOrReAddIfStatus(&request, &status));
    CHECK_EQUAL(1, testable.size());
    CHECK_FALSE(status);
    CHECK_EQUAL(RequestItemType::wqi_Scanner, testable.back().type);
    CHECK_EQUAL(ssid, testable.back().Payload.Scanner.ssid);
    CHECK_FALSE(testable.back().Payload.Scanner.status);

    testable.back().Payload.Scanner.status = true;

    CHECK_TRUE(testable.AddOrReAddIfStatus(&request, &status));
    CHECK_EQUAL(1, testable.size());
    CHECK_TRUE(status);
    CHECK_EQUAL(RequestItemType::wqi_Scanner, testable.back().type);
    CHECK_EQUAL(ssid, testable.back().Payload.Scanner.ssid);
    CHECK_FALSE(testable.back().Payload.Scanner.status);
}

TEST(LogicWiFiRequestsTestsGroup, Pop_is_FIFO_compliant) {
    TestableWiFiRequests testable;

    const char *ssid_0 = "test_0";
    const char *ssid_1 = "test_1";
    RequestItem request_Scanner_0 = {};
    request_Scanner_0.type = RequestItemType::wqi_Scanner;
    request_Scanner_0.Payload.Scanner.ssid = ssid_0;

    RequestItem request_AccessPoint_0 = {};
    request_AccessPoint_0.type = RequestItemType::wqi_AccessPoint;
    request_AccessPoint_0.Payload.AccessPoint.ssid = ssid_0;

    RequestItem request_Scanner_1 = {};
    request_Scanner_1.type = RequestItemType::wqi_Scanner;
    request_Scanner_1.Payload.Scanner.ssid = ssid_1;

    RequestItem request_AccessPoint_1 = {};
    request_AccessPoint_1.type = RequestItemType::wqi_AccessPoint;
    request_AccessPoint_1.Payload.AccessPoint.ssid = ssid_1;

    CHECK_TRUE(testable.Add(&request_Scanner_0));
    CHECK_TRUE(testable.Add(&request_AccessPoint_0));
    CHECK_TRUE(testable.Add(&request_Scanner_1));
    CHECK_TRUE(testable.Add(&request_AccessPoint_1));

    CHECK_EQUAL(4, testable.size());

    RequestItem request = testable.Pop();
    CHECK_EQUAL(RequestItemType::wqi_Scanner, request.type);
    STRCMP_EQUAL("test_0", request.Payload.Scanner.ssid);
    testable.pop_back();

    request = testable.Pop();
    CHECK_EQUAL(RequestItemType::wqi_AccessPoint, request.type);
    STRCMP_EQUAL("test_0", request.Payload.AccessPoint.ssid);
    testable.pop_back();

    request = testable.Pop();
    CHECK_EQUAL(RequestItemType::wqi_Scanner, request.type);
    STRCMP_EQUAL("test_1", request.Payload.Scanner.ssid);
    testable.pop_back();

    request = testable.Pop();
    CHECK_EQUAL(RequestItemType::wqi_AccessPoint, request.type);
    STRCMP_EQUAL("test_1", request.Payload.AccessPoint.ssid);
    testable.pop_back();
}

TEST(LogicWiFiRequestsTestsGroup, StationDone_removes_Station_request) {
    TestableWiFiRequests testable;

    const char *ssid_0 = "test_0";
    RequestItem request_Scanner_0 = {};
    request_Scanner_0.type = RequestItemType::wqi_Scanner;
    request_Scanner_0.Payload.Scanner.ssid = ssid_0;

    RequestItem request_AccessPoint_0 = {};
    request_AccessPoint_0.type = RequestItemType::wqi_AccessPoint;
    request_AccessPoint_0.Payload.AccessPoint.ssid = ssid_0;

    testable.Add(&request_Scanner_0);
    testable.Add(&request_AccessPoint_0);

    RequestItem request_Station = {};
    request_Station.type = RequestItemType::wqi_Station;

    testable.Add(&request_Station);
    CHECK_EQUAL(3, testable.size());

    testable.StationDone();
    CHECK_EQUAL(2, testable.size());

    for (auto it = testable.begin(); it != testable.end(); it++) {
        auto request = *it;
        CHECK(request.type != RequestItemType::wqi_Station);
    }
}

TEST(LogicWiFiRequestsTestsGroup,
     ScannerDone_change_status_in_Scanner_request__and_not_depends_to_adding_another_requests) {
    TestableWiFiRequests testable;

    const char *ssid_0 = "test_0";
    const char *ssid_1 = "test_1";
    RequestItem request_Scanner_0 = {};
    request_Scanner_0.type = RequestItemType::wqi_Scanner;
    request_Scanner_0.Payload.Scanner.ssid = ssid_0;

    RequestItem request_AccessPoint_0 = {};
    request_AccessPoint_0.type = RequestItemType::wqi_AccessPoint;
    request_AccessPoint_0.Payload.AccessPoint.ssid = ssid_0;

    RequestItem request_Scanner_1 = {};
    request_Scanner_1.type = RequestItemType::wqi_Scanner;
    request_Scanner_1.Payload.Scanner.ssid = ssid_1;

    RequestItem request_AccessPoint_1 = {};
    request_AccessPoint_1.type = RequestItemType::wqi_AccessPoint;
    request_AccessPoint_1.Payload.AccessPoint.ssid = ssid_1;

    testable.Add(&request_Scanner_1);
    testable.Add(&request_AccessPoint_1);

    RequestItem request_Station = {};
    request_Station.type = RequestItemType::wqi_Station;
    testable.Add(&request_Station);

    CHECK_EQUAL(3, testable.size());

    testable.ScannerDone(ssid_1);

    testable.Add(&request_AccessPoint_0);
    testable.Add(&request_Scanner_0);
    CHECK_EQUAL(5, testable.size());

    bool changed = false;
    for (auto it = testable.begin(); it != testable.end(); it++) {
        auto request = *it;

        changed = request.type == RequestItemType::wqi_Scanner
               && request.Payload.Scanner.ssid == ssid_1 && request.Payload.Scanner.status;
    }
    CHECK_TRUE(changed);
}

TEST(LogicWiFiRequestsTestsGroup, AccessPointDone_removes_AccessPoint_request) {
    TestableWiFiRequests testable;

    const char *ssid_0 = "test_0";
    RequestItem request_Scanner_0 = {};
    request_Scanner_0.type = RequestItemType::wqi_Scanner;
    request_Scanner_0.Payload.Scanner.ssid = ssid_0;

    RequestItem request_AccessPoint_0 = {};
    request_AccessPoint_0.type = RequestItemType::wqi_AccessPoint;
    request_AccessPoint_0.Payload.AccessPoint.ssid = ssid_0;

    testable.Add(&request_Scanner_0);
    testable.Add(&request_AccessPoint_0);

    RequestItem request_Station = {};
    request_Station.type = RequestItemType::wqi_Station;

    testable.Add(&request_Station);
    CHECK_EQUAL(3, testable.size());

    testable.AccessPointDone(ssid_0);
    CHECK_EQUAL(2, testable.size());

    for (auto it = testable.begin(); it != testable.end(); it++) {
        auto request = *it;
        CHECK(request.type != RequestItemType::wqi_AccessPoint);
    }
}

TEST(LogicWiFiRequestsTestsGroup, RemoveScanner) {
    TestableWiFiRequests testable;

    const char *ssid_0 = "test_0";
    const char *ssid_0_but_diff_address = "test_0";
    const char *ssid_1 = "test_1";
    RequestItem request_Scanner_0 = {};
    request_Scanner_0.type = RequestItemType::wqi_Scanner;
    request_Scanner_0.Payload.Scanner.ssid = ssid_0;

    RequestItem request_AccessPoint_0 = {};
    request_AccessPoint_0.type = RequestItemType::wqi_AccessPoint;
    request_AccessPoint_0.Payload.AccessPoint.ssid = ssid_0;

    RequestItem request_Scanner_1 = {};
    request_Scanner_1.type = RequestItemType::wqi_Scanner;
    request_Scanner_1.Payload.Scanner.ssid = ssid_1;

    RequestItem request_AccessPoint_1 = {};
    request_AccessPoint_1.type = RequestItemType::wqi_AccessPoint;
    request_AccessPoint_1.Payload.AccessPoint.ssid = ssid_1;

    RequestItem request_Station = {};
    request_Station.type = RequestItemType::wqi_Station;

    testable.Add(&request_AccessPoint_0);
    testable.Add(&request_Scanner_0);
    testable.Add(&request_Scanner_1);
    testable.Add(&request_AccessPoint_1);
    testable.Add(&request_Station);

    CHECK_TRUE(testable.RemoveScanner(ssid_0));
    CHECK_EQUAL(4, testable.size());

    CHECK_FALSE(testable.RemoveScanner(ssid_0_but_diff_address));
    CHECK_EQUAL(4, testable.size());
}