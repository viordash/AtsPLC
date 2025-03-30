#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/Datetime/DatetimeService.h"
#include "main/LogicProgram/Bindings/DateTimeBinding.h"
#include "main/LogicProgram/Bindings/DatetimePart.h"
#include "main/LogicProgram/Inputs/ComparatorEq.h"
#include "main/LogicProgram/Inputs/ComparatorGE.h"
#include "main/LogicProgram/Inputs/ComparatorGr.h"
#include "main/LogicProgram/Inputs/ComparatorLE.h"
#include "main/LogicProgram/Inputs/ComparatorLs.h"
#include "main/LogicProgram/Inputs/InputNC.h"
#include "main/LogicProgram/Inputs/InputNO.h"

static uint8_t frame_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8] = {};

static WiFiService *wifi_service;
static DatetimeService *datetime_service;
TEST_GROUP(LogicDateTimeBindingTestsGroup){
    //
    TEST_SETUP(){ memset(frame_buffer, 0, sizeof(frame_buffer));

mock().expectOneCall("vTaskDelay").ignoreOtherParameters();
mock().expectOneCall("xTaskCreate").ignoreOtherParameters();
wifi_service = new WiFiService();
datetime_service = new DatetimeService();
Controller::Start(NULL, wifi_service, NULL, datetime_service);
}

TEST_TEARDOWN() {
    Controller::V1.Unbind();
    Controller::V2.Unbind();
    Controller::V3.Unbind();
    Controller::V4.Unbind();
    Controller::Stop();
    delete wifi_service;
    delete datetime_service;
}
}
;

namespace {
    class TestableDateTimeBinding : public DateTimeBinding {
      public:
        TestableDateTimeBinding() : DateTimeBinding() {
        }
        virtual ~TestableDateTimeBinding() {
        }

        const char *GetLabel() {
            return label;
        }
        LogicItemState *PublicMorozov_Get_state() {
            return &state;
        }
        int *PublicMorozov_Get_editing_property_id() {
            return &editing_property_id;
        }
        DatetimePart *PublicMorozov_datetime_part() {
            return &datetime_part;
        }

        bool PublicMorozov_ValidateDatetimePart(DatetimePart datetime_part) {
            return ValidateDatetimePart(datetime_part);
        }
    };
} // namespace

