#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "main/Datetime/DatetimeService.h"
#include "main/LogicProgram/ControllerVariable.h"
#include "main/LogicProgram/LogicElement.h"

TEST_GROUP(LogicControllerVariableTestsGroup){ //
                                               TEST_SETUP(){}

                                               TEST_TEARDOWN(){}
};

namespace {
    class TestableControllerVariable : public ControllerVariable {
      public:
        bool *PublicMorozov_Get_required_reading() {
            return &required_reading;
        }
        bool *PublicMorozov_Get_required_writing() {
            return &required_writing;
        }
        uint8_t *PublicMorozov_Get_out_value() {
            return &out_value;
        }
        uint8_t *PublicMorozov_Get_value() {
            return &value;
        }
    };

    class TestableWiFiService : public WiFiService {
      public:
        WiFiRequests *PublicMorozov_Get_requests() {
            return &requests;
        }
    };
} // namespace

TEST(LogicControllerVariableTestsGroup, Init_reset_value_and_set_required) {
    TestableControllerVariable testable;
    *(testable.PublicMorozov_Get_required_reading()) = false;
    testable.UpdateValue(LogicElement::MaxValue);

    testable.Init();
    CHECK_TRUE(*(testable.PublicMorozov_Get_required_reading()));
    CHECK_FALSE(*(testable.PublicMorozov_Get_required_writing()));
    CHECK_EQUAL(LogicElement::MinValue, testable.PeekValue());
    CHECK_EQUAL(LogicElement::MinValue, *testable.PublicMorozov_Get_out_value());
}

TEST(LogicControllerVariableTestsGroup, FetchValue_reset_required_reading) {
    TestableControllerVariable testable;
    testable.Init();

    CHECK_TRUE(*(testable.PublicMorozov_Get_required_reading()));
    testable.FetchValue();
    CHECK_FALSE(*(testable.PublicMorozov_Get_required_reading()));
}

TEST(LogicControllerVariableTestsGroup, FetchValue) {
    TestableControllerVariable testable;
    testable.Init();

    *testable.PublicMorozov_Get_out_value() = 42;
    testable.FetchValue();
    CHECK_EQUAL(42, testable.ReadValue());
    CHECK_EQUAL(42, testable.PeekValue());
}

TEST(LogicControllerVariableTestsGroup, UpdateValue) {
    TestableControllerVariable testable;
    testable.Init();

    testable.UpdateValue(LogicElement::MaxValue);
    CHECK_EQUAL(LogicElement::MaxValue, testable.ReadValue());
    CHECK_EQUAL(LogicElement::MaxValue, testable.PeekValue());
}

TEST(LogicControllerVariableTestsGroup, ReadValue_returns_value) {
    TestableControllerVariable testable;
    testable.Init();
    testable.UpdateValue(LogicElement::MaxValue);

    CHECK_EQUAL(LogicElement::MaxValue, testable.ReadValue());
}

TEST(LogicControllerVariableTestsGroup,
     ReadValue_set_required_reading_and_not_touch_required_writing) {
    TestableControllerVariable testable;
    testable.Init();
    *(testable.PublicMorozov_Get_required_reading()) = false;

    testable.ReadValue();
    CHECK_TRUE(*(testable.PublicMorozov_Get_required_reading()));
    CHECK_FALSE(*(testable.PublicMorozov_Get_required_writing()));
}

TEST(LogicControllerVariableTestsGroup, WriteValue_changes_out_value) {
    TestableControllerVariable testable;

    testable.WriteValue(LogicElement::MaxValue);
    CHECK_EQUAL(LogicElement::MaxValue, *testable.PublicMorozov_Get_out_value());

    testable.WriteValue(42);
    CHECK_EQUAL(42, *testable.PublicMorozov_Get_out_value());
    CHECK_TRUE(*(testable.PublicMorozov_Get_required_writing()));
}

