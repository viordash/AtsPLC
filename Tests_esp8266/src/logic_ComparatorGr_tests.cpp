#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/ComparatorGr.cpp"
#include "main/LogicProgram/Inputs/ComparatorGr.h"

static uint8_t frame_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8] = {};

TEST_GROUP(LogicComparatorGrTestsGroup){
    //
    TEST_SETUP(){ memset(frame_buffer, 0, sizeof(frame_buffer));
mock().expectOneCall("vTaskDelay").ignoreOtherParameters();
Controller::Stop();
}

TEST_TEARDOWN() {
}
}
;

namespace {
    class TestableComparatorGr : public ComparatorGr {
      public:
        TestableComparatorGr() : ComparatorGr() {
        }
        virtual ~TestableComparatorGr() {
        }

        LogicItemState *PublicMorozov_Get_state() {
            return &state;
        }
        TvElementType PublicMorozov_GetElementType() {
            return GetElementType();
        }
    };
} // namespace

TEST(LogicComparatorGrTestsGroup, Render) {
    TestableComparatorGr testable;
    testable.SetReference(42);
    testable.SetIoAdr(MapIO::AI);

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

TEST(LogicComparatorGrTestsGroup, DoAction_skip_when_incoming_passive) {
    mock().expectNoCall("adc_read");

    TestableComparatorGr testable;
    testable.SetReference(42);
    testable.SetIoAdr(MapIO::AI);

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
}

TEST(LogicComparatorGrTestsGroup, DoAction_change_state_to_active) {
    volatile uint16_t adc = 51 / 0.1;
    mock("0").expectNCalls(2, "gpio_get_level").ignoreOtherParameters();
    mock("2").expectNCalls(2, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(2, "gpio_get_level").ignoreOtherParameters();
    mock()
        .expectNCalls(2, "adc_read")
        .withOutputParameterReturning("adc", (const void *)&adc, sizeof(adc));

    TestableComparatorGr testable;
    testable.SetReference(51 / 0.4);
    testable.SetIoAdr(MapIO::AI);
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());

    adc = 52 / 0.1;
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());
}

TEST(LogicComparatorGrTestsGroup, DoAction_change_state_to_passive) {
    volatile uint16_t adc = 49 / 0.1;
    mock("0").expectNCalls(2, "gpio_get_level").ignoreOtherParameters();
    mock("2").expectNCalls(2, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(2, "gpio_get_level").ignoreOtherParameters();
    mock()
        .expectNCalls(2, "adc_read")
        .withOutputParameterReturning("adc", (const void *)&adc, sizeof(adc));

    TestableComparatorGr testable;
    testable.SetReference(48 / 0.4);
    testable.SetIoAdr(MapIO::AI);
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());

    adc = 47 / 0.1;
    CHECK_TRUE(Controller::SampleIOValues());
    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
}

TEST(LogicComparatorGrTestsGroup, GetElementType_returns_et_ComparatorGr) {
    TestableComparatorGr testable;
    testable.SetIoAdr(MapIO::AI);
    CHECK_EQUAL(TvElementType::et_ComparatorGr, testable.PublicMorozov_GetElementType());
}

TEST(LogicComparatorGrTestsGroup, Serialize) {
    uint8_t buffer[256] = {};
    TestableComparatorGr testable;
    testable.SetReference(42);
    testable.SetIoAdr(MapIO::AI);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(3, writed);

    CHECK_EQUAL(TvElementType::et_ComparatorGr, *((TvElementType *)&buffer[0]));
}

TEST(LogicComparatorGrTestsGroup, Deserialize) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_ComparatorGr;
    *((uint8_t *)&buffer[1]) = 42;
    *((MapIO *)&buffer[2]) = MapIO::V3;

    TestableComparatorGr testable;
    testable.SetReference(19);
    testable.SetIoAdr(MapIO::DI);

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(2, readed);
}

TEST(LogicComparatorGrTestsGroup, GetElementType) {
    TestableComparatorGr testable;
    CHECK_EQUAL(TvElementType::et_ComparatorGr, testable.GetElementType());
}