TEST(LogicDateTimeBindingTestsGroup, Render) {
    TestableDateTimeBinding testable;
    testable.SetIoAdr(MapIO::V1);
    *testable.PublicMorozov_datetime_part() = DatetimePart::t_minute;

    Point start_point = { INCOME_RAIL_WIDTH, INCOME_RAIL_TOP + INCOME_RAIL_NETWORK_TOP };
    CHECK_TRUE(testable.Render(frame_buffer, LogicItemState::lisActive, &start_point));

    bool any_pixel_coloring = false;
    for (size_t i = 0; i < sizeof(frame_buffer); i++) {
        if (frame_buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_TRUE(any_pixel_coloring);
    CHECK_EQUAL(113, start_point.x);
}

TEST(LogicDateTimeBindingTestsGroup, DoAction_skip_when_incoming_passive) {
    TestableDateTimeBinding testable;
    testable.SetIoAdr(MapIO::V1);

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
}

TEST(LogicDateTimeBindingTestsGroup, SelectNext_changing_IoAdr) {
    TestableDateTimeBinding testable;
    testable.SetIoAdr(MapIO::DI);
    *testable.PublicMorozov_datetime_part() = DatetimePart::t_minute;
    testable.BeginEditing();
    testable.Change();
    testable.SelectNext();
    CHECK_EQUAL(MapIO::V1, testable.GetIoAdr());
    testable.SelectNext();
    CHECK_EQUAL(MapIO::V2, testable.GetIoAdr());
    testable.SelectNext();
    CHECK_EQUAL(MapIO::V3, testable.GetIoAdr());
    testable.SelectNext();
    CHECK_EQUAL(MapIO::V4, testable.GetIoAdr());
    testable.SelectNext();
    CHECK_EQUAL(MapIO::V1, testable.GetIoAdr());
}

TEST(LogicDateTimeBindingTestsGroup, SelectPrior_changing_IoAdr) {
    TestableDateTimeBinding testable;
    testable.SetIoAdr(MapIO::DI);
    *testable.PublicMorozov_datetime_part() = DatetimePart::t_minute;
    testable.BeginEditing();
    testable.Change();
    testable.SelectPrior();
    CHECK_EQUAL(MapIO::V4, testable.GetIoAdr());
    testable.SelectPrior();
    CHECK_EQUAL(MapIO::V3, testable.GetIoAdr());
    testable.SelectPrior();
    CHECK_EQUAL(MapIO::V2, testable.GetIoAdr());
    testable.SelectPrior();
    CHECK_EQUAL(MapIO::V1, testable.GetIoAdr());
    testable.SelectPrior();
    CHECK_EQUAL(MapIO::V4, testable.GetIoAdr());
}

TEST(LogicDateTimeBindingTestsGroup, SelectNext_changing_DatetimePart) {
    TestableDateTimeBinding testable;
    testable.SetIoAdr(MapIO::DI);
    *testable.PublicMorozov_datetime_part() = DatetimePart::t_minute;
    testable.BeginEditing();
    testable.Change();
    testable.Change();
    testable.SelectNext();
    CHECK_EQUAL(DatetimePart::t_hour, *testable.PublicMorozov_datetime_part());
    testable.SelectNext();
    CHECK_EQUAL(DatetimePart::t_day, *testable.PublicMorozov_datetime_part());
    testable.SelectNext();
    CHECK_EQUAL(DatetimePart::t_weekday, *testable.PublicMorozov_datetime_part());
    testable.SelectNext();
    CHECK_EQUAL(DatetimePart::t_month, *testable.PublicMorozov_datetime_part());
    testable.SelectNext();
    CHECK_EQUAL(DatetimePart::t_year, *testable.PublicMorozov_datetime_part());
    testable.SelectNext();
    CHECK_EQUAL(DatetimePart::t_second, *testable.PublicMorozov_datetime_part());
    testable.SelectNext();
    CHECK_EQUAL(DatetimePart::t_minute, *testable.PublicMorozov_datetime_part());
    testable.SelectNext();
    CHECK_EQUAL(DatetimePart::t_hour, *testable.PublicMorozov_datetime_part());
}

TEST(LogicDateTimeBindingTestsGroup, SelectPrior_changing_DatetimePart) {
    TestableDateTimeBinding testable;
    testable.SetIoAdr(MapIO::DI);
    *testable.PublicMorozov_datetime_part() = DatetimePart::t_minute;
    testable.BeginEditing();
    testable.Change();
    testable.Change();
    testable.SelectPrior();
    CHECK_EQUAL(DatetimePart::t_second, *testable.PublicMorozov_datetime_part());
    testable.SelectPrior();
    CHECK_EQUAL(DatetimePart::t_year, *testable.PublicMorozov_datetime_part());
    testable.SelectPrior();
    CHECK_EQUAL(DatetimePart::t_month, *testable.PublicMorozov_datetime_part());
    testable.SelectPrior();
    CHECK_EQUAL(DatetimePart::t_weekday, *testable.PublicMorozov_datetime_part());
    testable.SelectPrior();
    CHECK_EQUAL(DatetimePart::t_day, *testable.PublicMorozov_datetime_part());
    testable.SelectPrior();
    CHECK_EQUAL(DatetimePart::t_hour, *testable.PublicMorozov_datetime_part());
    testable.SelectPrior();
    CHECK_EQUAL(DatetimePart::t_minute, *testable.PublicMorozov_datetime_part());
    testable.SelectPrior();
    CHECK_EQUAL(DatetimePart::t_second, *testable.PublicMorozov_datetime_part());
}

TEST(LogicDateTimeBindingTestsGroup, Serialize) {
    uint8_t buffer[256] = {};
    TestableDateTimeBinding testable;
    testable.SetIoAdr(MapIO::V2);
    *testable.PublicMorozov_datetime_part() = DatetimePart::t_minute;

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(6, writed);

    CHECK_EQUAL(TvElementType::et_DateTimeBinding, *((TvElementType *)&buffer[0]));
    CHECK_EQUAL(MapIO::V2, *((MapIO *)&buffer[1]));
    CHECK_EQUAL(DatetimePart::t_minute,
                *((DatetimePart *)&buffer[2]));
}

TEST(LogicDateTimeBindingTestsGroup, Serialize_just_for_obtain_size) {
    TestableDateTimeBinding testable;
    testable.SetIoAdr(MapIO::DI);
    *testable.PublicMorozov_datetime_part() = DatetimePart::t_minute;

    size_t writed = testable.Serialize(NULL, SIZE_MAX);
    CHECK_EQUAL(6, writed);

    writed = testable.Serialize(NULL, 0);
    CHECK_EQUAL(6, writed);
}

TEST(LogicDateTimeBindingTestsGroup, Serialize_to_small_buffer_return_zero) {
    uint8_t buffer[1] = {};
    TestableDateTimeBinding testable;
    testable.SetIoAdr(MapIO::DI);
    *testable.PublicMorozov_datetime_part() = DatetimePart::t_minute;

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, writed);
}

TEST(LogicDateTimeBindingTestsGroup, Deserialize) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_DateTimeBinding;
    *((MapIO *)&buffer[1]) = MapIO::V3;
    *((DatetimePart *)&buffer[2]) = DatetimePart::t_weekday;

    TestableDateTimeBinding testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(5, readed);

    CHECK_EQUAL(MapIO::V3, testable.GetIoAdr());
    CHECK(&Controller::V3 == testable.Input);
    CHECK_EQUAL(DatetimePart::t_weekday, *testable.PublicMorozov_datetime_part());
}

