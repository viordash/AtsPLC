#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/WiFi/WiFiRequests.h"

TEST_GROUP(WiFiRequestsTestsGroup){ //
                                    TEST_SETUP(){}

                                    TEST_TEARDOWN(){}
};

namespace {
    class TestableWiFiRequests : public WiFiRequests {
      public:
        bool PublicMorozov_Equals(const RequestItem *a, const RequestItem *b) const {
            return Equals(a, b);
        }

        size_t PublicMorozov_size() const {
            return GetSize();
        }
        const RequestItem &PublicMorozov_back() const {
            return GetBack();
        }
        std::list<RequestItem>::iterator PublicMorozov_begin() {
            return GetBegin();
        }
        std::list<RequestItem>::iterator PublicMorozov_end() {
            return GetEnd();
        }
    };
} // namespace

TEST(WiFiRequestsTestsGroup, Equals_by_Type) {
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

TEST(WiFiRequestsTestsGroup, Equals_by_Scanner_payload) {
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

TEST(WiFiRequestsTestsGroup, Equals_by_AccessPoint_payload) {
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

TEST(WiFiRequestsTestsGroup, Scan_is_unique) {
    TestableWiFiRequests testable;

    CHECK_EQUAL(0, testable.PublicMorozov_size());

    const char *ssid = "test";

    testable.Scan(ssid);
    CHECK_EQUAL(1, testable.PublicMorozov_size());

    testable.Scan(ssid);
    CHECK_EQUAL(1, testable.PublicMorozov_size());
    CHECK_EQUAL(RequestItemType::wqi_Scanner, testable.PublicMorozov_back().Type);
    CHECK_EQUAL(ssid, testable.PublicMorozov_back().Payload.Scanner.ssid);
}

TEST(WiFiRequestsTestsGroup, Pop_is_queue_compliant) {
    TestableWiFiRequests testable;

    const char *ssid_0 = "test_0";
    const char *ssid_1 = "test_1";

    testable.Station();
    testable.Scan(ssid_0);
    testable.AccessPoint(ssid_0, NULL, NULL);
    testable.Scan(ssid_1);
    testable.AccessPoint(ssid_1, NULL, NULL);
    CHECK_EQUAL(5, testable.PublicMorozov_size());

    RequestItem request;
    CHECK_TRUE(testable.Pop(&request));
    CHECK_EQUAL(RequestItemType::wqi_Station, request.Type);
    CHECK_EQUAL(4, testable.PublicMorozov_size());

    CHECK_TRUE(testable.Pop(&request));
    CHECK_EQUAL(RequestItemType::wqi_Scanner, request.Type);
    STRCMP_EQUAL("test_0", request.Payload.Scanner.ssid);
    CHECK_EQUAL(3, testable.PublicMorozov_size());

    CHECK_TRUE(testable.Pop(&request));
    CHECK_EQUAL(RequestItemType::wqi_AccessPoint, request.Type);
    STRCMP_EQUAL("test_0", request.Payload.AccessPoint.ssid);
    CHECK_EQUAL(2, testable.PublicMorozov_size());

    CHECK_TRUE(testable.Pop(&request));
    CHECK_EQUAL(RequestItemType::wqi_Scanner, request.Type);
    STRCMP_EQUAL("test_1", request.Payload.Scanner.ssid);
    CHECK_EQUAL(1, testable.PublicMorozov_size());

    CHECK_TRUE(testable.Pop(&request));
    CHECK_EQUAL(RequestItemType::wqi_AccessPoint, request.Type);
    STRCMP_EQUAL("test_1", request.Payload.AccessPoint.ssid);
    CHECK_EQUAL(0, testable.PublicMorozov_size());
}

TEST(WiFiRequestsTestsGroup, HasAnother_when_current_is_Station) {
    TestableWiFiRequests testable;

    RequestItem current_req = { RequestItemType::wqi_Station, {} };
    CHECK_FALSE(testable.HasAnother(&current_req));

    testable.Station();
    CHECK_FALSE(testable.HasAnother(&current_req));

    const char *ssid_0 = "test_0";
    testable.Scan(ssid_0);
    CHECK_TRUE(testable.HasAnother(&current_req));
}

TEST(WiFiRequestsTestsGroup, HasAnother_when_current_is_Scanner) {
    TestableWiFiRequests testable;

    const char *ssid_0 = "test_0";
    RequestItem current_req = { RequestItemType::wqi_Scanner, { ssid_0 } };
    CHECK_FALSE(testable.HasAnother(&current_req));

    testable.Scan(ssid_0);
    CHECK_FALSE(testable.HasAnother(&current_req));

    const char *ssid_1 = "test_1";
    testable.Scan(ssid_1);
    CHECK_TRUE(testable.HasAnother(&current_req));
}

TEST(WiFiRequestsTestsGroup, HasAnother_when_current_is_AccessPoint) {
    TestableWiFiRequests testable;

    const char *ssid_0 = "test_0";
    RequestItem current_req = { RequestItemType::wqi_AccessPoint, { ssid_0 } };
    CHECK_FALSE(testable.HasAnother(&current_req));

    testable.AccessPoint(ssid_0, NULL, NULL);
    CHECK_FALSE(testable.HasAnother(&current_req));

    const char *ssid_1 = "test_1";
    testable.AccessPoint(ssid_1, NULL, NULL);
    CHECK_TRUE(testable.HasAnother(&current_req));
}