TEST(LogicControllerVariableTestsGroup,
     WriteValue_set_required_writing_and_not_touch_required_reading) {
    TestableControllerVariable testable;
    testable.Init();
    *(testable.PublicMorozov_Get_required_reading()) = false;

    testable.WriteValue(42);
    CHECK_TRUE(*(testable.PublicMorozov_Get_required_writing()));
    CHECK_FALSE(*(testable.PublicMorozov_Get_required_reading()));
}

TEST(LogicControllerVariableTestsGroup, CommitChanges_reset_required_writing) {
    TestableControllerVariable testable;
    testable.Init();
    *(testable.PublicMorozov_Get_required_writing()) = true;

    testable.CommitChanges();
    CHECK_FALSE(*(testable.PublicMorozov_Get_required_writing()));
}

TEST(LogicControllerVariableTestsGroup, CommitChanges_updated_value) {
    TestableControllerVariable testable;
    testable.Init();
    testable.WriteValue(42);

    testable.CommitChanges();
    CHECK_EQUAL(42, *testable.PublicMorozov_Get_value());
}

TEST(LogicControllerVariableTestsGroup, Value_changes_in_transaction) {
    TestableControllerVariable testable;
    testable.Init();

    testable.FetchValue();
    CHECK_EQUAL(LogicElement::MinValue, testable.ReadValue());

    testable.WriteValue(42);
    CHECK_EQUAL(LogicElement::MinValue, testable.ReadValue());

    testable.CommitChanges();
    CHECK_EQUAL(42, testable.ReadValue());

    testable.FetchValue();
    CHECK_EQUAL(42, testable.ReadValue());

    testable.WriteValue(41);
    testable.FetchValue();
    CHECK_EQUAL(41, testable.ReadValue());
    CHECK_EQUAL(41, testable.PeekValue());
}

TEST(LogicControllerVariableTestsGroup, CancelReadingProcess_reset_values_when_binded_to_wifi) {
    TestableControllerVariable testable;
    testable.Init();
    testable.WriteValue(42);
    testable.CommitChanges();

    CHECK_EQUAL(42, testable.ReadValue());
    testable.CancelReadingProcess();
    CHECK_EQUAL(42, testable.ReadValue());
    CHECK_EQUAL(42, *testable.PublicMorozov_Get_out_value());

    WiFiService wifi_service;
    testable.BindToInsecureWiFi(&wifi_service, "test");

    testable.CancelReadingProcess();
    CHECK_EQUAL(LogicElement::MinValue, testable.ReadValue());
    CHECK_EQUAL(LogicElement::MinValue, testable.PeekValue());
    CHECK_EQUAL(LogicElement::MinValue, *testable.PublicMorozov_Get_out_value());
}

TEST(LogicControllerVariableTestsGroup, FetchValue_from_wifi_sta_client_calls_ConnectToStation) {
    TestableControllerVariable testable;
    testable.Init();

    mock()
        .expectNCalls(1, "xTaskGenericNotify")
        .withUnsignedIntParameter("ulValue", 0)
        .withIntParameter("eAction", eNotifyAction::eNoAction)
        .ignoreOtherParameters();

    TestableWiFiService wifi_service;
    testable.BindToStaWiFi(&wifi_service);

    testable.FetchValue();
    CHECK_EQUAL(1, wifi_service.PublicMorozov_Get_requests()->size());
    CHECK_EQUAL(RequestItemType::wqi_Station,
                (wifi_service.PublicMorozov_Get_requests()->begin())->Type);
}

TEST(LogicControllerVariableTestsGroup, CommitChanges_for_wifi_sta_client_does_nothing) {
    TestableControllerVariable testable;
    testable.Init();

    TestableWiFiService wifi_service;
    testable.BindToStaWiFi(&wifi_service);

    testable.WriteValue(42);
    testable.CommitChanges();
    CHECK_EQUAL(0, wifi_service.PublicMorozov_Get_requests()->size());
}

