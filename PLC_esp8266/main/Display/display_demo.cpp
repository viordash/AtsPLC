#include "Display/Common.h"
#include "Display/DisplayItemBase.h"
#include "LogicProgram/InputNO.h"
#include "LogicProgram/LogicItemBase.h"
#include "LogicProgram/MapIO.h"
#include "demo.h"
#include "display.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

InputNO input1(MapIO::DI, { 60, 32 });

void display_demo_0() {
    draw_demo(input1.GetLocation().x,
              input1.GetLocation().y,
              input1.GetBitmap().data,
              input1.GetBitmap().size.width,
              input1.GetBitmap().size.height);
}

void display_demo_1() {
    draw_demo(0, 1, cmp_equal_active, cmp_equal_active_height, cmp_equal_active_width);
}

void display_demo_2() {
    draw_demo(0, 2, cmp_equal_active, cmp_equal_active_height, cmp_equal_active_width);
}

void display_demo(int8_t x, int8_t y) {
    draw_demo(x, y, cmp_equal_active, cmp_equal_active_height, cmp_equal_active_width);
}
