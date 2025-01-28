#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/Display/LogsList.cpp"
#include "main/Display/LogsList.h"

static uint8_t frame_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8] = {};

TEST_GROUP(LogsListTestsGroup){ //
                                TEST_SETUP(){ memset(frame_buffer, 0, sizeof(frame_buffer));
mock().disable();
}

TEST_TEARDOWN() {
    mock().enable();
}
}
;

class TestableLogsList : public LogsList {
  public:
    const static int line_size = 16;
    const static int lines_count = 5;

    explicit TestableLogsList() : LogsList() {
        static_assert(line_size == LogsList::line_size);
        static_assert(lines_count == LogsList::lines_count);
    }

    char *PublicMorozov_GetLine(int line_num) {
        return lines[line_num];
    }
};

TEST(LogsListTestsGroup, Ctor_reset_lines) {
    TestableLogsList testable;

    for (size_t i = 0; i < testable.lines_count; i++) {
        CHECK_EQUAL(0, strlen(testable.PublicMorozov_GetLine(i)));
    }
}

TEST(LogsListTestsGroup, Append_short_line__puts_unchanged) {
    TestableLogsList testable;

    testable.Append("shorter0than 16");
    testable.Append("shorter1than 16");
    testable.Append("shorter2than 16");
    testable.Append("shorter3than 16");
    testable.Append("shorter4than 16");

    STRCMP_EQUAL("shorter0than 16", testable.PublicMorozov_GetLine(0));
    STRCMP_EQUAL("shorter1than 16", testable.PublicMorozov_GetLine(1));
    STRCMP_EQUAL("shorter2than 16", testable.PublicMorozov_GetLine(2));
    STRCMP_EQUAL("shorter3than 16", testable.PublicMorozov_GetLine(3));
    STRCMP_EQUAL("shorter4than 16", testable.PublicMorozov_GetLine(4));
}

TEST(LogsListTestsGroup, Append_long_line__puts_with_end_elipses) {
    TestableLogsList testable;

    testable.Append("looonger0than 16");
    testable.Append("looonger1than 16a");
    testable.Append("looonger2than 16ab");
    testable.Append("looonger3than 16abc");
    testable.Append("looonger4than 16abcd");

    STRCMP_EQUAL("looonger0tha...", testable.PublicMorozov_GetLine(0));
    STRCMP_EQUAL("looonger1tha...", testable.PublicMorozov_GetLine(1));
    STRCMP_EQUAL("looonger2tha...", testable.PublicMorozov_GetLine(2));
    STRCMP_EQUAL("looonger3tha...", testable.PublicMorozov_GetLine(3));
    STRCMP_EQUAL("looonger4tha...", testable.PublicMorozov_GetLine(4));
}

TEST(LogsListTestsGroup, Append_scroll_lines) {
    TestableLogsList testable;

    testable.Append("line 0");
    STRCMP_EQUAL("line 0", testable.PublicMorozov_GetLine(0));
    STRCMP_EQUAL("", testable.PublicMorozov_GetLine(1));
    STRCMP_EQUAL("", testable.PublicMorozov_GetLine(2));
    STRCMP_EQUAL("", testable.PublicMorozov_GetLine(3));
    STRCMP_EQUAL("", testable.PublicMorozov_GetLine(4));

    testable.Append("line 1");
    STRCMP_EQUAL("line 0", testable.PublicMorozov_GetLine(0));
    STRCMP_EQUAL("line 1", testable.PublicMorozov_GetLine(1));
    STRCMP_EQUAL("", testable.PublicMorozov_GetLine(2));
    STRCMP_EQUAL("", testable.PublicMorozov_GetLine(3));
    STRCMP_EQUAL("", testable.PublicMorozov_GetLine(4));

    testable.Append("line 2");
    STRCMP_EQUAL("line 0", testable.PublicMorozov_GetLine(0));
    STRCMP_EQUAL("line 1", testable.PublicMorozov_GetLine(1));
    STRCMP_EQUAL("line 2", testable.PublicMorozov_GetLine(2));
    STRCMP_EQUAL("", testable.PublicMorozov_GetLine(3));
    STRCMP_EQUAL("", testable.PublicMorozov_GetLine(4));

    testable.Append("line 3");
    STRCMP_EQUAL("line 0", testable.PublicMorozov_GetLine(0));
    STRCMP_EQUAL("line 1", testable.PublicMorozov_GetLine(1));
    STRCMP_EQUAL("line 2", testable.PublicMorozov_GetLine(2));
    STRCMP_EQUAL("line 3", testable.PublicMorozov_GetLine(3));
    STRCMP_EQUAL("", testable.PublicMorozov_GetLine(4));

    testable.Append("line 4");
    STRCMP_EQUAL("line 0", testable.PublicMorozov_GetLine(0));
    STRCMP_EQUAL("line 1", testable.PublicMorozov_GetLine(1));
    STRCMP_EQUAL("line 2", testable.PublicMorozov_GetLine(2));
    STRCMP_EQUAL("line 3", testable.PublicMorozov_GetLine(3));
    STRCMP_EQUAL("line 4", testable.PublicMorozov_GetLine(4));

    testable.Append("line 5");
    STRCMP_EQUAL("line 1", testable.PublicMorozov_GetLine(0));
    STRCMP_EQUAL("line 2", testable.PublicMorozov_GetLine(1));
    STRCMP_EQUAL("line 3", testable.PublicMorozov_GetLine(2));
    STRCMP_EQUAL("line 4", testable.PublicMorozov_GetLine(3));
    STRCMP_EQUAL("line 5", testable.PublicMorozov_GetLine(4));

    testable.Append("line 6");
    STRCMP_EQUAL("line 2", testable.PublicMorozov_GetLine(0));
    STRCMP_EQUAL("line 3", testable.PublicMorozov_GetLine(1));
    STRCMP_EQUAL("line 4", testable.PublicMorozov_GetLine(2));
    STRCMP_EQUAL("line 5", testable.PublicMorozov_GetLine(3));
    STRCMP_EQUAL("line 6", testable.PublicMorozov_GetLine(4));
}
