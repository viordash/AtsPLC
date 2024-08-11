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

TEST_GROUP(LogicInputNOTestsGroup){ //
                                    TEST_SETUP(){}

                                    TEST_TEARDOWN(){}
};

class TestableInputNO : public InputNO {
  public:
    TestableInputNO(const MapIO io_adr, InputBase *incoming_item) : InputNO(io_adr, incoming_item) {
    }
    virtual ~TestableInputNO() {
    }

    const char *GetLabel() {
        return label;
    }
    InputBase *PublicMorozov_incoming_item() {
        return incoming_item;
    }
};

TEST(LogicInputNOTestsGroup, GetLabel_DI) {
    Controller controller;
    IncomeRail incomeRail0(controller, 0);
    TestableInputNO testable(MapIO::DI, &incomeRail0);
    STRCMP_EQUAL("DI", testable.GetLabel());
}

TEST(LogicInputNOTestsGroup, GetLabel_AI) {
    Controller controller;
    IncomeRail incomeRail0(controller, 0);
    TestableInputNO testable(MapIO::AI, &incomeRail0);
    STRCMP_EQUAL("AI", testable.GetLabel());
}

TEST(LogicInputNOTestsGroup, GetLabel_V1) {
    Controller controller;
    IncomeRail incomeRail0(controller, 0);
    TestableInputNO testable(MapIO::V1, &incomeRail0);
    STRCMP_EQUAL("V1", testable.GetLabel());
}

TEST(LogicInputNOTestsGroup, Passive_is_init_state) {
    Controller controller;
    IncomeRail incomeRail0(controller, 0);
    TestableInputNO testable_0(MapIO::V1, &incomeRail0);
    TestableInputNO testable_1(MapIO::V2, &testable_0);
    CHECK_EQUAL(LogicItemState::lisPassive, testable_0.GetState());
    CHECK_EQUAL(LogicItemState::lisPassive, testable_1.GetState());
}

TEST(LogicInputNOTestsGroup, chain_of_items) {
    Controller controller;
    IncomeRail incomeRail0(controller, 0);
    TestableInputNO testable_0(MapIO::V1, &incomeRail0);
    TestableInputNO testable_1(MapIO::V2, &testable_0);
    TestableInputNO testable_2(MapIO::V3, &testable_1);
    CHECK_EQUAL(&incomeRail0, testable_0.PublicMorozov_incoming_item());
    CHECK_EQUAL(&testable_0, testable_1.PublicMorozov_incoming_item());
    CHECK_EQUAL(&testable_1, testable_2.PublicMorozov_incoming_item());
}