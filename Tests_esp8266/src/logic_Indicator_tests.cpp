#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/Indicator.cpp"
#include "main/LogicProgram/Inputs/Indicator.h"
#include "main/LogicProgram/Inputs/InputNO.h"

static uint8_t frame_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8] = {};

TEST_GROUP(LogicIndicatorTestsGroup){ //
                                      TEST_SETUP(){ memset(frame_buffer, 0, sizeof(frame_buffer));
mock().disable();
}

TEST_TEARDOWN() {
    mock().enable();
}
}
;

namespace {
    class TestableIndicator : public Indicator {
      public:
        TestableIndicator() : Indicator() {
        }
        virtual ~TestableIndicator() {
        }
        LogicItemState *PublicMorozov_Get_state() {
            return &state;
        }
        f_GetValue PublicMorozov_GetValue() {
            return GetValue;
        }
        void PublicMorozov_UpdateScale() {
            UpdateScale();
        }
        void PublicMorozov_PrintOutValue(uint8_t eng_value) {
            PrintOutValue(eng_value);
        }
        float *PublicMorozov_Get_high_scale() {
            return &high_scale;
        }
        float *PublicMorozov_Get_low_scale() {
            return &low_scale;
        }
        uint8_t *PublicMorozov_Get_decimal_point() {
            return &decimal_point;
        }
        char *PublicMorozov_Get_str_value() {
            return str_value;
        }
    };
} // namespace

