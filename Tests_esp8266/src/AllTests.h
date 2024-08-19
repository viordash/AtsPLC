
#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"

TEST_GROUP_C_WRAPPER(Crc32TestsGroup){};
TEST_C_WRAPPER(Crc32TestsGroup, data_32);
TEST_C_WRAPPER(Crc32TestsGroup, data_16);
TEST_C_WRAPPER(Crc32TestsGroup, empty_data);
TEST_C_WRAPPER(Crc32TestsGroup, null_data);

TEST_GROUP_C_WRAPPER(HotReloadTestsGroup) {
    TEST_GROUP_C_SETUP_WRAPPER(HotReloadTestsGroup);
    TEST_GROUP_C_TEARDOWN_WRAPPER(HotReloadTestsGroup);
};
TEST_C_WRAPPER(HotReloadTestsGroup, load_store);
TEST_C_WRAPPER(HotReloadTestsGroup, load_if_memory_cleared);

TEST_GROUP_C_WRAPPER(GpioTestsGroup) {
    TEST_GROUP_C_SETUP_WRAPPER(GpioTestsGroup);
    TEST_GROUP_C_TEARDOWN_WRAPPER(GpioTestsGroup);
};
TEST_C_WRAPPER(GpioTestsGroup, gpio_init__on_startup);
TEST_C_WRAPPER(GpioTestsGroup, BUTTON_UP_IO_isr_handler__when_input_is_open);
TEST_C_WRAPPER(GpioTestsGroup, BUTTON_UP_IO_isr_handler__when_input_is_close);
TEST_C_WRAPPER(GpioTestsGroup, BUTTON_DOWN_IO_isr_handler__when_input_is_open);
TEST_C_WRAPPER(GpioTestsGroup, BUTTON_DOWN_IO_isr_handler__when_input_is_close);
TEST_C_WRAPPER(GpioTestsGroup, BUTTON_RIGHT_IO_isr_handler__when_input_is_open);
TEST_C_WRAPPER(GpioTestsGroup, BUTTON_RIGHT_IO_isr_handler__when_input_is_close);
TEST_C_WRAPPER(GpioTestsGroup, BUTTON_SELECT_IO_isr_handler__when_input_is_open);
TEST_C_WRAPPER(GpioTestsGroup, BUTTON_SELECT_IO_isr_handler__when_input_is_close);
TEST_C_WRAPPER(GpioTestsGroup, set_OUTPUT_0_to_active);
TEST_C_WRAPPER(GpioTestsGroup, set_OUTPUT_0_to_passive);
TEST_C_WRAPPER(GpioTestsGroup, set_OUTPUT_1_to_active);
TEST_C_WRAPPER(GpioTestsGroup, set_OUTPUT_1_to_passive);

IMPORT_TEST_GROUP(RedundantStorageTestsGroup);

IMPORT_TEST_GROUP(SettingsTestsGroup);

TEST_GROUP_C_WRAPPER(RestartCounterTestsGroup) {
    TEST_GROUP_C_SETUP_WRAPPER(RestartCounterTestsGroup);
    TEST_GROUP_C_TEARDOWN_WRAPPER(RestartCounterTestsGroup);
};
TEST_C_WRAPPER(RestartCounterTestsGroup, hot_restart_counter);

IMPORT_TEST_GROUP(ButtonTestsGroup);
IMPORT_TEST_GROUP(DisplayTestsGroup);
IMPORT_TEST_GROUP(LogicInputNOTestsGroup);
IMPORT_TEST_GROUP(LogicInputNCTestsGroup);
IMPORT_TEST_GROUP(LogicCommonInputTestsGroup);
IMPORT_TEST_GROUP(LogicCommonComparatorTestsGroup);
IMPORT_TEST_GROUP(LogicTimerSecsTestsGroup);
IMPORT_TEST_GROUP(LogicTimerMSecsTestsGroup);
IMPORT_TEST_GROUP(LogicCommonTimerTestsGroup);
IMPORT_TEST_GROUP(LogicIncomeRailTestsGroup);
IMPORT_TEST_GROUP(LogicComparatorLsTestsGroup);
IMPORT_TEST_GROUP(LogicComparatorLETestsGroup);
IMPORT_TEST_GROUP(LogicComparatorGrTestsGroup);
IMPORT_TEST_GROUP(LogicComparatorGETestsGroup);

IMPORT_TEST_GROUP(StatusBarTestsGroup);