#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/SquareWaveGenerator.h"
#include "main/LogicProgram/Inputs/TimerSecs.h"

static FrameBuffer frame_buffer = {};

TEST_GROUP(LogicSquareWaveGeneratorTestsGroup){
    //
    TEST_SETUP(){ memset(&frame_buffer.buffer, 0, sizeof(frame_buffer.buffer));
}

TEST_TEARDOWN() {
}
}
;

namespace {
    class TestableSquareWaveGenerator : public SquareWaveGenerator {
      public:
        const char *PublicMorozov_Get_str_period0() {
            return str_period0;
        }
        const char *PublicMorozov_Get_str_period1() {
            return str_period1;
        }
    };
} // namespace

TEST(LogicSquareWaveGeneratorTestsGroup, Render) {
    SquareWaveGenerator testable(100, 200);

    Point start_point = { INCOME_RAIL_WIDTH, INCOME_RAIL_TOP + INCOME_RAIL_NETWORK_TOP };
    CHECK_TRUE(testable.Render(&frame_buffer, LogicItemState::lisActive, &start_point));

    bool any_pixel_coloring = false;
    for (size_t i = 0; i < sizeof(frame_buffer.buffer); i++){
        if (frame_buffer.buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_TRUE(any_pixel_coloring);
    CHECK_EQUAL(34, start_point.x);
}

TEST(LogicSquareWaveGeneratorTestsGroup, SetPeriod0_in_limit_10_to_99990) {
    SquareWaveGenerator testable;
    testable.SetPeriod0(0);
    CHECK_EQUAL(10 * 1000LL, testable.GetPeriod0Us());

    testable.SetPeriod0(99989);
    CHECK_EQUAL(99989 * 1000LL, testable.GetPeriod0Us());

    testable.SetPeriod0(99990);
    CHECK_EQUAL(99990 * 1000LL, testable.GetPeriod0Us());

    testable.SetPeriod0(99991);
    CHECK_EQUAL(99990 * 1000LL, testable.GetPeriod0Us());
}

TEST(LogicSquareWaveGeneratorTestsGroup, SetPeriod1_in_limit_10_to_99990) {
    SquareWaveGenerator testable;
    testable.SetPeriod1(0);
    CHECK_EQUAL(10 * 1000LL, testable.GetPeriod1Us());

    testable.SetPeriod1(99989);
    CHECK_EQUAL(99989 * 1000LL, testable.GetPeriod1Us());

    testable.SetPeriod1(99990);
    CHECK_EQUAL(99990 * 1000LL, testable.GetPeriod1Us());

    testable.SetPeriod1(99991);
    CHECK_EQUAL(99990 * 1000LL, testable.GetPeriod1Us());
}

TEST(LogicSquareWaveGeneratorTestsGroup, Serialize) {
    uint8_t buffer[256] = {};
    SquareWaveGenerator testable;
    testable.SetPeriod0(12345);
    testable.SetPeriod1(67890);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(17, writed);

    CHECK_EQUAL(TvElementType::et_SquareWaveGenerator, *((TvElementType *)&buffer[0]));
    CHECK_EQUAL(12345000, *((uint64_t *)&buffer[1]));
    CHECK_EQUAL(67890000, *((uint64_t *)&buffer[9]));
}

TEST(LogicSquareWaveGeneratorTestsGroup, Serialize_just_for_obtain_size) {
    SquareWaveGenerator testable;
    testable.SetPeriod0(12345);
    testable.SetPeriod1(67890);

    size_t writed = testable.Serialize(NULL, SIZE_MAX);
    CHECK_EQUAL(17, writed);

    writed = testable.Serialize(NULL, 0);
    CHECK_EQUAL(17, writed);
}

TEST(LogicSquareWaveGeneratorTestsGroup, Serialize_to_small_buffer_return_zero) {
    uint8_t buffer[1] = {};
    SquareWaveGenerator testable;
    testable.SetPeriod0(12345);
    testable.SetPeriod1(67890);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, writed);
}

TEST(LogicSquareWaveGeneratorTestsGroup, Deserialize) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_SquareWaveGenerator;
    *((uint64_t *)&buffer[1]) = 123456;
    *((uint64_t *)&buffer[9]) = 678901;

    TestableSquareWaveGenerator testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(16, readed);

    CHECK_EQUAL(123000, testable.GetPeriod0Us());
    STRCMP_EQUAL("123", testable.PublicMorozov_Get_str_period0());
    CHECK_EQUAL(678000, testable.GetPeriod1Us());
    STRCMP_EQUAL("678", testable.PublicMorozov_Get_str_period1());
}

