#include "Display/Common.h"
#include "LogicProgram/LogicProgram.h"
#include "display.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void display_demo_0(bool active) {
    begin_render();

    InputNO input1(MapIO::DI, { 2, 26 });
    InputNC input2(MapIO::V2, input1);
    ComparatorGE comparator1(123, MapIO::AI, input2);
    ComparatorGE comparator2(42, MapIO::V1, comparator1);

    if (active) {
        input1.DoAction();
        input2.DoAction();
        comparator1.DoAction();
        comparator2.DoAction();
    }

    input1.Render(get_display_buffer());
    input2.Render(get_display_buffer());
    comparator1.Render(get_display_buffer());
    comparator2.Render(get_display_buffer());
    end_render();
}
