#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/ComparatorBase.h"
#include "main/LogicProgram/Inputs/IncomeRail.h"

TEST_GROUP(LogicComparatorBaseTestsGroup){ //
                                           TEST_SETUP(){}

                                           TEST_TEARDOWN(){}
};

static const Bitmap bitmap_active = { //
    { 16,                             // width
      16 },                           // height
    { 0xFF, 0x00, 0x00, 0x0A, 0x0A, 0x0A, 0x0A, 0x00, 0x0A, 0x0A, 0x0A,
      0x0A, 0x00, 0x00, 0xFF, 0x80, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x01 }
};
static const Bitmap bitmap_passive = { //
    { 16,                              // width
      16 },                            // height
    { 0xDB, 0x00, 0x00, 0x0A, 0x0A, 0x0A, 0x0A, 0x00, 0x0A, 0x0A, 0x0A,
      0x0A, 0x00, 0x00, 0xDB, 0x80, 0xB6, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB6, 0x01 }
};

class TestableComparatorBase : public ComparatorBase {
  public:
    TestableComparatorBase(uint16_t reference, const MapIO io_adr, InputBase *incoming_item)
        : ComparatorBase(reference, io_adr, incoming_item) {
    }
    virtual ~TestableComparatorBase() {
    }

    const Bitmap *GetCurrentBitmap() {
        switch (state) {
            case LogicItemState::lisActive:
                return &bitmap_active;

            default:
                return &bitmap_passive;
        }
    }

    bool DoAction() {
        return true;
    }
    uint16_t GetReference() {
        return reference;
    }
};

TEST(LogicComparatorBaseTestsGroup, Reference_in_limit_0_to_999) {
    Controller controller;
    IncomeRail incomeRail0(controller, 0);
    TestableComparatorBase testable_0(0, MapIO::DI, &incomeRail0);
    CHECK_EQUAL(0, testable_0.GetReference());

    TestableComparatorBase testable_998(998, MapIO::DI, &incomeRail0);
    CHECK_EQUAL(998, testable_998.GetReference());

    TestableComparatorBase testable_999(999, MapIO::DI, &incomeRail0);
    CHECK_EQUAL(999, testable_999.GetReference());

    TestableComparatorBase testable_1000(1000, MapIO::DI, &incomeRail0);
    CHECK_EQUAL(999, testable_1000.GetReference());
}
