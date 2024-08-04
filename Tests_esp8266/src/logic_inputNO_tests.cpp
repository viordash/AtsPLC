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
    TestableInputNO(const MapIO io_adr, InputBase &prior_item) : InputNO(io_adr, prior_item) {
    }
    virtual ~TestableInputNO() {
    }

    const char *GetLabel() {
        return label;
    }
};

TEST(LogicInputNOTestsGroup, GetLabel_DI) {
    IncomeRail incomeRail0(0);
    TestableInputNO testable(MapIO::DI, incomeRail0);
    STRCMP_EQUAL("DI", testable.GetLabel());
}

TEST(LogicInputNOTestsGroup, GetLabel_AI) {
    IncomeRail incomeRail0(0);
    TestableInputNO testable(MapIO::AI, incomeRail0);
    STRCMP_EQUAL("AI", testable.GetLabel());
}

TEST(LogicInputNOTestsGroup, GetLabel_V1) {
    IncomeRail incomeRail0(0);
    TestableInputNO testable(MapIO::V1, incomeRail0);
    STRCMP_EQUAL("V1", testable.GetLabel());
}