TEST(LogicDateTimeBindingTestsGroup, Deserialize_with_small_buffer_return_zero) {
    uint8_t buffer[1] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_DateTimeBinding;

    TestableDateTimeBinding testable;

    size_t readed = testable.Deserialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, readed);
}

TEST(LogicDateTimeBindingTestsGroup, Deserialize_with_wrong_io_adr_return_zero) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_DateTimeBinding;
    *((DatetimePart *)&buffer[2]) = DatetimePart::t_weekday;

    TestableDateTimeBinding testable;

    *((MapIO *)&buffer[1]) = (MapIO)(MapIO::DI - 1);
    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);

    *((MapIO *)&buffer[1]) = (MapIO)(MapIO::V4 + 1);
    readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);

    *((MapIO *)&buffer[1]) = MapIO::DI;
    readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(5, readed);
}

TEST(LogicDateTimeBindingTestsGroup, Deserialize_with_wrong_DatetimePart_return_zero) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_DateTimeBinding;
    *((MapIO *)&buffer[1]) = MapIO::V3;

    TestableDateTimeBinding testable;

    *((DatetimePart *)&buffer[2]) =
        (DatetimePart)(DatetimePart::t_second - 1);
    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);

    *((DatetimePart *)&buffer[2]) =
        (DatetimePart)(DatetimePart::t_year + 1);
    readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);

    *((DatetimePart *)&buffer[2]) =
        (DatetimePart)(DatetimePart::t_month);
    readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(5, readed);
}

TEST(LogicDateTimeBindingTestsGroup, GetElementType) {
    TestableDateTimeBinding testable;
    CHECK_EQUAL(TvElementType::et_DateTimeBinding, testable.GetElementType());
}

TEST(LogicDateTimeBindingTestsGroup, TryToCast) {
    InputNC inputNC;
    CHECK_TRUE(DateTimeBinding::TryToCast(&inputNC) == NULL);

    InputNO inputNO;
    CHECK_TRUE(DateTimeBinding::TryToCast(&inputNO) == NULL);

    ComparatorEq comparatorEq;
    CHECK_TRUE(DateTimeBinding::TryToCast(&comparatorEq) == NULL);

    ComparatorGE comparatorGE;
    CHECK_TRUE(DateTimeBinding::TryToCast(&comparatorGE) == NULL);

    ComparatorGr comparatorGr;
    CHECK_TRUE(DateTimeBinding::TryToCast(&comparatorGr) == NULL);

    ComparatorLE comparatorLE;
    CHECK_TRUE(DateTimeBinding::TryToCast(&comparatorLE) == NULL);

    ComparatorLs comparatorLs;
    CHECK_TRUE(DateTimeBinding::TryToCast(&comparatorLs) == NULL);

    DateTimeBinding dateTimeBinding;
    CHECK_TRUE(DateTimeBinding::TryToCast(&dateTimeBinding) == &dateTimeBinding);
}

TEST(LogicDateTimeBindingTestsGroup, ValidateDatetimePart) {
    TestableDateTimeBinding testable;
    CHECK_TRUE(
        testable.PublicMorozov_ValidateDatetimePart(DatetimePart::t_second));
    CHECK_TRUE(
        testable.PublicMorozov_ValidateDatetimePart(DatetimePart::t_minute));
    CHECK_TRUE(testable.PublicMorozov_ValidateDatetimePart(DatetimePart::t_hour));
    CHECK_TRUE(testable.PublicMorozov_ValidateDatetimePart(DatetimePart::t_day));
    CHECK_TRUE(
        testable.PublicMorozov_ValidateDatetimePart(DatetimePart::t_weekday));
    CHECK_TRUE(testable.PublicMorozov_ValidateDatetimePart(DatetimePart::t_month));
    CHECK_TRUE(testable.PublicMorozov_ValidateDatetimePart(DatetimePart::t_year));

    CHECK_FALSE(testable.PublicMorozov_ValidateDatetimePart(
        (DatetimePart)((int)DatetimePart::t_second - 1)));

    CHECK_FALSE(testable.PublicMorozov_ValidateDatetimePart(
        (DatetimePart)((int)DatetimePart::t_year + 1)));
}