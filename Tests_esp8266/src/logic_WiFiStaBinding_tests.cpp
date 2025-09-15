#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/LogicProgram/Bindings/WiFiStaBinding.h"
#include "main/LogicProgram/Inputs/ComparatorEq.h"
#include "main/LogicProgram/Inputs/ComparatorGE.h"
#include "main/LogicProgram/Inputs/ComparatorGr.h"
#include "main/LogicProgram/Inputs/ComparatorLE.h"
#include "main/LogicProgram/Inputs/ComparatorLs.h"
#include "main/LogicProgram/Inputs/InputNC.h"
#include "main/LogicProgram/Inputs/InputNO.h"

static FrameBuffer frame_buffer = {};

static WiFiService *wifi_service;
TEST_GROUP(LogicWiFiStaBindingTestsGroup){
    //
    TEST_SETUP(){ memset(&frame_buffer.buffer, 0, sizeof(frame_buffer.buffer));

mock().expectOneCall("vTaskDelay").ignoreOtherParameters();
mock().expectOneCall("xTaskCreate").ignoreOtherParameters();
wifi_service = new WiFiService();
Controller::Start(NULL, wifi_service, NULL, NULL);
}

TEST_TEARDOWN() {
    Controller::V1.Unbind();
    Controller::V2.Unbind();
    Controller::V3.Unbind();
    Controller::V4.Unbind();
    Controller::Stop();
    delete wifi_service;
}
}
;

namespace {
    class TestableWiFiStaBinding : public WiFiStaBinding {
      public:
        TestableWiFiStaBinding() : WiFiStaBinding() {
        }
        virtual ~TestableWiFiStaBinding() {
        }

        const char *GetLabel() {
            return label;
        }
        LogicItemState *PublicMorozov_Get_state() {
            return &state;
        }
        int *PublicMorozov_Get_editing_property_id() {
            return &editing_property_id;
        }
    };
} // namespace

TEST(LogicWiFiStaBindingTestsGroup, DoAction_skip_when_incoming_passive) {
    TestableWiFiStaBinding testable;
    testable.SetIoAdr(MapIO::V1);

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisPassive).any_changes);
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
}

TEST(LogicWiFiStaBindingTestsGroup,
     DoAction_change_state_to_active_also_switch_variable_binding_to_wifi) {
    TestableWiFiStaBinding testable;
    testable.SetIoAdr(MapIO::V1);

    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive).any_changes);
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());
    CHECK_TRUE(Controller::V1.BindedToWiFi());
}

TEST(LogicWiFiStaBindingTestsGroup,
     DoAction_change_state_to_passive_also_switch_variable_binding_to_default) {
    TestableWiFiStaBinding testable;
    testable.SetIoAdr(MapIO::V1);
    Controller::V1.BindToStaWiFi(wifi_service);
    *(testable.PublicMorozov_Get_state()) = LogicItemState::lisActive;

    CHECK_TRUE(testable.DoAction(true, LogicItemState::lisPassive).any_changes);
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
    CHECK_FALSE(Controller::V1.BindedToWiFi());
}

TEST(LogicWiFiStaBindingTestsGroup, DoAction_change_state_only_by_reason) {
    TestableWiFiStaBinding testable;
    testable.SetIoAdr(MapIO::V1);

    CHECK_TRUE(testable.DoAction(false, LogicItemState::lisActive).any_changes);
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());
    CHECK_TRUE(Controller::V1.BindedToWiFi());

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisActive).any_changes);
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());
    CHECK_TRUE(Controller::V1.BindedToWiFi());

    CHECK_TRUE(testable.DoAction(true, LogicItemState::lisPassive).any_changes);
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
    CHECK_FALSE(Controller::V1.BindedToWiFi());

    CHECK_FALSE(testable.DoAction(false, LogicItemState::lisPassive).any_changes);
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
    CHECK_FALSE(Controller::V1.BindedToWiFi());

    CHECK_TRUE(testable.DoAction(true, LogicItemState::lisActive).any_changes);
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());
    CHECK_TRUE(Controller::V1.BindedToWiFi());

    CHECK_FALSE(testable.DoAction(true, LogicItemState::lisActive).any_changes);
    CHECK_EQUAL(LogicItemState::lisActive, *testable.PublicMorozov_Get_state());
    CHECK_TRUE(Controller::V1.BindedToWiFi());

    CHECK_TRUE(testable.DoAction(true, LogicItemState::lisPassive).any_changes);
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
    CHECK_FALSE(Controller::V1.BindedToWiFi());

    CHECK_FALSE(testable.DoAction(true, LogicItemState::lisPassive).any_changes);
    CHECK_EQUAL(LogicItemState::lisPassive, *testable.PublicMorozov_Get_state());
    CHECK_FALSE(Controller::V1.BindedToWiFi());
}

TEST(LogicWiFiStaBindingTestsGroup, SelectNext_changing_IoAdr) {
    TestableWiFiStaBinding testable;
    testable.SetIoAdr(MapIO::DI);
    testable.BeginEditing();
    testable.Change();
    testable.SelectNext();
    CHECK_EQUAL(MapIO::V1, testable.GetIoAdr());
    testable.SelectNext();
    CHECK_EQUAL(MapIO::V2, testable.GetIoAdr());
    testable.SelectNext();
    CHECK_EQUAL(MapIO::V3, testable.GetIoAdr());
    testable.SelectNext();
    CHECK_EQUAL(MapIO::V4, testable.GetIoAdr());
    testable.SelectNext();
    CHECK_EQUAL(MapIO::V1, testable.GetIoAdr());
}

