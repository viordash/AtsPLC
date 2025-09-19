#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>
#include <driver/i2c.h>
#include <ssd1306/ssd1306.h>

#include "main/Display/ListBox.h"
#include "main/Display/display.h"

static FrameBuffer frame_buffer = {};
extern ssd1306_color_t foreground_color;
extern ssd1306_color_t background_color;

TEST_GROUP(ListBoxTestsGroup){ //
                               TEST_SETUP(){ memset(&frame_buffer.buffer, 0, sizeof(frame_buffer.buffer));
}

TEST_TEARDOWN() {
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
    int PublicMorozov_Get_selected() {
        return selected;
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

TEST(ListBoxTestsGroup, Insert__trim_text_to_line_size) {
    TestableListBox list_box("");

    list_box.Insert(0, "text looonger than 20");

    STRCMP_EQUAL("text looonger than 2", list_box.PublicMorozov_GetLine(0));

    list_box.Insert(1, "text length equal 20");
    STRCMP_EQUAL("text length equal 20", list_box.PublicMorozov_GetLine(1));
}

TEST(ListBoxTestsGroup, Render) {
    TestableListBox testable("list_box");

    testable.Insert(0, "line 0");
    testable.Render(&frame_buffer);

    testable.Insert(1, "line 1");
    testable.Render(&frame_buffer);

    testable.Insert(2, "line 2");
    testable.Render(&frame_buffer);

    testable.Insert(3, "line 3");
    testable.Render(&frame_buffer);

    bool any_pixel_coloring = false;
    for (size_t i = 0; i < sizeof(frame_buffer.buffer); i++){
        if (frame_buffer.buffer[i] != 0) {
            any_pixel_coloring = true;
            break;
        }
    }
    CHECK_TRUE(any_pixel_coloring);
}

TEST(ListBoxTestsGroup, No_selection_on_ctor) {
    TestableListBox testable("");

    CHECK_EQUAL(-1, testable.PublicMorozov_Get_selected());
}

TEST(ListBoxTestsGroup, Render_selected_item) {
    TestableListBox testable("");

    testable.Insert(0, "line 0");

    testable.Render(&frame_buffer);
    CHECK_EQUAL(OLED_COLOR_WHITE, foreground_color);
    CHECK_EQUAL(OLED_COLOR_BLACK, background_color);

    testable.Select(0);

    testable.Render(&frame_buffer);
    CHECK_EQUAL(OLED_COLOR_BLACK, foreground_color);
    CHECK_EQUAL(OLED_COLOR_WHITE, background_color);
}

TEST(ListBoxTestsGroup, Insert_changed_frame_buffer) {
    TestableListBox testable("");

    testable.Insert(0, "text");
    testable.Render(&frame_buffer);
    CHECK_TRUE(frame_buffer.has_changes);
}

TEST(ListBoxTestsGroup, Select_changed_frame_buffer) {
    TestableListBox testable("");

    testable.Select(0);
    testable.Render(&frame_buffer);
    CHECK_TRUE(frame_buffer.has_changes);
}

