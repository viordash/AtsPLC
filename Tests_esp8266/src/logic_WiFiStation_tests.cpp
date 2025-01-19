#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/ComparatorEq.h"
#include "main/LogicProgram/Inputs/ComparatorGE.h"
#include "main/LogicProgram/Inputs/ComparatorGr.h"
#include "main/LogicProgram/Inputs/ComparatorLE.h"
#include "main/LogicProgram/Inputs/ComparatorLs.h"
#include "main/LogicProgram/Inputs/InputNO.h"
#include "main/LogicProgram/Inputs/WiFiStation.h"

static uint8_t frame_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8] = {};

TEST_GROUP(LogicWiFiStationTestsGroup){ //
                                        TEST_SETUP(){ memset(frame_buffer, 0, sizeof(frame_buffer));

mock().expectOneCall("vTaskDelay").ignoreOtherParameters();
mock().expectOneCall("xTaskCreate").ignoreOtherParameters();
Controller::Start(NULL, NULL);
}

TEST_TEARDOWN() {
    Controller::Stop();
}
}
;

namespace {
    class TestableWiFiStation : public WiFiStation {
      public:
        TestableWiFiStation() : WiFiStation() {
        }
        virtual ~TestableWiFiStation() {
        }

        LogicItemState *PublicMorozov_Get_state() {
            return &state;
        }
    };
} // namespace

TEST(LogicWiFiStationTestsGroup, Render_when_active) {
    TestableWiFiStation testable;

    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;

    Point start_point = { 0, INCOME_RAIL_TOP };
    CHECK_TRUE(testable.Render(frame_buffer, LogicItemState::lisActive, &start_point));

    bool any_pixel_coloring = false;
    for (size_t i = 0; i < sizeof(frame_buffer); i++) {
        if (frame_buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_TRUE(any_pixel_coloring);
    CHECK_EQUAL(32, start_point.x);
}

TEST(LogicWiFiStationTestsGroup, Render_when_passive) {
    TestableWiFiStation testable;

    Point start_point = { 0, INCOME_RAIL_TOP };
    CHECK_TRUE(testable.Render(frame_buffer, LogicItemState::lisActive, &start_point));

    bool any_pixel_coloring = false;
    for (size_t i = 0; i < sizeof(frame_buffer); i++) {
        if (frame_buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_TRUE(any_pixel_coloring);
    CHECK_EQUAL(32, start_point.x);
}

TEST(LogicWiFiStationTestsGroup, DoAction_skip_when_incoming_passive) {
    TestableWiFiStation testable;

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
}

TEST(LogicWiFiStationTestsGroup, DoAction_change_state_to_passive__due_incoming_switch_to_passive) {
    TestableWiFiStation testable;
    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;

    Controller::FetchIOValues();
    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisActive));
    Controller::CommitChanges();
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());

    Controller::FetchIOValues();
    CHECK_TRUE(testable.DoAction(true, LogicItemState::lisPassive));
    Controller::CommitChanges();
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());

    Controller::FetchIOValues();
    CHECK_FALSE_TEXT(testable.DoAction(true, LogicItemState::lisPassive),
                     "no changes are expected to be detected");
}

TEST(LogicWiFiStationTestsGroup, DoAction_change_state_to_active) {
    TestableWiFiStation testable;

    Controller::FetchIOValues();
    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    Controller::CommitChanges();
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());
}

TEST(LogicWiFiStationTestsGroup, DoAction_change_state_to_passive) {
    TestableWiFiStation testable;
    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;

    Controller::FetchIOValues();
    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
}

TEST(LogicWiFiStationTestsGroup, Serialize) {
    uint8_t buffer[256] = {};
    TestableWiFiStation testable;

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(2, writed);

    CHECK_EQUAL(TvElementType::et_WiFiStation, *((TvElementType *)&buffer[0]));
    CHECK_EQUAL(MapIO::V2, *((MapIO *)&buffer[1]));
}

TEST(LogicWiFiStationTestsGroup, Serialize_just_for_obtain_size) {
    TestableWiFiStation testable;

    size_t writed = testable.Serialize(NULL, SIZE_MAX);
    CHECK_EQUAL(2, writed);

    writed = testable.Serialize(NULL, 0);
    CHECK_EQUAL(2, writed);
}

TEST(LogicWiFiStationTestsGroup, Serialize_to_small_buffer_return_zero) {
    uint8_t buffer[1] = {};
    TestableWiFiStation testable;

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, writed);
}

TEST(LogicWiFiStationTestsGroup, Deserialize) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_WiFiStation;
    *((MapIO *)&buffer[1]) = MapIO::V3;

    TestableWiFiStation testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(1, readed);
}

TEST(LogicWiFiStationTestsGroup, Deserialize_with_small_buffer_return_zero) {
    uint8_t buffer[0] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_WiFiStation;

    TestableWiFiStation testable;

    size_t readed = testable.Deserialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, readed);
}

TEST(LogicWiFiStationTestsGroup, Deserialize_with_wrong_io_adr_return_zero) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_WiFiStation;

    TestableWiFiStation testable;

    *((MapIO *)&buffer[1]) = (MapIO)(MapIO::DI - 1);
    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);

    *((MapIO *)&buffer[1]) = (MapIO)(MapIO::V4 + 1);
    readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);

    *((MapIO *)&buffer[1]) = MapIO::DI;
    readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(1, readed);
}

TEST(LogicWiFiStationTestsGroup, GetElementType) {
    TestableWiFiStation testable;
    CHECK_EQUAL(TvElementType::et_WiFiStation, testable.GetElementType());
}

TEST(LogicWiFiStationTestsGroup, TryToCast) {
    WiFiStation wiFiStation;
    CHECK_TRUE(WiFiStation::TryToCast(&wiFiStation) == &wiFiStation);

    InputNO inputNO;
    CHECK_TRUE(WiFiStation::TryToCast(&inputNO) == NULL);

    ComparatorEq comparatorEq;
    CHECK_TRUE(WiFiStation::TryToCast(&comparatorEq) == NULL);

    ComparatorGE comparatorGE;
    CHECK_TRUE(WiFiStation::TryToCast(&comparatorGE) == NULL);

    ComparatorGr comparatorGr;
    CHECK_TRUE(WiFiStation::TryToCast(&comparatorGr) == NULL);

    ComparatorLE comparatorLE;
    CHECK_TRUE(WiFiStation::TryToCast(&comparatorLE) == NULL);

    ComparatorLs comparatorLs;
    CHECK_TRUE(WiFiStation::TryToCast(&comparatorLs) == NULL);
}