TEST(LogicWiFiStaBindingTestsGroup, SelectPrior_changing_IoAdr) {
    TestableWiFiStaBinding testable;
    testable.SetIoAdr(MapIO::DI);
    testable.BeginEditing();
    testable.Change();
    testable.SelectPrior();
    CHECK_EQUAL(MapIO::V4, testable.GetIoAdr());
    testable.SelectPrior();
    CHECK_EQUAL(MapIO::V3, testable.GetIoAdr());
    testable.SelectPrior();
    CHECK_EQUAL(MapIO::V2, testable.GetIoAdr());
    testable.SelectPrior();
    CHECK_EQUAL(MapIO::V1, testable.GetIoAdr());
    testable.SelectPrior();
    CHECK_EQUAL(MapIO::V4, testable.GetIoAdr());
}

TEST(LogicWiFiStaBindingTestsGroup, Change_after_IoAdr_modified_then_EndEditing) {
    TestableWiFiStaBinding testable;
    testable.SetIoAdr(MapIO::DI);
    testable.BeginEditing();
    testable.Change();
    testable.SelectNext();
    CHECK_TRUE(testable.Editing());
    testable.Change();
    CHECK_FALSE(testable.Editing());
}

TEST(LogicWiFiStaBindingTestsGroup, Serialize) {
    uint8_t buffer[256] = {};
    TestableWiFiStaBinding testable;
    testable.SetIoAdr(MapIO::V2);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(2, writed);

    CHECK_EQUAL(TvElementType::et_WiFiStaBinding, *((TvElementType *)&buffer[0]));
    CHECK_EQUAL(MapIO::V2, *((MapIO *)&buffer[1]));
}

TEST(LogicWiFiStaBindingTestsGroup, Serialize_just_for_obtain_size) {
    TestableWiFiStaBinding testable;
    testable.SetIoAdr(MapIO::DI);

    size_t writed = testable.Serialize(NULL, SIZE_MAX);
    CHECK_EQUAL(2, writed);

    writed = testable.Serialize(NULL, 0);
    CHECK_EQUAL(2, writed);
}

TEST(LogicWiFiStaBindingTestsGroup, Serialize_to_small_buffer_return_zero) {
    uint8_t buffer[1] = {};
    TestableWiFiStaBinding testable;
    testable.SetIoAdr(MapIO::DI);

    size_t writed = testable.Serialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, writed);
}

TEST(LogicWiFiStaBindingTestsGroup, Deserialize) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_WiFiStaBinding;
    *((MapIO *)&buffer[1]) = MapIO::V3;

    TestableWiFiStaBinding testable;

    size_t readed = testable.Deserialize(&buffer[1], sizeof(buffer) - 1);
    CHECK_EQUAL(1, readed);

    CHECK_EQUAL(MapIO::V3, testable.GetIoAdr());
    CHECK(&Controller::V3 == testable.Input);
}

TEST(LogicWiFiStaBindingTestsGroup, Deserialize_with_small_buffer_return_zero) {
    uint8_t buffer[0] = {};

    TestableWiFiStaBinding testable;

    size_t readed = testable.Deserialize(buffer, sizeof(buffer));
    CHECK_EQUAL(0, readed);
}

TEST(LogicWiFiStaBindingTestsGroup, Deserialize_with_wrong_io_adr_return_zero) {
    uint8_t buffer[256] = {};
    *((TvElementType *)&buffer[0]) = TvElementType::et_WiFiStaBinding;

    TestableWiFiStaBinding testable;

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

TEST(LogicWiFiStaBindingTestsGroup, GetElementType) {
    TestableWiFiStaBinding testable;
    CHECK_EQUAL(TvElementType::et_WiFiStaBinding, testable.GetElementType());
}

TEST(LogicWiFiStaBindingTestsGroup, TryToCast) {
    InputNC inputNC;
    CHECK_TRUE(WiFiStaBinding::TryToCast(&inputNC) == NULL);

    InputNO inputNO;
    CHECK_TRUE(WiFiStaBinding::TryToCast(&inputNO) == NULL);

    ComparatorEq comparatorEq;
    CHECK_TRUE(WiFiStaBinding::TryToCast(&comparatorEq) == NULL);

    ComparatorGE comparatorGE;
    CHECK_TRUE(WiFiStaBinding::TryToCast(&comparatorGE) == NULL);

    ComparatorGr comparatorGr;
    CHECK_TRUE(WiFiStaBinding::TryToCast(&comparatorGr) == NULL);

    ComparatorLE comparatorLE;
    CHECK_TRUE(WiFiStaBinding::TryToCast(&comparatorLE) == NULL);

    ComparatorLs comparatorLs;
    CHECK_TRUE(WiFiStaBinding::TryToCast(&comparatorLs) == NULL);

    WiFiStaBinding wiFiStaBinding;
    CHECK_TRUE(WiFiStaBinding::TryToCast(&wiFiStaBinding) == &wiFiStaBinding);
}