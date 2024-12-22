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

    a.type = RequestItemType::wqi_Connect;
    b.type = RequestItemType::wqi_Connect;
    CHECK_TRUE(testable.PublicMorozov_Equals(&a, &b));

    a.type = RequestItemType::wqi_Connect;
    b.type = RequestItemType::wqi_ScanSsid;
    CHECK_FALSE(testable.PublicMorozov_Equals(&a, &b));

    a.type = RequestItemType::wqi_ScanSsid;
    b.type = RequestItemType::wqi_ScanSsid;
    CHECK_TRUE(testable.PublicMorozov_Equals(&a, &b));

    a.type = RequestItemType::wqi_ScanSsid;
    b.type = RequestItemType::wqi_GenerateSsid;
    CHECK_FALSE(testable.PublicMorozov_Equals(&a, &b));

    a.type = RequestItemType::wqi_GenerateSsid;
    b.type = RequestItemType::wqi_GenerateSsid;
    CHECK_TRUE(testable.PublicMorozov_Equals(&a, &b));
}

TEST(LogicWiFiRequestsTestsGroup, Equals_by_ScanSsid_payload) {
    TestableWiFiRequests testable;

    char ssid[16];
    char ssid_with_same_value_but_diff_address[16];
    strcpy(ssid, "test");
    strcpy(ssid_with_same_value_but_diff_address, "test");

    RequestItem a = {};
    RequestItem b = {};

    a.type = RequestItemType::wqi_ScanSsid;
    b.type = RequestItemType::wqi_ScanSsid;
    a.Payload.ScanSsid.ssid = ssid;
    b.Payload.ScanSsid.ssid = ssid;
    CHECK_TRUE(testable.PublicMorozov_Equals(&a, &b));

    b.Payload.ScanSsid.ssid = ssid_with_same_value_but_diff_address;
    CHECK_FALSE(testable.PublicMorozov_Equals(&a, &b));

    a.Payload.ScanSsid.ssid = ssid_with_same_value_but_diff_address;
    CHECK_TRUE(testable.PublicMorozov_Equals(&a, &b));

    a.Payload.ScanSsid.status = true;
    b.Payload.ScanSsid.status = false;
    CHECK_TRUE_TEXT(testable.PublicMorozov_Equals(&a, &b), "status should not affect comparison");

    a.Payload.ScanSsid.status = true;
    b.Payload.ScanSsid.status = true;
    CHECK_TRUE_TEXT(testable.PublicMorozov_Equals(&a, &b), "status should not affect comparison");
}

TEST(LogicWiFiRequestsTestsGroup, Equals_by_GenerateSsid_payload) {
    TestableWiFiRequests testable;

    char ssid[16];
    char ssid_with_same_value_but_diff_address[16];
    strcpy(ssid, "test");
    strcpy(ssid_with_same_value_but_diff_address, "test");

    RequestItem a = {};
    RequestItem b = {};

    a.type = RequestItemType::wqi_GenerateSsid;
    b.type = RequestItemType::wqi_GenerateSsid;
    a.Payload.GenerateSsid.ssid = ssid;
    b.Payload.GenerateSsid.ssid = ssid;
    CHECK_TRUE(testable.PublicMorozov_Equals(&a, &b));

    b.Payload.GenerateSsid.ssid = ssid_with_same_value_but_diff_address;
    CHECK_FALSE(testable.PublicMorozov_Equals(&a, &b));

    a.Payload.GenerateSsid.ssid = ssid_with_same_value_but_diff_address;
    CHECK_TRUE(testable.PublicMorozov_Equals(&a, &b));
}

TEST(LogicWiFiRequestsTestsGroup, AddRequest_are_unique) {
    TestableWiFiRequests testable;

    CHECK_EQUAL(0, testable.size());

    const char *ssid = "test";
    RequestItem request = { RequestItemType::wqi_ScanSsid, ssid, false };

    auto iter_to_end = testable.AddRequest(&request);
    CHECK(iter_to_end == testable.end());
    CHECK_EQUAL(1, testable.size());

    auto iter_to_previously_added_item = testable.AddRequest(&request);
    CHECK(iter_to_previously_added_item != testable.end());
    CHECK_EQUAL(1, testable.size());
    CHECK_EQUAL(RequestItemType::wqi_ScanSsid, iter_to_previously_added_item->type);
    CHECK_EQUAL(ssid, iter_to_previously_added_item->Payload.ScanSsid.ssid);
}