TEST(LogicControllerVariableTestsGroup,
     CancelReadingProcess_from_wifi_sta_client_calls_DisconnectFromStation) {
    TestableControllerVariable testable;
    testable.Init();

    mock()
        .expectNCalls(1, "xTaskGenericNotify")
        .withUnsignedIntParameter("ulValue", 0)
        .withIntParameter("eAction", eNotifyAction::eNoAction)
        .ignoreOtherParameters();

    mock()
        .expectNCalls(1, "xTaskGenericNotify")
        .withUnsignedIntParameter("ulValue", WiFiService::CANCEL_REQUEST_BIT)
        .withIntParameter("eAction", eNotifyAction::eSetBits)
        .ignoreOtherParameters();

    TestableWiFiService wifi_service;
    wifi_service.ConnectToStation();

    CHECK_EQUAL(1, wifi_service.PublicMorozov_Get_requests()->size());
    testable.BindToStaWiFi(&wifi_service);

    testable.CancelReadingProcess();
    CHECK_EQUAL(0, wifi_service.PublicMorozov_Get_requests()->size());
}

TEST(LogicControllerVariableTestsGroup, FetchValue_from_InsecureWiFi_calls_Scanner) {
    TestableControllerVariable testable;
    testable.Init();

    mock()
        .expectNCalls(1, "xTaskGenericNotify")
        .withUnsignedIntParameter("ulValue", 0)
        .withIntParameter("eAction", eNotifyAction::eNoAction)
        .ignoreOtherParameters();

    TestableWiFiService wifi_service;
    testable.BindToInsecureWiFi(&wifi_service, "any_ssid");

    testable.FetchValue();
    CHECK_EQUAL(1, wifi_service.PublicMorozov_Get_requests()->size());
    CHECK_EQUAL(RequestItemType::wqi_Scanner,
                (wifi_service.PublicMorozov_Get_requests()->begin())->Type);
}

TEST(LogicControllerVariableTestsGroup, CommitChanges_for_InsecureWiFi_calls_AccessPoint) {
    TestableControllerVariable testable;
    testable.Init();

    mock()
        .expectNCalls(1, "xTaskGenericNotify")
        .withUnsignedIntParameter("ulValue", 0)
        .withIntParameter("eAction", eNotifyAction::eNoAction)
        .ignoreOtherParameters();

    TestableWiFiService wifi_service;
    testable.BindToInsecureWiFi(&wifi_service, "any_ssid");

    testable.WriteValue(42);
    testable.CommitChanges();
    CHECK_EQUAL(1, wifi_service.PublicMorozov_Get_requests()->size());
    CHECK_EQUAL(RequestItemType::wqi_AccessPoint,
                (wifi_service.PublicMorozov_Get_requests()->begin())->Type);
}

TEST(LogicControllerVariableTestsGroup, CancelReadingProcess_from_InsecureWiFi_calls_CancelScan) {
    TestableControllerVariable testable;
    testable.Init();

    mock()
        .expectNCalls(1, "xTaskGenericNotify")
        .withUnsignedIntParameter("ulValue", 0)
        .withIntParameter("eAction", eNotifyAction::eNoAction)
        .ignoreOtherParameters();

    mock()
        .expectNCalls(1, "xTaskGenericNotify")
        .withUnsignedIntParameter("ulValue", WiFiService::CANCEL_REQUEST_BIT)
        .withIntParameter("eAction", eNotifyAction::eSetBits)
        .ignoreOtherParameters();

    TestableWiFiService wifi_service;
    const char *ssid = "any_ssid";
    wifi_service.Scan(ssid);

    CHECK_EQUAL(1, wifi_service.PublicMorozov_Get_requests()->size());
    testable.BindToInsecureWiFi(&wifi_service, ssid);

    testable.CancelReadingProcess();
    CHECK_EQUAL(0, wifi_service.PublicMorozov_Get_requests()->size());
}

