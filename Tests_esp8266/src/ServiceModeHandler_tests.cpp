#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/Maintenance/ServiceModeHandler.cpp"
#include "main/Maintenance/ServiceModeHandler.h"
#include "main/Maintenance/ServiceModeHandler_SmartConfig.cpp"

TEST_GROUP(ServiceModeHandlerTestsGroup){ //
                                          TEST_SETUP(){}

                                          TEST_TEARDOWN(){}
};

namespace {
    class TestableServiceModeHandler : public ServiceModeHandler {
      public:
        static void PublicMorozov_RenderMainMenu(Mode mode) {
            RenderMainMenu(mode);
        }
        static Mode PublicMorozov_ChangeModeToPrev(Mode mode) {
            return ChangeModeToPrev(mode);
        }
        static Mode PublicMorozov_ChangeModeToNext(Mode mode) {
            return ChangeModeToNext(mode);
        }
    };
} // namespace

TEST(ServiceModeHandlerTestsGroup, ChangeModeToPrev) {
    CHECK_EQUAL(ServiceModeHandler::Mode::sm_ResetToDefault,
                TestableServiceModeHandler::PublicMorozov_ChangeModeToPrev(
                    ServiceModeHandler::Mode::sm_SmartConfig));

    CHECK_EQUAL(ServiceModeHandler::Mode::sm_RestoreLogic,
                TestableServiceModeHandler::PublicMorozov_ChangeModeToPrev(
                    ServiceModeHandler::Mode::sm_ResetToDefault));

    CHECK_EQUAL(ServiceModeHandler::Mode::sm_BackupLogic,
                TestableServiceModeHandler::PublicMorozov_ChangeModeToPrev(
                    ServiceModeHandler::Mode::sm_RestoreLogic));

    CHECK_EQUAL(ServiceModeHandler::Mode::sm_SmartConfig,
                TestableServiceModeHandler::PublicMorozov_ChangeModeToPrev(
                    ServiceModeHandler::Mode::sm_BackupLogic));
}

TEST(ServiceModeHandlerTestsGroup, ChangeModeToNext) {
    CHECK_EQUAL(ServiceModeHandler::Mode::sm_BackupLogic,
                TestableServiceModeHandler::PublicMorozov_ChangeModeToNext(
                    ServiceModeHandler::Mode::sm_SmartConfig));

    CHECK_EQUAL(ServiceModeHandler::Mode::sm_RestoreLogic,
                TestableServiceModeHandler::PublicMorozov_ChangeModeToNext(
                    ServiceModeHandler::Mode::sm_BackupLogic));

    CHECK_EQUAL(ServiceModeHandler::Mode::sm_ResetToDefault,
                TestableServiceModeHandler::PublicMorozov_ChangeModeToNext(
                    ServiceModeHandler::Mode::sm_RestoreLogic));

    CHECK_EQUAL(ServiceModeHandler::Mode::sm_SmartConfig,
                TestableServiceModeHandler::PublicMorozov_ChangeModeToNext(
                    ServiceModeHandler::Mode::sm_ResetToDefault));
}
