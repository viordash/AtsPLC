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
#include "main/LogicProgram/Inputs/ComparatorLs.cpp"
#include "main/LogicProgram/Inputs/ComparatorLs.h"

static FrameBuffer frame_buffer = {};

TEST_GROUP(LogicComparatorLsTestsGroup){
    //
    TEST_SETUP(){ memset(&frame_buffer.buffer, 0, sizeof(frame_buffer.buffer));
mock().expectOneCall("vTaskDelay").ignoreOtherParameters();
mock().expectOneCall("xTaskCreate").ignoreOtherParameters();
Controller::Start(NULL, NULL, NULL, NULL);
}

TEST_TEARDOWN() {
    Controller::Stop();
}
}
;

namespace {
    class TestableComparatorLs : public ComparatorLs {
      public:
        TestableComparatorLs() : ComparatorLs() {
        }
        virtual ~TestableComparatorLs() {
        }
        LogicItemState *PublicMorozov_Get_state() {
            return &state;
        }
        TvElementType PublicMorozov_GetElementType() {
            return GetElementType();
        }
        const char *PublicMorozov_Get_str_reference() {
            return str_reference;
        }
    };
} // namespace

TEST(LogicComparatorLsTestsGroup, Render) {

    TestableComparatorLs testable;
    testable.SetReference(42);
    testable.SetIoAdr(MapIO::AI);

    Point start_point = { 0, INCOME_RAIL_TOP };
    CHECK_TRUE(testable.Render(&frame_buffer, LogicItemState::lisActive, &start_point));

    bool any_pixel_coloring = false;
    for (size_t i = 0; i < sizeof(frame_buffer.buffer); i++){
        if (frame_buffer.buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_TRUE(any_pixel_coloring);
    CHECK_EQUAL(32, start_point.x);
}

TEST(LogicComparatorLsTestsGroup, DoAction_skip_when_incoming_passive) {
    mock().expectNoCall("adc_read");

    TestableComparatorLs testable;
    testable.SetReference(42);
    testable.SetIoAdr(MapIO::AI);

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
}

TEST(LogicComparatorLsTestsGroup, DoAction_change_state_to_active) {
    volatile uint16_t adc = 51 / 0.1;
    mock("0").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(2, "esp_timer_get_time").ignoreOtherParameters();
    mock()
        .expectNCalls(2, "adc_read")
        .withOutputParameterReturning("adc", (const void *)&adc, sizeof(adc));

    TestableComparatorLs testable;
    testable.SetReference(51 / 0.4);
    testable.SetIoAdr(MapIO::AI);
    Controller::FetchIOValues();
    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());

    adc = 49 / 0.1;
    Controller::RemoveRequestWakeupMs((void *)&Controller::AI);
    Controller::FetchIOValues();
    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());
}

TEST(LogicComparatorLsTestsGroup, DoAction_change_state_to_passive) {
    volatile uint16_t adc = 49 / 0.1;
    mock("0").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(2, "esp_timer_get_time").ignoreOtherParameters();
    mock()
        .expectNCalls(2, "adc_read")
        .withOutputParameterReturning("adc", (const void *)&adc, sizeof(adc));

    TestableComparatorLs testable;
    testable.SetReference(50 / 0.4);
    testable.SetIoAdr(MapIO::AI);
    Controller::FetchIOValues();
    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());

    adc = 51 / 0.1;
    Controller::RemoveRequestWakeupMs((void *)&Controller::AI);
    Controller::FetchIOValues();
    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
}

TEST(LogicComparatorLsTestsGroup, GetElementType_returns_et_ComparatorLs) {
    TestableComparatorLs testable;
    testable.SetIoAdr(MapIO::AI);
    CHECK_EQUAL(TvElementType::et_ComparatorLs, testable.PublicMorozov_GetElementType());
}

TEST(LogicComparatorLsTestsGroup, Serialize) {
    uint8_t buffer[256] = {};
    TestableComparatorLs testable;
    testable.SetReference(42);
    testable.SetIoAdr(MapIO::AI);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(3, writed);

    CHECK_EQUAL(TvElementType::et_ComparatorLs, *((TvElementType *)&buffer[0]));
}

TEST(LogicComparatorLsTestsGroup, Deserialize) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_ComparatorLs;
    *((uint8_t *)&buffer[1]) = 42;
    *((MapIO *)&buffer[2]) = MapIO::V3;

    TestableComparatorLs testable;
    testable.SetReference(19);
    testable.SetIoAdr(MapIO::AI);

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(2, readed);
    CHECK(&Controller::V3 == testable.Input);
    STRCMP_EQUAL("42", testable.PublicMorozov_Get_str_reference());
}

TEST(LogicComparatorLsTestsGroup, GetElementType) {
    TestableComparatorLs testable;
    CHECK_EQUAL(TvElementType::et_ComparatorLs, testable.GetElementType());
}

TEST(LogicComparatorLsTestsGroup, TryToCast) {
    ComparatorEq comparatorEq;
    CHECK_TRUE(ComparatorLs::TryToCast(&comparatorEq) == NULL);

    ComparatorGE comparatorGE;
    CHECK_TRUE(ComparatorLs::TryToCast(&comparatorGE) == NULL);

    ComparatorGr comparatorGr;
    CHECK_TRUE(ComparatorLs::TryToCast(&comparatorGr) == NULL);

    ComparatorLE comparatorLE;
    CHECK_TRUE(ComparatorLs::TryToCast(&comparatorLE) == NULL);

    ComparatorLs comparatorLs;
    CHECK_TRUE(ComparatorLs::TryToCast(&comparatorLs) == &comparatorLs);
}
