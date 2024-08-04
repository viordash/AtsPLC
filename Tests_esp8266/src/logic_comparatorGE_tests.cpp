#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/ComparatorGE.cpp"
#include "main/LogicProgram/Inputs/ComparatorGE.h"
#include "main/LogicProgram/Inputs/IncomeRail.h"

TEST_GROUP(LogicComparatorGETestsGroup){ //
                                         TEST_SETUP(){}

                                         TEST_TEARDOWN(){}
};

class TestableComparatorGE : public ComparatorGE {
  public:
    TestableComparatorGE(uint16_t reference, const MapIO io_adr, InputBase &prior_item)
        : ComparatorGE(reference, io_adr, prior_item) {
    }
    virtual ~TestableComparatorGE() {
    }

    uint16_t GetReference() {
        return reference;
    }
};

TEST(LogicComparatorGETestsGroup, Reference_in_limit_0_to_999) {
    IncomeRail incomeRail0(0);
    TestableComparatorGE testable_0(0, MapIO::DI, incomeRail0);
    CHECK_EQUAL(0, testable_0.GetReference());

    TestableComparatorGE testable_998(998, MapIO::DI, incomeRail0);
    CHECK_EQUAL(998, testable_998.GetReference());

    TestableComparatorGE testable_999(999, MapIO::DI, incomeRail0);
    CHECK_EQUAL(999, testable_999.GetReference());

    TestableComparatorGE testable_1000(1000, MapIO::DI, incomeRail0);
    CHECK_EQUAL(999, testable_1000.GetReference());
}