TEST(LogicControllerVariableTestsGroup, FetchValue_from_SecureWiFi_calls_AccessPoint) {
    TestableControllerVariable testable;
    testable.Init();

    mock()
        .expectNCalls(1, "xTaskGenericNotify")
        .withUnsignedIntParameter("ulValue", 0)
        .withIntParameter("eAction", eNotifyAction::eNoAction)
        .ignoreOtherParameters();

    TestableWiFiService wifi_service;
    testable.BindToSecureWiFi(&wifi_service, "any_ssid", "pass", "************");

    testable.FetchValue();
    CHECK_EQUAL(1, wifi_service.PublicMorozov_Get_requests()->size());
    CHECK_EQUAL(RequestItemType::wqi_AccessPoint,
                (wifi_service.PublicMorozov_Get_requests()->begin())->Type);
}

TEST(LogicControllerVariableTestsGroup, CommitChanges_for_SecureWiFi_does_nothing) {
    TestableControllerVariable testable;
    testable.Init();

    TestableWiFiService wifi_service;
    testable.BindToSecureWiFi(&wifi_service, "any_ssid", "pass", "************");

    testable.WriteValue(42);
    testable.CommitChanges();
    CHECK_EQUAL(0, wifi_service.PublicMorozov_Get_requests()->size());
}

TEST(LogicControllerVariableTestsGroup,
     CancelReadingProcess_from_SecureWiFi_calls_CancelAccessPoint) {
    TestableControllerVariable testable;
    testable.Init();

    mock()
        .expectNCalls(1, "xTaskGenericNotify")
        .withUnsignedIntParameter("ulValue", 0)
        .withIntParameter("eAction", eNotifyAction::eNoAction)
        .ignoreOtherParameters();

    mock()
        .expectNCalls(1, "xTaskGenericNotify")
        .withUnsignedIntParameter("ulValue", WiFiService::CANCEL_REQUEST_BIT)
        .withIntParameter("eAction", eNotifyAction::eSetBits)
        .ignoreOtherParameters();

    TestableWiFiService wifi_service;
    const char *ssid = "any_ssid";
    const char *password = "pass";
    const char *mac = "************";
    wifi_service.AccessPoint(ssid, password, mac);

    CHECK_EQUAL(1, wifi_service.PublicMorozov_Get_requests()->size());
    testable.BindToSecureWiFi(&wifi_service, ssid, password, mac);

    testable.CancelReadingProcess();
    CHECK_EQUAL(0, wifi_service.PublicMorozov_Get_requests()->size());
}

TEST(LogicControllerVariableTestsGroup, FetchValue_when_binded_to_datetime_seconds) {
    TestableControllerVariable testable;
    testable.Init();

    DatetimeService datetime_service;
    testable.BindToDateTime(&datetime_service, DatetimePart::t_second);

    testable.FetchValue();
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    DOUBLES_EQUAL(tm.tm_sec, testable.PeekValue(), 1);
}

TEST(LogicControllerVariableTestsGroup, FetchValue_when_binded_to_datetime_minutes) {
    TestableControllerVariable testable;
    testable.Init();

    DatetimeService datetime_service;
    testable.BindToDateTime(&datetime_service, DatetimePart::t_minute);

    testable.FetchValue();
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    DOUBLES_EQUAL(tm.tm_min, testable.PeekValue(), 1);
}

TEST(LogicControllerVariableTestsGroup, FetchValue_when_binded_to_datetime_hours) {
    TestableControllerVariable testable;
    testable.Init();

    DatetimeService datetime_service;
    testable.BindToDateTime(&datetime_service, DatetimePart::t_hour);

    testable.FetchValue();
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    DOUBLES_EQUAL(tm.tm_hour, testable.PeekValue(), 1);
}

TEST(LogicControllerVariableTestsGroup, FetchValue_when_binded_to_datetime_days) {
    TestableControllerVariable testable;
    testable.Init();

    DatetimeService datetime_service;
    testable.BindToDateTime(&datetime_service, DatetimePart::t_day);

    testable.FetchValue();
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    DOUBLES_EQUAL(tm.tm_mday, testable.PeekValue(), 1);
}