TEST(LogicSquareWaveGeneratorTestsGroup, Deserialize_with_small_buffer_return_zero) {
    uint8_t buffer[1] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_SquareWaveGenerator;

    SquareWaveGenerator testable;

    size_t readed = testable.Deserialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, readed);
}

TEST(LogicSquareWaveGeneratorTestsGroup, Deserialize_with_less_period0_value_return_zero) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_SquareWaveGenerator;
    *((uint64_t *)&buffer[1]) = 123000;
    *((uint64_t *)&buffer[9]) = 0;

    SquareWaveGenerator testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);
}

TEST(LogicSquareWaveGeneratorTestsGroup, Deserialize_with_less_period1_value_return_zero) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_SquareWaveGenerator;
    *((uint64_t *)&buffer[1]) = 0;
    *((uint64_t *)&buffer[9]) = 678000;

    SquareWaveGenerator testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);
}

TEST(LogicSquareWaveGeneratorTestsGroup, Deserialize_with_greater_period0_value_return_zero) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_SquareWaveGenerator;
    *((uint64_t *)&buffer[1]) = 99990 * 1000LL + 1;

    SquareWaveGenerator testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);
}

TEST(LogicSquareWaveGeneratorTestsGroup, Deserialize_with_greater_period1_value_return_zero) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_SquareWaveGenerator;
    *((uint64_t *)&buffer[1]) = 123000;
    *((uint64_t *)&buffer[9]) = 99990 * 1000LL + 1;

    SquareWaveGenerator testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(0, readed);
}

TEST(LogicSquareWaveGeneratorTestsGroup, GetElementType) {
    SquareWaveGenerator testable;
    CHECK_EQUAL(TvElementType::et_SquareWaveGenerator, testable.GetElementType());
}

TEST(LogicSquareWaveGeneratorTestsGroup, TryToCast) {
    SquareWaveGenerator generator;
    CHECK_TRUE(SquareWaveGenerator::TryToCast(&generator) == &generator);

    TimerSecs timerSecs;
    CHECK_TRUE(SquareWaveGenerator::TryToCast(&timerSecs) == NULL);
}

TEST(LogicSquareWaveGeneratorTestsGroup, SelectPrior_changing_period0) {
    SquareWaveGenerator testable(100, 200);
    testable.BeginEditing();
    testable.Change();
    testable.SelectPrior();
    CHECK_EQUAL(110 * 1000L, testable.GetPeriod0Us());
    testable.SelectPrior();
    CHECK_EQUAL(120 * 1000L, testable.GetPeriod0Us());

    testable.SetPeriod0(99970);
    testable.SelectPrior();
    CHECK_EQUAL(99980 * 1000L, testable.GetPeriod0Us());
    testable.SelectPrior();
    CHECK_EQUAL(99990 * 1000L, testable.GetPeriod0Us());
    testable.SelectPrior();
    CHECK_EQUAL(99990 * 1000L, testable.GetPeriod0Us());
}

TEST(LogicSquareWaveGeneratorTestsGroup, SelectPrior_changing_period1) {
    SquareWaveGenerator testable(100, 200);
    testable.BeginEditing();
    testable.Change();
    testable.Change();
    testable.SelectPrior();
    CHECK_EQUAL(210 * 1000L, testable.GetPeriod1Us());
    testable.SelectPrior();
    CHECK_EQUAL(220 * 1000L, testable.GetPeriod1Us());

    testable.SetPeriod1(99970);
    testable.SelectPrior();
    CHECK_EQUAL(99980 * 1000L, testable.GetPeriod1Us());
    testable.SelectPrior();
    CHECK_EQUAL(99990 * 1000L, testable.GetPeriod1Us());
    testable.SelectPrior();
    CHECK_EQUAL(99990 * 1000L, testable.GetPeriod1Us());
}

TEST(LogicSquareWaveGeneratorTestsGroup, SelectNext_changing_period0) {
    SquareWaveGenerator testable(30, 40);
    testable.BeginEditing();
    testable.Change();
    testable.SelectNext();
    CHECK_EQUAL(20 * 1000L, testable.GetPeriod0Us());
    testable.SelectNext();
    CHECK_EQUAL(10 * 1000L, testable.GetPeriod0Us());
    testable.SelectNext();
    CHECK_EQUAL(10 * 1000L, testable.GetPeriod0Us());

    testable.SetPeriod0(99990);
    testable.SelectNext();
    CHECK_EQUAL(99980 * 1000L, testable.GetPeriod0Us());
    testable.SelectNext();
    CHECK_EQUAL(99970 * 1000L, testable.GetPeriod0Us());
}

