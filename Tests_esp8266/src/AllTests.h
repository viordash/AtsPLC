
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
TEST_C_WRAPPER(HotReloadTestsGroup, store_load);
TEST_C_WRAPPER(HotReloadTestsGroup, load_if_memory_cleared);

TEST_GROUP_C_WRAPPER(GpioTestsGroup){};
TEST_C_WRAPPER(GpioTestsGroup, gpio_init__use_cleared_startup_value);
TEST_C_WRAPPER(GpioTestsGroup, gpio_init__use_startup_for_out0);
TEST_C_WRAPPER(GpioTestsGroup, gpio_init__use_startup_for_out1);
TEST_C_WRAPPER(GpioTestsGroup, BUTTON_UP_IO_isr_handler__when_input_is_open);
TEST_C_WRAPPER(GpioTestsGroup, BUTTON_UP_IO_isr_handler__when_input_is_close);
TEST_C_WRAPPER(GpioTestsGroup, BUTTON_DOWN_IO_isr_handler__when_input_is_open);
TEST_C_WRAPPER(GpioTestsGroup, BUTTON_DOWN_IO_isr_handler__when_input_is_close);
TEST_C_WRAPPER(GpioTestsGroup, BUTTON_RIGHT_IO_isr_handler__when_input_is_open);
TEST_C_WRAPPER(GpioTestsGroup, BUTTON_RIGHT_IO_isr_handler__when_input_is_close);
TEST_C_WRAPPER(GpioTestsGroup, BUTTON_SELECT_IO_isr_handler__when_input_is_open);
TEST_C_WRAPPER(GpioTestsGroup, BUTTON_SELECT_IO_isr_handler__when_input_is_close);

IMPORT_TEST_GROUP(RedundantStorageTestsGroup);

IMPORT_TEST_GROUP(SettingsTestsGroup);

TEST_GROUP_C_WRAPPER(RestartCounterTestsGroup) {
    TEST_GROUP_C_SETUP_WRAPPER(RestartCounterTestsGroup);
    TEST_GROUP_C_TEARDOWN_WRAPPER(RestartCounterTestsGroup);
};
TEST_C_WRAPPER(RestartCounterTestsGroup, hot_restart_counter);

IMPORT_TEST_GROUP(ButtonTestsGroup);
IMPORT_TEST_GROUP(DisplayItemBaseTestsGroup);
IMPORT_TEST_GROUP(LogicInputNOTestsGroup);