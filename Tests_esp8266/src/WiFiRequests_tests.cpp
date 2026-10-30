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

TEST(WiFiRequestsTestsGroup, Equals_by_Type) {

    RequestItem a = {};
    RequestItem b = {};

    a.Type = RequestItemType::wqi_Station;
    b.Type = RequestItemType::wqi_Station;
    CHECK_TRUE(a.Equals(&b));

    a.Type = RequestItemType::wqi_Station;
    b.Type = RequestItemType::wqi_Scanner;
    CHECK_FALSE(a.Equals(&b));

    a.Type = RequestItemType::wqi_Scanner;
    b.Type = RequestItemType::wqi_Scanner;
    CHECK_TRUE(a.Equals(&b));

    a.Type = RequestItemType::wqi_Scanner;
    b.Type = RequestItemType::wqi_AccessPoint;
    CHECK_FALSE(a.Equals(&b));

    a.Type = RequestItemType::wqi_AccessPoint;
    b.Type = RequestItemType::wqi_AccessPoint;
    CHECK_TRUE(a.Equals(&b));
}

TEST(WiFiRequestsTestsGroup, Equals_by_Scanner_payload) {

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
    CHECK_TRUE(a.Equals(&b));

    b.Payload.Scanner.ssid = ssid_with_same_value_but_diff_address;
    CHECK_FALSE(a.Equals(&b));

    a.Payload.Scanner.ssid = ssid_with_same_value_but_diff_address;
    CHECK_TRUE(a.Equals(&b));

    CHECK_TRUE_TEXT(a.Equals(&b), "status should not affect comparison");

    CHECK_TRUE_TEXT(a.Equals(&b), "status should not affect comparison");
}

TEST(WiFiRequestsTestsGroup, Equals_by_AccessPoint_payload) {

    char ssid[16];
    char ssid_with_same_value_but_diff_address[16];
    strcpy(ssid, "test");
    strcpy(ssid_with_same_value_but_diff_address, "test");

    char password[16];
    char password_with_same_value_but_diff_address[16];
    strcpy(password, "pwd");
    strcpy(password_with_same_value_but_diff_address, "pwd");

    char mac[16];
    char mac_with_same_value_but_diff_address[16];
    strcpy(mac, "AABBCCDDEEFF");
    strcpy(mac_with_same_value_but_diff_address, "AABBCCDDEEFF");

    RequestItem a = {};
    RequestItem b = {};

    a.Type = RequestItemType::wqi_AccessPoint;
    b.Type = RequestItemType::wqi_AccessPoint;
    a.Payload.AccessPoint.ssid = ssid;
    b.Payload.AccessPoint.ssid = ssid;
    CHECK_TRUE(a.Equals(&b));

    b.Payload.AccessPoint.ssid = ssid_with_same_value_but_diff_address;
    CHECK_FALSE(a.Equals(&b));

    a.Payload.AccessPoint.ssid = ssid_with_same_value_but_diff_address;
    CHECK_TRUE(a.Equals(&b));

    a.Payload.AccessPoint.password = password;
    b.Payload.AccessPoint.password = password;
    CHECK_TRUE(a.Equals(&b));

    b.Payload.AccessPoint.password = password_with_same_value_but_diff_address;
    CHECK_FALSE(a.Equals(&b));

    b.Payload.AccessPoint.password = password;
    a.Payload.AccessPoint.mac = mac;
    b.Payload.AccessPoint.mac = mac;
    CHECK_TRUE(a.Equals(&b));

    b.Payload.AccessPoint.mac = mac_with_same_value_but_diff_address;
    CHECK_FALSE(a.Equals(&b));
}

TEST(WiFiRequestsTestsGroup, Scan_is_unique) {
    WiFiRequests testable;

    CHECK_EQUAL(0, testable.Size());

    const char *ssid = "test";

    testable.Scan(ssid);
    CHECK_EQUAL(1, testable.Size());

    testable.Scan(ssid);
    CHECK_EQUAL(1, testable.Size());
    CHECK_EQUAL(RequestItemType::wqi_Scanner, testable.First()->Type);
    CHECK_EQUAL(ssid, testable.First()->Payload.Scanner.ssid);
}