TEST(LogicIndicatorTestsGroup, Render_when_active) {
    TestableIndicator testable;
    testable.SetIoAdr(MapIO::DI);

    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;

    Point start_point = { INCOME_RAIL_WIDTH, INCOME_RAIL_TOP + INCOME_RAIL_NETWORK_TOP };
    CHECK_TRUE(testable.Render(frame_buffer, LogicItemState::lisActive, &start_point));

    bool any_pixel_coloring = false;
    for (size_t i = 0; i < sizeof(frame_buffer); i++) {
        if (frame_buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_TRUE(any_pixel_coloring);
    CHECK_EQUAL(113, start_point.x);
}

TEST(LogicIndicatorTestsGroup, Render_when_passive) {
    TestableIndicator testable;
    testable.SetIoAdr(MapIO::DI);

    Point start_point = { INCOME_RAIL_WIDTH, INCOME_RAIL_TOP + INCOME_RAIL_NETWORK_TOP };
    CHECK_TRUE(testable.Render(frame_buffer, LogicItemState::lisActive, &start_point));

    bool any_pixel_coloring = false;
    for (size_t i = 0; i < sizeof(frame_buffer); i++) {
        if (frame_buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_TRUE(any_pixel_coloring);
    CHECK_EQUAL(113, start_point.x);
}

TEST(LogicIndicatorTestsGroup, TryToCast) {
    Indicator indicator;
    CHECK_TRUE(Indicator::TryToCast(&indicator) == &indicator);

    InputNC inputNC;
    CHECK_TRUE(Indicator::TryToCast(&inputNC) == NULL);

    InputNO inputNO;
    CHECK_TRUE(Indicator::TryToCast(&inputNO) == NULL);
}

TEST(LogicIndicatorTestsGroup, SelectNext_changing_IoAdr) {
    TestableIndicator testable;
    testable.SetIoAdr(MapIO::DI);
    testable.BeginEditing();
    testable.Change();
    testable.SelectNext();
    CHECK_EQUAL(MapIO::AI, testable.GetIoAdr());
    testable.SelectNext();
    CHECK_EQUAL(MapIO::V1, testable.GetIoAdr());
    testable.SelectNext();
    CHECK_EQUAL(MapIO::V2, testable.GetIoAdr());
    testable.SelectNext();
    CHECK_EQUAL(MapIO::V3, testable.GetIoAdr());
    testable.SelectNext();
    CHECK_EQUAL(MapIO::V4, testable.GetIoAdr());
    testable.SelectNext();
    CHECK_EQUAL(MapIO::O1, testable.GetIoAdr());
    testable.SelectNext();
    CHECK_EQUAL(MapIO::O2, testable.GetIoAdr());
    testable.SelectNext();
    CHECK_EQUAL(MapIO::DI, testable.GetIoAdr());
}

TEST(LogicIndicatorTestsGroup, SelectPrior_changing_IoAdr) {
    TestableIndicator testable;
    testable.SetIoAdr(MapIO::DI);
    testable.BeginEditing();
    testable.Change();
    testable.SelectPrior();
    CHECK_EQUAL(MapIO::O2, testable.GetIoAdr());
    testable.SelectPrior();
    CHECK_EQUAL(MapIO::O1, testable.GetIoAdr());
    testable.SelectPrior();
    CHECK_EQUAL(MapIO::V4, testable.GetIoAdr());
    testable.SelectPrior();
    CHECK_EQUAL(MapIO::V3, testable.GetIoAdr());
    testable.SelectPrior();
    CHECK_EQUAL(MapIO::V2, testable.GetIoAdr());
    testable.SelectPrior();
    CHECK_EQUAL(MapIO::V1, testable.GetIoAdr());
    testable.SelectPrior();
    CHECK_EQUAL(MapIO::AI, testable.GetIoAdr());
    testable.SelectPrior();
    CHECK_EQUAL(MapIO::DI, testable.GetIoAdr());
}

TEST(LogicIndicatorTestsGroup, Change_calls_end_editing) {
    TestableIndicator testable;
    testable.SetIoAdr(MapIO::O1);
    testable.BeginEditing();
    CHECK_TRUE(testable.Editing());
    testable.Change();
    CHECK_TRUE(testable.Editing());
    testable.Change();
    CHECK_TRUE(testable.Editing());
    testable.Change();
    CHECK_TRUE(testable.Editing());
    testable.Change();
    CHECK_TRUE(testable.Editing());
    testable.Change();
    CHECK_TRUE(testable.Editing());
    testable.Change();
    CHECK_TRUE(testable.Editing());
    testable.Change();
    CHECK_TRUE(testable.Editing());
    testable.Change();
    CHECK_TRUE(testable.Editing());
    testable.Change();
    CHECK_TRUE(testable.Editing());
    testable.Change();
    CHECK_FALSE(testable.Editing());
}

TEST(LogicIndicatorTestsGroup, Serialize) {
    uint8_t buffer[256] = {};
    TestableIndicator testable;
    testable.SetIoAdr(MapIO::V2);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(2, writed);

    CHECK_EQUAL(TvElementType::et_Indicator, *((TvElementType *)&buffer[0]));
    CHECK_EQUAL(MapIO::V2, *((MapIO *)&buffer[1]));
}

TEST(LogicIndicatorTestsGroup, Serialize_just_for_obtain_size) {
    TestableIndicator testable;
    testable.SetIoAdr(MapIO::DI);

    size_t writed = testable.Serialize(NULL, SIZE_MAX);
    CHECK_EQUAL(2, writed);

    writed = testable.Serialize(NULL, 0);
    CHECK_EQUAL(2, writed);
}

TEST(LogicIndicatorTestsGroup, Serialize_to_small_buffer_return_zero) {
    uint8_t buffer[1] = {};
    TestableIndicator testable;
    testable.SetIoAdr(MapIO::DI);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, writed);
}

TEST(LogicIndicatorTestsGroup, Deserialize) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_Indicator;
    *((MapIO *)&buffer[1]) = MapIO::V3;

    TestableIndicator testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(1, readed);

    CHECK_EQUAL(MapIO::V3, testable.GetIoAdr());
    CHECK(Controller::GetV3RelativeValue == testable.PublicMorozov_GetValue());
}

TEST(LogicIndicatorTestsGroup, Deserialize_with_small_buffer_return_zero) {
    uint8_t buffer[0] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_Indicator;

    TestableIndicator testable;

    size_t readed = testable.Deserialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, readed);
}

TEST(LogicIndicatorTestsGroup, Deserialize_with_wrong_io_adr_return_zero) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_Indicator;

    TestableIndicator testable;

    *((MapIO *)&buffer[1]) = (MapIO)(MapIO::DI - 1);
    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);

    *((MapIO *)&buffer[1]) = (MapIO)(MapIO::V4 + 1);
    readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);

    *((MapIO *)&buffer[1]) = MapIO::DI;
    readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(1, readed);
}

