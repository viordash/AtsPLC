#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Inputs/InputNO.cpp"
#include "main/LogicProgram/Inputs/InputNO.h"

static uint8_t frame_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8] = {};

TEST_GROUP(LogicInputNOTestsGroup){ //
                                    TEST_SETUP(){ memset(frame_buffer, 0, sizeof(frame_buffer));
}

TEST_TEARDOWN() {
}
}
;

namespace {
    class TestableInputNO : public InputNO {
      public:
        TestableInputNO(const MapIO io_adr) : InputNO(io_adr) {
        }
        virtual ~TestableInputNO() {
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

TEST(LogicInputNOTestsGroup, GetLabel_DI) {
    TestableInputNO testable(MapIO::DI);
    STRCMP_EQUAL("DI", testable.GetLabel());
}

TEST(LogicInputNOTestsGroup, GetLabel_AI) {
    TestableInputNO testable(MapIO::AI);
    STRCMP_EQUAL("AI", testable.GetLabel());
}

TEST(LogicInputNOTestsGroup, GetLabel_V1) {
    TestableInputNO testable(MapIO::V1);
    STRCMP_EQUAL("V1", testable.GetLabel());
}

TEST(LogicInputNOTestsGroup, Passive_is_init_state) {
    TestableInputNO testable_0(MapIO::V1);
    TestableInputNO testable_1(MapIO::V2);
    CHECK_EQUAL(LogicItemState::lisPassive, *testable_0.PublicMorozov_Get_state());
    CHECK_EQUAL(LogicItemState::lisPassive, *testable_1.PublicMorozov_Get_state());
}

TEST(LogicInputNOTestsGroup, DoAction_skip_when_incoming_passive) {
    mock("0").expectNoCall("gpio_get_level");

    TestableInputNO testable(MapIO::DI);

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisPassive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
}

TEST(LogicInputNOTestsGroup, DoAction_change_state_to_active) {
    mock("0").expectOneCall("gpio_get_level").andReturnValue(0);

    TestableInputNO testable(MapIO::DI);

    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());
}

TEST(LogicInputNOTestsGroup, DoAction_change_state_to_passive) {
    mock("0").expectOneCall("gpio_get_level").andReturnValue(1);

    TestableInputNO testable(MapIO::DI);
    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;

    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive));
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
}

TEST(LogicInputNOTestsGroup, Serialize) {
    uint8_t buffer[256] = {};
    TestableInputNO testable(MapIO::V2);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(2, writed);

    CHECK_EQUAL(TvElementType::et_InputNO, *((TvElementType *)&buffer[0]));
    CHECK_EQUAL(MapIO::V2, *((MapIO *)&buffer[1]));
}

TEST(LogicInputNOTestsGroup, Serialize_just_for_obtain_size) {
    TestableInputNO testable(MapIO::V2);

    size_t writed = testable.Serialize(NULL, SIZE_MAX);
    CHECK_EQUAL(2, writed);
}

TEST(LogicInputNOTestsGroup, Serialize_to_small_buffer_return_zero) {
    uint8_t buffer[1] = {};
    TestableInputNO testable(MapIO::V2);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, writed);
}

TEST(LogicInputNOTestsGroup, Deserialize) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_InputNO;
    *((MapIO *)&buffer[1]) = MapIO::V3;

    TestableInputNO testable(MapIO::DI);

    size_t readed = testable.Deserialize(buffer, sizeof(buffer));
    CHECK_EQUAL(2, readed);

    CHECK_EQUAL(MapIO::V3, testable.PublicMorozov_Get_io_adr());
}

TEST(LogicInputNOTestsGroup, Deserialize_with_small_buffer_return_zero) {
    uint8_t buffer[1] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_InputNO;

    TestableInputNO testable(MapIO::V2);

    size_t readed = testable.Deserialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, readed);
}

TEST(LogicInputNOTestsGroup, Deserialize_when_wrong_element_type_return_zero) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_Undef;
    *((MapIO *)&buffer[1]) = MapIO::V3;

    TestableInputNO testable(MapIO::V2);

    size_t readed = testable.Deserialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, readed);
}