TEST(LogicControllerVariableTestsGroup, FetchValue_when_binded_to_datetime_weekdays) {
    TestableControllerVariable testable;
    testable.Init();

    DatetimeService datetime_service;
    testable.BindToDateTime(&datetime_service, DatetimePart::t_weekday);

    testable.FetchValue();
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    DOUBLES_EQUAL(tm.tm_wday, testable.PeekValue(), 1);
}

TEST(LogicControllerVariableTestsGroup, FetchValue_when_binded_to_datetime_months) {
    TestableControllerVariable testable;
    testable.Init();

    DatetimeService datetime_service;
    testable.BindToDateTime(&datetime_service, DatetimePart::t_month);

    testable.FetchValue();
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    DOUBLES_EQUAL(tm.tm_mon, testable.PeekValue(), 1);
}

TEST(LogicControllerVariableTestsGroup, FetchValue_when_binded_to_datetime_years) {
    TestableControllerVariable testable;
    testable.Init();

    DatetimeService datetime_service;
    testable.BindToDateTime(&datetime_service, DatetimePart::t_year);

    testable.FetchValue();
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    DOUBLES_EQUAL(tm.tm_year, testable.PeekValue(), 1);
}

TEST(LogicControllerVariableTestsGroup, CancelReadingProcess_reset_values_when_binded_to_datetime) {
    TestableControllerVariable testable;
    testable.Init();
    testable.WriteValue(42);
    testable.CommitChanges();

    CHECK_EQUAL(42, testable.ReadValue());
    testable.CancelReadingProcess();
    CHECK_EQUAL(42, testable.ReadValue());
    CHECK_EQUAL(42, *testable.PublicMorozov_Get_out_value());
    
    DatetimeService datetime_service;
    testable.BindToDateTime(&datetime_service, DatetimePart::t_year);

    testable.CancelReadingProcess();
    CHECK_EQUAL(LogicElement::MinValue, testable.ReadValue());
    CHECK_EQUAL(LogicElement::MinValue, testable.PeekValue());
    CHECK_EQUAL(LogicElement::MinValue, *testable.PublicMorozov_Get_out_value());
}

TEST(LogicControllerVariableTestsGroup, Unbind_remove_InsecureWiFi_binding) {
    TestableControllerVariable testable;
    testable.Init();

    WiFiService wifi_service;
    testable.BindToInsecureWiFi(&wifi_service, "test");

    CHECK_TRUE(testable.BindedToWiFi());
    testable.Unbind();
    CHECK_FALSE(testable.BindedToWiFi());
}

TEST(LogicControllerVariableTestsGroup, Unbind_remove_StaWiFi_binding) {
    TestableControllerVariable testable;
    testable.Init();

    WiFiService wifi_service;
    testable.BindToStaWiFi(&wifi_service);

    CHECK_TRUE(testable.BindedToWiFi());
    testable.Unbind();
    CHECK_FALSE(testable.BindedToWiFi());
}

TEST(LogicControllerVariableTestsGroup, Unbind_remove_SecureWiFi_binding) {
    TestableControllerVariable testable;
    testable.Init();

    WiFiService wifi_service;
    testable.BindToSecureWiFi(&wifi_service, "any_ssid", "pass", "************");

    CHECK_TRUE(testable.BindedToWiFi());
    testable.Unbind();
    CHECK_FALSE(testable.BindedToWiFi());
}

TEST(LogicControllerVariableTestsGroup, Unbind_remove_DateTime_binding) {
    TestableControllerVariable testable;
    testable.Init();

    DatetimeService datetime_service;
    testable.BindToDateTime(&datetime_service, DatetimePart::t_second);

    CHECK_TRUE(testable.BindedToDateTime());
    testable.Unbind();
    CHECK_FALSE(testable.BindedToDateTime());
}