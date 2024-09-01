#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/InputNC.h"
#include "main/LogicProgram/Outputs/SetOutput.h"

TEST_GROUP(LogicSetOutputTestsGroup){ //
                                      TEST_SETUP(){}

                                      TEST_TEARDOWN(){}
};

namespace {
    class TestableSetOutput : public SetOutput {
      public:
        TestableSetOutput(const MapIO io_adr) : SetOutput(io_adr) {
        }
        virtual ~TestableSetOutput() {
        }
        LogicItemState *PublicMorozov_Get_state() {
            return &state;
        }
        TvElementType PublicMorozov_GetElementType() {
            return GetElementType();
        }
    };
} // namespace

TEST(LogicSetOutputTestsGroup, DoAction_skip_when_incoming_passive) {

    TestableSetOutput testable(MapIO::V1);

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
}

TEST(LogicSetOutputTestsGroup, DoAction_change_state_to_active__and_second_call_does_nothing) {

    TestableSetOutput testable(MapIO::V1);

    Controller::SetV1RelativeValue(LogicElement::MinValue);

    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());
    CHECK_EQUAL(LogicElement::MaxValue, Controller::GetV1RelativeValue());

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicElement::MaxValue, Controller::GetV1RelativeValue());
}

TEST(LogicSetOutputTestsGroup, DoAction_change_state_to_passive) {

    Controller::SetV1RelativeValue(LogicElement::MinValue);

    TestableSetOutput testable(MapIO::V1);
    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;

    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
    CHECK_EQUAL(LogicElement::MinValue, Controller::GetV1RelativeValue());
}


TEST(LogicSetOutputTestsGroup, GetElementType_returns_et_SetOutput) {
    TestableSetOutput testable(MapIO::O1);
    CHECK_EQUAL(TvElementType::et_SetOutput, testable.PublicMorozov_GetElementType());
}

TEST(LogicSetOutputTestsGroup, Serialize) {
    uint8_t buffer[256] = {};
    TestableSetOutput testable(MapIO::O1);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(1, writed);

    CHECK_EQUAL(TvElementType::et_SetOutput, *((TvElementType *)&buffer[0]));
}

TEST(LogicSetOutputTestsGroup, Deserialize) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_SetOutput;

    TestableSetOutput testable(MapIO::O1);

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);
}