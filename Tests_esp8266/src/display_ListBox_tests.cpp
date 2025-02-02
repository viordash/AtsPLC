#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>


#include "main/Display/display.h"
#include "main/Display/ListBox.h"

static uint8_t frame_buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8] = {};

TEST_GROUP(ListBoxTestsGroup){ //
                                TEST_SETUP(){ memset(frame_buffer, 0, sizeof(frame_buffer));
mock().disable();
}

TEST_TEARDOWN() {
    mock().enable();
}
}
;

class TestableListBox : public ListBox {
  public:
    const static int line_size = 21;
    const static int lines_count = 4;

    explicit TestableListBox(const char *title) : ListBox(title) {
        static_assert(line_size == ListBox::line_size);
        static_assert(lines_count == ListBox::lines_count);
    }

    char *PublicMorozov_GetLine(int line_num) {
        return lines[line_num];
    }
    char *PublicMorozov_GetTitle() {
        return title;
    }
    int PublicMorozov_GetTitle_x() {
        return title_x;
    }
};

TEST(ListBoxTestsGroup, Ctor_reset_lines) {
    TestableListBox testable("list_box");

    for (size_t i = 0; i < testable.lines_count; i++) {
        CHECK_EQUAL(0, strlen(testable.PublicMorozov_GetLine(i)));
    }
}

TEST(ListBoxTestsGroup, Very_long_title__trim_to_line_size) {
    TestableListBox trimmed_title("title looonger than 20 abcd");

    STRCMP_EQUAL("> title looonger t <", trimmed_title.PublicMorozov_GetTitle());

    TestableListBox normal_title("not trimmed text");

    STRCMP_EQUAL("> not trimmed text <", normal_title.PublicMorozov_GetTitle());
}

TEST(ListBoxTestsGroup, Title_center_aligned) {
    TestableListBox testable_0("012345678901234567");
    STRCMP_EQUAL("> 0123456789012345 <", testable_0.PublicMorozov_GetTitle());
    CHECK_EQUAL(6, testable_0.PublicMorozov_GetTitle_x());

    TestableListBox testable_1("0123456789012345");
    STRCMP_EQUAL("> 0123456789012345 <", testable_1.PublicMorozov_GetTitle());
    CHECK_EQUAL(6, testable_1.PublicMorozov_GetTitle_x());

    TestableListBox testable_2("012345678901234");
    STRCMP_EQUAL("> 012345678901234 <", testable_2.PublicMorozov_GetTitle());
    CHECK_EQUAL(9, testable_2.PublicMorozov_GetTitle_x());

    TestableListBox testable_3("01234567890123");
    STRCMP_EQUAL("> 01234567890123 <", testable_3.PublicMorozov_GetTitle());
    CHECK_EQUAL(12, testable_3.PublicMorozov_GetTitle_x());

    TestableListBox testable_4("0123");
    STRCMP_EQUAL("> 0123 <", testable_4.PublicMorozov_GetTitle());
    CHECK_EQUAL(42, testable_4.PublicMorozov_GetTitle_x());
}