TEST(LogicIndicatorTestsGroup, GetElementType) {
    TestableIndicator testable;
    CHECK_EQUAL(TvElementType::et_Indicator, testable.GetElementType());
}

TEST(LogicIndicatorTestsGroup, PrintOutValue_positive_values) {
    TestableIndicator testable;

    *testable.PublicMorozov_Get_high_scale() = 2.55;
    *testable.PublicMorozov_Get_low_scale() = 0;
    *testable.PublicMorozov_Get_decimal_point() = 2;
    testable.PublicMorozov_UpdateScale();
    testable.PublicMorozov_PrintOutValue(255);
    STRCMP_EQUAL("    2.55", testable.PublicMorozov_Get_str_value());
    testable.PublicMorozov_PrintOutValue(127);
    STRCMP_EQUAL("    1.27", testable.PublicMorozov_Get_str_value());
    testable.PublicMorozov_PrintOutValue(1);
    STRCMP_EQUAL("    0.01", testable.PublicMorozov_Get_str_value());
    testable.PublicMorozov_PrintOutValue(0);
    STRCMP_EQUAL("    0.00", testable.PublicMorozov_Get_str_value());

    *testable.PublicMorozov_Get_high_scale() = 255;
    *testable.PublicMorozov_Get_low_scale() = 0;
    *testable.PublicMorozov_Get_decimal_point() = 0;
    testable.PublicMorozov_UpdateScale();
    testable.PublicMorozov_PrintOutValue(255);
    STRCMP_EQUAL("     255", testable.PublicMorozov_Get_str_value());
    testable.PublicMorozov_PrintOutValue(127);
    STRCMP_EQUAL("     127", testable.PublicMorozov_Get_str_value());
    testable.PublicMorozov_PrintOutValue(64);
    STRCMP_EQUAL("      64", testable.PublicMorozov_Get_str_value());
    testable.PublicMorozov_PrintOutValue(1);
    STRCMP_EQUAL("       1", testable.PublicMorozov_Get_str_value());
    testable.PublicMorozov_PrintOutValue(0);
    STRCMP_EQUAL("       0", testable.PublicMorozov_Get_str_value());

    *testable.PublicMorozov_Get_high_scale() = 12345678;
    *testable.PublicMorozov_Get_low_scale() = 0;
    *testable.PublicMorozov_Get_decimal_point() = 0;
    testable.PublicMorozov_UpdateScale();
    testable.PublicMorozov_PrintOutValue(255);
    STRCMP_EQUAL("12345678", testable.PublicMorozov_Get_str_value());
    testable.PublicMorozov_PrintOutValue(127);
    STRCMP_EQUAL(" 6148632", testable.PublicMorozov_Get_str_value());
    testable.PublicMorozov_PrintOutValue(1);
    STRCMP_EQUAL("   48414", testable.PublicMorozov_Get_str_value());
    testable.PublicMorozov_PrintOutValue(0);
    STRCMP_EQUAL("       0", testable.PublicMorozov_Get_str_value());

    *testable.PublicMorozov_Get_high_scale() = 1;
    *testable.PublicMorozov_Get_low_scale() = 0;
    *testable.PublicMorozov_Get_decimal_point() = 4;
    testable.PublicMorozov_UpdateScale();
    testable.PublicMorozov_PrintOutValue(255);
    STRCMP_EQUAL("  1.0000", testable.PublicMorozov_Get_str_value());
    testable.PublicMorozov_PrintOutValue(127);
    STRCMP_EQUAL("  0.4980", testable.PublicMorozov_Get_str_value());
    testable.PublicMorozov_PrintOutValue(1);
    STRCMP_EQUAL("  0.0039", testable.PublicMorozov_Get_str_value());
    testable.PublicMorozov_PrintOutValue(0);
    STRCMP_EQUAL("  0.0000", testable.PublicMorozov_Get_str_value());
}

