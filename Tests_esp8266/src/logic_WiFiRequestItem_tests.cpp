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

TEST(LogicWiFiRequestItemTestsGroup, Compare_by_type) {
    RequestItemComparator comparator;

    RequestItem a = {};
    RequestItem b = {};

    a.type = RequestItemType::wqi_Connect;
    b.type = RequestItemType::wqi_Connect;
    CHECK_TRUE(comparator(a, b));

    a.type = RequestItemType::wqi_Connect;
    b.type = RequestItemType::wqi_ScanSsid;
    CHECK_FALSE(comparator(a, b));

    a.type = RequestItemType::wqi_ScanSsid;
    b.type = RequestItemType::wqi_ScanSsid;
    CHECK_TRUE(comparator(a, b));

    a.type = RequestItemType::wqi_ScanSsid;
    b.type = RequestItemType::wqi_GenerateSsid;
    CHECK_FALSE(comparator(a, b));

    a.type = RequestItemType::wqi_GenerateSsid;
    b.type = RequestItemType::wqi_GenerateSsid;
    CHECK_TRUE(comparator(a, b));
}

TEST(LogicWiFiRequestItemTestsGroup, Compare_by_ScanSsid_payload) {
    RequestItemComparator comparator;

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
    CHECK_TRUE(comparator(a, b));

    b.Payload.ScanSsid.ssid = ssid_with_same_value_but_diff_address;
    CHECK_FALSE(comparator(a, b));

    a.Payload.ScanSsid.ssid = ssid_with_same_value_but_diff_address;
    CHECK_TRUE(comparator(a, b));

    a.Payload.ScanSsid.status = true;
    b.Payload.ScanSsid.status = false;
    CHECK_TRUE_TEXT(comparator(a, b), "status should not affect comparison");

    a.Payload.ScanSsid.status = true;
    b.Payload.ScanSsid.status = true;
    CHECK_TRUE_TEXT(comparator(a, b), "status should not affect comparison");
}

TEST(LogicWiFiRequestItemTestsGroup, Compare_by_GenerateSsid_payload) {
    RequestItemComparator comparator;

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
    CHECK_TRUE(comparator(a, b));

    b.Payload.GenerateSsid.ssid = ssid_with_same_value_but_diff_address;
    CHECK_FALSE(comparator(a, b));

    a.Payload.GenerateSsid.ssid = ssid_with_same_value_but_diff_address;
    CHECK_TRUE(comparator(a, b));
}

TEST(LogicWiFiRequestItemTestsGroup, RequestItemHash_by_type) {
    RequestItemHash hash;

    RequestItem a = {};
    RequestItem b = {};

    a.type = RequestItemType::wqi_Connect;
    b.type = RequestItemType::wqi_Connect;
    CHECK_COMPARE(hash(a), ==, hash(b));

    b.type = RequestItemType::wqi_ScanSsid;
    CHECK_COMPARE(hash(a), !=, hash(b));

    a.type = RequestItemType::wqi_ScanSsid;
    CHECK_COMPARE(hash(a), ==, hash(b));

    b.type = RequestItemType::wqi_GenerateSsid;
    CHECK_COMPARE(hash(a), !=, hash(b));

    a.type = RequestItemType::wqi_GenerateSsid;
    CHECK_COMPARE(hash(a), ==, hash(b));
}

TEST(LogicWiFiRequestItemTestsGroup, RequestItemHash_by_ScanSsid_payload) {
    RequestItemHash hash;

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
    CHECK_COMPARE(hash(a), ==, hash(b));

    b.Payload.ScanSsid.ssid = ssid_with_same_value_but_diff_address;
    CHECK_COMPARE(hash(a), !=, hash(b));

    a.Payload.ScanSsid.ssid = ssid_with_same_value_but_diff_address;
    CHECK_COMPARE(hash(a), ==, hash(b));

    a.Payload.ScanSsid.ssid = NULL;
    CHECK_COMPARE(hash(a), !=, hash(b));

    b.Payload.ScanSsid.ssid = NULL;
    CHECK_COMPARE(hash(a), ==, hash(b));

    a.Payload.ScanSsid.status = true;
    b.Payload.ScanSsid.status = false;
    CHECK_COMPARE_TEXT(hash(a), ==, hash(b), "status should not affect to hash");

    a.Payload.ScanSsid.status = true;
    b.Payload.ScanSsid.status = true;
    CHECK_COMPARE_TEXT(hash(a), ==, hash(b), "status should not affect to hash");
}

TEST(LogicWiFiRequestItemTestsGroup, RequestItemHash_by_GenerateSsid_payload) {
    RequestItemHash hash;

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
    CHECK_COMPARE(hash(a), ==, hash(b));

    b.Payload.GenerateSsid.ssid = ssid_with_same_value_but_diff_address;
    CHECK_COMPARE(hash(a), !=, hash(b));

    a.Payload.GenerateSsid.ssid = ssid_with_same_value_but_diff_address;
    CHECK_COMPARE(hash(a), ==, hash(b));

    a.Payload.GenerateSsid.ssid = NULL;
    CHECK_COMPARE(hash(a), !=, hash(b));

    b.Payload.GenerateSsid.ssid = NULL;
    CHECK_COMPARE(hash(a), ==, hash(b));
}