TEST(LogicSquareWaveGeneratorTestsGroup, SelectNext_changing_period1) {
    SquareWaveGenerator testable(40, 30);
    testable.BeginEditing();
    testable.Change();
    testable.Change();
    testable.SelectNext();
    CHECK_EQUAL(20 * 1000L, testable.GetPeriod1Us());
    testable.SelectNext();
    CHECK_EQUAL(10 * 1000L, testable.GetPeriod1Us());
    testable.SelectNext();
    CHECK_EQUAL(10 * 1000L, testable.GetPeriod1Us());

    testable.SetPeriod1(99990);
    testable.SelectNext();
    CHECK_EQUAL(99980 * 1000L, testable.GetPeriod1Us());
    testable.SelectNext();
    CHECK_EQUAL(99970 * 1000L, testable.GetPeriod1Us());
}

TEST(LogicSquareWaveGeneratorTestsGroup, PageUp_changing_period0) {
    SquareWaveGenerator testable(100, 200);
    testable.BeginEditing();
    testable.Change();
    testable.PageUp();
    CHECK_EQUAL(350 * 1000L, testable.GetPeriod0Us());
    testable.PageUp();
    CHECK_EQUAL(600 * 1000L, testable.GetPeriod0Us());

    testable.SetPeriod0(99700);
    testable.PageUp();
    CHECK_EQUAL(99950 * 1000L, testable.GetPeriod0Us());
    testable.PageUp();
    CHECK_EQUAL(99990 * 1000L, testable.GetPeriod0Us());
    testable.PageUp();
    CHECK_EQUAL(99990 * 1000L, testable.GetPeriod0Us());
}

TEST(LogicSquareWaveGeneratorTestsGroup, PageUp_changing_period1) {
    SquareWaveGenerator testable(100, 200);
    testable.BeginEditing();
    testable.Change();
    testable.Change();
    testable.PageUp();
    CHECK_EQUAL(450 * 1000L, testable.GetPeriod1Us());
    testable.PageUp();
    CHECK_EQUAL(700 * 1000L, testable.GetPeriod1Us());

    testable.SetPeriod1(99700);
    testable.PageUp();
    CHECK_EQUAL(99950 * 1000L, testable.GetPeriod1Us());
    testable.PageUp();
    CHECK_EQUAL(99990 * 1000L, testable.GetPeriod1Us());
    testable.PageUp();
    CHECK_EQUAL(99990 * 1000L, testable.GetPeriod1Us());
}

TEST(LogicSquareWaveGeneratorTestsGroup, PageDown_changing_period0) {
    SquareWaveGenerator testable(300, 400);
    testable.BeginEditing();
    testable.Change();
    testable.PageDown();
    CHECK_EQUAL(50 * 1000L, testable.GetPeriod0Us());
    testable.PageDown();
    CHECK_EQUAL(10 * 1000L, testable.GetPeriod0Us());
    testable.PageDown();
    CHECK_EQUAL(10 * 1000L, testable.GetPeriod0Us());

    testable.SetPeriod0(99990);
    testable.PageDown();
    CHECK_EQUAL(99740 * 1000L, testable.GetPeriod0Us());
    testable.PageDown();
    CHECK_EQUAL(99490 * 1000L, testable.GetPeriod0Us());
}

TEST(LogicSquareWaveGeneratorTestsGroup, PageDown_changing_period1) {
    SquareWaveGenerator testable(300, 400);
    testable.BeginEditing();
    testable.Change();
    testable.Change();
    testable.PageDown();
    CHECK_EQUAL(150 * 1000L, testable.GetPeriod1Us());
    testable.PageDown();
    CHECK_EQUAL(10 * 1000L, testable.GetPeriod1Us());
    testable.PageDown();
    CHECK_EQUAL(10 * 1000L, testable.GetPeriod1Us());

    testable.SetPeriod1(99990);
    testable.PageDown();
    CHECK_EQUAL(99740 * 1000L, testable.GetPeriod1Us());
    testable.PageDown();
    CHECK_EQUAL(99490 * 1000L, testable.GetPeriod1Us());
}
