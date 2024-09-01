#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/InputNC.h"
#include "main/LogicProgram/Outputs/DecOutput.h"

TEST_GROUP(LogicDecOutputTestsGroup){ //
                                      TEST_SETUP(){}

                                      TEST_TEARDOWN(){}
};

namespace {
    class TestableDecOutput : public DecOutput {
      public:
        TestableDecOutput(const MapIO io_adr) : DecOutput(io_adr) {
        }
        virtual ~TestableDecOutput() {
        }
        LogicItemState *PublicMorozov_Get_state() {
            return &state;
        }
        TvElementType PublicMorozov_GetElementType() {
            return GetElementType();
        }
    };
} // namespace

TEST(LogicDecOutputTestsGroup, DoAction_skip_when_incoming_passive) {
    TestableDecOutput testable(MapIO::V1);

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
}

TEST(LogicDecOutputTestsGroup,
     DoAction_change_state_to_active__and_second_call_does_not_decrement) {

    TestableDecOutput testable(MapIO::V1);

    Controller::SetV1RelativeValue(42);

    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());
    CHECK_EQUAL(41, Controller::GetV1RelativeValue());

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(41, Controller::GetV1RelativeValue());
}

TEST(LogicDecOutputTestsGroup, DoAction_change_state_to_passive) {

    Controller::SetV1RelativeValue(42);

    TestableDecOutput testable(MapIO::V1);
    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;

    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
    CHECK_EQUAL(42, Controller::GetV1RelativeValue());
}


TEST(LogicDecOutputTestsGroup, GetElementType_returns_et_DecOutput) {
    TestableDecOutput testable(MapIO::O1);
    CHECK_EQUAL(TvElementType::et_DecOutput, testable.PublicMorozov_GetElementType());
}

TEST(LogicDecOutputTestsGroup, Serialize) {
    uint8_t buffer[256] = {};
    TestableDecOutput testable(MapIO::O1);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(1, writed);

    CHECK_EQUAL(TvElementType::et_DecOutput, *((TvElementType *)&buffer[0]));
}

TEST(LogicDecOutputTestsGroup, Deserialize) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_DecOutput;

    TestableDecOutput testable(MapIO::O1);

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);
}