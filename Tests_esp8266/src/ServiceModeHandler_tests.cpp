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
#include "main/Maintenance/ServiceModeHandler_Backup.cpp"
#include "main/Maintenance/ServiceModeHandler_Restore.cpp"
#include "main/Maintenance/ServiceModeHandler_SmartConfig.cpp"

#include "main/redundant_storage.h"

#include "tests_utils.h"

TEST_GROUP(ServiceModeHandlerTestsGroup){ //
                                          TEST_SETUP(){ mock().disable();
create_storage_0();
create_storage_1();
create_backups_storage();
}

TEST_TEARDOWN() {
    remove_storage_0();
    remove_storage_1();
    remove_backups_storage();
    mock().enable();
}
}
;

namespace {
    class TestableServiceModeHandler : public ServiceModeHandler {
      public:
        static Mode PublicMorozov_ChangeModeToPrev(Mode mode) {
            return ChangeModeToPrev(mode);
        }
        static Mode PublicMorozov_ChangeModeToNext(Mode mode) {
            return ChangeModeToNext(mode);
        }
        static bool PublicMorozov_CreateBackup(uint32_t fileno) {
            return CreateBackup(fileno);
        }
        static bool PublicMorozov_DoRestore(uint32_t fileno) {
            return DoRestore(fileno);
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

TEST(ServiceModeHandlerTestsGroup, CreateBackup) {
    uint8_t data[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
                       0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
                       0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f };

    redundant_storage storage;
    storage.data = data;
    storage.size = sizeof(data);
    storage.version = LADDER_VERSION;

    redundant_storage_store(storage_0_partition,
                            storage_0_path,
                            storage_1_partition,
                            storage_1_path,
                            ladder_storage_name,
                            &storage);

    CHECK_TRUE(TestableServiceModeHandler::PublicMorozov_CreateBackup(0));

    backups_storage load_storage;
    CHECK_TRUE(backups_storage_load("ladder_0", &load_storage));

    MEMCMP_EQUAL(data, load_storage.data, sizeof(data));
    CHECK_EQUAL(sizeof(data), load_storage.size);
    CHECK_EQUAL(BACKUPS_VERSION, load_storage.version);
    delete[] load_storage.data;
}

TEST(ServiceModeHandlerTestsGroup, CreateBackup_error_if_ladder_version_is_other_than_current) {
    uint8_t data[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
                       0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
                       0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f };

    redundant_storage storage;
    storage.data = data;
    storage.size = sizeof(data);
    storage.version = -1;

    redundant_storage_store(storage_0_partition,
                            storage_0_path,
                            storage_1_partition,
                            storage_1_path,
                            ladder_storage_name,
                            &storage);

    CHECK_FALSE(TestableServiceModeHandler::PublicMorozov_CreateBackup(0));
}

TEST(ServiceModeHandlerTestsGroup, CreateBackup_error_if_no_ladder) {
    CHECK_FALSE(TestableServiceModeHandler::PublicMorozov_CreateBackup(0));
}

TEST(ServiceModeHandlerTestsGroup, DoRestore) {
    uint8_t data[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
                       0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
                       0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f };

    backups_storage storage;
    storage.data = data;
    storage.size = sizeof(data);
    storage.version = BACKUPS_VERSION;
    backups_storage_store("ladder_0", &storage);

    CHECK_TRUE(TestableServiceModeHandler::PublicMorozov_DoRestore(0));

    redundant_storage load_storage;

    load_storage = redundant_storage_load(storage_0_partition,
                                          storage_0_path,
                                          storage_1_partition,
                                          storage_1_path,
                                          ladder_storage_name);
    MEMCMP_EQUAL(data, load_storage.data, sizeof(data));
    CHECK_EQUAL(sizeof(data), load_storage.size);
    CHECK_EQUAL(LADDER_VERSION, load_storage.version);
    delete[] load_storage.data;
}

TEST(ServiceModeHandlerTestsGroup, DoRestore_error_if_backup_version_is_other_than_current) {
    uint8_t data[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
                       0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
                       0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f };

    backups_storage storage;
    storage.data = data;
    storage.size = sizeof(data);
    storage.version = -1;
    backups_storage_store("ladder_0", &storage);

    CHECK_FALSE(TestableServiceModeHandler::PublicMorozov_DoRestore(0));
}

TEST(ServiceModeHandlerTestsGroup, DoRestore_error_if_no_backup) {
    CHECK_FALSE(TestableServiceModeHandler::PublicMorozov_DoRestore(0));
}
