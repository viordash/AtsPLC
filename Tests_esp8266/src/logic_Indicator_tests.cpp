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
        int *PublicMorozov_Get_editing_property_id() {
            return &editing_property_id;
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

    *testable.PublicMorozov_Get_high_scale() = 1000;
    *testable.PublicMorozov_Get_low_scale() = 500;
    *testable.PublicMorozov_Get_decimal_point() = 1;
    testable.PublicMorozov_UpdateScale();
    testable.PublicMorozov_PrintOutValue(255);
    STRCMP_EQUAL("  1000.0", testable.PublicMorozov_Get_str_value());
    testable.PublicMorozov_PrintOutValue(127);
    STRCMP_EQUAL("   749.0", testable.PublicMorozov_Get_str_value());
    testable.PublicMorozov_PrintOutValue(64);
    STRCMP_EQUAL("   625.5", testable.PublicMorozov_Get_str_value());
    testable.PublicMorozov_PrintOutValue(1);
    STRCMP_EQUAL("   502.0", testable.PublicMorozov_Get_str_value());
    testable.PublicMorozov_PrintOutValue(0);
    STRCMP_EQUAL("   500.0", testable.PublicMorozov_Get_str_value());
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

    *testable.PublicMorozov_Get_high_scale() = -500;
    *testable.PublicMorozov_Get_low_scale() = -1000;
    *testable.PublicMorozov_Get_decimal_point() = 1;
    testable.PublicMorozov_UpdateScale();
    testable.PublicMorozov_PrintOutValue(255);
    STRCMP_EQUAL("  -500.0", testable.PublicMorozov_Get_str_value());
    testable.PublicMorozov_PrintOutValue(127);
    STRCMP_EQUAL("  -751.0", testable.PublicMorozov_Get_str_value());
    testable.PublicMorozov_PrintOutValue(64);
    STRCMP_EQUAL("  -874.5", testable.PublicMorozov_Get_str_value());
    testable.PublicMorozov_PrintOutValue(1);
    STRCMP_EQUAL("  -998.0", testable.PublicMorozov_Get_str_value());
    testable.PublicMorozov_PrintOutValue(0);
    STRCMP_EQUAL(" -1000.0", testable.PublicMorozov_Get_str_value());
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

TEST(LogicIndicatorTestsGroup, Change__switching__editing_property_id) {
    TestableIndicator testable;
    testable.SetIoAdr(MapIO::DI);
    CHECK_FALSE(testable.Editing());
    CHECK_EQUAL(Indicator::EditingPropertyId::ciepi_None,
                *testable.PublicMorozov_Get_editing_property_id());

    testable.BeginEditing();
    testable.Change();
    CHECK_TRUE(testable.Editing());
    CHECK_EQUAL(Indicator::EditingPropertyId::ciepi_ConfigureIOAdr,
                *testable.PublicMorozov_Get_editing_property_id());
    testable.Change();
    CHECK_TRUE(testable.Editing());
    CHECK_EQUAL(Indicator::EditingPropertyId::ciepi_ConfigureLowScale_0,
                *testable.PublicMorozov_Get_editing_property_id());
    testable.Change();
    CHECK_TRUE(testable.Editing());
    CHECK_EQUAL(Indicator::EditingPropertyId::ciepi_ConfigureLowScale_1,
                *testable.PublicMorozov_Get_editing_property_id());
    testable.Change();
    CHECK_TRUE(testable.Editing());
    CHECK_EQUAL(Indicator::EditingPropertyId::ciepi_ConfigureLowScale_2,
                *testable.PublicMorozov_Get_editing_property_id());
    testable.Change();
    CHECK_TRUE(testable.Editing());
    CHECK_EQUAL(Indicator::EditingPropertyId::ciepi_ConfigureLowScale_3,
                *testable.PublicMorozov_Get_editing_property_id());
    testable.Change();
    CHECK_TRUE(testable.Editing());
    CHECK_EQUAL(Indicator::EditingPropertyId::ciepi_ConfigureLowScale_4,
                *testable.PublicMorozov_Get_editing_property_id());
    testable.Change();
    CHECK_TRUE(testable.Editing());
    CHECK_EQUAL(Indicator::EditingPropertyId::ciepi_ConfigureLowScale_5,
                *testable.PublicMorozov_Get_editing_property_id());
    testable.Change();
    CHECK_TRUE(testable.Editing());
    CHECK_EQUAL(Indicator::EditingPropertyId::ciepi_ConfigureLowScale_6,
                *testable.PublicMorozov_Get_editing_property_id());
    testable.Change();
    CHECK_TRUE(testable.Editing());
    CHECK_EQUAL(Indicator::EditingPropertyId::ciepi_ConfigureLowScale_7,
                *testable.PublicMorozov_Get_editing_property_id());
    testable.Change();
    CHECK_TRUE(testable.Editing());
    CHECK_EQUAL(Indicator::EditingPropertyId::ciepi_ConfigureHighScale_0,
                *testable.PublicMorozov_Get_editing_property_id());
    testable.Change();
    CHECK_TRUE(testable.Editing());
    CHECK_EQUAL(Indicator::EditingPropertyId::ciepi_ConfigureHighScale_1,
                *testable.PublicMorozov_Get_editing_property_id());
    testable.Change();
    CHECK_TRUE(testable.Editing());
    CHECK_EQUAL(Indicator::EditingPropertyId::ciepi_ConfigureHighScale_2,
                *testable.PublicMorozov_Get_editing_property_id());
    testable.Change();
    CHECK_TRUE(testable.Editing());
    CHECK_EQUAL(Indicator::EditingPropertyId::ciepi_ConfigureHighScale_3,
                *testable.PublicMorozov_Get_editing_property_id());
    testable.Change();
    CHECK_TRUE(testable.Editing());
    CHECK_EQUAL(Indicator::EditingPropertyId::ciepi_ConfigureHighScale_4,
                *testable.PublicMorozov_Get_editing_property_id());
    testable.Change();
    CHECK_TRUE(testable.Editing());
    CHECK_EQUAL(Indicator::EditingPropertyId::ciepi_ConfigureHighScale_5,
                *testable.PublicMorozov_Get_editing_property_id());
    testable.Change();
    CHECK_TRUE(testable.Editing());
    CHECK_EQUAL(Indicator::EditingPropertyId::ciepi_ConfigureHighScale_6,
                *testable.PublicMorozov_Get_editing_property_id());
    testable.Change();
    CHECK_TRUE(testable.Editing());
    CHECK_EQUAL(Indicator::EditingPropertyId::ciepi_ConfigureHighScale_7,
                *testable.PublicMorozov_Get_editing_property_id());
    testable.Change();
    CHECK_FALSE(testable.Editing());
    CHECK_EQUAL(Indicator::EditingPropertyId::ciepi_None,
                *testable.PublicMorozov_Get_editing_property_id());
}

TEST(LogicIndicatorTestsGroup, Show_low_scale_when_editing) {
    TestableIndicator testable;
    testable.SetIoAdr(MapIO::DI);
    *testable.PublicMorozov_Get_high_scale() = 0;
    *testable.PublicMorozov_Get_low_scale() = 0;
    *testable.PublicMorozov_Get_decimal_point() = 0;
    testable.BeginEditing();
    testable.Change();
    testable.Change();
    STRCMP_EQUAL("00000000", testable.PublicMorozov_Get_str_value());

    *testable.PublicMorozov_Get_decimal_point() = 1;
    *testable.PublicMorozov_Get_editing_property_id() =
        Indicator::EditingPropertyId::ciepi_ConfigureIOAdr;
    testable.Change();
    STRCMP_EQUAL("000000.0", testable.PublicMorozov_Get_str_value());

    *testable.PublicMorozov_Get_decimal_point() = 2;
    *testable.PublicMorozov_Get_editing_property_id() =
        Indicator::EditingPropertyId::ciepi_ConfigureIOAdr;
    testable.Change();
    STRCMP_EQUAL("00000.00", testable.PublicMorozov_Get_str_value());

    *testable.PublicMorozov_Get_decimal_point() = 3;
    *testable.PublicMorozov_Get_editing_property_id() =
        Indicator::EditingPropertyId::ciepi_ConfigureIOAdr;
    testable.Change();
    STRCMP_EQUAL("0000.000", testable.PublicMorozov_Get_str_value());

    *testable.PublicMorozov_Get_decimal_point() = 4;
    *testable.PublicMorozov_Get_editing_property_id() =
        Indicator::EditingPropertyId::ciepi_ConfigureIOAdr;
    testable.Change();
    STRCMP_EQUAL("000.0000", testable.PublicMorozov_Get_str_value());

    *testable.PublicMorozov_Get_decimal_point() = 5;
    *testable.PublicMorozov_Get_editing_property_id() =
        Indicator::EditingPropertyId::ciepi_ConfigureIOAdr;
    testable.Change();
    STRCMP_EQUAL("00.00000", testable.PublicMorozov_Get_str_value());

    *testable.PublicMorozov_Get_decimal_point() = 6;
    *testable.PublicMorozov_Get_editing_property_id() =
        Indicator::EditingPropertyId::ciepi_ConfigureIOAdr;
    testable.Change();
    STRCMP_EQUAL("0.000000", testable.PublicMorozov_Get_str_value());

    *testable.PublicMorozov_Get_decimal_point() = 2;
    *testable.PublicMorozov_Get_low_scale() = 12345.678f;
    *testable.PublicMorozov_Get_editing_property_id() =
        Indicator::EditingPropertyId::ciepi_ConfigureIOAdr;
    testable.Change();
    STRCMP_EQUAL("12345.68", testable.PublicMorozov_Get_str_value());

    *testable.PublicMorozov_Get_decimal_point() = 4;
    *testable.PublicMorozov_Get_low_scale() = 0.1575f;
    *testable.PublicMorozov_Get_editing_property_id() =
        Indicator::EditingPropertyId::ciepi_ConfigureIOAdr;
    testable.Change();
    STRCMP_EQUAL("000.1575", testable.PublicMorozov_Get_str_value());
}

TEST(LogicIndicatorTestsGroup, Show_high_scale_when_editing) {
    TestableIndicator testable;
    testable.SetIoAdr(MapIO::DI);
    *testable.PublicMorozov_Get_high_scale() = 100;
    *testable.PublicMorozov_Get_low_scale() = 0;
    *testable.PublicMorozov_Get_decimal_point() = 0;
    testable.BeginEditing();
    *testable.PublicMorozov_Get_editing_property_id() =
        Indicator::EditingPropertyId::ciepi_ConfigureLowScale_7;
    testable.Change();
    STRCMP_EQUAL("00000100", testable.PublicMorozov_Get_str_value());

    *testable.PublicMorozov_Get_decimal_point() = 1;
    *testable.PublicMorozov_Get_editing_property_id() =
        Indicator::EditingPropertyId::ciepi_ConfigureLowScale_7;
    testable.Change();
    STRCMP_EQUAL("000100.0", testable.PublicMorozov_Get_str_value());

    *testable.PublicMorozov_Get_decimal_point() = 2;
    *testable.PublicMorozov_Get_editing_property_id() =
        Indicator::EditingPropertyId::ciepi_ConfigureLowScale_7;
    testable.Change();
    STRCMP_EQUAL("00100.00", testable.PublicMorozov_Get_str_value());

    *testable.PublicMorozov_Get_decimal_point() = 3;
    *testable.PublicMorozov_Get_editing_property_id() =
        Indicator::EditingPropertyId::ciepi_ConfigureLowScale_7;
    testable.Change();
    STRCMP_EQUAL("0100.000", testable.PublicMorozov_Get_str_value());

    *testable.PublicMorozov_Get_decimal_point() = 4;
    *testable.PublicMorozov_Get_editing_property_id() =
        Indicator::EditingPropertyId::ciepi_ConfigureLowScale_7;
    testable.Change();
    STRCMP_EQUAL("100.0000", testable.PublicMorozov_Get_str_value());

    *testable.PublicMorozov_Get_high_scale() = 1;
    *testable.PublicMorozov_Get_decimal_point() = 5;
    *testable.PublicMorozov_Get_editing_property_id() =
        Indicator::EditingPropertyId::ciepi_ConfigureLowScale_7;
    testable.Change();
    STRCMP_EQUAL("01.00000", testable.PublicMorozov_Get_str_value());

    *testable.PublicMorozov_Get_decimal_point() = 6;
    *testable.PublicMorozov_Get_editing_property_id() =
        Indicator::EditingPropertyId::ciepi_ConfigureLowScale_7;
    testable.Change();
    STRCMP_EQUAL("1.000000", testable.PublicMorozov_Get_str_value());

    *testable.PublicMorozov_Get_decimal_point() = 2;
    *testable.PublicMorozov_Get_high_scale() = 12345.678f;
    *testable.PublicMorozov_Get_editing_property_id() =
        Indicator::EditingPropertyId::ciepi_ConfigureLowScale_7;
    testable.Change();
    STRCMP_EQUAL("12345.68", testable.PublicMorozov_Get_str_value());

    *testable.PublicMorozov_Get_decimal_point() = 4;
    *testable.PublicMorozov_Get_high_scale() = 0.1575f;
    *testable.PublicMorozov_Get_editing_property_id() =
        Indicator::EditingPropertyId::ciepi_ConfigureLowScale_7;
    testable.Change();
    STRCMP_EQUAL("000.1575", testable.PublicMorozov_Get_str_value());
}

TEST(LogicIndicatorTestsGroup, Editing_scale_symbol_0) {
    TestableIndicator testable;
    testable.SetIoAdr(MapIO::DI);
    *testable.PublicMorozov_Get_high_scale() = 0;
    *testable.PublicMorozov_Get_low_scale() = 0;
    *testable.PublicMorozov_Get_decimal_point() = 0;
    testable.BeginEditing();
    testable.Change();
    testable.Change();
    CHECK_EQUAL(Indicator::EditingPropertyId::ciepi_ConfigureLowScale_0,
                *testable.PublicMorozov_Get_editing_property_id());
    STRCMP_EQUAL("00000000", testable.PublicMorozov_Get_str_value());

    testable.SelectPrior();
    STRCMP_EQUAL("90000000", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("80000000", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("70000000", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("60000000", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("50000000", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("40000000", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("30000000", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("20000000", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("10000000", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("00000000", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("90000000", testable.PublicMorozov_Get_str_value());

    testable.SelectNext();
    STRCMP_EQUAL("00000000", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("10000000", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("20000000", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("30000000", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("40000000", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("50000000", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("60000000", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("70000000", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("80000000", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("90000000", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("00000000", testable.PublicMorozov_Get_str_value());
}

TEST(LogicIndicatorTestsGroup, Editing_scale_symbol_1) {
    TestableIndicator testable;
    testable.SetIoAdr(MapIO::DI);
    *testable.PublicMorozov_Get_high_scale() = 0;
    *testable.PublicMorozov_Get_low_scale() = 0;
    *testable.PublicMorozov_Get_decimal_point() = 0;
    testable.BeginEditing();
    testable.Change();
    testable.Change();
    testable.Change();
    CHECK_EQUAL(Indicator::EditingPropertyId::ciepi_ConfigureLowScale_1,
                *testable.PublicMorozov_Get_editing_property_id());
    STRCMP_EQUAL("00000000", testable.PublicMorozov_Get_str_value());

    testable.SelectPrior();
    STRCMP_EQUAL("0.000000", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("09000000", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("08000000", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("07000000", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("06000000", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("05000000", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("04000000", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("03000000", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("02000000", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("01000000", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("00000000", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("0.000000", testable.PublicMorozov_Get_str_value());

    testable.SelectNext();
    STRCMP_EQUAL("00000000", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("01000000", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("02000000", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("03000000", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("04000000", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("05000000", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("06000000", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("07000000", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("08000000", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("09000000", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("0.000000", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("00000000", testable.PublicMorozov_Get_str_value());
}

TEST(LogicIndicatorTestsGroup, Editing_scale_symbol_4) {
    TestableIndicator testable;
    testable.SetIoAdr(MapIO::DI);
    *testable.PublicMorozov_Get_high_scale() = 0;
    *testable.PublicMorozov_Get_low_scale() = 0;
    *testable.PublicMorozov_Get_decimal_point() = 0;
    testable.BeginEditing();
    testable.Change();
    testable.Change();
    testable.Change();
    testable.Change();
    testable.Change();
    testable.Change();
    CHECK_EQUAL(Indicator::EditingPropertyId::ciepi_ConfigureLowScale_4,
                *testable.PublicMorozov_Get_editing_property_id());
    STRCMP_EQUAL("00000000", testable.PublicMorozov_Get_str_value());

    testable.SelectPrior();
    STRCMP_EQUAL("0000.000", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("00009000", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("00008000", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("00007000", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("00006000", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("00005000", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("00004000", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("00003000", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("00002000", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("00001000", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("00000000", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("0000.000", testable.PublicMorozov_Get_str_value());

    testable.SelectNext();
    STRCMP_EQUAL("00000000", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("00001000", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("00002000", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("00003000", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("00004000", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("00005000", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("00006000", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("00007000", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("00008000", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("00009000", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("0000.000", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("00000000", testable.PublicMorozov_Get_str_value());
}

TEST(LogicIndicatorTestsGroup, Editing_scale_symbol_7) {
    TestableIndicator testable;
    testable.SetIoAdr(MapIO::DI);
    *testable.PublicMorozov_Get_high_scale() = 0;
    *testable.PublicMorozov_Get_low_scale() = 0;
    *testable.PublicMorozov_Get_decimal_point() = 0;
    testable.BeginEditing();
    testable.Change();
    testable.Change();
    testable.Change();
    testable.Change();
    testable.Change();
    testable.Change();
    testable.Change();
    testable.Change();
    testable.Change();
    CHECK_EQUAL(Indicator::EditingPropertyId::ciepi_ConfigureLowScale_7,
                *testable.PublicMorozov_Get_editing_property_id());
    STRCMP_EQUAL("00000000", testable.PublicMorozov_Get_str_value());

    testable.SelectPrior();
    STRCMP_EQUAL("00000009", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("00000008", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("00000007", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("00000006", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("00000005", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("00000004", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("00000003", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("00000002", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("00000001", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("00000000", testable.PublicMorozov_Get_str_value());
    testable.SelectPrior();
    STRCMP_EQUAL("00000009", testable.PublicMorozov_Get_str_value());

    testable.SelectNext();
    STRCMP_EQUAL("00000000", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("00000001", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("00000002", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("00000003", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("00000004", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("00000005", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("00000006", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("00000007", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("00000008", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("00000009", testable.PublicMorozov_Get_str_value());
    testable.SelectNext();
    STRCMP_EQUAL("00000000", testable.PublicMorozov_Get_str_value());
}