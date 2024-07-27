#include "Display/Common.h"
#include "LogicProgram/LogicProgram.h"
#include "display.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void display_demo_0() {
    begin_render();

    InputNO input1(MapIO::DI, { 5, 32 });
    InputNC input2(MapIO::AI, input1.OutcomingPoint());
    InputNO input3(MapIO::V1, input2.OutcomingPoint());

    input1.Render(get_display_buffer());
    input2.Render(get_display_buffer());
    input3.Render(get_display_buffer());
    end_render();
}