TEST(WiFiRequestsTestsGroup, Pop_is_queue_compliant) {
    WiFiRequests testable;

    const char *ssid_0 = "test_0";
    const char *ssid_1 = "test_1";

    testable.Station();
    testable.Scan(ssid_0);
    testable.AccessPoint(ssid_0, NULL, NULL);
    testable.Scan(ssid_1);
    testable.AccessPoint(ssid_1, NULL, NULL);
    CHECK_EQUAL(5, testable.Size());

    RequestItem request;
    CHECK_TRUE(testable.Pop(&request));
    CHECK_EQUAL(RequestItemType::wqi_Station, request.Type);
    CHECK_EQUAL(4, testable.Size());

    CHECK_TRUE(testable.Pop(&request));
    CHECK_EQUAL(RequestItemType::wqi_Scanner, request.Type);
    STRCMP_EQUAL("test_0", request.Payload.Scanner.ssid);
    CHECK_EQUAL(3, testable.Size());

    CHECK_TRUE(testable.Pop(&request));
    CHECK_EQUAL(RequestItemType::wqi_AccessPoint, request.Type);
    STRCMP_EQUAL("test_0", request.Payload.AccessPoint.ssid);
    CHECK_EQUAL(2, testable.Size());

    CHECK_TRUE(testable.Pop(&request));
    CHECK_EQUAL(RequestItemType::wqi_Scanner, request.Type);
    STRCMP_EQUAL("test_1", request.Payload.Scanner.ssid);
    CHECK_EQUAL(1, testable.Size());

    CHECK_TRUE(testable.Pop(&request));
    CHECK_EQUAL(RequestItemType::wqi_AccessPoint, request.Type);
    STRCMP_EQUAL("test_1", request.Payload.AccessPoint.ssid);
    CHECK_EQUAL(0, testable.Size());
}

TEST(WiFiRequestsTestsGroup, HasAnother_when_current_is_Station) {
    WiFiRequests testable;

    RequestItem current_req = { RequestItemType::wqi_Station, {} };
    CHECK_FALSE(testable.HasAnother(&current_req));

    testable.Station();
    CHECK_FALSE(testable.HasAnother(&current_req));

    const char *ssid_0 = "test_0";
    testable.Scan(ssid_0);
    CHECK_TRUE(testable.HasAnother(&current_req));
}

TEST(WiFiRequestsTestsGroup, HasAnother_when_current_is_Scanner) {
    WiFiRequests testable;

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
    WiFiRequests testable;

    const char *ssid_0 = "test_0";
    RequestItem current_req = { RequestItemType::wqi_AccessPoint, { ssid_0 } };
    CHECK_FALSE(testable.HasAnother(&current_req));

    testable.AccessPoint(ssid_0, NULL, NULL);
    CHECK_FALSE(testable.HasAnother(&current_req));

    const char *ssid_1 = "test_1";
    testable.AccessPoint(ssid_1, NULL, NULL);
    CHECK_TRUE(testable.HasAnother(&current_req));
}

TEST(WiFiRequestsTestsGroup, Remove_deletes_only_matching_requests) {
    WiFiRequests testable;
    const char *ssid_0 = "ssid_0";
    const char *ssid_1 = "ssid_1";

    testable.Scan(ssid_0);
    testable.Station();
    testable.Scan(ssid_1);
    const char *password = "pass";
    const char *mac = "************";
    testable.AccessPoint(ssid_0, password, mac);

    RequestItem scan_0 = { RequestItemType::wqi_Scanner, { ssid_0 } };
    testable.Remove(&scan_0);
    CHECK_EQUAL(3, testable.Size());

    RequestItem request;
    CHECK_TRUE(testable.Pop(&request));
    CHECK_EQUAL(RequestItemType::wqi_Station, request.Type);

    RequestItem access_point_0 = { RequestItemType::wqi_AccessPoint, { ssid_0 } };
    access_point_0.Payload.AccessPoint.password = password;
    access_point_0.Payload.AccessPoint.mac = mac;
    testable.Remove(&access_point_0);
    CHECK_EQUAL(1, testable.Size());
    CHECK_TRUE(testable.Pop(&request));
    CHECK_EQUAL(RequestItemType::wqi_Scanner, request.Type);
    CHECK_EQUAL(ssid_1, request.Payload.Scanner.ssid);
}
