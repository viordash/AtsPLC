#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/InputNC.h"

static uint8_t frame_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8] = {};

TEST_GROUP(LogicInputNCTestsGroup){ //
                                    TEST_SETUP(){ memset(frame_buffer, 0, sizeof(frame_buffer));
}

TEST_TEARDOWN() {
}
}
;

namespace {
    class TestableInputNC : public InputNC {
      public:
        TestableInputNC(const MapIO io_adr) : InputNC(io_adr) {
        }
        virtual ~TestableInputNC() {
        }

        const char *GetLabel() {
            return label;
        }
        LogicItemState *PublicMorozov_Get_state() {
            return &state;
        }
        MapIO PublicMorozov_Get_io_adr() {
            return io_adr;
        }
    };
} // namespace

TEST(LogicInputNCTestsGroup, DoAction_skip_when_incoming_passive) {
    mock("0").expectNoCall("gpio_get_level");

    TestableInputNC testable(MapIO::DI);

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
}

TEST(LogicInputNCTestsGroup, DoAction_change_state_to_active) {
    mock("0").expectOneCall("gpio_get_level").andReturnValue(1);

    TestableInputNC testable(MapIO::DI);

    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());
}

TEST(LogicInputNCTestsGroup, DoAction_change_state_to_passive) {
    mock("0").expectOneCall("gpio_get_level").andReturnValue(0);

    TestableInputNC testable(MapIO::DI);
    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;

    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
}

TEST(LogicInputNCTestsGroup, Serialize) {
    uint8_t buffer[256] = {};
    TestableInputNC testable(MapIO::V2);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(2, writed);

    CHECK_EQUAL(TvElementType::et_InputNC, *((TvElementType *)&buffer[0]));
    CHECK_EQUAL(MapIO::V2, *((MapIO *)&buffer[1]));
}

TEST(LogicInputNCTestsGroup, Serialize_just_for_obtain_size) {
    TestableInputNC testable(MapIO::V2);

    size_t writed = testable.Serialize(NULL, SIZE_MAX);
    CHECK_EQUAL(2, writed);
}

TEST(LogicInputNCTestsGroup, Serialize_to_small_buffer_return_zero) {
    uint8_t buffer[1] = {};
    TestableInputNC testable(MapIO::V2);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, writed);
}

TEST(LogicInputNCTestsGroup, Deserialize) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_InputNC;
    *((MapIO *)&buffer[1]) = MapIO::V3;

    TestableInputNC testable(MapIO::DI);

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(1, readed);

    CHECK_EQUAL(MapIO::V3, testable.PublicMorozov_Get_io_adr());
}

TEST(LogicInputNCTestsGroup, Deserialize_with_small_buffer_return_zero) {
    uint8_t buffer[0] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_InputNC;

    TestableInputNC testable(MapIO::V2);

    size_t readed = testable.Deserialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, readed);
}

TEST(LogicInputNCTestsGroup, Deserialize_with_wrong_io_adr_return_zero) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_InputNC;

    TestableInputNC testable(MapIO::V1);

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