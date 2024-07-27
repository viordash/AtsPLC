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
    InputNO input3(MapIO::V1, input2);
    ComparatorGE comparator(MapIO::AI, input3);

    if (active) {
        input1.DoAction();
        input2.DoAction();
        input3.DoAction();
        comparator.DoAction();
    }

    input1.Render(get_display_buffer());
    input2.Render(get_display_buffer());
    input3.Render(get_display_buffer());
    comparator.Render(get_display_buffer());
    end_render();
}
