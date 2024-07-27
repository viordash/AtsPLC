#include "Display/Common.h"
#include "Display/DisplayItemBase.h"
#include "LogicProgram/InputNO.h"
#include "LogicProgram/LogicItemBase.h"
#include "LogicProgram/MapIO.h"
#include "display.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void display_demo_0() {
    begin_render();

    InputNO input1(MapIO::DI, { 5, 32 });
    InputNO input2(MapIO::AI, input1.OutcomingPoint());
    InputNO input3(MapIO::V1, input2.OutcomingPoint());

    input1.Render(get_display_buffer());
    input2.Render(get_display_buffer());
    input3.Render(get_display_buffer());
    end_render();
}