TEST(LogicIndicatorTestsGroup, PrintOutValue_negative_values) {
    TestableIndicator testable;

    *testable.PublicMorozov_Get_high_scale() = 10;
    *testable.PublicMorozov_Get_low_scale() = -10;
    *testable.PublicMorozov_Get_decimal_point() = 1;
    testable.PublicMorozov_UpdateScale();
    testable.PublicMorozov_PrintOutValue(255);
    STRCMP_EQUAL("    10.0", testable.PublicMorozov_Get_str_value());
    testable.PublicMorozov_PrintOutValue(127);
    STRCMP_EQUAL("    -0.0", testable.PublicMorozov_Get_str_value());
    testable.PublicMorozov_PrintOutValue(1);
    STRCMP_EQUAL("    -9.9", testable.PublicMorozov_Get_str_value());
    testable.PublicMorozov_PrintOutValue(0);
    STRCMP_EQUAL("   -10.0", testable.PublicMorozov_Get_str_value());

    *testable.PublicMorozov_Get_high_scale() = -255;
    *testable.PublicMorozov_Get_low_scale() = 0;
    *testable.PublicMorozov_Get_decimal_point() = 0;
    testable.PublicMorozov_UpdateScale();
    testable.PublicMorozov_PrintOutValue(255);
    STRCMP_EQUAL("    -255", testable.PublicMorozov_Get_str_value());
    testable.PublicMorozov_PrintOutValue(127);
    STRCMP_EQUAL("    -127", testable.PublicMorozov_Get_str_value());
    testable.PublicMorozov_PrintOutValue(64);
    STRCMP_EQUAL("     -64", testable.PublicMorozov_Get_str_value());
    testable.PublicMorozov_PrintOutValue(1);
    STRCMP_EQUAL("      -1", testable.PublicMorozov_Get_str_value());
    testable.PublicMorozov_PrintOutValue(0);
    STRCMP_EQUAL("       0", testable.PublicMorozov_Get_str_value());

    *testable.PublicMorozov_Get_high_scale() = 0;
    *testable.PublicMorozov_Get_low_scale() = -9999999;
    *testable.PublicMorozov_Get_decimal_point() = 0;
    testable.PublicMorozov_UpdateScale();
    testable.PublicMorozov_PrintOutValue(255);
    STRCMP_EQUAL("       0", testable.PublicMorozov_Get_str_value());
    testable.PublicMorozov_PrintOutValue(127);
    STRCMP_EQUAL("-5019607", testable.PublicMorozov_Get_str_value());
    testable.PublicMorozov_PrintOutValue(1);
    STRCMP_EQUAL("-9960783", testable.PublicMorozov_Get_str_value());
    testable.PublicMorozov_PrintOutValue(0);
    STRCMP_EQUAL("-9999999", testable.PublicMorozov_Get_str_value());
}

TEST(LogicIndicatorTestsGroup, PrintOutValue_overflow_values_will_restricted_by_buffer_size) {
    TestableIndicator testable;

    *testable.PublicMorozov_Get_high_scale() = 10000000.0f;
    *testable.PublicMorozov_Get_low_scale() = 0;
    *testable.PublicMorozov_Get_decimal_point() = 2;
    testable.PublicMorozov_UpdateScale();
    testable.PublicMorozov_PrintOutValue(255);
    STRCMP_EQUAL("10000000", testable.PublicMorozov_Get_str_value());
    testable.PublicMorozov_PrintOutValue(127);
    STRCMP_EQUAL("4980392.", testable.PublicMorozov_Get_str_value());
    testable.PublicMorozov_PrintOutValue(1);
    STRCMP_EQUAL("39215.69", testable.PublicMorozov_Get_str_value());
    testable.PublicMorozov_PrintOutValue(0);
    STRCMP_EQUAL("    0.00", testable.PublicMorozov_Get_str_value());

}