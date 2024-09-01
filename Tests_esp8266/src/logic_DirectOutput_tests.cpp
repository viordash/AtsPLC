#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/InputNC.h"
#include "main/LogicProgram/Outputs/DirectOutput.h"

TEST_GROUP(LogicDirectOutputTestsGroup){ //
                                         TEST_SETUP(){}

                                         TEST_TEARDOWN(){}
};

namespace {
    class TestableDirectOutput : public DirectOutput {
      public:
        TestableDirectOutput(const MapIO io_adr) : DirectOutput(io_adr) {
        }
        virtual ~TestableDirectOutput() {
        }
        LogicItemState *PublicMorozov_Get_state() {
            return &state;
        }
        TvElementType PublicMorozov_GetElementType() {
            return GetElementType();
        }
    };
} // namespace

TEST(LogicDirectOutputTestsGroup, DoAction_skip_when_incoming_passive) {
    TestableDirectOutput testable(MapIO::V1);

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
}

TEST(LogicDirectOutputTestsGroup, DoAction_change_state_to_active) {
    TestableDirectOutput testable(MapIO::V1);

    Controller::SetV1RelativeValue(LogicElement::MinValue);

    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());
    CHECK_EQUAL(LogicElement::MaxValue, Controller::GetV1RelativeValue());
}

TEST(LogicDirectOutputTestsGroup, DoAction_change_state_to_passive) {

    Controller::SetV1RelativeValue(LogicElement::MaxValue);

    TestableDirectOutput testable(MapIO::V1);
    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;

    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
    CHECK_EQUAL(LogicElement::MinValue, Controller::GetV1RelativeValue());
}

TEST(LogicDirectOutputTestsGroup, GetElementType_returns_et_DirectOutput) {
    TestableDirectOutput testable(MapIO::O1);
    CHECK_EQUAL(TvElementType::et_DirectOutput, testable.PublicMorozov_GetElementType());
}

TEST(LogicDirectOutputTestsGroup, Serialize) {
    uint8_t buffer[256] = {};
    TestableDirectOutput testable(MapIO::O1);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(2, writed);

    CHECK_EQUAL(TvElementType::et_DirectOutput, *((TvElementType *)&buffer[0]));
    CHECK_EQUAL(MapIO::O1, *((MapIO *)&buffer[1]));
}

TEST(LogicDirectOutputTestsGroup, Deserialize) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_DirectOutput;

    TestableDirectOutput testable(MapIO::O1);

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(1, readed);
}