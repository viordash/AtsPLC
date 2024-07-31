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

    InputRail inputRail0(0);
    InputNO input1(MapIO::DI, inputRail0);
    InputNC input2(MapIO::V2, input1);
    ComparatorGE comparator1(123, MapIO::AI, input2);
    ComparatorGE comparator2(42, MapIO::V1, comparator1);

    if (active) {
        input1.DoAction();
        input2.DoAction();
        comparator1.DoAction();
        comparator2.DoAction();
    }
    inputRail0.Render(get_display_buffer());
    input1.Render(get_display_buffer());
    input2.Render(get_display_buffer());
    comparator1.Render(get_display_buffer());
    comparator2.Render(get_display_buffer());

    InputRail inputRail1(1);
    InputNO input11(MapIO::V1, inputRail1);
    InputNO input12(MapIO::V2, input11);
    ComparatorGE comparator11(99, MapIO::V3, input12);
    ComparatorGE comparator12(999, MapIO::V4, comparator11);

    inputRail1.Render(get_display_buffer());
    input11.Render(get_display_buffer());
    input12.Render(get_display_buffer());
    comparator11.Render(get_display_buffer());
    comparator12.Render(get_display_buffer());
    end_render();
}
