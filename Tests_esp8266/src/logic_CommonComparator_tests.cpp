#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/CommonComparator.h"

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
        TestableCommonComparator(uint8_t ref_percent04, const MapIO io_adr)
            : CommonComparator(ref_percent04, io_adr) {
        }
        virtual ~TestableCommonComparator() {
        }

        const Bitmap *GetCurrentBitmap() {
            return &bitmap;
        }

        bool CompareFunction() override {
            return true;
        }

        uint8_t GetReference() {
            return ref_percent04;
        }
    };
} // namespace

TEST(LogicCommonComparatorTestsGroup, Reference_in_limit_0_to_250) {
    TestableCommonComparator testable_0(0, MapIO::DI);
    CHECK_EQUAL(0, testable_0.GetReference());

    TestableCommonComparator testable_100(100, MapIO::DI);
    CHECK_EQUAL(100, testable_100.GetReference());

    TestableCommonComparator testable_250(250, MapIO::DI);
    CHECK_EQUAL(250, testable_250.GetReference());

    TestableCommonComparator testable_251(251, MapIO::DI);
    CHECK_EQUAL(250, testable_251.GetReference());
}

TEST(LogicCommonComparatorTestsGroup, Render) {
    TestableCommonComparator testable(0, MapIO::DI);

    Point start_point = { 0, INCOME_RAIL_TOP };
    CHECK_TRUE(testable.Render(frame_buffer, LogicItemState::lisActive, &start_point));

    bool any_pixel_coloring = false;
    for (size_t i = 0; i < sizeof(frame_buffer); i++) {
        if (frame_buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_TRUE(any_pixel_coloring);
    CHECK_EQUAL(30, start_point.x);
}