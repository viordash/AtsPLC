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
    b.type = RequestItemType::wqi_Scaner;
    CHECK_FALSE(testable.PublicMorozov_Equals(&a, &b));

    a.type = RequestItemType::wqi_Scaner;
    b.type = RequestItemType::wqi_Scaner;
    CHECK_TRUE(testable.PublicMorozov_Equals(&a, &b));

    a.type = RequestItemType::wqi_Scaner;
    b.type = RequestItemType::wqi_AccessPoint;
    CHECK_FALSE(testable.PublicMorozov_Equals(&a, &b));

    a.type = RequestItemType::wqi_AccessPoint;
    b.type = RequestItemType::wqi_AccessPoint;
    CHECK_TRUE(testable.PublicMorozov_Equals(&a, &b));
}

TEST(LogicWiFiRequestsTestsGroup, Equals_by_Scaner_payload) {
    TestableWiFiRequests testable;

    char ssid[16];
    char ssid_with_same_value_but_diff_address[16];
    strcpy(ssid, "test");
    strcpy(ssid_with_same_value_but_diff_address, "test");

    RequestItem a = {};
    RequestItem b = {};

    a.type = RequestItemType::wqi_Scaner;
    b.type = RequestItemType::wqi_Scaner;
    a.Payload.Scaner.ssid = ssid;
    b.Payload.Scaner.ssid = ssid;
    CHECK_TRUE(testable.PublicMorozov_Equals(&a, &b));

    b.Payload.Scaner.ssid = ssid_with_same_value_but_diff_address;
    CHECK_FALSE(testable.PublicMorozov_Equals(&a, &b));

    a.Payload.Scaner.ssid = ssid_with_same_value_but_diff_address;
    CHECK_TRUE(testable.PublicMorozov_Equals(&a, &b));

    a.Payload.Scaner.status = true;
    b.Payload.Scaner.status = false;
    CHECK_TRUE_TEXT(testable.PublicMorozov_Equals(&a, &b), "status should not affect comparison");

    a.Payload.Scaner.status = true;
    b.Payload.Scaner.status = true;
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

TEST(LogicWiFiRequestsTestsGroup, AddRequest_are_unique) {
    TestableWiFiRequests testable;

    CHECK_EQUAL(0, testable.size());

    const char *ssid = "test";
    RequestItem request = {};
    request.type = RequestItemType::wqi_Scaner;
    request.Payload.Scaner.ssid = ssid;
    request.Payload.Scaner.status = false;

    auto iter_to_end = testable.AddRequest(&request);
    CHECK(iter_to_end == testable.end());
    CHECK_EQUAL(1, testable.size());

    auto iter_to_previously_added_item = testable.AddRequest(&request);
    CHECK(iter_to_previously_added_item != testable.end());
    CHECK_EQUAL(1, testable.size());
    CHECK_EQUAL(RequestItemType::wqi_Scaner, iter_to_previously_added_item->type);
    CHECK_EQUAL(ssid, iter_to_previously_added_item->Payload.Scaner.ssid);
}

TEST(LogicWiFiRequestsTestsGroup, AddRequest_returned_item_is_mutable) {
    TestableWiFiRequests testable;

    CHECK_EQUAL(0, testable.size());

    const char *ssid = "test";
    RequestItem request = {};
    request.type = RequestItemType::wqi_Scaner;
    request.Payload.Scaner.ssid = ssid;
    request.Payload.Scaner.status = false;

    testable.AddRequest(&request);

    auto iter_to_previously_added_item = testable.AddRequest(&request);
    CHECK(iter_to_previously_added_item != testable.end());
    CHECK_EQUAL(1, testable.size());
    CHECK_EQUAL(RequestItemType::wqi_Scaner, iter_to_previously_added_item->type);
    CHECK_EQUAL(ssid, iter_to_previously_added_item->Payload.Scaner.ssid);
    CHECK_FALSE(iter_to_previously_added_item->Payload.Scaner.status);

    iter_to_previously_added_item->Payload.Scaner.status = true;

    RequestItem pop_request = testable.PopRequest();
    CHECK_EQUAL(RequestItemType::wqi_Scaner, pop_request.type);
    STRCMP_EQUAL("test", pop_request.Payload.Scaner.ssid);
    CHECK_TRUE(pop_request.Payload.Scaner.status);
}

TEST(LogicWiFiRequestsTestsGroup, PopRequest_is_FIFO_compliant) {
    TestableWiFiRequests testable;

    const char *ssid_0 = "test_0";
    const char *ssid_1 = "test_1";
    RequestItem request_Scaner_0 = {};
    request_Scaner_0.type = RequestItemType::wqi_Scaner;
    request_Scaner_0.Payload.Scaner.ssid = ssid_0;

    RequestItem request_AccessPoint_0 = {};
    request_AccessPoint_0.type = RequestItemType::wqi_AccessPoint;
    request_AccessPoint_0.Payload.AccessPoint.ssid = ssid_0;

    RequestItem request_Scaner_1 = {};
    request_Scaner_1.type = RequestItemType::wqi_Scaner;
    request_Scaner_1.Payload.Scaner.ssid = ssid_1;

    RequestItem request_AccessPoint_1 = {};
    request_AccessPoint_1.type = RequestItemType::wqi_AccessPoint;
    request_AccessPoint_1.Payload.AccessPoint.ssid = ssid_1;

    CHECK(testable.AddRequest(&request_Scaner_0) == testable.end());
    CHECK(testable.AddRequest(&request_AccessPoint_0) == testable.end());
    CHECK(testable.AddRequest(&request_Scaner_1) == testable.end());
    CHECK(testable.AddRequest(&request_AccessPoint_1) == testable.end());

    CHECK_EQUAL(4, testable.size());

    RequestItem request = testable.PopRequest();
    CHECK_EQUAL(RequestItemType::wqi_Scaner, request.type);
    STRCMP_EQUAL("test_0", request.Payload.Scaner.ssid);

    request = testable.PopRequest();
    CHECK_EQUAL(RequestItemType::wqi_AccessPoint, request.type);
    STRCMP_EQUAL("test_0", request.Payload.AccessPoint.ssid);

    request = testable.PopRequest();
    CHECK_EQUAL(RequestItemType::wqi_Scaner, request.type);
    STRCMP_EQUAL("test_1", request.Payload.Scaner.ssid);

    request = testable.PopRequest();
    CHECK_EQUAL(RequestItemType::wqi_AccessPoint, request.type);
    STRCMP_EQUAL("test_1", request.Payload.AccessPoint.ssid);
}