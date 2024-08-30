#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/IncomeRail.h"
#include "main/LogicProgram/Inputs/InputNO.cpp"
#include "main/LogicProgram/Inputs/InputNO.h"

static uint8_t frame_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8] = {};

TEST_GROUP(LogicInputNOTestsGroup){ //
                                    TEST_SETUP(){ memset(frame_buffer, 0, sizeof(frame_buffer));
}

TEST_TEARDOWN() {
}
}
;

namespace {
    class TestableInputNO : public InputNO {
      public:
        TestableInputNO(const MapIO io_adr) : InputNO(io_adr) {
        }
        virtual ~TestableInputNO() {
        }

        const char *GetLabel() {
            return label;
        }
        LogicItemState *PublicMorozov_Get_state() {
            return &state;
        }
    };
} // namespace

TEST(LogicInputNOTestsGroup, GetLabel_DI) {

    IncomeRail incomeRail0(0, LogicItemState::lisActive);
    TestableInputNO testable(MapIO::DI);
    STRCMP_EQUAL("DI", testable.GetLabel());
}

TEST(LogicInputNOTestsGroup, GetLabel_AI) {

    IncomeRail incomeRail0(0, LogicItemState::lisActive);
    TestableInputNO testable(MapIO::AI);
    STRCMP_EQUAL("AI", testable.GetLabel());
}

TEST(LogicInputNOTestsGroup, GetLabel_V1) {

    IncomeRail incomeRail0(0, LogicItemState::lisActive);
    TestableInputNO testable(MapIO::V1);
    STRCMP_EQUAL("V1", testable.GetLabel());
}

TEST(LogicInputNOTestsGroup, Passive_is_init_state) {

    IncomeRail incomeRail0(0, LogicItemState::lisActive);
    TestableInputNO testable_0(MapIO::V1);
    TestableInputNO testable_1(MapIO::V2);
    CHECK_EQUAL(LogicItemState::lisPassive, *testable_0.PublicMorozov_Get_state());
    CHECK_EQUAL(LogicItemState::lisPassive, *testable_1.PublicMorozov_Get_state());
}

// TEST(LogicInputNOTestsGroup, chain_of_items) {

//     IncomeRail incomeRail0(0, LogicItemState::lisActive);
//     TestableInputNO testable_0(MapIO::V1);
//     TestableInputNO testable_1(MapIO::V2, &testable_0);
//     TestableInputNO testable_2(MapIO::V3, &testable_1);
//     CHECK_EQUAL(&incomeRail0, testable_0.PublicMorozov_incoming_item());
//     CHECK_EQUAL(&testable_0, testable_1.PublicMorozov_incoming_item());
//     CHECK_EQUAL(&testable_1, testable_2.PublicMorozov_incoming_item());
// }

TEST(LogicInputNOTestsGroup, DoAction_skip_when_incoming_passive) {
    mock("0").expectNoCall("gpio_get_level");

    IncomeRail incomeRail(0, LogicItemState::lisPassive);

    TestableInputNO testable(MapIO::DI);

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
}

TEST(LogicInputNOTestsGroup, DoAction_change_state_to_active) {
    mock("0").expectOneCall("gpio_get_level").andReturnValue(0);

    IncomeRail incomeRail(0, LogicItemState::lisActive);

    TestableInputNO testable(MapIO::DI);

    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());
}

TEST(LogicInputNOTestsGroup, DoAction_change_state_to_passive) {
    mock("0").expectOneCall("gpio_get_level").andReturnValue(1);

    IncomeRail incomeRail(0, LogicItemState::lisActive);

    TestableInputNO testable(MapIO::DI);
    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;

    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
}
