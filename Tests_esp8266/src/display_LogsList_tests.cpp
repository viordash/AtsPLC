#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "main/Display/LogsList.h"
#include "main/Display/display.h"

static FrameBuffer frame_buffer = {};

TEST_GROUP(LogsListTestsGroup){ //
                                TEST_SETUP(){ memset(&frame_buffer.buffer, 0, sizeof(frame_buffer.buffer));
mock().disable();
}

TEST_TEARDOWN() {
    mock().enable();
}
}
;

class TestableLogsList : public LogsList {
  public:
    const static int lines_count = 4;

    explicit TestableLogsList(const char *title) : LogsList(title) {
        static_assert(lines_count == LogsList::lines_count);
    }

    char *PublicMorozov_GetLine(int line_num) {
        return lines[line_num];
    }
};

TEST(LogsListTestsGroup, Ctor_reset_lines) {
    TestableLogsList testable("list_box");

    for (size_t i = 0; i < testable.lines_count; i++) {
        CHECK_EQUAL(0, strlen(testable.PublicMorozov_GetLine(i)));
    }
}

TEST(LogsListTestsGroup, Append_short_line__puts_unchanged) {
    TestableLogsList testable("list_box");

    testable.Append("shorter 0 than 21 ab");
    testable.Append("shorter 1 than 21 ab");
    testable.Append("shorter 2 than 21 ab");
    testable.Append("shorter 3 than 21 ab");

    STRCMP_EQUAL("shorter 0 than 21 ab", testable.PublicMorozov_GetLine(0));
    STRCMP_EQUAL("shorter 1 than 21 ab", testable.PublicMorozov_GetLine(1));
    STRCMP_EQUAL("shorter 2 than 21 ab", testable.PublicMorozov_GetLine(2));
    STRCMP_EQUAL("shorter 3 than 21 ab", testable.PublicMorozov_GetLine(3));
}

TEST(LogsListTestsGroup, Append_long_line__puts_with_end_elipses) {
    TestableLogsList testable("list_box");

    testable.Append("looonger 0 than 21 ab");
    testable.Append("looonger 1 than 21 abc");
    testable.Append("looonger 2 than 21 abcd");
    testable.Append("looonger 3 than 21 abcde");

    STRCMP_EQUAL("looonger 0 than 2...", testable.PublicMorozov_GetLine(0));
    STRCMP_EQUAL("looonger 1 than 2...", testable.PublicMorozov_GetLine(1));
    STRCMP_EQUAL("looonger 2 than 2...", testable.PublicMorozov_GetLine(2));
    STRCMP_EQUAL("looonger 3 than 2...", testable.PublicMorozov_GetLine(3));
}

TEST(LogsListTestsGroup, Append_scroll_lines) {
    TestableLogsList testable("list_box");

    testable.Append("line 0");
    STRCMP_EQUAL("line 0", testable.PublicMorozov_GetLine(0));
    STRCMP_EQUAL("", testable.PublicMorozov_GetLine(1));
    STRCMP_EQUAL("", testable.PublicMorozov_GetLine(2));
    STRCMP_EQUAL("", testable.PublicMorozov_GetLine(3));

    testable.Append("line 1");
    STRCMP_EQUAL("line 0", testable.PublicMorozov_GetLine(0));
    STRCMP_EQUAL("line 1", testable.PublicMorozov_GetLine(1));
    STRCMP_EQUAL("", testable.PublicMorozov_GetLine(2));
    STRCMP_EQUAL("", testable.PublicMorozov_GetLine(3));

    testable.Append("line 2");
    STRCMP_EQUAL("line 0", testable.PublicMorozov_GetLine(0));
    STRCMP_EQUAL("line 1", testable.PublicMorozov_GetLine(1));
    STRCMP_EQUAL("line 2", testable.PublicMorozov_GetLine(2));
    STRCMP_EQUAL("", testable.PublicMorozov_GetLine(3));

    testable.Append("line 3");
    STRCMP_EQUAL("line 0", testable.PublicMorozov_GetLine(0));
    STRCMP_EQUAL("line 1", testable.PublicMorozov_GetLine(1));
    STRCMP_EQUAL("line 2", testable.PublicMorozov_GetLine(2));
    STRCMP_EQUAL("line 3", testable.PublicMorozov_GetLine(3));

    testable.Append("line 4");
    STRCMP_EQUAL("line 1", testable.PublicMorozov_GetLine(0));
    STRCMP_EQUAL("line 2", testable.PublicMorozov_GetLine(1));
    STRCMP_EQUAL("line 3", testable.PublicMorozov_GetLine(2));
    STRCMP_EQUAL("line 4", testable.PublicMorozov_GetLine(3));

    testable.Append("line 5");
    STRCMP_EQUAL("line 2", testable.PublicMorozov_GetLine(0));
    STRCMP_EQUAL("line 3", testable.PublicMorozov_GetLine(1));
    STRCMP_EQUAL("line 4", testable.PublicMorozov_GetLine(2));
    STRCMP_EQUAL("line 5", testable.PublicMorozov_GetLine(3));

    testable.Append("line 6");
    STRCMP_EQUAL("line 3", testable.PublicMorozov_GetLine(0));
    STRCMP_EQUAL("line 4", testable.PublicMorozov_GetLine(1));
    STRCMP_EQUAL("line 5", testable.PublicMorozov_GetLine(2));
    STRCMP_EQUAL("line 6", testable.PublicMorozov_GetLine(3));
}

TEST(LogsListTestsGroup, Render) {
    TestableLogsList testable("list_box");

    testable.Append("line 0");
    CHECK_TRUE(testable.Render(&frame_buffer));

    testable.Append("line 1");
    CHECK_TRUE(testable.Render(&frame_buffer));

    testable.Append("line 2");
    CHECK_TRUE(testable.Render(&frame_buffer));

    testable.Append("line 3");
    CHECK_TRUE(testable.Render(&frame_buffer));

    testable.Append("line 4");
    CHECK_TRUE(testable.Render(&frame_buffer));

    testable.Append("line 5");
    CHECK_TRUE(testable.Render(&frame_buffer));

    bool any_pixel_coloring = false;
    for (size_t i = 0; i < sizeof(frame_buffer.buffer); i++){
        if (frame_buffer.buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_TRUE(any_pixel_coloring);
}