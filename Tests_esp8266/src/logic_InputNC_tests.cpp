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
#include "main/LogicProgram/Inputs/InputNC.h"
#include "main/LogicProgram/Inputs/InputNO.h"

static FrameBuffer frame_buffer = {};

TEST_GROUP(LogicInputNCTestsGroup){
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
    class TestableInputNC : public InputNC {
      public:
        TestableInputNC() : InputNC() {
        }
        virtual ~TestableInputNC() {
        }

        const char *GetLabel() {
            return label;
        }
        LogicItemState *PublicMorozov_Get_state() {
            return &state;
        }
    };
} // namespace

TEST(LogicInputNCTestsGroup, DoAction_skip_when_incoming_passive) {
    TestableInputNC testable;
    testable.SetIoAdr(MapIO::DI);

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisPassive).any_changes);
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
}

TEST(LogicInputNCTestsGroup, DoAction_change_state_to_passive__due_incoming_switch_to_passive) {
    mock().expectNCalls(1, "adc_read").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();

    mock("0").expectNCalls(2, "gpio_get_level").andReturnValue(1);
    TestableInputNC testable;
    testable.SetIoAdr(MapIO::DI);
    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;

    Controller::FetchIOValues();
    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisActive).any_changes);
    Controller::CommitChanges();
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());

    Controller::FetchIOValues();
    CHECK_TRUE(testable.DoAction(true, LogicItemState::lisPassive).any_changes);
    Controller::CommitChanges();
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());

    Controller::FetchIOValues();
    CHECK_FALSE_TEXT(testable.DoAction(true, LogicItemState::lisPassive).any_changes,
                     "no changes are expected to be detected");
}

TEST(LogicInputNCTestsGroup, DoAction_change_state_to_active) {
    mock().expectNCalls(1, "adc_read").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();

    mock("0").expectNCalls(1, "gpio_get_level").andReturnValue(1);

    TestableInputNC testable;
    testable.SetIoAdr(MapIO::DI);

    Controller::FetchIOValues();
    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive).any_changes);
    Controller::CommitChanges();
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());
}

TEST(LogicInputNCTestsGroup, DoAction_change_state_to_passive) {
    mock().expectNCalls(1, "adc_read").ignoreOtherParameters();
    mock("2").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock("15").expectNCalls(1, "gpio_get_level").ignoreOtherParameters();
    mock().expectNCalls(1, "esp_timer_get_time").ignoreOtherParameters();

    mock("0").expectNCalls(1, "gpio_get_level").andReturnValue(0);

    TestableInputNC testable;
    testable.SetIoAdr(MapIO::DI);
    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;

    Controller::FetchIOValues();
    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive).any_changes);
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
}

TEST(LogicInputNCTestsGroup, Serialize) {
    uint8_t buffer[256] = {};
    TestableInputNC testable;
    testable.SetIoAdr(MapIO::V2);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(2, writed);

    CHECK_EQUAL(TvElementType::et_InputNC, *((TvElementType *)&buffer[0]));
    CHECK_EQUAL(MapIO::V2, *((MapIO *)&buffer[1]));
}

TEST(LogicInputNCTestsGroup, Serialize_just_for_obtain_size) {
    TestableInputNC testable;
    testable.SetIoAdr(MapIO::DI);

    size_t writed = testable.Serialize(NULL, SIZE_MAX);
    CHECK_EQUAL(2, writed);

    writed = testable.Serialize(NULL, 0);
    CHECK_EQUAL(2, writed);
}

TEST(LogicInputNCTestsGroup, Serialize_to_small_buffer_return_zero) {
    uint8_t buffer[1] = {};
    TestableInputNC testable;
    testable.SetIoAdr(MapIO::DI);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, writed);
}

TEST(LogicInputNCTestsGroup, Deserialize) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_InputNC;
    *((MapIO *)&buffer[1]) = MapIO::V3;

    TestableInputNC testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(1, readed);

    CHECK_EQUAL(MapIO::V3, testable.GetIoAdr());
    CHECK(&Controller::V3 == testable.Input);
}

TEST(LogicInputNCTestsGroup, Deserialize_with_small_buffer_return_zero) {
    uint8_t buffer[0] = {};

    TestableInputNC testable;

    size_t readed = testable.Deserialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, readed);
}

TEST(LogicInputNCTestsGroup, Deserialize_with_wrong_io_adr_return_zero) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_InputNC;

    TestableInputNC testable;

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

TEST(LogicInputNCTestsGroup, GetElementType) {
    TestableInputNC testable;
    CHECK_EQUAL(TvElementType::et_InputNC, testable.GetElementType());
}

TEST(LogicInputNCTestsGroup, TryToCast) {
    InputNC inputNC;
    CHECK_TRUE(InputNC::TryToCast(&inputNC) == &inputNC);

    InputNO inputNO;
    CHECK_TRUE(InputNC::TryToCast(&inputNO) == NULL);

    ComparatorEq comparatorEq;
    CHECK_TRUE(InputNC::TryToCast(&comparatorEq) == NULL);

    ComparatorGE comparatorGE;
    CHECK_TRUE(InputNC::TryToCast(&comparatorGE) == NULL);

    ComparatorGr comparatorGr;
    CHECK_TRUE(InputNC::TryToCast(&comparatorGr) == NULL);

    ComparatorLE comparatorLE;
    CHECK_TRUE(InputNC::TryToCast(&comparatorLE) == NULL);

    ComparatorLs comparatorLs;
    CHECK_TRUE(InputNC::TryToCast(&comparatorLs) == NULL);
}