TEST(LogicWiFiRequestsTestsGroup, AddRequest_returned_item_is_mutable) {
    TestableWiFiRequests testable;

    CHECK_EQUAL(0, testable.size());

    const char *ssid = "test";
    RequestItem request = { RequestItemType::wqi_ScanSsid, ssid, false };

    testable.AddRequest(&request);

    auto iter_to_previously_added_item = testable.AddRequest(&request);
    CHECK(iter_to_previously_added_item != testable.end());
    CHECK_EQUAL(1, testable.size());
    CHECK_EQUAL(RequestItemType::wqi_ScanSsid, iter_to_previously_added_item->type);
    CHECK_EQUAL(ssid, iter_to_previously_added_item->Payload.ScanSsid.ssid);
    CHECK_FALSE(iter_to_previously_added_item->Payload.ScanSsid.status);

    iter_to_previously_added_item->Payload.ScanSsid.status = true;

    RequestItem pop_request = testable.PopRequest();
    CHECK_EQUAL(RequestItemType::wqi_ScanSsid, pop_request.type);
    STRCMP_EQUAL("test", pop_request.Payload.ScanSsid.ssid);
    CHECK_TRUE(pop_request.Payload.ScanSsid.status);
}

TEST(LogicWiFiRequestsTestsGroup, PopRequest_returns_connect_request_when_empty) {
    TestableWiFiRequests testable;

    CHECK_EQUAL(0, testable.size());

    RequestItem request = testable.PopRequest();
    CHECK_EQUAL(RequestItemType::wqi_Connect, request.type);
}

TEST(LogicWiFiRequestsTestsGroup, PopRequest_is_FIFO_compliant) {
    TestableWiFiRequests testable;

    const char *ssid_0 = "test_0";
    const char *ssid_1 = "test_1";
    RequestItem request_ScanSsid_0 = {};
    request_ScanSsid_0.type = RequestItemType::wqi_ScanSsid;
    request_ScanSsid_0.Payload.ScanSsid.ssid = ssid_0;

    RequestItem request_GenerateSsid_0 = {};
    request_GenerateSsid_0.type = RequestItemType::wqi_GenerateSsid;
    request_GenerateSsid_0.Payload.GenerateSsid.ssid = ssid_0;

    RequestItem request_ScanSsid_1 = {};
    request_ScanSsid_1.type = RequestItemType::wqi_ScanSsid;
    request_ScanSsid_1.Payload.ScanSsid.ssid = ssid_1;

    RequestItem request_GenerateSsid_1 = {};
    request_GenerateSsid_1.type = RequestItemType::wqi_GenerateSsid;
    request_GenerateSsid_1.Payload.GenerateSsid.ssid = ssid_1;

    CHECK(testable.AddRequest(&request_ScanSsid_0) == testable.end());
    CHECK(testable.AddRequest(&request_GenerateSsid_0) == testable.end());
    CHECK(testable.AddRequest(&request_ScanSsid_1) == testable.end());
    CHECK(testable.AddRequest(&request_GenerateSsid_1) == testable.end());

    CHECK_EQUAL(4, testable.size());

    RequestItem request = testable.PopRequest();
    CHECK_EQUAL(RequestItemType::wqi_ScanSsid, request.type);
    STRCMP_EQUAL("test_0", request.Payload.ScanSsid.ssid);

    request = testable.PopRequest();
    CHECK_EQUAL(RequestItemType::wqi_GenerateSsid, request.type);
    STRCMP_EQUAL("test_0", request.Payload.GenerateSsid.ssid);

    request = testable.PopRequest();
    CHECK_EQUAL(RequestItemType::wqi_ScanSsid, request.type);
    STRCMP_EQUAL("test_1", request.Payload.ScanSsid.ssid);

    request = testable.PopRequest();
    CHECK_EQUAL(RequestItemType::wqi_GenerateSsid, request.type);
    STRCMP_EQUAL("test_1", request.Payload.GenerateSsid.ssid);
}