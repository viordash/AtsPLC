#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/CommonComparator.h"
#include "main/LogicProgram/Inputs/IncomeRail.h"

static uint8_t frame_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8] = {};
TEST_GROUP(LogicCommonComparatorTestsGroup){
    //
    TEST_SETUP(){ memset(frame_buffer, 0, sizeof(frame_buffer));
}

TEST_TEARDOWN() {
}
}
;

namespace {
    static const Bitmap bitmap = { //
        { 16,                      // width
          16 },                    // height
        { 0xFF, 0x00, 0x00, 0x0A, 0x0A, 0x0A, 0x0A, 0x00, 0x0A, 0x0A, 0x0A,
          0x0A, 0x00, 0x00, 0xFF, 0x80, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x01 }
    };

    class TestableCommonComparator : public CommonComparator {
      public:
        TestableCommonComparator(uint16_t reference, const MapIO io_adr, InputBase *incoming_item)
            : CommonComparator(reference, io_adr, incoming_item) {
        }
        virtual ~TestableCommonComparator() {
        }

        const Bitmap *GetCurrentBitmap() {
            return &bitmap;
        }

        bool CompareFunction() override {
            return true;
        }

        uint16_t GetReference() {
            return reference;
        }
    };
} // namespace

TEST(LogicCommonComparatorTestsGroup, Reference_in_limit_0_to_999) {
    Controller controller;
    IncomeRail incomeRail0(controller, 0);
    TestableCommonComparator testable_0(0, MapIO::DI, &incomeRail0);
    CHECK_EQUAL(0, testable_0.GetReference());

    TestableCommonComparator testable_998(998, MapIO::DI, &incomeRail0);
    CHECK_EQUAL(998, testable_998.GetReference());

    TestableCommonComparator testable_999(999, MapIO::DI, &incomeRail0);
    CHECK_EQUAL(999, testable_999.GetReference());

    TestableCommonComparator testable_1000(1000, MapIO::DI, &incomeRail0);
    CHECK_EQUAL(999, testable_1000.GetReference());
}

TEST(LogicCommonComparatorTestsGroup, Render) {

    Controller controller;
    IncomeRail incomeRail(controller, 0);
    TestableCommonComparator testable(0, MapIO::DI, &incomeRail);

    CHECK_TRUE(testable.Render(frame_buffer));

    bool any_pixel_coloring = false;
    for (size_t i = 0; i < sizeof(frame_buffer); i++) {
        if (frame_buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_TRUE(any_pixel_coloring);
}